#pragma once
#include <FreeRTOS.h>
#include <components/heartrate/Ppg.h>
#include <queue.h>
#include <task.h>

#define DURATION_BETWEEN_BACKGROUND_MEASUREMENTS pdMS_TO_TICKS(5 * 60 * 1000) // 5 Minutes
#define BACKGROUND_MEASUREMENT_DURATION          pdMS_TO_TICKS(15000)         // 15 seconds

namespace Pinetime {
  namespace Drivers {
    class Hrs3300;
  }

  namespace Controllers {
    class HeartRateController;
  }

  namespace Applications {
    class HeartRateTask {
    public:
      enum class Messages : uint8_t { GoToSleep, WakeUp, StartMeasurement, StopMeasurement, ChangeMode };
      enum class States { Stopped, Idle, Measuring, BackgroundMeasuring };

      explicit HeartRateTask(Drivers::Hrs3300& heartRateSensor, Controllers::HeartRateController& controller);
      void Start();
      void Work();
      void PushMessage(Messages msg);

    private:
      enum class Modes { NoBackground, Periodic, Continuous };
      static void Process(void* instance);
      void StartMeasurement();
      void StopMeasurement();

      void HandleBackgroundWaiting();
      void HandleSensorData(int* lastBpm);
      int CurrentTaskDelay();

      TaskHandle_t taskHandle;
      QueueHandle_t messageQueue;
      States state = States::Stopped;
      Modes mode = Modes::NoBackground;
      Drivers::Hrs3300& heartRateSensor;
      Controllers::HeartRateController& controller;
      Controllers::Ppg ppg;
      TickType_t backgroundMeasurementWaitingStart = 0;
      TickType_t ppgStartTime = 0;
    };

  }
}
