#pragma once

#include "animation.hpp"
#include "leds.hpp"
#include "bitset.hpp"

class SnakeAnimation : public FrameAnimation<50> {
public:
  SnakeAnimation() : _reverse(randomBool()), _length(startLength), _position(random8(NUM_LEDS)) {}

  virtual bool finished() override {
    return _length == 0;
  }
protected:
  virtual void step() override;
private:
  static constexpr uint8_t startLength = 3;
  static constexpr uint8_t maxApples = 4;
  static constexpr uint8_t maxLength = NUM_LEDS / 4 * 3;
  const CRGB head = CRGB::DarkGreen;
  const CRGB oddBody = CRGB::Green;
  const CRGB evenBody = CRGB::Turquoise;

  Bitset<NUM_LEDS> apples;
  bool _reverse;
  uint8_t _length;
  uint8_t _position;
  bool _shrinking { false };

  void shrink();

  void move();
};