#pragma once

#ifdef ARDUINO_ARCH_ESP32
#include <freertos/task.h>
#endif

#include <Arduino.h>
#include "leds.hpp"

#include "animation.hpp"
#include "alternating.hpp"
#include "snake.hpp"
#include "island.hpp"
#include "move.hpp"
#include "sprinkle.hpp"
#include "stacks.hpp"
#include "rotation.hpp"

#include "animationbuffer.hpp"

namespace Ferriswheel
{

typedef void (*publish_animation_t)(const Animation* animation);

template<uint8_t DATA_PIN>
class Controller {
public:
  virtual void setupTimer() = 0;

  virtual void run() = 0;

  constexpr uint8_t rgbLEDpin() { return DATA_PIN; }

  const bool animationsEnabled() const { return _animationsEnabled; }
  void setAnimationsEnabled(bool enabled) { _animationsEnabled = enabled; }

  void onPublishAnimation(publish_animation_t handler) { _publishAnimation = handler; }
protected:
  virtual void delayFrame() = 0;

  void animationLoop(Animation& animation) {
    if (animation.clearOnStart()) {
      allBlack();
    }
    while (_animationsEnabled) {
      delayFrame();
      // FIXME: This should be overhauled, as this leads to code which changed
      //        something in frame(), only to determine that it has finished.
      //        When an animation made one step there it would be only visible for
      //        a very short duration of time.
      //        Checking finished() before, does not help very much, as it would
      //        just check it 10ms later on the next iteration. A better solution
      //        is probably to add something to FrameAnimation, so that finished()
      //        changes on the last tick before the next frame is calculated.
      animation.frame();
      if (animation.finished()) {
        return;
      }
    }
  }

  void outsideLoop() {
    while (true) {
      if (!_animationsEnabled) {
        allBlack();
        FastLED.show();
        while (!_animationsEnabled) {
          delayFrame();
        }
      }
      switch (random8(8))
      {
      case 0: RUN_ANIMATION_ARGS(AlternatingBlink)
      case 1: RUN_ANIMATION_ARGS(GlitterBlink)
      case 2: RUN_ANIMATION_ARGS(SnakeAnimation)
      case 3: RUN_ANIMATION_ARGS(IslandAnimation)
      case 4: RUN_ANIMATION_ARGS(MoveAnimation)
      case 5: RUN_ANIMATION_ARGS(SprinkleAnimation)
      case 6: RUN_ANIMATION_ARGS(FallingStacks)
      case 7:
        RotationAnimation::createRandom(animationBuffer);
        break;
      }
      Animation& animation = *animationBuffer.get();
      const char* name = animation.name();
      publishAnimation(&animation);
      if (name) {
        Serial.print("Selected animation: ");
        Serial.println(name);
      } else {
        Serial.println("Selected animation without name.");
      }
      animationLoop(animation);
    }
  }
private:
  bool _animationsEnabled;

  publish_animation_t _publishAnimation;

  void publishAnimation(const Animation* animation) {
    if (_publishAnimation) {
      _publishAnimation(animation);
    }
  }
};

};

