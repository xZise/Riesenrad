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
    _light_on = loadSettingBool(Setting::LIGHT_ON);
    _static_light_mode_color.red = loadSettingInt(Setting::STATIC_LIGHT_MODE_COLOR_RED);
    _static_light_mode_color.green = loadSettingInt(Setting::STATIC_LIGHT_MODE_COLOR_GREEN);
    _static_light_mode_color.blue = loadSettingInt(Setting::STATIC_LIGHT_MODE_COLOR_BLUE);

    bool was_enabled = loadSettingBool(Setting::ANIMATIONS_ENABLED);
    setAnimationsEnabled(was_enabled);

    _motor_enabled = loadSettingBool(Setting::MOTOR_ENABLED);
    _motor_max_speed = loadSettingInt(Setting::MOTOR_MAX_SPEED);
    _inner_light_brightness = loadSettingInt(Setting::INNER_LIGHT_BRIGHTNESS);
  }

  virtual void run_loop() = 0;

  virtual bool isMagnetDetected() = 0;

  void run() {
    while (true) {
      run_loop();

      if (isMagnetDetected() && !_magnet_detected && (_last_magnet_detection_timestamp == 0 || millis() - _last_magnet_detection_timestamp > 200)) {
        const unsigned long time_diff = millis() - _last_magnet_detection_timestamp;
        const float speed = (time_diff > 0) ? (Config::WHEEL_DIAMETER_MM * M_PI / Config::WHEEL_NUMBER_OF_CABINS / (time_diff / 1000.0)) : 0;
        Serial.printf("Wheel speed: %.2f mm/sec\n", speed);

        _last_magnet_detection_timestamp = millis();

        if (!continuousMode() && ++_passed_cabins >= Config::STOP_EVERY_N_CABINS) {
          Serial.println("Stopping for passengers");
          _motor_should_stop_for_passengers = true;
          _passed_cabins = 0;
        }
      }

      _magnet_detected = isMagnetDetected();
    }
  }

  constexpr uint8_t rgbLEDpin() { return DATA_PIN; }

  enum class Setting {
    ANIMATIONS_ENABLED = 1,
    LIGHT_ON = 2,
    STATIC_LIGHT_MODE_COLOR_RED = 3,
    STATIC_LIGHT_MODE_COLOR_GREEN = 4,
    STATIC_LIGHT_MODE_COLOR_BLUE = 5,
    MOTOR_ENABLED = 6,
    MOTOR_MAX_SPEED = 7,
    INNER_LIGHT_ON = 8,
    INNER_LIGHT_BRIGHTNESS = 9,
    CONTINUOUS_MODE = 10,
  };

  virtual void saveSettingInt(Setting setting, int value) = 0;
  virtual void saveSettingBool(Setting setting, bool value) = 0;
  virtual int loadSettingInt(Setting setting) = 0;
  virtual bool loadSettingBool(Setting setting) = 0;

  bool motorEnabled() const { return _motor_enabled; }
  uint8_t motorMaxSpeed() const { return _motor_max_speed; }
  bool innerLightOn() const { return _inner_light_on; }
  uint8_t innerLightBrightness() const { return _inner_light_brightness; }
  bool animationsEnabled() const { return _animations_enabled; }
  bool lightOn() const { return _light_on; }
  CRGB staticLightModeColor() const { return _static_light_mode_color; }
  bool nextAnimationRequested() const { return _next_animation_requested; }
  bool continuousMode() const { return _continuous_mode; }
  bool motorShouldStopForPassengers() const { return _motor_should_stop_for_passengers; }

  void setInnerLightOn(bool on) {
    _inner_light_on = on;
    saveSettingBool(Setting::INNER_LIGHT_ON, on);
  }

  void setInnerLightBrightness(uint8_t brightness) {
    _inner_light_brightness = brightness;
    saveSettingInt(Setting::INNER_LIGHT_BRIGHTNESS, brightness);
  }

  void setMotorEnabled(bool enabled) {
    _motor_enabled = enabled;
    saveSettingBool(Setting::MOTOR_ENABLED, enabled);
  }

  void setMotorMaxSpeed(uint8_t speed) {
    _motor_max_speed = speed;
    saveSettingInt(Setting::MOTOR_MAX_SPEED, speed);
  }

  void setAnimationsEnabled(bool enabled) {
    _animations_enabled = enabled;
    saveSettingBool(Setting::ANIMATIONS_ENABLED, enabled);
  }

  void setLightOn(bool light_on) {
    _light_on = light_on;
    saveSettingBool(Setting::LIGHT_ON, light_on);
  }

  void setStaticLightModeColor(CRGB color) {
    _static_light_mode_color = color;
    saveSettingInt(Setting::STATIC_LIGHT_MODE_COLOR_RED, color.red);
    saveSettingInt(Setting::STATIC_LIGHT_MODE_COLOR_GREEN, color.green);
    saveSettingInt(Setting::STATIC_LIGHT_MODE_COLOR_BLUE, color.blue);
  }

  void requestNextAnimation() { _next_animation_requested = true; }

  void setContinuousMode(bool enabled) {
    _continuous_mode = enabled;
    saveSettingBool(Setting::CONTINUOUS_MODE, enabled);
  }

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
    while (animationsEnabled() && lightOn()) {
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
      if (!(animationsEnabled() && lightOn())) {
        publishAnimation(nullptr);
      }
      while (!(animationsEnabled() && lightOn())) {
        if (lightOn()) {
          fill_solid(leds, NUM_LEDS, staticLightModeColor());
        } else {
          allBlack();
        }
        FastLED.show();
        delayFrame();
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
  bool _continuous_mode;
  bool _motor_enabled;
  uint8_t _motor_max_speed = Config::MAX_SPEED_UPPER_LIMIT;
  bool _animations_enabled = false;
  bool _light_on = false;
  CRGB _static_light_mode_color = CRGB::White;
  bool _inner_light_on = false;
  uint8_t _inner_light_brightness = 0;
  bool _motor_should_stop_for_passengers = false;
  unsigned long _last_magnet_detection_timestamp = 0;
  bool _magnet_detected = false;
  uint8_t _passed_cabins = 0;

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

