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
  void setMotorSpeed(uint8_t speed) { _motorSpeed = speed; }

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
    uint8_t speed = 0;
    uint16_t remainingSteps = 0;
    MotorState state = MotorState::Stopped;
    while(true) {
      if (!this->motorEnabled() && state != MotorState::Stopped && state != MotorState::RampDown) {
        state = MotorState::RampDown;
      }
      bool updateSpeed = false;
      switch (state)
      {
      case MotorState::RampDown:
        if (speed > min_speed + speed_step) {
          speed -= speed_step;
        } else {
          speed = 0;
          state = MotorState::Stopped;
          remainingSteps = stopped_steps;
        }
        Serial.println("RampDown");
        updateSpeed = true;
        break;
      case MotorState::RampUp:
        if (speed < _motorSpeed - speed_step) {
          speed += speed_step;
        } else {
          speed = _motorSpeed;
          state = MotorState::Running;
          remainingSteps = running_steps;
        }
        Serial.println("RampUp");
        updateSpeed = true;
        break;
      }
      if (updateSpeed) {
        Serial.printf("New speed 0x%02x\n", speed);
        ledcWrite(STRING_CHAN, speed);
      } else {
        if (remainingSteps == 0) {
          if (this->motorEnabled()) {
            if (state == MotorState::Stopped) {
              state = MotorState::RampUp;
            } else {
              state = MotorState::RampDown;
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

  enum class MotorState {
    Stopped,
    RampUp,
    Running,
    RampDown,
  };

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

