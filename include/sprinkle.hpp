#pragma once

#include <stdint.h>
#include "animation.hpp"

class SprinkleState
{
public:
  SprinkleState();

  void init(const uint8_t led, const uint8_t dimStep);

  uint8_t getLed() const;

  bool isActive() const;

  bool step();

private:
  uint8_t _led;
  uint8_t _dimFactor;
  uint8_t _dimStep;
  bool _brighten;
};

class SprinkleAnimation : public FrameAnimation<50> {
public:
  SprinkleAnimation();

  virtual bool finished() override {
    return _remainingSprinkles == 0;
  }

  ANIMATIONNAME("Sprinkle")
protected:
  virtual void step() override;
private:
  static constexpr uint8_t num_sprinkles = NUM_LEDS / 2;

  uint8_t _sprinkles = 0;
  uint8_t _remainingSprinkles = NUM_LEDS * 2;
  SprinkleState _sprinkleLeds[num_sprinkles];
};
