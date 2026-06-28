#include <Arduino.h>
#include "spectrum.h"

extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
extern Adafruit_NeoPixel pixels;

// ── Pins (same as analyzer) ──────────────────────────────────────────────────
#define SP_CE  5
#define SP_CSN 17

// ── Layout constants ─────────────────────────────────────────────────────────
#define SP_CHANNELS      128          // nRF24 channels 0-127
#define SCREEN_W         128
#define SCREEN_H         64

// Split: top 38px = spectrum bars + peak, bottom 22px = waterfall (11 rows×2px)
#define BAR_AREA_H       40           // height of bar graph zone
#define WF_AREA_Y        40           // waterfall starts here
#define WF_AREA_H        24           // waterfall height
#define WF_ROWS          12           // number of waterfall history rows
#define WF_ROW_H         2            // pixels per waterfall row

// ── nRF24 register addresses ─────────────────────────────────────────────────
#define SP_NRF_CONFIG    0x00
#define SP_NRF_EN_AA     0x01
#define SP_NRF_RF_CH     0x05
#define SP_NRF_RF_SETUP  0x06
#define SP_NRF_RPD       0x09

// ── Data buffers ─────────────────────────────────────────────────────────────
static uint8_t  spValues[SP_CHANNELS];      // current scan result (0-50)
static uint8_t  spPeak[SP_CHANNELS];        // peak hold values
static uint8_t  spPeakTimer[SP_CHANNELS];   // peak hold countdown
static uint8_t  spWaterfall[WF_ROWS][SP_CHANNELS]; // history rows

// ── Button (SELECT = back to menu) ───────────────────────────────────────────
#define SP_BTN_SELECT 32

// ── nRF24 low-level helpers ──────────────────────────────────────────────────
static byte sp_getRegister(byte r) {
  byte c;
  digitalWrite(SP_CSN, LOW);
  SPI.transfer(r & 0x1F);
  c = SPI.transfer(0);
  digitalWrite(SP_CSN, HIGH);
  return c;
}

static void sp_setRegister(byte r, byte v) {
  digitalWrite(SP_CSN, LOW);
  SPI.transfer((r & 0x1F) | 0x20);
  SPI.transfer(v);
  digitalWrite(SP_CSN, HIGH);
}

static void sp_powerUp() {
  sp_setRegister(SP_NRF_CONFIG, sp_getRegister(SP_NRF_CONFIG) | 0x02);
  delayMicroseconds(130);
}

static void sp_enable()  { digitalWrite(SP_CE, HIGH); }
static void sp_disable() { digitalWrite(SP_CE, LOW);  }

static void sp_setRX() {
  sp_setRegister(SP_NRF_CONFIG, sp_getRegister(SP_NRF_CONFIG) | 0x01);
  sp_enable();
  delayMicroseconds(100);
}

// ── Scan all 128 channels (50 samples each) ──────────────────────────────────
static void sp_scan() {
  memset(spValues, 0, sizeof(spValues));

  const int SAMPLES = 50;
  for (int s = 0; s < SAMPLES; s++) {
    for (int i = 0; i < SP_CHANNELS; i++) {
      sp_setRegister(SP_NRF_RF_CH, i);
      sp_setRX();
      delayMicroseconds(128);
      sp_disable();
      if (sp_getRegister(SP_NRF_RPD) & 0x01) {
        spValues[i]++;
      }
    }
  }
  setNeoPixelColour("purple");
}

// ── Update peak hold ─────────────────────────────────────────────────────────
static void sp_updatePeak() {
  for (int i = 0; i < SP_CHANNELS; i++) {
    if (spValues[i] >= spPeak[i]) {
      spPeak[i]      = spValues[i];
      spPeakTimer[i] = 30;           // hold for 30 frames
    } else {
      if (spPeakTimer[i] > 0) {
        spPeakTimer[i]--;
      } else if (spPeak[i] > 0) {
        spPeak[i]--;                 // slow decay
      }
    }
  }
}

// ── Push new row into waterfall (scroll down) ────────────────────────────────
static void sp_pushWaterfall() {
  // shift rows down
  for (int r = WF_ROWS - 1; r > 0; r--) {
    memcpy(spWaterfall[r], spWaterfall[r - 1], SP_CHANNELS);
  }
  // newest row = current values
  memcpy(spWaterfall[0], spValues, SP_CHANNELS);
}

