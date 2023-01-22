#pragma once

#include "controller/controller_avr.hpp"

namespace Ferriswheel
{

template<uint8_t DATA_PIN>
class ArduinoMicroController final : public AvrController<DATA_PIN> {
public:
  virtual void setupTimer() override {
    // Arduino micro: Timer 3
    OCR3A = 156;
    TCCR3B = (1 << WGM32) | (1 << CS30) | (1 << CS32);
    TIMSK3 = (1 << OCIE3A);
  }
};

// Mikro
#define TIMER_VEC TIMER3_COMPA_vect

};

