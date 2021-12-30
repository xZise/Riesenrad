#pragma once

#include "animation.hpp"

class AlternatingBlink: public IterationAnimation<10, 500> {
private:
  static constexpr uint8_t iterationCount = 10;
  static constexpr uint16_t frameDelay = 500;

  CRGB _firstColor;
  CRGB _secondColor;
protected:
  virtual void step() override {
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
      const CRGB ledColor = ((led % 2) == (_iteration % 2)) ? _secondColor : _firstColor;
      leds[led] = ledColor;
    }
  }
public:
  AlternatingBlink(const CRGB& firstColor, const CRGB& secondColor) : _firstColor(firstColor), _secondColor(secondColor == firstColor ? CRGB::Black : secondColor) {
  }

  AlternatingBlink() : AlternatingBlink(getRandomColor(), getRandomColor()) {
  }

  void reset() override {
    IterationAnimation<10, 500>::reset();
    _firstColor = getRandomColor();
    _secondColor = getRandomColor();
    if (_secondColor == _firstColor) {
      _secondColor = CRGB::Black;
    }
  }
};