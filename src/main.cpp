#include <Arduino.h>
#include <FastLED.h>

#include "leds.hpp"
#include "sprinkle.hpp"

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(30);
}

void move1() {
  CRGB color = getRandomColor();
  uint8_t offset = random8(NUM_LEDS);

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    for (uint8_t j = 0; j < NUM_LEDS - i; j++) {
      uint8_t led = (j + offset) % NUM_LEDS;
      if (j > 0) {
        leds[(led == 0 ? NUM_LEDS : led) - 1] = CRGB::Black;
      }
      leds[led] = color;
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
    *getLed(center_near + offset) = CRGB::Black;
    center_near--;
    if (center_far < NUM_LEDS) {
      *getLed(center_far + offset) = CRGB::Black;
      center_far++;
    }
    FastLED.show();
    delay(100);
  }
}

void move2() {
  CRGB color = getRandomColor();
  CRGB bgColor = getRandomColor();
  if (color == bgColor) {
    bgColor = CRGB::Black;
  }
  for (uint8_t iteration = 0; iteration < 10; iteration++) {
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
      CRGB ledColor = ((led % 2) == (iteration % 2)) ? bgColor : color;
      leds[led] = ledColor;
    }
    FastLED.show();
    delay(500);
  }
}

void move3() {
  constexpr uint8_t trail_length = NUM_LEDS / 10 + 1;

  const uint8_t start = random8(NUM_LEDS);
  const bool reverse = random(2) == 0;

  for (uint8_t remaining_iterations = NUM_LEDS; remaining_iterations > 0; remaining_iterations--) {
    uint8_t actualTrail = min(trail_length, remaining_iterations - 1);
    for (uint8_t i = 0; i < actualTrail; i++) {
      int8_t index = remaining_iterations + start + i;
      if (reverse) {
        index = NUM_LEDS - index;
      }
      CRGB* led = getLed(index);
      if (i == 0) {
        *led = CRGB::Red;
      } else {
        *led = CRGB::Wheat;
      }
    }

    if (remaining_iterations < 4) {
      fadeToBlackBy(leds, NUM_LEDS, 255 / remaining_iterations);
    }

    FastLED.show();
    delay(100);
    allBlack();
  }
  delay(100);
}

void move4() {
  constexpr uint8_t glitterSpecs = NUM_LEDS / 5;

  uint8_t iteration = 20;
  uint8_t newSpecs;
  do {
    newSpecs = glitterSpecs;
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
      if (leds[led] != CRGB(0, 0, 0)) {
        if (random8() < 150) {
          leds[led] = CRGB::Black;
        } else {
          newSpecs--;
        }
      }
    }
    if (iteration > 0) {
      iteration--;
      while (newSpecs-- > 0) {
        leds[random8(NUM_LEDS)] = CRGB::White;
      }
    }
    FastLED.show();
    delay(100);
  } while (iteration > 0 || newSpecs < glitterSpecs);
}

typedef void (*t_movefunc)();

constexpr t_movefunc movefuncs[] = {&move1, &move2, &move3, &move4, &sprinkle};
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