#pragma once
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#include <atomic>
#undef max
#undef min

namespace Pinetime {
  namespace Controllers {
    class HeartRateController;
    class NimbleController;

    class PPGService {
    public:
      PPGService(NimbleController& nimble, Controllers::HeartRateController& heartRateController);
      void Init();
      void OnNewPPGValue(uint32_t hrs, uint32_t als);

      void SubscribeNotification(uint16_t attributeHandle);
      void UnsubscribeNotification(uint16_t attributeHandle);

    private:
      NimbleController& nimble;
      Controllers::HeartRateController& heartRateController;
      static constexpr uint16_t ppgServiceId {0xFEBC};
      static constexpr uint16_t ppgValueId {0x0100};
      static constexpr ble_uuid128_t ppgServiceUuid {
        .u {.type = BLE_UUID_TYPE_128},
        .value = {0xD6, 0xDA, 0x83, 0x07, 0x57, 0x12, 0x44, 0x9A, 0x96, 0x4A, 0xC3, 0xAB, 0x00, 0x00, 0x00, 0x00}};

      static constexpr ble_uuid128_t ppgValueUuid {
        .u {.type = BLE_UUID_TYPE_128},
        .value = {0xD6, 0xDA, 0x83, 0x07, 0x57, 0x12, 0x44, 0x9A, 0x96, 0x4A, 0xC3, 0xAB, 0x01, 0x00, 0x00, 0x00}};

      struct ble_gatt_chr_def characteristicDefinition[2];
      struct ble_gatt_svc_def serviceDefinition[2];

      uint16_t ppgValueHandle;
      std::atomic_bool ppgValueNotificationEnabled {false};
    };
  }
}
