#ifndef spectrum_H
#define spectrum_H

#include <SPI.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_NeoPixel.h>
#include "esp_bt.h"
#include "esp_wifi.h"
#include "neopixel.h"

void spectrumSetup();
void spectrumLoop();

#endif
