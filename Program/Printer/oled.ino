#include <Arduino.h>
#include <U8g2lib.h>
#include <BluetoothSerial.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include "config.h"

#define UI_RUNNING_CORE ARDUINO_RUNNING_CORE

extern bool printerPaperOut;
extern bool printerPlatenOut;
extern bool printerIsPrinting;
extern uint32_t printDataCount;
extern uint32_t printCurrentCount;

extern BluetoothSerial SerialBT;

// U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R2, /* clock=*/22, /* data=*/23, /* reset=*/U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/U8X8_PIN_NONE, /* clock=*/OLED_CLK, /* data=*/OLED_DAT);

void oledSetup()
{
  u8g2.begin();
  u8g2.enableUTF8Print(); // enable UTF8 support for the Arduino print() function
  u8g2.setFont(u8g2_font_wqy14_t_gb2312);
  xTaskCreatePinnedToCore(
      oledUiLoop, "oledUiLoop", 2048 // Stack size
      ,
      NULL, 1 // Priority
      ,
      NULL, UI_RUNNING_CORE);
}

void oledTest()
{
  u8g2.setFontDirection(0);
  u8g2.clearBuffer();
  u8g2.setCursor(0, 15);
  u8g2.print("Hello World!");
  u8g2.setCursor(0, 40);
  u8g2.print("你好世界"); // Chinese "Hello World"
  u8g2.sendBuffer();

  // u8g2.setFont(u8g2_font_unifont_t_chinese2);  // use chinese2 for all the glyphs of "你好世界"
  // //u8g2.setFont(u8g2_font_b10_t_japanese1);  // all the glyphs of "こんにちは世界" are already included in japanese1: Lerning Level 1-6
  // u8g2.setFontDirection(0);
  // u8g2.firstPage();
  // do {
  //   u8g2.setCursor(0, 15);
  //   u8g2.print("Hello World!");
  //   u8g2.setCursor(0, 40);
  //   u8g2.print("你好世界");  // Chinese "Hello World"
  //   //u8g2.print("こんにちは世界");		// Japanese "Hello World"
  // } while (u8g2.nextPage());
}

void oledRefreshStatus()
{
  u8g2.setFontDirection(0);
  u8g2.clearBuffer();
  if (printerPlatenOut)
  {
    u8g2.setCursor(0, 15);
    u8g2.print("开盖");
  }
  else if (printerPaperOut)
  {
    u8g2.setCursor(0, 15);
    u8g2.print("缺纸");
  }
  if (SerialBT.hasClient())
  {
    u8g2.setCursor(40, 15);
    u8g2.print("已连接");
  }

  if (printerIsPrinting)
  {
    u8g2.setCursor(0, 31);
    u8g2.print("正在打印");
    u8g2.setCursor(0, 45);
    u8g2.printf("%d/%d行", printCurrentCount, printDataCount);
  }
  else if(!emergencyStop())
  {
    u8g2.setCursor(0, 31);
    u8g2.print("就绪");
  }
  u8g2.sendBuffer();
}

void oledUiLoop(void *pvParameters)
{
  (void)pvParameters;
  while (1)
  {
    oledRefreshStatus();
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}