// ── Draw everything onto OLED ─────────────────────────────────────────────────
static void sp_draw() {
  u8g2.clearBuffer();

  // ── 1. Frequency axis labels (very small, top-left & mid) ────────────────
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.setCursor(0, 6);
  u8g2.print("2.4G");
  u8g2.setCursor(52, 6);
  u8g2.print("2.46G");
  u8g2.setCursor(104, 6);
  u8g2.print("2.5G");

  // ── 2. Spectrum bars ─────────────────────────────────────────────────────
  //    Each channel = 1px wide, bar drawn from bottom of BAR_AREA up
  //    BAR_AREA: y = 8 .. 39  (height 32px usable for bars)
  const int BAR_TOP    = 8;
  const int BAR_BOTTOM = BAR_AREA_H - 1;   // y=39
  const int BAR_MAX_H  = BAR_BOTTOM - BAR_TOP; // 31px

  for (int i = 0; i < SP_CHANNELS; i++) {
    // map value 0-50 → bar height 0-BAR_MAX_H
    int barH = map(spValues[i], 0, 50, 0, BAR_MAX_H);
    if (barH > BAR_MAX_H) barH = BAR_MAX_H;
    if (barH > 0) {
      u8g2.drawVLine(i, BAR_BOTTOM - barH, barH);
    }
  }

  // ── 3. Peak hold dots ────────────────────────────────────────────────────
  for (int i = 0; i < SP_CHANNELS; i++) {
    int pkH = map(spPeak[i], 0, 50, 0, BAR_MAX_H);
    if (pkH > BAR_MAX_H) pkH = BAR_MAX_H;
    if (pkH > 0) {
      u8g2.drawPixel(i, BAR_BOTTOM - pkH);
    }
  }

  // ── 4. Divider line between bar and waterfall ─────────────────────────────
  u8g2.drawHLine(0, WF_AREA_Y - 1, SCREEN_W);

  // ── 5. Waterfall (dithered density) ──────────────────────────────────────
  //    value 0-50 → pixel density using thresholds
  for (int r = 0; r < WF_ROWS; r++) {
    int y = WF_AREA_Y + r * WF_ROW_H;
    for (int i = 0; i < SP_CHANNELS; i++) {
      uint8_t v = spWaterfall[r][i];
      // Draw top pixel always if signal present
      if (v > 5)  u8g2.drawPixel(i, y);
      // Draw bottom pixel only for stronger signal (density effect)
      if (v > 20) u8g2.drawPixel(i, y + 1);
    }
  }

  // ── 6. Peak channel info (top-right corner) ───────────────────────────────
  uint8_t peakVal = 0;
  int     peakCh  = 0;
  for (int i = 0; i < SP_CHANNELS; i++) {
    if (spValues[i] > peakVal) { peakVal = spValues[i]; peakCh = i; }
  }
  // Draw small box
  u8g2.setDrawColor(0);
  u8g2.drawBox(76, 0, 52, 8);
  u8g2.setDrawColor(1);
  u8g2.drawFrame(76, 0, 52, 8);
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.setCursor(78, 6);
  // Show peak channel and % activity
  int pct = (peakVal * 100) / 50;
  char buf[20];
  snprintf(buf, sizeof(buf), "CH:%d %d%%", peakCh, pct);
  u8g2.print(buf);

  u8g2.sendBuffer();
}

// ── Public API ────────────────────────────────────────────────────────────────
void spectrumSetup() {
  Serial.begin(115200);

  esp_bt_controller_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();

  memset(spValues,    0, sizeof(spValues));
  memset(spPeak,      0, sizeof(spPeak));
  memset(spPeakTimer, 0, sizeof(spPeakTimer));
  memset(spWaterfall, 0, sizeof(spWaterfall));

  SPI.begin(18, 19, 23, 17);
  SPI.setDataMode(SPI_MODE0);
  SPI.setFrequency(10000000);
  SPI.setBitOrder(MSBFIRST);

  pinMode(SP_CE,  OUTPUT);
  pinMode(SP_CSN, OUTPUT);

  sp_disable();
  sp_powerUp();
  sp_setRegister(SP_NRF_EN_AA,    0x00);
  sp_setRegister(SP_NRF_RF_SETUP, 0x0F);

  // Splash screen
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(10, 28);
  u8g2.print("RF Spectrum Analyzer");
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.setCursor(22, 42);
  u8g2.print("2.400 - 2.527 GHz");
  u8g2.setCursor(14, 54);
  u8g2.print("[SELECT] to back menu");
  u8g2.sendBuffer();
  delay(1500);
}

void spectrumLoop() {
  sp_scan();
  sp_updatePeak();
  sp_pushWaterfall();
  sp_draw();
  setNeoPixelColour("0");
}
