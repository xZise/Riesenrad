#pragma once

#include <ArduinoHA.h>
#include <ArduinoNvs.h>

#include <freertos/task.h>
#include "controller/controller.hpp"
#include "config.hpp"

namespace Ferriswheel
{

template<uint8_t DATA_PIN>
class ESP32Controller final : public Controller<DATA_PIN> {
public:
  void setMqtt(HAMqtt* mqtt) { _mqtt = mqtt; }

  virtual void setupTimer() override {
    xTaskCreatePinnedToCore(&taskLoop, "Animationloop", 2000, this, 1, nullptr, 1);
    xTaskCreatePinnedToCore(&motorLoop, "Motorloop", 2000, this, 1, nullptr, 1);
  }

  virtual bool motorEnabled() const override { return _motorEnabled; }
  virtual uint8_t motorMaxSpeed() const override { return _max_speed; }
  virtual bool innerLightOn() const override { return ledcRead(INNER_LIGHT_CHAN) > 0; }
  virtual uint8_t innerLightBrightness() const override { return _inner_light_brightness; }
  virtual bool animationsEnabled() const override { return NVS.getInt(NVS_KEY_ANIMATIONS, 0) > 0; }
  virtual bool staticLightModeLightsOn() const override { return _static_light_mode_lights_on; }
  virtual CRGB staticLightModeColor() const override { return _static_light_mode_color; }

  virtual void setAnimationsEnabled(bool enabled) override {
    _animationsEnabled = enabled;
    NVS.setInt(NVS_KEY_ANIMATIONS, enabled ? 1 : 0);
  }
  virtual void setStaticLightModeLightsOn(bool lights_on) override {
    _static_light_mode_lights_on = lights_on;
    NVS.setInt(NVS_KEY_STATIC_LIGHT_MODE_LIGHTS_ON, lights_on ? 1 : 0);
  }
  virtual void setStaticLightModeColor(CRGB color) override {
    _static_light_mode_color = color;
    NVS.setInt(NVS_KEY_STATIC_LIGHT_MODE_COLOR_RED, color.red);
    NVS.setInt(NVS_KEY_STATIC_LIGHT_MODE_COLOR_GREEN, color.green);
    NVS.setInt(NVS_KEY_STATIC_LIGHT_MODE_COLOR_BLUE, color.blue);
  }

  static constexpr uint8_t INNER_LIGHT_CHAN = 2;

  virtual void begin() override {
    Controller<DATA_PIN>::begin();

    NVS.begin();

    _static_light_mode_lights_on = NVS.getInt(NVS_KEY_STATIC_LIGHT_MODE_LIGHTS_ON, 0) > 0;
    _static_light_mode_color.red = NVS.getInt(NVS_KEY_STATIC_LIGHT_MODE_COLOR_RED, 0);
    _static_light_mode_color.green = NVS.getInt(NVS_KEY_STATIC_LIGHT_MODE_COLOR_GREEN, 0);
    _static_light_mode_color.blue = NVS.getInt(NVS_KEY_STATIC_LIGHT_MODE_COLOR_BLUE, 0);

    bool wasEnabled = NVS.getInt(NVS_KEY_ANIMATIONS, 0) > 0;
    setAnimationsEnabled(wasEnabled);

    _motorEnabled = NVS.getInt(NVS_KEY_MOTOR_ENABLED, 0) > 0;
    _max_speed = NVS.getInt(NVS_KEY_MOTOR_SPEED, Config::MAX_SPEED_UPPER_LIMIT);
    _inner_light_brightness = NVS.getInt(NVS_KEY_INNER_LIGHT_BRIGHTNESS, 125);

    constexpr uint8_t INNER_LIGHT_PIN = 32;
    ledcSetup(INNER_LIGHT_CHAN, 20000, 8);
    ledcWrite(INNER_LIGHT_CHAN, _inner_light_brightness);
    ledcAttachPin(INNER_LIGHT_PIN, INNER_LIGHT_CHAN);
  }

  virtual void setInnerLightOn(bool on) override {
    if (on) {
      ledcWrite(INNER_LIGHT_CHAN, _inner_light_brightness);
    } else {
      ledcWrite(INNER_LIGHT_CHAN, 0);
    }
  }

  virtual void setInnerLightBrightness(uint8_t brightness) override {
    NVS.setInt(NVS_KEY_INNER_LIGHT_BRIGHTNESS, brightness);
    _inner_light_brightness = brightness;
    ledcWrite(INNER_LIGHT_CHAN, brightness);
  }

