#pragma once

#include <stdint.h>
#include <FastLED.h>

bool randomBool();

template <typename T, size_t N>
constexpr size_t array_size(T (&)[N]) {
    return N;
}

constexpr uint8_t NUM_LEDS = 30;
static_assert(NUM_LEDS < 0xff);


#ifdef ARDUINO_ARCH_ESP32
constexpr uint8_t DATA_PIN = 12;
#else
constexpr uint8_t DATA_PIN = 8;

constexpr uint8_t LED_PIN = 7;
#endif

constexpr uint32_t availableColors[] = {CRGB::Red, CRGB::Yellow, CRGB::Green, CRGB::Ivory};
constexpr uint8_t availableColorsLength = array_size(availableColors);

extern CRGB leds[NUM_LEDS];

void allBlack();

const CRGB getRandomColor();

const uint8_t getLedIndex(int8_t index);
const uint8_t getLedOffsetIndex(const uint8_t index, const uint8_t offset, const bool reverse = false);

CRGB* getLed(int8_t index);
CRGB* getLedOffset(const uint8_t index, const uint8_t offset, const bool reverse = false);
