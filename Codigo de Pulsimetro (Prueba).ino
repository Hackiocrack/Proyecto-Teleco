/*
 * PulseSensor_XIAO_ESP32S3_OLED_Lib.ino
 * OLED display with library-powered beat detection.
 * 
 * Hardware: XIAO ESP32S3, PulseSensor, 1.3" SH1106 OLED
 * Libraries: PulseSensor Playground, Adafruit SH110X, Adafruit GFX
 * >> https://pulsesensor.com
 */

#include <PulseSensorPlayground.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

const int PULSE_PIN = 1;
const int LED_PIN = 21;

PulseSensorPlayground pulseSensor;
Adafruit_SH1106G display(128, 64, &Wire, -1);

#define WAVE_WIDTH 100
int waveBuffer[WAVE_WIDTH];
int waveIndex = 0;

void setup() {
  Serial.begin(115200);
  
  pulseSensor.analogInput(PULSE_PIN);
  pulseSensor.blinkOnPulse(LED_PIN);
  pulseSensor.setThreshold(550);
  pulseSensor.begin();
  
  Wire.begin(5, 6);
  display.begin(0x3C, true);
  display.setTextColor(SH110X_WHITE);
  
  display.clearDisplay();
  display.setCursor(20, 20);
  display.println("PulseSensor");
  display.setCursor(20, 35);
  display.println("OLED + Library");
  display.display();
  delay(2000);
  
  for (int i = 0; i < WAVE_WIDTH; i++) waveBuffer[i] = 32;
}

void loop() {
  int signal = pulseSensor.getLatestSample();
  int bpm = pulseSensor.getBeatsPerMinute();
  bool beat = pulseSensor.sawStartOfBeat();
  
  int y = map(signal, 0, 4095, 63, 20);
  y = constrain(y, 20, 63);
  waveBuffer[waveIndex] = y;
  waveIndex = (waveIndex + 1) % WAVE_WIDTH;
  
  display.clearDisplay();
  
  // BPM display
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("BPM: ");
  if (bpm > 40 && bpm < 200) {
    display.print(bpm);
  } else {
    display.print("--");
  }
  
  // Beat indicator
  if (beat) {
    display.setCursor(60, 0);
    display.print("*");
  }
  
  // Draw waveform
  for (int i = 0; i < WAVE_WIDTH - 1; i++) {
    int idx = (waveIndex + i) % WAVE_WIDTH;
    int idxNext = (waveIndex + i + 1) % WAVE_WIDTH;
    display.drawLine(14 + i, waveBuffer[idx], 14 + i + 1, waveBuffer[idxNext], SH110X_WHITE);
  }
  
  display.display();
  delay(20);
}