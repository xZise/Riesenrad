#include "leds.hpp"

CRGB leds[NUM_LEDS];

void allBlack() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

const CRGB getRandomColor() {
  return CRGB(availableColors[random8(availableColorsLength)]);
}

const uint8_t getLedIndex(int8_t index) {
  if (index < 0) {
    while (index < 0) {
      index += NUM_LEDS;
    }
    return index;
  } else {
    return index % NUM_LEDS;
  }
}

CRGB* getLed(int8_t index) {
  uint8_t absoluteIndex = getLedIndex(index);
  return &leds[absoluteIndex];
}