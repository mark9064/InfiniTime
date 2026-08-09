#pragma once
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min
#include "components/fs/FS.h"
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <array>

namespace Pinetime {
  namespace System {
    class SystemTask;
  }

  namespace Controllers {
    class Ble;
    class HeartRateController;

    class FileDelegationTask {
    public:
      enum class Messages : uint8_t { SendHRSACC, ClearHRSACC, WriteHRSACC, FlushHRSACC };
      explicit FileDelegationTask(Pinetime::Controllers::FS& fs, Pinetime::Controllers::HeartRateController& heartRateController);
      void Start();
      void Work();
      void PushMessage(void* msg);

      struct SendHRSACCData {
        Messages message;
        uint16_t connectionHandle;
        uint16_t transferCharacteristicHandle;
      };

      struct WriteHRSACCData {
        Messages message;
        uint16_t hrs;
        uint16_t als;
        uint16_t accX;
        uint16_t accY;
        uint16_t accZ;
      };

    private:
      static void Process(void* instance);

      struct HRSACCData {
        uint16_t hrs;
        uint16_t als;
        uint16_t accX;
        uint16_t accY;
        uint16_t accZ;
      };
      TaskHandle_t taskHandle;
      QueueHandle_t messageQueue;
      Pinetime::Controllers::FS& fs;
      Pinetime::Controllers::HeartRateController& heartRateController;
      const char* name = "hrs-acc-bin";
      const unsigned int maxFileSize = 3 * 1024 * 1024;
      static constexpr uint16_t bufferLen = 100;
      std::array<HRSACCData, bufferLen> writeBuffer;
      uint16_t bufferItems = 0;
      os_mbuf* om;
      lfs_info info;
      lfs_file f;
    };

  }
}
