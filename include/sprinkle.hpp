#pragma once

#include <stdint.h>


class SprinkleState
{
public:
  void setUp();

  void init(const uint8_t led, const uint8_t dimStep);

  int getLed() const;

  bool isActive() const;

  bool step();

private:
  uint8_t led;
  uint8_t dimFactor;
  uint8_t dimStep;
  bool brighten;
};

void sprinkle();