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
#ifdef MOTOR_AVAILABLE
    xTaskCreatePinnedToCore(&motorLoop, "Motorloop", 2000, this, 1, nullptr, 1);
#endif // MOTOR_AVAILABLE
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

#ifdef MOTOR_AVAILABLE
  void innerMotorLoop() {
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    constexpr uint8_t MOTOR_CHANNEL = 1;
    ledcSetup(MOTOR_CHANNEL, 20000, 8);
    ledcWrite(MOTOR_CHANNEL, 0);
    ledcAttachPin(Config::MOTOR_PIN, MOTOR_CHANNEL);

    constexpr uint8_t speed_step = (Config::MAX_SPEED - Config::MIN_SPEED) / calculateSteps(Config::ACCELERATION_SECONDS * 1000);
    constexpr uint16_t running_steps = calculateSteps(Config::STOP_EVERY_N_SECONDS * 1000);
    constexpr uint16_t stopped_steps = calculateSteps(Config::STOP_FOR_N_SECONDS * 1000);
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
        if (speed > Config::MIN_SPEED + speed_step) {
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
        if (speed < Config::MAX_SPEED - speed_step) {
          speed += speed_step;
        } else {
          speed = Config::MAX_SPEED;
          state = MotorState::Running;
          remainingSteps = running_steps;
        }
        Serial.println("RampUp");
        updateSpeed = true;
        break;
      }
      if (updateSpeed) {
        Serial.printf("New speed 0x%02x\n", speed);
        ledcWrite(MOTOR_CHANNEL, speed);
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
#endif // MOTOR_AVAILABLE
protected:
  virtual void delayFrame() override {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
private:
  static constexpr const char* NVS_KEY_ANIMATIONS = "animations";

  HAMqtt* _mqtt;

#ifdef MOTOR_AVAILABLE
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

  static void motorLoop(void* parameters) {
    static_cast<ESP32Controller<DATA_PIN>*>(parameters)->innerMotorLoop();
  }
#endif // MOTOR_AVAILABLE

  static void taskLoop(void* parameters) {
    static_cast<ESP32Controller<DATA_PIN>*>(parameters)->outsideLoop();
  }
};

};

