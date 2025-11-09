#include "FileDelegationTask.h"
#include "components/heartrate/HeartRateController.h"

using namespace Pinetime::Controllers;

FileDelegationTask::FileDelegationTask(Pinetime::Controllers::FS& fs, Pinetime::Controllers::HeartRateController& heartRateController)
  : fs {fs}, heartRateController {heartRateController} {
  heartRateController.SetFileDelegation(this);
}

void FileDelegationTask::Start() {
  messageQueue = xQueueCreate(40, 4);

  if (!xTaskCreate(FileDelegationTask::Process, "FSD", 400, this, 0, &taskHandle)) {
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
  }
}

void FileDelegationTask::Process(void* instance) {
  auto* app = static_cast<FileDelegationTask*>(instance);
  app->Work();
}

void FileDelegationTask::PushMessage(void* msg) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (xQueueSendFromISR(messageQueue, &msg, &xHigherPriorityTaskWoken) == errQUEUE_FULL) {
    free(msg);
    NVIC_SystemReset(); // add a dropped member, set to true if in here and then disable further appends (just process flush)
  }
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void FileDelegationTask::Work() {
  void* msg;
  Messages type;
  int res;
  while (true) {
    if (xQueueReceive(messageQueue, &msg, portMAX_DELAY)) {
      type = *static_cast<Messages*>(msg);
      if (type == Messages::SendHRSACC) {
        auto* data = static_cast<SendHRSACCData*>(msg);
        res = fs.Stat(name, &info);
        if (res != LFS_ERR_OK) {
          free(data);
          continue;
        }
        res = fs.FileOpen(&f, name, LFS_O_RDONLY);
        if (res != LFS_ERR_OK) {
          free(data);
          continue;
        }
        om = ble_hs_mbuf_from_flat(&info.size, 4);
        ble_gattc_notify_custom(data->connectionHandle, data->transferCharacteristicHandle, om);
        int read = 0;
        int lastRead;
        int error;
        constexpr int maxSize = 256 - 3 - 4; // 256 MTU - 3 GATT overhead - 4 size/status
        uint8_t* buf = (uint8_t*) malloc(maxSize);
        if (buf == nullptr) {
          APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
        }
        while (read < (int) info.size) {
          lastRead = fs.FileRead(&f, buf, maxSize);
          if (lastRead < 0) {
            om = ble_hs_mbuf_from_flat(&lastRead, 4);
            ble_gattc_notify_custom(data->connectionHandle, data->transferCharacteristicHandle, om);
            break;
          }
          read += lastRead;
          if (false) {
          retry:
            vTaskDelay(5);
          }
          om = ble_hs_mbuf_from_flat(&read, 4);
          if (om == NULL) {
            goto retry;
          }
          if (os_mbuf_append(om, buf, lastRead) != 0) {
            os_mbuf_free_chain(om);
            goto retry;
          }
          if ((error = ble_gattc_notify_custom(data->connectionHandle, data->transferCharacteristicHandle, om)) != 0) {
            if (error == BLE_HS_ENOMEM) {
              goto retry;
            }
            vTaskDelay(100);
            om = ble_hs_mbuf_from_flat(&error, 4);
            ble_gattc_notify_custom(data->connectionHandle, data->transferCharacteristicHandle, om);
            break;
          }
        }
        vTaskDelay(100);
        read = 0;
        om = ble_hs_mbuf_from_flat(&read, 1);
        ble_gattc_notify_custom(data->connectionHandle, data->transferCharacteristicHandle, om);
        fs.FileClose(&f);
        free(buf);
        free(data);
      } else if (type == Messages::ClearHRSACC) {
        free(msg);
        bufferItems = 0;
        res = fs.FileOpen(&f, name, LFS_O_WRONLY | LFS_O_TRUNC);
        if (res != LFS_ERR_OK) {
          continue;
        }
        fs.FileClose(&f);
      } else if (type == Messages::WriteHRSACC) {
        auto* data = static_cast<WriteHRSACCData*>(msg);
        HRSACCData values = {data->hrs, data->als, data->accX, data->accY, data->accZ};
        writeBuffer[bufferItems++] = values;
        if (bufferItems == bufferLen) {
          bufferItems = 0;
          res = fs.Stat(name, &info);
          if (res != LFS_ERR_OK) {
            free(data);
            continue;
          }
          if (info.size > maxFileSize) {
            free(data);
            continue;
          }
          res = fs.FileOpen(&f, name, LFS_O_WRONLY | LFS_O_APPEND);
          if (res != LFS_ERR_OK) {
            free(data);
            continue;
          }
          // even if this fails we still need to close the file
          fs.FileWrite(&f, (uint8_t*) writeBuffer.data(), sizeof(HRSACCData) * bufferLen);
          fs.FileClose(&f);
        }
        free(data);
      } else if (type == Messages::FlushHRSACC) {
        free(msg);
        res = fs.Stat(name, &info);
        if (res != LFS_ERR_OK) {
          continue;
        }
        if (info.size > maxFileSize) {
          continue;
        }
        res = fs.FileOpen(&f, name, LFS_O_WRONLY | LFS_O_APPEND);
        if (res != LFS_ERR_OK) {
          continue;
        }
        // even if this fails we still need to close the file
        fs.FileWrite(&f, (uint8_t*) writeBuffer.data(), sizeof(HRSACCData) * bufferItems);
        fs.FileClose(&f);
      } else {
        free(msg);
      }
    }
  }
}