#pragma once

#include "animation.hpp"
#include "animationbuffer.hpp"

class RotationAnimation : public FrameAnimation<50> {
public:
  template<size_t numColors>
  RotationAnimation(const uint32_t (&sectionColors)[numColors],
                    const uint8_t sectionMultiply,
                    const bool isSolid) : _steps(rotationCount * NUM_LEDS) {
    const uint8_t numSections = sectionMultiply * numColors;
    const uint8_t colorWidth = NUM_LEDS / numSections;

    const uint8_t colorOffset = random8(numColors);

    uint8_t offset = 0;

    for (int8_t section = numSections - 1; section >= 0; section--) {
      uint8_t len = colorWidth;
      if (section == 0) {
        len = NUM_LEDS - offset;
      }
      CRGB color = sectionColors[(colorOffset + section) % numColors];
      if (isSolid) {
        fill_solid(&leds[offset], len, color);
      } else {
        CRGB sndColor = sectionColors[(colorOffset + section + 1) % numColors];
        fill_gradient_RGB(&leds[offset], len, sndColor, color);
      }
      offset += colorWidth;
    }
  }

  template<size_t numColors>
  RotationAnimation(const uint32_t (&sectionColors)[numColors])
    : RotationAnimation(sectionColors, random8(3) + 1, randomBool()) {
  }

  static void createRandom(AnimationBuffer& buffer) {
    switch (random8(3))
    {
    case 0:
      {
        const uint32_t c[] = {CRGB::White, CRGB::Black};
        buffer.create<RotationAnimation>(c);
        break;
      }
    case 1:
      {
        const uint32_t c[] = {CRGB::SkyBlue, CRGB::RoyalBlue, CRGB::Blue};
        buffer.create<RotationAnimation>(c);
        break;
      }
    case 2:
      buffer.create<RotationAnimation>(availableColors);
      break;
    }
  }

  virtual bool finished() override {
    return _steps == 0;
  }

  virtual bool clearOnStart() const override {
    return false;
  }

  virtual const char* name() const override { return "RotationAnimation"; }
protected:
  virtual void step() override {
    _steps--;

    CRGB temp = _steps < NUM_LEDS ? CRGB::Black : leds[0];
    memmove(leds, &leds[1], sizeof(CRGB) * (NUM_LEDS - 1));
    leds[NUM_LEDS - 1] = temp;
  }
private:
  static constexpr uint8_t rotationCount = 5;

  uint16_t _steps;
};
