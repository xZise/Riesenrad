#include "snake.hpp"

void SnakeAnimation::step() {
    if (_shrinking) {
      shrink();
    } else {
      move();
      if (_length >= maxLength) {
        _shrinking = true;
      }
    }
}

void SnakeAnimation::shrink() {
  if (_length-- > 0) {
    *getLedOffset(_position, _length + 1, _reverse) = CRGB::Black;
  }
}

void SnakeAnimation::move() {
  uint8_t missingApples = maxApples - apples.count();
  if (missingApples > 0) {
    uint8_t remainingLengthUnfed = maxLength - _length - apples.count();
    if (remainingLengthUnfed < missingApples) {
      missingApples = remainingLengthUnfed;
    }
  }
  while (missingApples-- > 0) {
    uint8_t index = 0;
    uint8_t newApple;
    do {
      newApple = random8(NUM_LEDS);
      index = snakeIndex(newApple);
    } while(index < _length || apples[newApple]);
    apples.set(newApple);
  }

  // draw current state
  for (uint8_t led = 0; led < NUM_LEDS; led++) {
    CRGB color;
    if (led == _position) {
      color = head;
    } else {
      uint8_t index = snakeIndex(led);
      if (index <= _length) {
        color = ((index - 1) >> 1) % 2 == 0 ? evenBody : oddBody;
      } else if (apples[led]) {
        color = CRGB::Red;
      } else {
        color = CRGB::Black;
      }
    }
    const uint8_t actualIndex = _reverse ? NUM_LEDS - led - 1 : led;
    leds[actualIndex] = color;
  }
  if (apples.reset(_position)) {
    _length++;
  }
  if (_position > 0) {
    _position -= 1;
  } else {
    _position = NUM_LEDS - 1;
  }
}

uint8_t SnakeAnimation::snakeIndex(const uint8_t index) const {
  uint8_t relative = index - _position;
  // if index < position -> the index "would be" the number of leds higher
  if (index < _position) {
    relative += NUM_LEDS;
  }
  return relative;
}