#pragma once

#include <stdint.h>

namespace Config
{

static constexpr uint8_t MAX_BRIGHTNESS = 0x30;
static constexpr bool USE_EXTENDED_UNIQUE_IDS = true;

static constexpr uint8_t MIN_SPEED = 0x60;
static constexpr uint8_t MAX_SPEED_UPPER_LIMIT = 0xe0;
static constexpr uint8_t ACCELERATION_SECONDS = 2;
static constexpr uint32_t STEP_TIME = 100;
static constexpr uint8_t STOP_FOR_N_SECONDS = 10;

static constexpr float WHEEL_DIAMETER_MM = 590;
static constexpr float WHEEL_NUMBER_OF_CABINS = 9;
static constexpr uint8_t STOP_EVERY_N_CABINS = 8;
static constexpr uint16_t MAG_SENSOR_DEBOUNCE_TIME_MS = 1000;

#ifdef ESP32
static constexpr uint8_t MOTOR_PIN = 13;
static constexpr uint8_t MAG_SENSOR_PIN = 26;
#endif

}
