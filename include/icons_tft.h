#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// 用繪圖原語畫出簡單天氣圖示（避免大位圖，速度佳）
enum IconType {
  ICON_UNKNOWN = 0,
  ICON_SUNNY,
  ICON_CLOUDY,
  ICON_RAIN,
  ICON_SNOW,
  ICON_FOG,
  ICON_THUNDER
};

// 基本圖形工具
static void drawSun(Adafruit_ILI9341& tft, int x, int y, int r, uint16_t color) {
  tft.fillCircle(x, y, r, color);
  for (int i = 0; i < 8; i++) {
    float a = i * PI / 4.0;
    int x1 = x + cos(a) * (r + 2);
    int y1 = y + sin(a) * (r + 2);
    int x2 = x + cos(a) * (r + 10);
    int y2 = y + sin(a) * (r + 10);
    tft.drawLine(x1, y1, x2, y2, color);
  }
}

static void drawCloud(Adafruit_ILI9341& tft, int x, int y, int w, int h, uint16_t color) {
  int r = h/2;
  tft.fillRoundRect(x, y, w, h, r/2, color);
  tft.fillCircle(x + r, y, r, color);
  tft.fillCircle(x + w/2, y - r/3, r*0.9, color);
  tft.fillCircle(x + w - r, y, r, color);
}

static void drawRaindrop(Adafruit_ILI9341& tft, int x, int y, uint16_t color) {
  tft.drawLine(x, y, x-2, y+6, color);
  tft.drawLine(x, y, x+2, y+6, color);
  tft.drawPixel(x, y+7, color);
}

static void drawSnowflake(Adafruit_ILI9341& tft, int x, int y, uint16_t color) {
  tft.drawLine(x-4, y, x+4, y, color);
  tft.drawLine(x, y-4, x, y+4, color);
  tft.drawLine(x-3, y-3, x+3, y+3, color);
  tft.drawLine(x-3, y+3, x+3, y-3, color);
}

static void drawBolt(Adafruit_ILI9341& tft, int x, int y, uint16_t color) {
  tft.drawLine(x, y, x-8, y+10, color);
  tft.drawLine(x-8, y+10, x-2, y+10, color);
  tft.drawLine(x-2, y+10, x-10, y+22, color);
}

// 大圖標（約寬高 48x48）
static void drawWeatherIconLarge(Adafruit_ILI9341& tft, IconType t, int x, int y, uint16_t baseColor) {
  switch (t) {
    case ICON_SUNNY:
      drawSun(tft, x+20, y+20, 12, baseColor);
      break;
    case ICON_CLOUDY:
      drawCloud(tft, x+2, y+18, 50, 20, ILI9341_LIGHTGREY);
      break;
    case ICON_RAIN:
      drawCloud(tft, x+2, y+12, 50, 20, ILI9341_LIGHTGREY);
      for (int i=0;i<5;i++) drawRaindrop(tft, x+10+i*8, y+36, ILI9341_CYAN);
      break;
    case ICON_SNOW:
      drawCloud(tft, x+2, y+12, 50, 20, ILI9341_LIGHTGREY);
      for (int i=0;i<4;i++) drawSnowflake(tft, x+12+i*9, y+36, ILI9341_WHITE);
      break;
    case ICON_FOG:
      for (int i=0;i<4;i++) tft.drawFastHLine(x, y+14+i*6, 52, ILI9341_LIGHTGREY);
      break;
    case ICON_THUNDER:
      drawCloud(tft, x+2, y+12, 50, 20, ILI9341_LIGHTGREY);
      drawBolt(tft, x+36, y+30, ILI9341_YELLOW);
      break;
    default:
      tft.drawRect(x+8, y+8, 32, 32, ILI9341_WHITE);
      tft.drawLine(x+8, y+8, x+40, y+40, ILI9341_WHITE);
      tft.drawLine(x+40, y+8, x+8, y+40, ILI9341_WHITE);
      break;
  }
}

// 小圖標（約 24x24）
static void drawWeatherIconSmall(Adafruit_ILI9341& tft, IconType t, int x, int y, uint16_t baseColor) {
  switch (t) {
    case ICON_SUNNY:
      drawSun(tft, x+10, y+10, 6, baseColor);
      break;
    case ICON_CLOUDY:
      drawCloud(tft, x, y+8, 28, 12, ILI9341_LIGHTGREY);
      break;
    case ICON_RAIN:
      drawCloud(tft, x, y+4, 28, 12, ILI9341_LIGHTGREY);
      for (int i=0;i<3;i++) drawRaindrop(tft, x+6+i*7, y+20, ILI9341_CYAN);
      break;
    case ICON_SNOW:
      drawCloud(tft, x, y+4, 28, 12, ILI9341_LIGHTGREY);
      for (int i=0;i<3;i++) drawSnowflake(tft, x+6+i*7, y+20, ILI9341_WHITE);
      break;
    case ICON_FOG:
      for (int i=0;i<3;i++) tft.drawFastHLine(x, y+8+i*5, 28, ILI9341_LIGHTGREY);
      break;
    case ICON_THUNDER:
      drawCloud(tft, x, y+4, 28, 12, ILI9341_LIGHTGREY);
      drawBolt(tft, x+20, y+18, ILI9341_YELLOW);
      break;
    default:
      tft.drawRect(x+2, y+2, 20, 20, ILI9341_WHITE);
      break;
  }
}