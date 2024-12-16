#pragma once

#ifdef ARDUINO_ARCH_ESP32
#include <freertos/task.h>
#endif

#include <Arduino.h>
#include "leds.hpp"

#include "config.hpp"

#include "animation.hpp"
#include "alternating.hpp"
#include "snake.hpp"
#include "island.hpp"
#include "move.hpp"
#include "sprinkle.hpp"
#include "stacks.hpp"
#include "rotation.hpp"

#include "config.hpp"
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

  virtual void begin() {
    bool was_enabled = loadSettingBool(Setting::ANIMATIONS_ENABLED);
    setAnimationsEnabled(was_enabled);

#ifdef MOTOR_AVAILABLE
    _motor_enabled = loadSettingBool(Setting::MOTOR_ENABLED);
#endif // MOTOR_AVAILABLE
  }

  virtual void run() = 0;

  constexpr uint8_t rgbLEDpin() { return DATA_PIN; }

  enum class Setting {
    ANIMATIONS_ENABLED = 1,
#ifdef MOTOR_AVAILABLE
    MOTOR_ENABLED = 2,
#endif // MOTOR_AVAILABLE
  };

  virtual void saveSettingInt(Setting setting, uint8_t value) = 0;
  virtual void saveSettingBool(Setting setting, bool value) = 0;
  virtual uint8_t loadSettingInt(Setting setting) = 0;
  virtual bool loadSettingBool(Setting setting) = 0;

  bool animationsEnabled() const { return _animations_enabled; }
  bool nextAnimationRequested() const { return _next_animation_requested; }

  void setAnimationsEnabled(bool enabled) {
    _animations_enabled = enabled;
    saveSettingBool(Setting::ANIMATIONS_ENABLED, enabled);
  }

  void requestNextAnimation() { _next_animation_requested = true; }

#ifdef MOTOR_AVAILABLE
  bool motorEnabled() const { return _motor_enabled; }
  void setMotorEnabled(bool enabled) {
    _motor_enabled = enabled;
    saveSettingBool(Setting::MOTOR_ENABLED, enabled);
  }
#endif // MOTOR_AVAILABLE

  void onPublishAnimation(publish_animation_t handler) { _publish_animation = handler; }

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
    while (animationsEnabled()) {
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
      if (_next_animation_requested || animation.finished()) {
        _next_animation_requested = false;
        return;
      }
    }
  }

  void outsideLoop() {
    while (true) {
      if (!animationsEnabled()) {
        publishAnimation(nullptr);
        allBlack();
        FastLED.show();
        while (!animationsEnabled()) {
          delayFrame();
        }
      }

      if (createAnimation()) {
        Animation& animation = *animation_buffer.get();
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
  bool _next_animation_requested;
  bool _animations_enabled = false;
#ifdef MOTOR_AVAILABLE
  bool _motor_enabled;
#endif // MOTOR_AVAILABLE

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
    uint8_t animation_count = enabledAnimationCount();
    uint8_t selected_animation = random8(animation_count);
    Serial.print("Selected animation ");
    Serial.print(selected_animation);
    Serial.print(" of ");
    Serial.println(animation_count);
    uint8_t original_selected_animation = selected_animation;
    if (checkEnabled(selected_animation, _enabledAlternatingBlink)) {
      animation_buffer.create<AlternatingBlink>();
      return true;
    }
    if (checkEnabled(selected_animation, _enabledGlitterBlink)) {
      animation_buffer.create<GlitterBlink>();
      return true;
    }
    if (checkEnabled(selected_animation, _enabledSnakeAnimation)) {
      animation_buffer.create<SnakeAnimation>();
      return true;
    }
    if (checkEnabled(selected_animation, _enabledIslandAnimation)) {
      animation_buffer.create<IslandAnimation>();
      return true;
    }
    if (checkEnabled(selected_animation, _enabledMoveAnimation)) {
      animation_buffer.create<MoveAnimation>();
      return true;
    }
    if (checkEnabled(selected_animation, _enabledSprinkleAnimation)) {
      animation_buffer.create<SprinkleAnimation>();
      return true;
    }
    if (checkEnabled(selected_animation, _enabledFallingStacks)) {
      animation_buffer.create<FallingStacks>();
      return true;
    }
    if (checkEnabled(selected_animation, _enabledRotationAnimation)) {
      RotationAnimation::createRandom(animation_buffer);
      return true;
    }
    Serial.print("Original animation selected was index ");
    Serial.print(original_selected_animation);
    Serial.print(" remaining value is ");
    Serial.println(selected_animation);
    return false;
  }

  publish_animation_t _publish_animation;

  void publishAnimation(const Animation* animation) {
    if (_publish_animation) {
      _publish_animation(animation);
    }
  }
};

};

