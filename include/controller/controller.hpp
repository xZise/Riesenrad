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

#define ENABLED_ANIMATIONS_LIST \
    X(AlternatingBlink)         \
    X(GlitterBlink)             \
    X(SnakeAnimation)           \
    X(IslandAnimation)          \
    X(MoveAnimation)            \
    X(SprinkleAnimation)        \
    X(FallingStacks)            \
    X(RotationAnimation)

template<uint8_t DATA_PIN>
class Controller {
public:
  virtual void setupTimer() = 0;
  virtual void begin() {}

  virtual void run() = 0;

  constexpr uint8_t rgbLEDpin() { return DATA_PIN; }

  const bool animationsEnabled() const { return _animationsEnabled; }
  const bool nextAnimationRequested() const { return _nextAnimationRequested; }
  const bool motorEnabled() const { return _motorEnabled; }

  virtual void setInnerLightBrightness(uint8_t brightness) = 0;
  virtual void setAnimationsEnabled(bool enabled) { _animationsEnabled = enabled; }
  void requestNextAnimation() { _nextAnimationRequested = true; }
  void setMotorEnabled(bool enabled) { _motorEnabled = enabled; }

  void onPublishAnimation(publish_animation_t handler) { _publishAnimation = handler; }

#define X(field) \
  bool is##field##Enabled() const { return _enabled##field; } \
  void set##field##Enabled(bool enabled) { _enabled##field = enabled; }

ENABLED_ANIMATIONS_LIST
#undef X

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
      if (_nextAnimationRequested || animation.finished()) {
        _nextAnimationRequested = false;
        return;
      }
    }
  }

  void outsideLoop() {
    while (true) {
      if (!_animationsEnabled) {
        publishAnimation(nullptr);
        allBlack();
        FastLED.show();
        while (!_animationsEnabled) {
          delayFrame();
        }
      }

      if (createAnimation()) {
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
      } else {
        publishAnimation(nullptr);
      }
    }
  }
private:
  bool _nextAnimationRequested;
  bool _animationsEnabled;
  bool _motorEnabled;

#define X(field) \
  bool _enabled##field = true;

ENABLED_ANIMATIONS_LIST
#undef X

  uint8_t enabledAnimationCount() {
    uint8_t result = 0;

#define X(field) \
    if (_enabled##field) result++;

ENABLED_ANIMATIONS_LIST
#undef X
    return result;
  }

  bool checkEnabled(uint8_t& selected, bool currentState) {
    if (currentState) {
      if (selected == 0) {
        return true;
      }
      selected--;
    }
    return false;
  }

  bool createAnimation() {
    uint8_t animationCount = enabledAnimationCount();
    uint8_t selectedAnimation = random8(animationCount);
    Serial.print("Selected animation ");
    Serial.print(selectedAnimation);
    Serial.print(" of ");
    Serial.println(animationCount);
    uint8_t originalSelectedAnimation = selectedAnimation;
    if (checkEnabled(selectedAnimation, _enabledAlternatingBlink)) {
      animationBuffer.create<AlternatingBlink>();
      return true;
    }
    if (checkEnabled(selectedAnimation, _enabledGlitterBlink)) {
      animationBuffer.create<GlitterBlink>();
      return true;
    }
    if (checkEnabled(selectedAnimation, _enabledSnakeAnimation)) {
      animationBuffer.create<SnakeAnimation>();
      return true;
    }
    if (checkEnabled(selectedAnimation, _enabledIslandAnimation)) {
      animationBuffer.create<IslandAnimation>();
      return true;
    }
    if (checkEnabled(selectedAnimation, _enabledMoveAnimation)) {
      animationBuffer.create<MoveAnimation>();
      return true;
    }
    if (checkEnabled(selectedAnimation, _enabledSprinkleAnimation)) {
      animationBuffer.create<SprinkleAnimation>();
      return true;
    }
    if (checkEnabled(selectedAnimation, _enabledFallingStacks)) {
      animationBuffer.create<FallingStacks>();
      return true;
    }
    if (checkEnabled(selectedAnimation, _enabledRotationAnimation)) {
      RotationAnimation::createRandom(animationBuffer);
      return true;
    }
    Serial.print("Original animation selected was index ");
    Serial.print(originalSelectedAnimation);
    Serial.print(" remaining value is ");
    Serial.println(selectedAnimation);
    return false;
  }

  publish_animation_t _publishAnimation;

  void publishAnimation(const Animation* animation) {
    if (_publishAnimation) {
      _publishAnimation(animation);
    }
  }
};

};

