#pragma once

#include "displayapp/screens/Screen.h"
#include "systemtask/SystemTask.h"
#include <chrono>
#include <cstdint>
#include <lvgl/src/lv_core/lv_obj.h>
#include <lvgl/src/lv_core/lv_style.h>

namespace Pinetime {
  namespace Controllers {
    class HeartRateController;
  }

  namespace Applications {
    namespace Screens {

      class HeartRate : public Screen {
      public:
        HeartRate(Controllers::HeartRateController& HeartRateController, System::SystemTask& systemTask);
        ~HeartRate() override;

        void Refresh() override;

        void OnStartStopEvent(lv_event_t event);
        void OnRunModeEvent(lv_event_t event);

      private:
        Controllers::HeartRateController& heartRateController;
        Pinetime::System::SystemTask& systemTask;
        void UpdateStartStopButton(bool isRunning);
        void UpdateRunModeButton();
        lv_obj_t* label_hr;
        lv_obj_t* label_bpm;
        lv_obj_t* label_status;
        lv_obj_t* btn_startStop;
        lv_obj_t* label_startStop;
        lv_obj_t* btn_runMode;
        lv_obj_t* label_runMode;

        lv_task_t* taskRefresh;
      };
    }
  }
}
