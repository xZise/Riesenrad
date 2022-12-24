#pragma once

#include "animation.hpp"

class MoveAnimation : public IterationAnimation<NUM_LEDS + 1, 100> {
public:
  MoveAnimation();

  virtual const char* name() const override { return "MoveAnimation"; }
protected:
  virtual void step() override;
private:
  uint8_t _start;
  bool _reverse;
};