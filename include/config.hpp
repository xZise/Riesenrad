#pragma once

#include <stdint.h>

namespace Config
{

static constexpr uint8_t MIN_SPEED = 0x60;
static constexpr uint8_t MAX_SPEED = 0xe0;
static constexpr uint8_t ACCELERATION_SECONDS = 2;
static constexpr uint8_t STOP_EVERY_N_SECONDS = 60;
static constexpr uint8_t STOP_FOR_N_SECONDS = 10;

static constexpr uint8_t MOTOR_PIN = 13;

}
