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
    class HeartRateController {
    public:
      enum class States { Stopped, NotEnoughData, NoTouch, Running };
      enum class RunModes { NoBackground, Periodic, Continuous};

      HeartRateController() = default;
      void Start();
      void Stop();
      void Update(States newState, uint8_t heartRate);

      void UpdatePPG(uint32_t hrs, uint32_t als);

      void SetHeartRateTask(Applications::HeartRateTask* task);

      void SetMode(enum RunModes mode);

      States State() const {
        return state;
      }

      RunModes RunMode() const {
        return runMode;
      }

      uint8_t HeartRate() const {
        return heartRate;
      }

      void SetService(Pinetime::Controllers::HeartRateService* service);
      void SetPPGService(Pinetime::Controllers::PPGService* service);

    private:
      Applications::HeartRateTask* task = nullptr;
      States state = States::Stopped;
      RunModes runMode = RunModes::NoBackground;
      uint8_t heartRate = 0;
      Pinetime::Controllers::HeartRateService* service = nullptr;
      Pinetime::Controllers::PPGService* ppgService = nullptr;
    };
  }
}