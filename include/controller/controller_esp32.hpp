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

  enum class MotorState {
    Stopped,
    RampUp,
    Running,
    RampDown,
  };

  void setMqtt(HAMqtt* mqtt) { _mqtt = mqtt; }
  void setMotorSpeed(uint8_t speed) {
    _motorSpeed = speed;
    if (_motorState == MotorState::Running) {
      if (speed > _currentSpeed) _motorState = MotorState::RampUp;
      else if (speed < _currentSpeed) _motorState = MotorState::RampDown;
    }
  }

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
      if (_mqtt) {
        _mqtt->loop();
      }
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
    uint16_t remainingSteps = 0;

    while(true) {
      if (!this->motorEnabled() && _motorState != MotorState::Stopped && _motorState != MotorState::RampDown) {
        _motorState = MotorState::RampDown;
      }
      bool updateSpeed = false;
      switch (_motorState)
      {
      case MotorState::RampDown:
        if (_currentSpeed > min_speed + speed_step) {
          _currentSpeed -= speed_step;
        } else {
          _currentSpeed = 0;
          _motorState = MotorState::Stopped;
          remainingSteps = stopped_steps;
        }
        Serial.println("RampDown");
        updateSpeed = true;
        break;
      case MotorState::RampUp:
        if (_currentSpeed < _motorSpeed - speed_step) {
          _currentSpeed += speed_step;
        } else {
          _currentSpeed = _motorSpeed;
          _motorState = MotorState::Running;
          remainingSteps = running_steps;
        }
        Serial.println("RampUp");
        updateSpeed = true;
        break;
      }
      if (updateSpeed) {
        Serial.printf("New speed 0x%02x\n", _currentSpeed);
        ledcWrite(STRING_CHAN, _currentSpeed);
      } else {
        if (remainingSteps == 0) {
          if (this->motorEnabled()) {
            if (_motorState == MotorState::Stopped) {
              _motorState = MotorState::RampUp;
            } else {
              _motorState = MotorState::RampDown;
            }
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
  uint8_t _motorSpeed = min_speed;
  uint8_t _currentSpeed = 0;
  MotorState _motorState = MotorState::Stopped;

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

