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

const uint8_t getLedOffsetIndex(const uint8_t index, const uint8_t offset) {
  uint8_t newIndex = index + offset;
  // When this overflowed, the new value will always be lower than either of them
  if (newIndex < index) {
    // Decrement by NUM_LEDS count until the new value is larger than the
    // previous sum. In that case it undid the overflow (underflowed).
    uint8_t tempIndex = newIndex;
    while (newIndex <= tempIndex) {
      newIndex -= NUM_LEDS;
    }
  }
  while (newIndex >= NUM_LEDS) {
    newIndex -= NUM_LEDS;
  }
  return newIndex;
}

CRGB* getLed(int8_t index) {
  uint8_t absoluteIndex = getLedIndex(index);
  return &leds[absoluteIndex];
}