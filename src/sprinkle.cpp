#include "sprinkle.hpp"
#include "FastLED.h"
#include "leds.hpp"

void SprinkleState::setUp() {
  dimFactor = 0xff;
  brighten = false;
}

void SprinkleState::init(const uint8_t led, const uint8_t dimStep) {
  this->led = led;
  this->dimStep = dimStep;
  dimFactor = 0xff;
  brighten = true;
}

int SprinkleState::getLed() const {
  return led;
}

bool SprinkleState::isActive() const {
  return brighten || dimFactor < 0xff;
}

bool SprinkleState::step() {
  if (!isActive()) {
    return false;
  }

  bool hasStopped = false;
  if (brighten) {
    if (dimFactor > dimStep) {
      dimFactor -= dimStep;
    } else {
      dimFactor = 0;
      brighten = false;
    }
  } else {
    if (dimFactor < 0xff - dimStep) {
      dimFactor += dimStep;
    } else {
      dimFactor = 0xff;
      hasStopped = true;
    }
  }
  CRGB color = CRGB::White;
  color.fadeToBlackBy(dimFactor);
  leds[led] = color;

  return hasStopped;
}


void sprinkle() {
  constexpr uint8_t num_sprinkles = NUM_LEDS / 2;

  uint8_t sprinkles = 0;
  uint8_t remainingSprinkles = NUM_LEDS * 2;
  SprinkleState sprinkleLeds[num_sprinkles];

  for (uint8_t i = 0; i < num_sprinkles; i++) {
    sprinkleLeds[i].setUp();
  }

  while (remainingSprinkles > 0) {

    if (sprinkles < num_sprinkles && remainingSprinkles > sprinkles && random8() < 100) {
      // Try to get a new LED for a new sprinkle
      bool isUnique;
      uint8_t newLed;
      uint8_t freeIndex;
      do {
        isUnique = true;
        newLed = random8(NUM_LEDS);

        freeIndex = num_sprinkles;
        for (uint8_t sprinkleLed = 0; sprinkleLed < num_sprinkles; sprinkleLed++) {
          if (sprinkleLeds[sprinkleLed].isActive()) {
            if (sprinkleLeds[sprinkleLed].getLed() == newLed) {
              isUnique = false;
              break;
            }
          } else if (sprinkleLed < freeIndex) {
            freeIndex = sprinkleLed;
          }
        }
      } while (!isUnique);

      if (freeIndex < num_sprinkles) {
        sprinkleLeds[freeIndex].init(newLed, random8(10, 50));
      }

      sprinkles++;
    }

    if (sprinkles > 0) {
      for (uint8_t sprinkleLed = 0; sprinkleLed < num_sprinkles; sprinkleLed++) {
        if (sprinkleLeds[sprinkleLed].step()) {
          sprinkles--;
          remainingSprinkles--;
        }
      }
    }

    FastLED.show();
    delay(50);
  }
}