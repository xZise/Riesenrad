#include <Arduino.h>
#include <FastLED.h>
#include <new>

#ifdef ARDUINO_ARCH_ESP32
#include <freertos/task.h>
#endif

#ifdef ARDUINO_AVR_MICRO
#include "controller/controller_micro.hpp"
#elif ARDUINO_AVR_UNO
#include "controller/controller_uno.hpp"
#elif ARDUINO_ARCH_ESP32
#include "controller/controller_esp32.hpp"
#else
#error "Board is not supported"
#endif

#include "leds.hpp"

#ifdef ARDUINO_AVR_MICRO
Ferriswheel::ArduinoMicroController<8> controller;
#elif ARDUINO_AVR_UNO
Ferriswheel::ArduinoUnoController<8> controller;
#elif ARDUINO_ARCH_ESP32
Ferriswheel::ESP32Controller<12> controller;
#endif

#ifdef TIMER_VEC
ISR(TIMER_VEC) {
  controller.nextFrame();
}
#endif

void setup() {
  #ifndef ARDUINO_ARCH_ESP32
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  #endif

  Serial.begin(57600);

  FastLED.addLeds<WS2812B, controller.rgbLEDpin(), GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(30);

  controller.setupTimer();

  controller.run();
}

// The code should never reach this far
void loop() {}