  virtual void setMotorEnabled(bool enabled) override {
    _motorEnabled = enabled;
    NVS.setInt(NVS_KEY_MOTOR_ENABLED, enabled ? 1 : 0);
    if (!_motorEnabled) {
      _target_speed = 0;
    } else {
      _target_speed = _max_speed;
    }
  }

  virtual void setMotorMaxSpeed(uint8_t speed) override {
    _max_speed = speed;
    NVS.setInt(NVS_KEY_MOTOR_SPEED, speed);
    if (_motorEnabled) {
      _target_speed = _max_speed;
    }
  }

  virtual void run() override {
    while (true) {
      if (_mqtt) {
        _mqtt->loop();
      }
      taskYIELD();
    }
  }

  void innerMotorLoop() {
    constexpr uint8_t MOTOR_CHANNEL = 1;
    ledcSetup(MOTOR_CHANNEL, 20000, 8);
    ledcWrite(MOTOR_CHANNEL, 0);
    ledcAttachPin(Config::MOTOR_PIN, MOTOR_CHANNEL);

    constexpr uint32_t stop_every_steps = (Config::STOP_EVERY_N_SECONDS * 1000) / Config::STEP_TIME;
    constexpr uint32_t stop_for_steps = (Config::STOP_FOR_N_SECONDS * 1000) / Config::STEP_TIME;
    uint8_t current_speed = 0;
    uint32_t steps_counter = 0;
    uint8_t target_speed = _motorEnabled ? _max_speed : 0;

    for (;;) {
      vTaskDelay(Config::STEP_TIME / portTICK_PERIOD_MS);

      if (_motorEnabled) {
        if (this->continuousMode()) {
          if (_target_speed != _max_speed) {
            _target_speed = _max_speed;
          }
        } else {
          ++steps_counter;
          if (_target_speed == _max_speed) {
            if (steps_counter >= stop_every_steps) {
              _target_speed = 0;
              steps_counter = 0;
            }
          } else {
            if (steps_counter >= stop_for_steps) {
              _target_speed = _max_speed;
              steps_counter = 0;
            }
          }
        }
      }

      if (current_speed < _target_speed) {
        current_speed = std::min<int16_t>(static_cast<int16_t>(current_speed) + acceleration_per_step, _target_speed);
      } else if (current_speed > _target_speed) {
        current_speed = std::max<int16_t>(static_cast<int16_t>(current_speed) - acceleration_per_step, _target_speed);
      }

      ledcWrite(MOTOR_CHANNEL, current_speed);
    }
  }
protected:
  virtual void delayFrame() override {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
private:
  static constexpr const char* NVS_KEY_ANIMATIONS = "animations";
  static constexpr const char* NVS_KEY_INNER_LIGHT_BRIGHTNESS = "inner_light_brightness";
  static constexpr const char* NVS_KEY_MOTOR_ENABLED = "motor_enabled";
  static constexpr const char* NVS_KEY_MOTOR_SPEED = "motor_max_speed";
  static constexpr const char* NVS_KEY_STATIC_LIGHT_MODE_LIGHTS_ON = "static_light_mode_lights_on";
  static constexpr const char* NVS_KEY_STATIC_LIGHT_MODE_COLOR_RED = "static_light_mode_color_red";
  static constexpr const char* NVS_KEY_STATIC_LIGHT_MODE_COLOR_GREEN = "static_light_mode_color_green";
  static constexpr const char* NVS_KEY_STATIC_LIGHT_MODE_COLOR_BLUE = "static_light_mode_color_blue";
  static constexpr uint8_t acceleration_per_step = (Config::MAX_SPEED_UPPER_LIMIT - Config::MIN_SPEED) / (2000 / Config::STEP_TIME);

  HAMqtt* _mqtt;
  bool _motorEnabled;
  uint8_t _max_speed = Config::MAX_SPEED_UPPER_LIMIT;
  uint8_t _target_speed = _max_speed;
  bool _animationsEnabled;
  uint8_t _inner_light_brightness = 0;
  bool _static_light_mode_lights_on = false;
  CRGB _static_light_mode_color = CRGB::White;

  static void taskLoop(void* parameters) {
    static_cast<ESP32Controller<DATA_PIN>*>(parameters)->outsideLoop();
  }

  static void motorLoop(void* parameters) {
    static_cast<ESP32Controller<DATA_PIN>*>(parameters)->innerMotorLoop();
  }
};

};

