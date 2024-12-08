#pragma once

#include "animation.hpp"

class MoveAnimation : public IterationAnimation<NUM_LEDS + 1, 100> {
public:
  MoveAnimation();

  ANIMATIONNAME("Move")
protected:
  virtual void step() override;
private:
  uint8_t _start;
  bool _reverse;
};
