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


#ifndef ARDUINO_ARCH_ESP32
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

namespace divisions {

struct DivisorData {
  bool isDivisor;
  uint8_t divident;
  uint8_t divisor;
};

static constexpr DivisorData divisorData[] = {
  {NUM_LEDS % 1  == 0 && 1  < NUM_LEDS, NUM_LEDS / 1 , 1},
  {NUM_LEDS % 2  == 0 && 2  < NUM_LEDS, NUM_LEDS / 2 , 2},
  {NUM_LEDS % 3  == 0 && 3  < NUM_LEDS, NUM_LEDS / 3 , 3},
  {NUM_LEDS % 4  == 0 && 4  < NUM_LEDS, NUM_LEDS / 4 , 4},
  {NUM_LEDS % 5  == 0 && 5  < NUM_LEDS, NUM_LEDS / 5 , 5},
  {NUM_LEDS % 6  == 0 && 6  < NUM_LEDS, NUM_LEDS / 6 , 6},
  {NUM_LEDS % 7  == 0 && 7  < NUM_LEDS, NUM_LEDS / 7 , 7},
  {NUM_LEDS % 8  == 0 && 8  < NUM_LEDS, NUM_LEDS / 8 , 8},
  {NUM_LEDS % 9  == 0 && 9  < NUM_LEDS, NUM_LEDS / 9 , 9},
  {NUM_LEDS % 10 == 0 && 10 < NUM_LEDS, NUM_LEDS / 10, 10},
};
static constexpr uint8_t maximumDivisor =
  divisorData[9].isDivisor ? divisorData[9].divisor :
  divisorData[8].isDivisor ? divisorData[8].divisor :
  divisorData[7].isDivisor ? divisorData[7].divisor :
  divisorData[6].isDivisor ? divisorData[6].divisor :
  divisorData[5].isDivisor ? divisorData[5].divisor :
  divisorData[4].isDivisor ? divisorData[4].divisor :
  divisorData[3].isDivisor ? divisorData[3].divisor :
  divisorData[2].isDivisor ? divisorData[2].divisor :
  divisorData[1].isDivisor ? divisorData[1].divisor :
  divisorData[0].isDivisor ? divisorData[0].divisor :
  1;
static constexpr uint8_t numberOfDivisors =
  (divisorData[9].isDivisor ? 1 : 0) +
  (divisorData[8].isDivisor ? 1 : 0) +
  (divisorData[7].isDivisor ? 1 : 0) +
  (divisorData[6].isDivisor ? 1 : 0) +
  (divisorData[5].isDivisor ? 1 : 0) +
  (divisorData[4].isDivisor ? 1 : 0) +
  (divisorData[3].isDivisor ? 1 : 0) +
  (divisorData[2].isDivisor ? 1 : 0) +
  (divisorData[1].isDivisor ? 1 : 0) +
  (divisorData[0].isDivisor ? 1 : 0) +
  0;
static_assert(numberOfDivisors > 0);

}
