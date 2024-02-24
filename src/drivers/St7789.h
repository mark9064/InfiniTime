#pragma once
#include <cstddef>
#include <cstdint>

#include <hal/nrf_gpio.h>
#include <nrfx_log.h>
#include "drivers/Spi.h"
#include "drivers/PinMap.h"

namespace Pinetime {
  namespace Drivers {
    class Spi;

    class St7789 {
    public:
      explicit St7789(Spi& spi);
      St7789(const St7789&) = delete;
      St7789& operator=(const St7789&) = delete;
      St7789(St7789&&) = delete;
      St7789& operator=(St7789&&) = delete;

      void Init();
      void Uninit();

      void VerticalScrollStartAddress(uint16_t line);

      void DrawBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t* data, size_t size);

      void LowPowerOn();
      void LowPowerOff();
      void Sleep();
      void Wakeup();

    private:
      Spi& spi;
      uint8_t verticalScrollingStartAddress = 0;
      bool sleepIn;
      TickType_t lastSleepExit;

      void HardwareReset();
      void SoftwareReset();
      void Command2Enable();
      void SleepOut();
      void EnsureSleepOutPostDelay();
      void SleepIn();
      void ColMod();
      void MemoryDataAccessControl();
      void DisplayInversionOn();
      void NormalModeOn();
      void WriteToRam(const uint8_t* data, size_t size);
      void IdleModeOn();
      void IdleModeOff();
      void FrameRateNormal();
      void FrameRateLow();
      void DisplayOn();
      void DisplayOff();
      void PowerControl();

      void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
      void SetVdv();
      void WriteCommand(uint8_t cmd);
      void WriteSpi(const uint8_t* data, size_t size, void (*TransactionHook)(bool));
      static void EnableDataMode(bool isStart);
      static void EnableCommandMode(bool isStart);

      enum class Commands : uint8_t {
        SoftwareReset = 0x01,
        SleepIn = 0x10,
        SleepOut = 0x11,
        NormalModeOn = 0x13,
        DisplayInversionOn = 0x21,
        DisplayOff = 0x28,
        DisplayOn = 0x29,
        ColumnAddressSet = 0x2a,
        RowAddressSet = 0x2b,
        WriteToRam = 0x2c,
        MemoryDataAccessControl = 0x36,
        VerticalScrollDefinition = 0x33,
        VerticalScrollStartAddress = 0x37,
        IdleModeOff = 0x38,
        IdleModeOn = 0x39,
        ColMod = 0x3a,
        FrameRate = 0xb3,
        VdvSet = 0xc4,
        Command2Enable = 0xdf,
        PowerControl1 = 0xd0,
        PowerControl2 = 0xe8,
      };
      void WriteData(uint8_t data);
      void ColumnAddressSet();

      static constexpr uint16_t Width = 240;
      static constexpr uint16_t Height = 320;
      void RowAddressSet();
    };
  }
}
