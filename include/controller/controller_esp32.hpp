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

  static constexpr uint8_t INNER_LIGHT_CHAN = 2;

  virtual void begin() override {
    NVS.begin();

    Controller<DATA_PIN>::begin();

    pinMode(Config::MAG_SENSOR_PIN, INPUT_PULLUP);

    constexpr uint8_t INNER_LIGHT_PIN = 32;
    ledcSetup(INNER_LIGHT_CHAN, 20000, 8);
    ledcWrite(INNER_LIGHT_CHAN, this->innerLightBrightness());
    ledcAttachPin(INNER_LIGHT_PIN, INNER_LIGHT_CHAN);
  }

  virtual bool isMagnetDetected() override {
    return digitalRead(Config::MAG_SENSOR_PIN) == LOW;
  }

  virtual void saveSettingInt(typename Controller<DATA_PIN>::Setting setting, int value) override {
    NVS.setInt(settingToString(setting), value);
  }

  virtual void saveSettingBool(typename Controller<DATA_PIN>::Setting setting, bool value) override {
    NVS.setInt(settingToString(setting), value ? 1 : 0);
  }

  virtual int loadSettingInt(typename Controller<DATA_PIN>::Setting setting) override {
    return NVS.getInt(settingToString(setting));
  }

  virtual bool loadSettingBool(typename Controller<DATA_PIN>::Setting setting) override {
    return NVS.getInt(settingToString(setting)) > 0;
  }

  virtual void run_loop() override {
    if (_mqtt) {
      _mqtt->loop();
    }
    if (this->innerLightOn()) {
      ledcWrite(INNER_LIGHT_CHAN, this->innerLightBrightness());
    } else {
      ledcWrite(INNER_LIGHT_CHAN, 0);
    }
    taskYIELD();
  }

  float get_speed_at_time(float start_speed_mm_per_sec, float target_deceleration_time_ms, float passed_time_ms) {
    return start_speed_mm_per_sec - (start_speed_mm_per_sec / target_deceleration_time_ms) * passed_time_ms;
  }

  virtual void initiateMotorStopForPassengers(float current_speed_mm_per_sec, float target_deceleration_time_ms) override {
    Serial.printf("Initiating motor stop for passengers in %.0f ms (current speed: %.2f mm/sec)\n", target_deceleration_time_ms, current_speed_mm_per_sec);

    _motor_speed_to_mm_per_sec_factor = current_speed_mm_per_sec / _current_speed;
    _motor_stop_for_passengers_start_time = millis();
    _motor_stop_for_passengers_start_speed = current_speed_mm_per_sec;
    _motor_stop_for_passengers_target_deceleration_time_ms = target_deceleration_time_ms;
  }

  void innerMotorLoop() {
    constexpr uint8_t MOTOR_CHANNEL = 1;
    ledcSetup(MOTOR_CHANNEL, 20000, 8);
    ledcWrite(MOTOR_CHANNEL, 0);
    ledcAttachPin(Config::MOTOR_PIN, MOTOR_CHANNEL);

    constexpr uint32_t stop_for_steps = (Config::STOP_FOR_N_SECONDS * 1000) / Config::STEP_TIME;
    uint32_t steps_counter = 0;
    uint8_t target_speed = this->motorEnabled() ? this->motorMaxSpeed() : 0;

    for (;;) {
      vTaskDelay(Config::STEP_TIME / portTICK_PERIOD_MS);

      if (this->motorEnabled()) {
        if (this->continuousMode()) {
          if (target_speed != this->motorMaxSpeed()) {
            target_speed = this->motorMaxSpeed();
          }
        } else {
          if (_current_speed == 0 && target_speed == 0) {
            if (++steps_counter >= stop_for_steps) {
              target_speed = this->motorMaxSpeed();
              steps_counter = 0;
            }
          }
        }
      } else {
        target_speed = 0;
      }

      if (this->motorEnabled() && target_speed > 0 && this->motorMaxSpeed() > target_speed) {
        target_speed = this->motorMaxSpeed();
      }

      if (_motor_stop_for_passengers_start_time > 0 && !this->continuousMode()) {
        const unsigned long time_passed = millis() - _motor_stop_for_passengers_start_time;
        _current_speed = get_speed_at_time(_motor_stop_for_passengers_start_speed, _motor_stop_for_passengers_target_deceleration_time_ms, time_passed);
      } else if (_current_speed < target_speed) {
        _current_speed = std::min<int16_t>(static_cast<int16_t>(_current_speed) + acceleration_per_step, target_speed);
      } else if (_current_speed > target_speed) {
        _current_speed = std::max<int16_t>(static_cast<int16_t>(_current_speed) - acceleration_per_step, target_speed);
      }

      Serial.printf("Current speed: %.2f mm/sec, target speed: %.2f mm/sec\n", _current_speed, target_speed);

      ledcWrite(MOTOR_CHANNEL, _current_speed);
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
  static constexpr const char* NVS_KEY_LIGHT_ON = "lights_on";
  static constexpr const char* NVS_KEY_STATIC_LIGHT_MODE_COLOR_RED = "static_light_red";
  static constexpr const char* NVS_KEY_STATIC_LIGHT_MODE_COLOR_GREEN = "static_light_green";
  static constexpr const char* NVS_KEY_STATIC_LIGHT_MODE_COLOR_BLUE = "static_light_blue";
  static constexpr const char* NVS_KEY_CONTINUOUS_MODE = "continuous_mode";
  static constexpr const char* NVS_KEY_INNER_LIGHT_ON = "inner_light_on";
  static constexpr uint8_t acceleration_per_step = (Config::MAX_SPEED_UPPER_LIMIT - Config::MIN_SPEED) / (2000 / Config::STEP_TIME);
  float _motor_speed_to_mm_per_sec_factor = 0;
  unsigned long _motor_stop_for_passengers_start_time = 0;
  float _motor_stop_for_passengers_start_speed = 0;
  float _motor_stop_for_passengers_target_deceleration_time_ms = 0;
  uint8_t _current_speed = 0;
  HAMqtt* _mqtt;

  static const char* settingToString(typename Controller<DATA_PIN>::Setting setting) {
    switch (setting) {
      case Controller<DATA_PIN>::Setting::ANIMATIONS_ENABLED: return NVS_KEY_ANIMATIONS;
      case Controller<DATA_PIN>::Setting::INNER_LIGHT_BRIGHTNESS: return NVS_KEY_INNER_LIGHT_BRIGHTNESS;
      case Controller<DATA_PIN>::Setting::MOTOR_ENABLED: return NVS_KEY_MOTOR_ENABLED;
      case Controller<DATA_PIN>::Setting::MOTOR_MAX_SPEED: return NVS_KEY_MOTOR_SPEED;
      case Controller<DATA_PIN>::Setting::LIGHT_ON: return NVS_KEY_LIGHT_ON;
      case Controller<DATA_PIN>::Setting::STATIC_LIGHT_MODE_COLOR_RED: return NVS_KEY_STATIC_LIGHT_MODE_COLOR_RED;
      case Controller<DATA_PIN>::Setting::STATIC_LIGHT_MODE_COLOR_GREEN: return NVS_KEY_STATIC_LIGHT_MODE_COLOR_GREEN;
      case Controller<DATA_PIN>::Setting::STATIC_LIGHT_MODE_COLOR_BLUE: return NVS_KEY_STATIC_LIGHT_MODE_COLOR_BLUE;
      case Controller<DATA_PIN>::Setting::CONTINUOUS_MODE: return NVS_KEY_CONTINUOUS_MODE;
      case Controller<DATA_PIN>::Setting::INNER_LIGHT_ON: return NVS_KEY_INNER_LIGHT_ON;
      default: throw std::runtime_error("Invalid setting");
    }
  }

  static void taskLoop(void* parameters) {
    static_cast<ESP32Controller<DATA_PIN>*>(parameters)->outsideLoop();
  }

  static void motorLoop(void* parameters) {
    static_cast<ESP32Controller<DATA_PIN>*>(parameters)->innerMotorLoop();
  }
};

};

