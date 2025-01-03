#pragma once

#include <stdint.h>

// Enable to add motor specific code and settings
// #define MOTOR_AVAILABLE

namespace Config
{

static constexpr uint8_t MAX_BRIGHTNESS = 0x30;
static constexpr bool USE_EXTENDED_UNIQUE_IDS = true;

#ifdef MOTOR_AVAILABLE
static constexpr uint8_t MIN_SPEED = 0x60;
static constexpr uint8_t MAX_SPEED = 0xe0;
static constexpr uint8_t ACCELERATION_SECONDS = 2;
static constexpr uint8_t STOP_EVERY_N_SECONDS = 60;
static constexpr uint8_t STOP_FOR_N_SECONDS = 10;

static constexpr uint8_t MOTOR_PIN = 13;
#endif // MOTOR_AVAILABLE

}
