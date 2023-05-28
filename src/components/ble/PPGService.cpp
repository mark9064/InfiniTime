#include "components/ble/PPGService.h"
#include "components/heartrate/HeartRateController.h"
#include "components/ble/NimbleController.h"
#include <nrf_log.h>

using namespace Pinetime::Controllers;

constexpr ble_uuid128_t PPGService::ppgServiceUuid;
constexpr ble_uuid128_t PPGService::ppgValueUuid;

namespace {
  // the callback needs to exist or the ble stack rejects the characteristic
  // see https://github.com/apache/mynewt-nimble/blob/a60ee37b8ffa9f6c0a3e635480680069a0f351a9/nimble/host/mesh/src/pb_gatt_srv.c#L203
  int PPGDummyCallback(uint16_t, uint16_t, struct ble_gatt_access_ctxt*, void*) {
    return 0;
  }
}

PPGService::PPGService(NimbleController& nimble, Controllers::HeartRateController& heartRateController)
  : nimble {nimble},
    heartRateController {heartRateController},
    characteristicDefinition {{.uuid = &ppgValueUuid.u,
                               .access_cb = PPGDummyCallback,
                               .arg = nullptr,
                               .flags = BLE_GATT_CHR_F_NOTIFY,
                               .val_handle = &ppgValueHandle},
                              {0}},
    serviceDefinition {
      {/* Custom Service */
       .type = BLE_GATT_SVC_TYPE_PRIMARY,
       .uuid = &ppgServiceUuid.u,
       .characteristics = characteristicDefinition},
      {0},
    } {
  // TODO refactor to prevent this loop dependency (service depends on controller and controller depends on service)
  heartRateController.SetPPGService(this);
}

void PPGService::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

void PPGService::OnNewPPGValue(uint32_t hrs, uint32_t als) {
  if (!ppgValueNotificationEnabled)
    return;
  uint32_t buf[2] = {hrs, als};

  auto* om = ble_hs_mbuf_from_flat(buf, sizeof(buf));

  uint16_t connectionHandle = nimble.connHandle();

  if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
    return;
  }

  ble_gattc_notify_custom(connectionHandle, ppgValueHandle, om);
}

void PPGService::SubscribeNotification(uint16_t attributeHandle) {
  if (attributeHandle == ppgValueHandle)
    ppgValueNotificationEnabled = true;
}

void PPGService::UnsubscribeNotification(uint16_t attributeHandle) {
  if (attributeHandle == ppgValueHandle)
    ppgValueNotificationEnabled = false;
}
