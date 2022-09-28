#include "island.hpp"

bool IslandAnimation::finished() {
  // 6: [0 1 2 3 4 5]
  //     6 4 2 1 3 5
  // 5: [0 1 2 3 4]
  //     4 2 1 3 5

  if (islandWidth % 2 == 0) {
    return _islandIndex == (uint8_t)-1;
  } else {
    return _islandIndex == islandWidth;
  }
}

void IslandAnimation::step() {
  uint8_t offset = _islandIndex;
  while (offset < NUM_LEDS) {
    leds[offset] = _color;
    offset += islandWidth;
  }

  _islandIndex += _delta;
  if (_delta < 0) {
    _delta--;
  } else {
    _delta++;
  }
  _delta = -_delta;
}