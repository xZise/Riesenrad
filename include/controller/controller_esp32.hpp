#pragma once

#include <ArduinoHA.h>
#include <ArduinoNvs.h>
#include <freertos/task.h>
#include "controller/controller.hpp"

namespace Ferriswheel
{

template<uint8_t DATA_PIN>
class ESP32Controller final : public Controller<DATA_PIN> {
public:
  static constexpr uint8_t max_speed = 0xe0;
  static constexpr uint8_t min_speed = 0x60;

  void setMqtt(HAMqtt* mqtt) { _mqtt = mqtt; }
  void setMotorSpeed(uint8_t speed) { _targetSpeed = speed; }

  virtual void setupTimer() override {
    xTaskCreatePinnedToCore(&taskLoop, "Animationloop", 2000, this, 1, nullptr, 1);
    xTaskCreatePinnedToCore(&motorLoop, "Motorloop", 2000, this, 1, nullptr, 1);
  }

  virtual void begin() override {
    NVS.begin();
    bool wasEnabled = NVS.getInt(NVS_KEY_ANIMATIONS) > 0;
    Controller<DATA_PIN>::setAnimationsEnabled(wasEnabled);
  }

  virtual void setAnimationsEnabled(bool enabled) override {
    NVS.setInt(NVS_KEY_ANIMATIONS, enabled ? 1 : 0);
    Controller<DATA_PIN>::setAnimationsEnabled(enabled);
  }

  virtual void run() override {
    while (true) {
      if (_mqtt) _mqtt->loop();
      taskYIELD();
    }
  }

  void innerMotorLoop() {
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    constexpr uint8_t STRING_PIN = 14;
    constexpr uint8_t STRING_CHAN = 1;
    ledcSetup(STRING_CHAN, 20000, 8);
    ledcWrite(STRING_CHAN, 0);
    ledcAttachPin(STRING_PIN, STRING_CHAN);

    constexpr uint8_t speed_step = (max_speed - min_speed) / calculateSteps(2000);
    constexpr uint16_t running_steps = calculateSteps(60000);
    constexpr uint16_t stopped_steps = calculateSteps(10000);
    uint16_t remainingSteps = stopped_steps;
    _targetSpeed = 0;

    while(true) {
      if (!this->motorEnabled()) {
        _targetSpeed = 0;
      }
      if (_currentSpeed < _targetSpeed) {
        if (_currentSpeed + speed_step <= _targetSpeed) {
          _currentSpeed += speed_step;
        } else {
          _currentSpeed = _targetSpeed;
        }
      } else if (_currentSpeed > _targetSpeed) {
        if (_currentSpeed > min_speed + speed_step && _currentSpeed - speed_step >= _targetSpeed) {
          _currentSpeed -= speed_step;
        } else {
          _currentSpeed = _targetSpeed;
        }
      }
      ledcWrite(STRING_CHAN, _currentSpeed);

      if (_currentSpeed == _targetSpeed) {
        if (remainingSteps == 0) {
          if (_targetSpeed == 0) {
            _targetSpeed = max_speed;
            remainingSteps = running_steps;
          } else {
            _targetSpeed = 0;
            remainingSteps = stopped_steps;
          }
        } else {
          remainingSteps -= 1;
        }
      }
      vTaskDelay(step_time / portTICK_PERIOD_MS);
    }
  }

protected:
  virtual void delayFrame() override {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

private:
  static constexpr const char* NVS_KEY_ANIMATIONS = "animations";
  HAMqtt* _mqtt;
  uint8_t _targetSpeed = 0;
  uint8_t _currentSpeed = 0;
  static constexpr uint32_t step_time = 100;
  static constexpr uint32_t calculateSteps(uint32_t milliseconds) {
    return milliseconds / step_time;
  }

  static void taskLoop(void* parameters) {
    static_cast<ESP32Controller<DATA_PIN>*>(parameters)->outsideLoop();
  }

  static void motorLoop(void* parameters) {
    static_cast<ESP32Controller<DATA_PIN>*>(parameters)->innerMotorLoop();
  }
};

};
