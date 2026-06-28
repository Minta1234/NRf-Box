# NRf-Box

\# nRF-BOX V2



A portable ESP32-based wireless toolkit with 3× nRF24L01 modules, featuring jamming, scanning, spoofing, and BLE tools — all controlled via an OLED display and 3 buttons.



> Original by \\\[nRFBox](https://github.com/cifertech/nRFBox.git)

> by \[minta1234](https://github.com/minta1234)



\---



\## Features



| # | Menu Item | Description |

|---|-----------|-------------|

| 1 | \*\*Scanner\*\* | Scan 2.4 GHz nRF24L01 channels for activity |

| 2 | \*\*Analyzer\*\* | Analyze signal spectrum across all channels |

| 3 | \*\*WLAN Jammer\*\* | Jam 2.4 GHz Wi-Fi channels |

| 4 | \*\*Proto Kill\*\* | Disrupt nRF24-based protocols |

| 5 | \*\*BLE Jammer\*\* | Jam Bluetooth Low Energy channels |

| 6 | \*\*BLE Spoofer\*\* | Spoof BLE advertisement packets |

| 7 | \*\*Sour Apple\*\* | Send fake Apple BLE proximity notifications |

| 8 | \*\*BLE Scan\*\* | Scan and list nearby BLE devices |

| 9 | \*\*WiFi Scan\*\* | Scan and list nearby Wi-Fi networks |

| 10 | \*\*Spectrum\*\* | Live 2.4 GHz spectrum display |

| 11 | \*\*About\*\* | Project info and credits |

| 12 | \*\*Setting\*\* | Adjust OLED brightness and preferences |



\---



\## Hardware Required



\- ESP32 Dev Module

\- 3× nRF24L01 (or nRF24L01+PA+LNA) modules

\- SH1106 128×64 OLED display (I2C)

\- 1× NeoPixel RGB LED

\- 3× Tactile push buttons

\- 3.3V power supply / LiPo battery + regulator



\---



\## Pin Connection Guide



\### nRF24L01 × 3 — Shared SPI Bus



| Signal | Radio A | Radio B | Radio C |

|--------|---------|---------|---------|

| CE     | GPIO 5  | GPIO 16 | GPIO 15 |

| CSN    | GPIO 17 | GPIO 4  | GPIO 2  |

| MOSI   | GPIO 23 (shared) | ← | ← |

| MISO   | GPIO 19 (shared) | ← | ← |

| SCK    | GPIO 18 (shared) | ← | ← |

| VCC    | 3.3V    | 3.3V    | 3.3V    |

| GND    | GND     | GND     | GND     |



\### OLED Display — SH1106 I2C



| Signal | ESP32 Pin |

|--------|-----------|

| SDA    | GPIO 21   |

| SCL    | GPIO 22   |

| VCC    | 3.3V      |

| GND    | GND       |



\### NeoPixel LED



| Signal | ESP32 Pin |

|--------|-----------|

| DATA   | GPIO 14   |

| VCC    | 3.3V / 5V |

| GND    | GND       |



\### Buttons — INPUT\_PULLUP (connect to GND when pressed)



| Button | ESP32 Pin |

|--------|-----------|

| UP     | GPIO 26   |

| SELECT | GPIO 32   |

| DOWN   | GPIO 33   |



\---



\## Flashing Firmware



\### Option A — Web Flasher (Recommended)



1\. Open `nRFBox\_flasher.html` in \*\*Chrome\*\* or \*\*Edge\*\* (Chromium-based only)

2\. Connect your ESP32 via a \*\*USB data cable\*\* (not charge-only)

3\. Click \*\*Connect ESP32\*\* — hold the \*\*BOOT button\*\* on the ESP32 if it doesn't connect automatically

4\. Once connected, click \*\*Flash Device\*\*

5\. Wait for all 3 files to write — the ESP32 will reset automatically when done



The web flasher loads these 3 files automatically:



| Address  | File |

|----------|------|

| `0x1000`  | `nRFBox\_V2.ino.bootloader.bin` |

| `0x8000`  | `nRFBox\_V2.ino.partitions.bin` |

| `0x10000` | `nRFBox\_V2.ino.bin`            |



\### Option B — Arduino IDE



1\. Install \[Arduino IDE](https://www.arduino.cc/en/software)

2\. Add ESP32 board support via Board Manager

3\. Install libraries: `RF24`, `U8g2`, `Adafruit NeoPixel`

4\. Open `nRFBox\_V2.ino`

5\. Select board: \*\*ESP32 Dev Module\*\*, Partition: \*\*Huge APP (3MB No OTA)\*\*

6\. Upload



\---



\## Usage



\- \*\*UP / DOWN\*\* — navigate the menu

\- \*\*SELECT\*\* — enter / confirm

\- \*\*SELECT (hold)\*\* — go back to menu (in most modes)



\---



\## License



Distributed under the \*\*MIT License\*\* — see \[LICENSE.txt](https://github.com/cifertech/nRFBox/blob/main/LICENSE.txt) for full details.



```

MIT License



Copyright (c) 2024 CiferTech



Permission is hereby granted, free of charge, to any person obtaining a copy

of this software and associated documentation files (the "Software"), to deal

in the Software without restriction, including without limitation the rights

to use, copy, modify, merge, publish, distribute, sublicense, and/or sell

copies of the Software, and to permit persons to whom the Software is

furnished to do so, subject to the following conditions:



The above copyright notice and this permission notice shall be included in all

copies or substantial portions of the Software.



THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR

IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,

FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE

AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER

LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,

OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE

SOFTWARE.

```



> ⚠️ Jamming radio frequencies may be \*\*illegal\*\* in your country. This project is for \*\*educational and research purposes only\*\*. Do not use it for malicious activities or unauthorized access.

