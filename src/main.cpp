#include <Arduino.h>
#include <FastLED.h>
#include <new>

#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <ArduinoHA.h>
#include <freertos/task.h>
#endif

#ifdef ARDUINO_AVR_MICRO
#include "controller/controller_micro.hpp"
#elif ARDUINO_AVR_UNO
#include "controller/controller_uno.hpp"
#elif ARDUINO_ARCH_ESP32
#include "secrets.hpp"
#include "controller/controller_esp32.hpp"
#else
#error "Board is not supported"
#endif

#include "leds.hpp"

#ifdef ARDUINO_AVR_MICRO
Ferriswheel::ArduinoMicroController<8> controller;
#elif ARDUINO_AVR_UNO
Ferriswheel::ArduinoUnoController<8> controller;
#elif ARDUINO_ARCH_ESP32
Ferriswheel::ESP32Controller<12> controller;

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);

HALight animationsSwitch("animations", HALight::BrightnessFeature);
HAButton nextAnimation("next");

HASwitch motorSwitch("motor");

#define X(field) \
  HASwitch enable##field##Switch("enable-" #field);

ENABLED_ANIMATIONS_LIST
#undef X

HASensor currentAnimation("current-animation");
#endif

#ifdef TIMER_VEC
ISR(TIMER_VEC) {
  controller.nextFrame();
}
#endif

#ifdef ARDUINO_ARCH_ESP32
void onStateCommand(bool state, HALight* sender)
{
  controller.setAnimationsEnabled(state);
  sender->setState(state);
}

void onSwitchCommand(bool state, HASwitch* sender)
{
  controller.setMotorEnabled(state);
  sender->setState(state);
}

void onAnimationStateCommand(bool state, HASwitch* sender)
{
#define X(field)                                       \
  if (sender == &enable##field##Switch) {              \
    controller.set##field##Enabled(state);             \
    sender->setState(controller.is##field##Enabled()); \
    return;                                            \
  }

ENABLED_ANIMATIONS_LIST
#undef X
  Serial.println("Unknown animation switch!");
}

void onBrightnessCommand(uint8_t brightness, HALight* sender) {
  brightness = map(brightness, 0, 0xff, 0, Config::MAX_BRIGHTNESS);
  FastLED.setBrightness(brightness);
  brightness = map(brightness, 0, Config::MAX_BRIGHTNESS, 0, 0xff);
  sender->setBrightness(brightness);
}

void onButtonCommand(HAButton* button) {
  if (button == &nextAnimation) {
    controller.requestNextAnimation();
  }
}

void publishAnimation(const Animation* animation) {
  if (animation == nullptr) {
    currentAnimation.setValue("Stopped");
  } else {
    const char* name = animation->name();
    currentAnimation.setValue(name);
  }
}
#endif

void setup() {
  #ifndef ARDUINO_ARCH_ESP32
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  #endif

  Serial.begin(57600);

  FastLED.addLeds<WS2812B, controller.rgbLEDpin(), GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(30);

  controller.begin();

  #ifdef ARDUINO_ARCH_ESP32
  // Unique ID must be set!
  byte mac[6] = {0};
  WiFi.macAddress(mac);
  device.setUniqueId(mac, sizeof(mac));
  device.enableSharedAvailability();
  device.enableLastWill();
  if (Config::USE_EXTENDED_UNIQUE_IDS) {
    device.enableExtendedUniqueIds();
  }

  // connect to wifi
  WiFi.setHostname(Config::NAME);
  Serial.println("Start connection");
  WiFi.mode(WIFI_STA);
  WiFi.begin(Config::Secrets::WIFI_SSID, Config::Secrets::WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(500); // waiting for the connection
  }
  Serial.println();
  Serial.println("Connected to the network");

  // set device's details (optional)
  device.setName(Config::NAME);
  device.setSoftwareVersion("1.2.0");

  // handle switch state
  animationsSwitch.onStateCommand(onStateCommand);
  animationsSwitch.onBrightnessCommand(onBrightnessCommand);
  animationsSwitch.setName("Animations");

  motorSwitch.onCommand(onSwitchCommand);
  motorSwitch.setName("Motor");

#define X(field) \
  enable##field##Switch.setName(field::NAME); \
  enable##field##Switch.onCommand(onAnimationStateCommand);

ENABLED_ANIMATIONS_LIST
#undef X

  nextAnimation.onCommand(onButtonCommand);
  nextAnimation.setName("Skip Animation");
  nextAnimation.setIcon("mdi:skip-next");

  currentAnimation.setName("Current");
  currentAnimation.setIcon("mdi:animation");

  mqtt.begin(Config::Secrets::BROKER_ADDR, Config::Secrets::MQTT_USER, Config::Secrets::MQTT_PASSWORD);

  controller.setMqtt(&mqtt);

  mqtt.loop();

  animationsSwitch.setState(controller.animationsEnabled());

#define X(field)                                                   \
  enable##field##Switch.setState(controller.is##field##Enabled()); \

ENABLED_ANIMATIONS_LIST
#undef X

  publishAnimation(nullptr);
  #else
  controller.setMotorEnabled(true);
  controller.setAnimationsEnabled(true);
  #endif

  controller.setupTimer();

  controller.run();
}

// The code should never reach this far
void loop() {}
