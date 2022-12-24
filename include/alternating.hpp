#pragma once

#include "animation.hpp"

class AlternatingBlink: public IterationAnimation<10, 500> {
public:
  AlternatingBlink(const CRGB& firstColor, const CRGB& secondColor) : _firstColor(firstColor), _secondColor(secondColor == firstColor ? CRGB::Black : secondColor) {
  }

  AlternatingBlink() : AlternatingBlink(getRandomColor(), getRandomColor()) {
  }

  virtual const char* name() const override { return "AlternatingBlink"; }
protected:
  virtual void step() override {
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
      const CRGB ledColor = ((led % 2) == (_iteration % 2)) ? _secondColor : _firstColor;
      leds[led] = ledColor;
    }
    IterationAnimation::step();
  }
private:
  CRGB _firstColor;
  CRGB _secondColor;
};