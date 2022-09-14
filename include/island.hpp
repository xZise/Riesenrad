#pragma once

#include "animation.hpp"
#include "leds.hpp"

class IslandAnimation : public FrameAnimation<400> {
public:
  IslandAnimation() : _color(getRandomColor()) {}

  virtual bool finished() override;
protected:
  virtual void step() override;
private:
  static constexpr uint8_t numIslands = 9;
  static constexpr uint8_t islandWidth = NUM_LEDS / numIslands;

  CRGB _color;
  uint8_t _islandIndex { islandWidth / 2 };
  int8_t _delta { -1 };
};