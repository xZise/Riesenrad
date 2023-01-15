#include "move.hpp"

MoveAnimation::MoveAnimation() : _start(random8(NUM_LEDS)), _reverse(randomBool()) {}

void MoveAnimation::step() {
  constexpr uint8_t trail_length = NUM_LEDS / 10 + 1;

  allBlack();

  uint8_t remaining_iterations = iteration_count() - _iteration - 1;

  if (remaining_iterations > 0) {
    uint8_t actualTrail = min(trail_length, (uint8_t)(remaining_iterations - 1));
    for (uint8_t i = 0; i < actualTrail; i++) {
      uint8_t index = getLedOffsetIndex(remaining_iterations, _start);
      index = getLedOffsetIndex(index, i, _reverse);
      if (i == 0) {
        leds[index] = CRGB::Red;
      } else {
        leds[index] = CRGB::Wheat;
      }
    }

    if (remaining_iterations < 4) {
      fadeToBlackBy(leds, NUM_LEDS, 255 / remaining_iterations);
    }
  }
  IterationAnimation::step();
}