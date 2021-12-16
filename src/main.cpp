#include <Arduino.h>
#include <FastLED.h>

#include "leds.hpp"
#include "sprinkle.hpp"

void setup() {
  Serial.begin(57600);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(30);
}

void move1() {
  constexpr uint8_t colorCount = 3;
  CRGB colors[colorCount];
  for (uint8_t i = 0; i < colorCount; i++) {
    colors[i] = getRandomColor();
  }

  uint8_t offset = random8(NUM_LEDS);

  for (int i = 0; i < NUM_LEDS; i++) {
    for (int j = 0; j < NUM_LEDS - i; j++) {
      uint8_t led = (j + offset) % NUM_LEDS;
      if (j > 0) {
        leds[(led == 0 ? NUM_LEDS : led) - 1] = CRGB::Black;
      }
      leds[led] = colors[i % colorCount];
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
  for (int iteration = 0; iteration < 10; iteration++) {
    for (int led = 0; led < NUM_LEDS; led++) {
      CRGB ledColor = ((led % 2) == (iteration % 2)) ? bgColor : color;
      leds[led] = ledColor;
    }
    FastLED.show();
    delay(500);
  }
}

void move3() {
  constexpr uint8_t trail_length = 4;

  const int start = random(NUM_LEDS);
  const bool reverse = random(2) == 0;

  for (uint8_t remaining_iterations = NUM_LEDS; remaining_iterations > 0; remaining_iterations--) {
    uint8_t actualTrail = min(trail_length, remaining_iterations - 1);
    for (int i = 0; i < actualTrail; i++) {
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

  int iteration = 20;
  uint8_t newSpecs;
  do {
    newSpecs = glitterSpecs;
    for (int led = 0; led < NUM_LEDS; led++) {
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

void move5() {
  // Should be separated evenly?
  constexpr uint8_t numIslands = NUM_LEDS / 6;
  constexpr uint8_t islandWidth = NUM_LEDS / numIslands;
  CRGB color = getRandomColor();

  // 6: [0 1 2 3 4 5]
  //     6 4 2 1 3 5
  // 5: [0 1 2 3 4]
  //     4 2 1 3 5

  uint8_t islandIndex = islandWidth / 2;
  int8_t delta = -1;
  for (uint8_t width = 0; width < islandWidth; width++) {
    leds[islandIndex] = color;

    islandIndex += delta;
    if (delta < 0) {
      delta--;
    } else {
      delta++;
    }
    delta = -delta;
    for (uint8_t offset = islandWidth; offset + islandWidth <= NUM_LEDS; offset += islandWidth) {
      memcpy(&leds[offset], leds, islandWidth * sizeof(CRGB));
    }
    FastLED.show();
    delay(400);
  }
}

void flashAlternate() {
  constexpr uint8_t iterations = 10;
  constexpr uint8_t flashes = 3;

  CRGB evenColor = CRGB::White;
  CRGB oddColor = evenColor;
  if (random8() < 10) {
    evenColor = CRGB::Blue;
    oddColor = CRGB::Red;
  }

  for (uint8_t iteration = 0; iteration < iterations; iteration++) {
    CRGB iterationColor = iteration % 2 == 0 ? evenColor : oddColor;
    for (uint8_t flash = 0; flash < flashes * 2; flash++) {
      for (uint8_t led = 0; led < NUM_LEDS; led++) {
        leds[led] = (led % 2) == (iteration % 2) && (flash % 2 == 0) ? iterationColor : CRGB::Black;
      }
      FastLED.show();
      delay(50);
    }
  }
}

void fadeIn() {
  constexpr uint8_t dimBy[] = {0xf7, 0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0, 0x80, 0x40, 0x20, 0};
  constexpr uint8_t dimByCount = array_size(dimBy);
  // constexpr uint32_t colorStates[] = {CRGB::Black, CRGB::DarkRed, CRGB::Orange, CRGB::Yellow, CRGB::White};
  // constexpr uint8_t colorStateCount = array_size(colorStates);

  uint8_t litLeds = 0;
  uint8_t state[NUM_LEDS] = {0};

  while (litLeds < NUM_LEDS) {
    litLeds = 0;
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
      if (state[led] < dimByCount - 1) {
        if (random8() < 10) {
          state[led]++;
          CRGB clr = CRGB::White;
          clr.fadeToBlackBy(dimBy[state[led]]);
          leds[led] = clr;
        }
      } else {
        litLeds++;
      }
      // if (state[led] < colorStateCount - 1) {
      //   if (random8() < 10) {
      //     state[led]++;
      //     leds[led] = colorStates[state[led]];
      //   }
      // } else {
      //   litLeds++;
      // }
      // if (leds[led] == CRGB(0, 0, 0)) {
      //   if (random8() < 10) {
      //     leds[led] = CRGB::Red;
      //     litLeds++;
      //   }
      // } else {
      //   litLeds++;
      // }
    }
    FastLED.show();
    delay(100);
  }

  delay(500);
}

typedef void (*t_movefunc)();

constexpr t_movefunc movefuncs[] = {&move1, &move2, &move3, &move4, &move5, &flashAlternate, &fadeIn, &sprinkle};
constexpr uint8_t numMoveFuncs = array_size(movefuncs);

void move(uint8_t mode) {
  allBlack();
  if (mode < numMoveFuncs) {
    (*movefuncs[mode])();
  }
}

void fadeIn2() {
  constexpr uint8_t fadeAmount = 10;

  uint8_t dimFactor[NUM_LEDS] = {0xff};
  uint8_t remainingLights = NUM_LEDS;
  while (remainingLights > 0) {
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
      if (dimFactor[led] < 0xff) {
        if (random8() < 10) {
          if (dimFactor[led] > fadeAmount) {
            dimFactor[led] -= fadeAmount;
          } else {
            dimFactor[led] = 0;
            remainingLights--;
          }

          CRGB color = CRGB::White;
          color.fadeToBlackBy(dimFactor[led]);
          leds[led] = color;
        }
      }
    }
    FastLED.show();
    delay(50);
  }

  uint8_t dimFactor2 = 0xff;
  do {
    if (dimFactor2 > fadeAmount) {
      dimFactor2 -= fadeAmount;
    } else {
      dimFactor2 = 0;
    }
    fadeToBlackBy(leds, NUM_LEDS, 0xff - dimFactor2);
    FastLED.show();
    delay(50);
  } while (dimFactor2 > 0);
}

void loop() {
  allBlack();
  // fadeIn();
  // flashAlternate();
  for (int i = 0; i < numMoveFuncs; i++) {
    move(i);
  }
  // move(random8(numMoveFuncs));
}
