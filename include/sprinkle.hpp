#pragma once

#include <stdint.h>


class SprinkleState
{
public:
  SprinkleState();

  void init(const uint8_t led, const uint8_t dimStep);

  int getLed() const;

  bool isActive() const;

  bool step();

private:
  uint8_t _led;
  uint8_t _dimFactor;
  uint8_t _dimStep;
  bool _brighten;
};

void sprinkle();