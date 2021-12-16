#pragma once

#include <stdint.h>
#include <FastLED.h>

template <typename T, size_t N>
constexpr size_t array_size(T (&)[N]) {
    return N;
}

constexpr uint8_t NUM_LEDS = 30;
constexpr uint8_t DATA_PIN = 8;

constexpr uint32_t availableColors[] = {CRGB::Red, CRGB::Yellow, CRGB::Green, CRGB::Ivory};
constexpr uint8_t availableColorsLength = array_size(availableColors);

extern CRGB leds[NUM_LEDS];

void allBlack();

const CRGB getRandomColor();

const uint8_t getLedIndex(int8_t index);

CRGB* getLed(int8_t index);