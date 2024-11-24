#pragma once

#include "animation.hpp"

class FallingStacks: public DynamicFrameAnimation {
public:
  FallingStacks()
    : DynamicFrameAnimation(50)
    , _offset(random8(NUM_LEDS)) {
    for (uint8_t i = 0; i < stackLedCount; i++) {
      _colors[i] = getRandomColor();
    }
  }

  virtual bool finished() override {
    return wipe_mode() && _step > CENTER_FAR;
  }

  virtual const char* name() const override { return "StackAnimation"; }
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
  static constexpr uint8_t stackDivisor = 30;
  static constexpr uint8_t stackLength = NUM_LEDS >= stackDivisor ? NUM_LEDS / stackDivisor : 1;
  static constexpr uint8_t stackCount = stackLength < 4 ? 4 - stackLength : 1;
  // TODO: In theory this should be a clean divisor of NUM_LEDs, this is not guaranteed yet.
  static constexpr uint8_t stackLedCount = stackLength * stackCount;
  CRGB _colors[stackLedCount];

  // defines the start of the stacks
  uint8_t _offset;
  // total number of non-moving (already stacked) leds
  uint8_t _stack { 0 };
  // current step within a stack, when falling
  // current step of the complete wipe
  uint8_t _step { 0 };

  const bool wipe_mode() const { return _stack >= NUM_LEDS; }

  uint16_t step_fall() {
    for (uint8_t stack_offset = 0; stack_offset < stackLength; stack_offset++) {
      uint8_t index = (_step + _offset + stack_offset) % NUM_LEDS;

      // Clear previous stack, if this is not the first step
      if (_step > 0) {
        uint8_t oldIndex = index;
        if (oldIndex < stackLength) {
          oldIndex += NUM_LEDS;
        }
        oldIndex -= stackLength;
        leds[oldIndex] = CRGB::Black;
      }

      leds[index] = _colors[(_stack + stack_offset) % stackLedCount];
    }
    _step += stackLength;
    if (_step >= NUM_LEDS - _stack) {
      _step = 0;
      _stack += stackLength;
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
