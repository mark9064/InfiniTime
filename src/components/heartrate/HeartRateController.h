#pragma once

#include <cstdint>
#include <components/ble/HeartRateService.h>
#include <components/ble/PPGService.h>

namespace Pinetime {
  namespace Applications {
    class HeartRateTask;
  }

  namespace System {
    class SystemTask;
  }

  namespace Controllers {
    class FileDelegationTask;
    class HeartRateController {
    public:
      enum class States : uint8_t { Disabled, Stopped, NotEnoughData, Searching, Measuring, NoTouch };

      HeartRateController() = default;
      void Enable();
      void Disable();
      void UpdateState(States newState);
      void UpdateHeartRate(uint8_t heartRate);

      void UpdatePPG(uint16_t hrs, uint16_t als, int16_t x, int16_t y, int16_t z, uint16_t count);

      void SendFileMessage(void* msg);

      void SetHeartRateTask(Applications::HeartRateTask* task);

      States State() const {
        return state;
      }

      uint8_t HeartRate() const {
        return heartRate;
      }

      void SetService(Pinetime::Controllers::HeartRateService* service);
      void SetPPGService(Pinetime::Controllers::PPGService* service);
      void SetFileDelegation(Pinetime::Controllers::FileDelegationTask* task);

    private:
      Applications::HeartRateTask* task = nullptr;
      States state = States::Disabled;
      uint8_t heartRate = 0;
      Pinetime::Controllers::HeartRateService* service = nullptr;
      Pinetime::Controllers::PPGService* ppgService = nullptr;
      Pinetime::Controllers::FileDelegationTask* fileDelegation = nullptr;
    };
  }
}