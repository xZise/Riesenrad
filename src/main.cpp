#include <Arduino.h>
#include <FastLED.h>

#include "leds.hpp"
#include "sprinkle.hpp"

bool randomBool() {
  return (random8() >> 7) == 0;
}

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
  const bool reverse = randomBool();

  for (uint8_t remaining_iterations = NUM_LEDS; remaining_iterations > 0; remaining_iterations--) {
    uint8_t actualTrail = min(trail_length, remaining_iterations - 1);
    for (uint8_t i = 0; i < actualTrail; i++) {
      uint8_t index = getLedOffsetIndex(remaining_iterations, start);
      index = getLedOffsetIndex(index, i);
      if (reverse) {
        if (index >= NUM_LEDS) {
          index -= NUM_LEDS;
        }
        index = NUM_LEDS - index - 1;
      }
      if (i == 0) {
        leds[index] = CRGB::Red;
      } else {
        leds[index] = CRGB::Wheat;
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

void rotation() {
  constexpr uint8_t numColors = availableColorsLength;
  constexpr uint8_t colorWidth = NUM_LEDS / numColors;

  const bool isSolid = randomBool();
  const uint8_t colorOffset = random8(numColors);

  uint8_t offset = 0;

  for (int8_t section = numColors - 1; section >= 0; section--) {
    uint8_t len = colorWidth;
    if (section == 0) {
      len = NUM_LEDS - offset;
    }
    CRGB color = availableColors[(colorOffset + section) % availableColorsLength];
    if (isSolid) {
      fill_solid(&leds[offset], len, color);
    } else {
      CRGB sndColor = availableColors[(colorOffset + section + 1) % availableColorsLength];
      fill_gradient_RGB(&leds[offset], len, sndColor, color);
    }
    offset += colorWidth;
  }

  bool fadeOut = false;
  for (uint8_t step = 0; step < NUM_LEDS; step++) {
    FastLED.show();
    CRGB temp = fadeOut ? CRGB::Black : leds[0];
    memmove(leds, &leds[1], sizeof(CRGB) * (NUM_LEDS - 1));
    leds[NUM_LEDS - 1] = temp;
    delay(50);
    if (step == NUM_LEDS - 1 && !fadeOut) {
      step = 0;
      fadeOut = true;
    }
  }
}

typedef void (*t_movefunc)();

constexpr t_movefunc movefuncs[] = {
  &move1,
  &move2,
  &move3,
  &move4,
  &sprinkle,
  &rotation,
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