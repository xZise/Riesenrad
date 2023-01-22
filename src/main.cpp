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

  FastLED.addLeds<WS2812B, controller.rgbLEDpin(), GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(30);

  controller.setupTimer();

  controller.run();
}

void fallingStacks() {
  constexpr uint8_t stackDivisor = 30;
  constexpr uint8_t stackLength = NUM_LEDS >= stackDivisor ? NUM_LEDS / stackDivisor : 1;
  constexpr uint8_t stackCount = stackLength < 4 ? 4 - stackLength : 1;
  // TODO: In theory this should be a clean divisor of NUM_LEDs, this is not guaranteed yet.
  constexpr uint8_t stackLedCount = stackLength * stackCount;
  CRGB colors[stackLedCount];
  for (uint8_t i = 0; i < stackLedCount; i++) {
    colors[i] = getRandomColor();
  }

  uint8_t offset = random8(NUM_LEDS);

  for (uint8_t i = 0; i < NUM_LEDS; i += stackLength) {
    for (uint8_t j = 0; j < NUM_LEDS - i; j += stackLength) {
      for (uint8_t k = 0; k < stackLength; k++) {
        uint8_t led = (j + offset + k) % NUM_LEDS;
        if (j > 0) {
          uint8_t oldLed = led;
          if (led < stackLength) {
            oldLed += NUM_LEDS;
          }
          oldLed -= stackLength;
          leds[oldLed] = CRGB::Black;
        }
        leds[led] = colors[(i + k) % stackLedCount];
      }
      FastLED.show();
      delay(50);
    }
    delay(700);
  }

  uint8_t center_far = NUM_LEDS / 2;
  uint8_t center_near;
  if (NUM_LEDS % 2 == 1) {
    center_near = center_far;
  } else {
    center_near = center_far - 1;
  }
  while (center_near < 0xff) {
    *getLedOffset(center_near, offset) = CRGB::Black;
    center_near--;
    if (center_far < NUM_LEDS) {
      *getLedOffset(center_far, offset) = CRGB::Black;
      center_far++;
    }
    FastLED.show();
    delay(100);
  }
}

typedef void (*t_movefunc)();

constexpr t_movefunc movefuncs[] = {
  &fallingStacks,
};
constexpr uint8_t numMoveFuncs = array_size(movefuncs);

void move(uint8_t mode) {
  allBlack();
  if (mode < numMoveFuncs) {
    (*movefuncs[mode])();
  }
}

void loop() {
  allBlack();
  move(random8(numMoveFuncs));
}
