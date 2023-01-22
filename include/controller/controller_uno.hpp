#pragma once

#include "controller/controller_avr.hpp"

namespace Ferriswheel
{

template<uint8_t DATA_PIN>
class ArduinoUnoController final : public AvrController<DATA_PIN> {
public:
  virtual void setupTimer() override {
    // Uses timer 2 as the prescaler > 64 with timer 0 did not work
    TCCR2A = (1<<WGM21);    //Set the CTC mode
    OCR2A = 156; //Value for ORC0A for 10ms

    TIMSK2 |= (1<<OCIE2A);   //Set the interrupt request

    TCCR2B |= (1 << CS22) | (1<<CS21) | (1 << CS20);    //Set the prescale 1/1024 clock
  }
};

// Uno
#define TIMER_VEC TIMER2_COMPA_vect

};

