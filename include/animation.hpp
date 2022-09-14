#pragma once

#include "leds.hpp"

class Animation {
public:
  void frame() {
    if (calculateFrame()) {
      FastLED.show();
    }
  }
  virtual bool finished() = 0;
protected:
  static constexpr uint8_t frameMs = 10;

  template<uint16_t milliseconds>
  static constexpr uint8_t framesPerMs() {
    static_assert(milliseconds < 0xff * Animation::frameMs, "To high millisecond value");
    return milliseconds / Animation::frameMs;
  }

  virtual bool calculateFrame() = 0;
};

template<uint16_t frameDelay>
class FrameAnimation: public Animation {
protected:
  uint8_t _frame = 0;

  virtual bool calculateFrame() override {
    if (_frame < framesPerMs<frameDelay>()) {
      _frame++;
      if (_frame == 1) {
        step();
        return true;
      } else {
        return false;
      }
    } else {
      _frame = 0;
      return false;
    }
  }

  virtual void step() = 0;
};

template<uint8_t iterationCount, uint16_t frameDelay>
class IterationAnimation: public FrameAnimation<frameDelay> {
public:
  virtual bool finished() override {
    return _iteration >= iterationCount;
  }
protected:
  uint8_t _iteration = 0;

  virtual void step() override {
    _iteration += 1;
  }
};

class GlitterBlink: public IterationAnimation<20, 50> {
public:
  static constexpr uint8_t frameDelay = 100;
  static constexpr uint8_t iterationCount = 20;

  virtual bool finished() override {
    return (_iteration >= iterationCount) && (_newSpecs = glitterSpecs);
  }
protected:
  virtual void step() override {
    _newSpecs = glitterSpecs;
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
      if (leds[led] != CRGB(0, 0, 0)) {
        if (random8() < 150) {
          leds[led] = CRGB::Black;
        } else {
          _newSpecs--;
        }
      }
    }
    if (_iteration > 0) {
      while (_newSpecs-- > 0) {
        leds[random8(NUM_LEDS)] = CRGB::White;
      }
    }
    IterationAnimation::step();
  }
private:
  static constexpr uint8_t glitterSpecs = NUM_LEDS / 5;

  uint8_t _newSpecs = glitterSpecs;
};