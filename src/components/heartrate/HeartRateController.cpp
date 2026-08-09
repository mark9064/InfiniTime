#include "components/heartrate/HeartRateController.h"

#include <cstdint>
#include "heartratetask/HeartRateTask.h"
#include "components/ble/filedelegationtask/FileDelegationTask.h"

using namespace Pinetime::Controllers;

void HeartRateController::UpdateState(HeartRateController::States newState) {
  this->state = newState;
}

void HeartRateController::UpdateHeartRate(uint8_t heartRate) {
  if (this->heartRate != heartRate) {
    this->heartRate = heartRate;
    service->OnNewHeartRateValue(heartRate);
  }
}

void HeartRateController::UpdatePPG(uint16_t hrs, uint16_t als, int16_t x, int16_t y, int16_t z, uint16_t count) {
  ppgService->OnNewPPGValue(hrs, als, x, y, z, count);
}

void HeartRateController::SendFileMessage(void* msg) {
  fileDelegation->PushMessage(msg);
}

void HeartRateController::Enable() {
  if (task != nullptr) {
    state = States::Stopped;
    task->PushMessage(Pinetime::Applications::HeartRateTask::Messages::Enable);
  }
}

void HeartRateController::Disable() {
  if (task != nullptr) {
    state = States::Disabled;
    task->PushMessage(Pinetime::Applications::HeartRateTask::Messages::Disable);
  }
}

void HeartRateController::SetHeartRateTask(Pinetime::Applications::HeartRateTask* task) {
  this->task = task;
}

void HeartRateController::SetService(Pinetime::Controllers::HeartRateService* service) {
  this->service = service;
}

void HeartRateController::SetPPGService(Pinetime::Controllers::PPGService* service) {
  this->ppgService = service;
}

void HeartRateController::SetFileDelegation(Pinetime::Controllers::FileDelegationTask* task) {
  this->fileDelegation = task;
}
