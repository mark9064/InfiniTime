#include "heartratetask/HeartRateTask.h"
#include <components/heartrate/HeartRateController.h>
#include <drivers/Hrs3300.h>
#include <nrf_log.h>

using namespace Pinetime::Applications;

HeartRateTask::HeartRateTask(Drivers::Hrs3300& heartRateSensor, Controllers::HeartRateController& controller)
  : heartRateSensor {heartRateSensor}, controller {controller} {
}

void HeartRateTask::Start() {
  messageQueue = xQueueCreate(10, 1);
  controller.SetHeartRateTask(this);

  if (!xTaskCreate(HeartRateTask::Process, "HRM", 500, this, 0, &taskHandle)) {
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
  }
}

void HeartRateTask::Process(void* instance) {
  auto* app = static_cast<HeartRateTask*>(instance);
  app->Work();
}

void HeartRateTask::Work() {
  int lastBpm = 0;

  while (true) {
    auto delay = CurrentTaskDelay();
    Messages msg;

    if (xQueueReceive(messageQueue, &msg, delay)) {
      switch (msg) {
        case Messages::GoToSleep:
          if (state == States::Measuring) {
            // if a background measurement is due, keep PPG on
            // if the PPG has already been running for long enough, measurement will stop on the next HandleSensorData call
            if (xTaskGetTickCount() - backgroundMeasurementWaitingStart >= DURATION_BETWEEN_BACKGROUND_MEASUREMENTS) {
              state = States::BackgroundMeasuring;
            } else {
              state = States::BackgroundWaiting;
              StopMeasurement();
              lastBpm = 0;
            }
          }
          break;
        case Messages::WakeUp:
          if (state == States::BackgroundMeasuring) {
            state = States::Measuring;
          } else if (state == States::BackgroundWaiting) {
            state = States::Measuring;
            StartMeasurement();
          }
          break;
        case Messages::StartMeasurement:
          if (state == States::Measuring) {
            break;
          }
          state = States::Measuring;
          StartMeasurement();
          break;
        case Messages::StopMeasurement:
          if (state == States::Stopped) {
            break;
          }
          state = States::Stopped;
          StopMeasurement();
          lastBpm = 0;
          break;
      }
    }

    if (state == States::BackgroundWaiting) {
      HandleBackgroundWaiting();
    } else if (state == States::BackgroundMeasuring || state == States::Measuring) {
      HandleSensorData(&lastBpm);
    }
  }
}

void HeartRateTask::PushMessage(HeartRateTask::Messages msg) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(messageQueue, &msg, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    /* Actual macro used here is port specific. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

void HeartRateTask::StartMeasurement() {
  heartRateSensor.Enable();
  ppg.Reset(true);
  vTaskDelay(100);
  ppgStartTime = xTaskGetTickCount();
}

void HeartRateTask::StopMeasurement() {
  heartRateSensor.Disable();
  ppg.Reset(true);
  vTaskDelay(100);
}

void HeartRateTask::HandleBackgroundWaiting() {
  if (xTaskGetTickCount() - backgroundMeasurementWaitingStart >= DURATION_BETWEEN_BACKGROUND_MEASUREMENTS) {
    state = States::BackgroundMeasuring;
    StartMeasurement();
  }
}

void HeartRateTask::HandleSensorData(int* lastBpm) {
  int8_t ambient = ppg.Preprocess(heartRateSensor.ReadHrs(), heartRateSensor.ReadAls());
  int bpm = ppg.HeartRate();

  // If ambient light detected or a reset requested (bpm < 0)
  if (ambient > 0) {
    // Reset all DAQ buffers
    ppg.Reset(true);
    *lastBpm = 0;
    bpm = 0;
  } else if (bpm < 0) {
    // Reset all DAQ buffers except HRS buffer
    ppg.Reset(false);
    // Set HR to zero and update
    bpm = 0;
    controller.Update(Controllers::HeartRateController::States::Running, bpm);
  }

  // only works transiently as ALS trigger will always double the threshold
  if (ambient > 0) {
    controller.Update(Controllers::HeartRateController::States::NoTouch, bpm);
  } else if (*lastBpm == 0 && bpm == 0) {
    controller.Update(Controllers::HeartRateController::States::NotEnoughData, bpm);
  }

  if (bpm != 0) {
    *lastBpm = bpm;
    controller.Update(Controllers::HeartRateController::States::Running, bpm);
    // set measurement timer forward
    // only effective when not background measuring, as background measuring always sets this when it finishes
    backgroundMeasurementWaitingStart = xTaskGetTickCount();
  }
  // if measurement has been running for more than the background measurement period, stop
  if (state == States::BackgroundMeasuring && xTaskGetTickCount() - ppgStartTime > BACKGROUND_MEASUREMENT_DURATION) {
    backgroundMeasurementWaitingStart = xTaskGetTickCount();
    StopMeasurement();
    *lastBpm = 0;
    state = States::BackgroundWaiting;
  }
}

int HeartRateTask::CurrentTaskDelay() {
  switch (state) {
    case States::Measuring:
    case States::BackgroundMeasuring:
      return ppg.deltaTms;
    case States::BackgroundWaiting:
      return 10000;
    default:
      return portMAX_DELAY;
  }
}