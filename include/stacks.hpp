#pragma once

#include "animation.hpp"

class FallingStacks: public DynamicFrameAnimation {
public:
  FallingStacks()
    : DynamicFrameAnimation(50)
    , _offset(random8(NUM_LEDS)) {

    // in case the for-loop fails (would be a misconfiguration?!)
    _stackLength = 1;
    _stackCount = NUM_LEDS;
    uint8_t index = random8(divisions::numberOfDivisors);
    for (const divisions::DivisorData& data : divisions::divisorData) {
      if (data.isDivisor) {
        if (index == 0) {
          _stackLength = data.divisor;
          _stackCount = data.divident;
          break;
        }
        index--;
      }
    }

    // In theory this needs to now test the divisors of _stackLength, but except for primes, it can only be 4, 6, 8, 9
    // or 10. And every number (except 9) has the divisors 1, 2 and half, so this is close enough.
    if (_stackLength % 2 == 0) {
      switch (random8(3)) {
        case 0:
        default:
          _fallDistance = 1;
          break;
        case 1:
          _fallDistance = _stackLength / 2;
          break;
        case 2:
          _fallDistance = 2;
          break;
      }
    } else {
      _fallDistance = 1;
    }

    for (uint8_t i = 0; i < _stackLength; i++) {
      _stackColors[i] = getRandomColor();
    }
  }

  virtual bool finished() override {
    return wipe_mode() && _step > CENTER_FAR;
  }

  ANIMATIONNAME("Stacking")
protected:
  virtual uint16_t step() override {
    if (wipe_mode()) {
      step_wipe();
      return 100;
    } else {
      return step_fall();
    }
  }
private:
  CRGB _stackColors[divisions::maximumDivisor];

  // Determines the start of the stacks
  uint8_t _offset;
  // total number of lit leds
  uint8_t _stack { 0 };
  // current step within a stack, when falling
  // current step of the complete wipe
  uint8_t _step { 0 };
  uint8_t _stackLength;
  uint8_t _stackCount;
  uint8_t _fallDistance;

  const bool wipe_mode() const { return _stack >= NUM_LEDS; }

  uint16_t step_fall() {
    for (uint8_t stack_offset = 0; stack_offset < _stackLength; stack_offset++) {
      uint8_t index = (_step + _offset + stack_offset) % NUM_LEDS;

      // Clear previous stack, if this is not the first step
      if (_step > 0 && _stackLength - stack_offset <= _fallDistance) {
        uint8_t oldIndex = index;
        if (oldIndex < _stackLength) {
          oldIndex += NUM_LEDS;
        }
        oldIndex -= _stackLength;
        leds[oldIndex] = CRGB::Black;
      }

      leds[index] = _stackColors[(_stack + stack_offset) % _stackLength];
    }
    _step += _fallDistance;
    if (_step > NUM_LEDS - _stack - _stackLength) {
      _step = 0;
      _stack += _stackLength;
      return 700;
    } else {
      return 50;
    }
  }

  static constexpr uint8_t CENTER_FAR = NUM_LEDS / 2;
  static constexpr uint8_t CENTER_NEAR = NUM_LEDS / 2 - (1 - NUM_LEDS % 2);

  void step_wipe() {
    const uint8_t center_far = CENTER_FAR + _step;
    const uint8_t center_near = CENTER_NEAR - _step;

    *getLedOffset(center_near, _offset) = CRGB::Black;
    *getLedOffset(center_far, _offset) = CRGB::Black;
    _step++;
  }
};
