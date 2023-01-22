#pragma once

#include "controller/controller.hpp"

namespace Ferriswheel
{

template<uint8_t DATA_PIN>
class AvrController : public Controller<DATA_PIN> {
public:
  void nextFrame() { frame++; }

  virtual void run() override {
    this->outsideLoop();
  }
protected:
  virtual void delayFrame() override {
    bool update = false;
    while (!update) {
      cli();
      update = frame > 0;
      if (update) {
        frame--;
      }
      sei();
    }
  }
private:
  volatile uint8_t frame = 0;
};

};

