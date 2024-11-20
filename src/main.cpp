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

void onBrightnessCommand(uint8_t brightness, HALight* sender) {
  FastLED.setBrightness(brightness);
  sender->setBrightness(brightness); // report brightness back to the Home Assistant
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

  #ifdef ARDUINO_ARCH_ESP32
  // Unique ID must be set!
  byte mac[6] = {0};
  WiFi.macAddress(mac);
  device.setUniqueId(mac, sizeof(mac));

  // connect to wifi
  WiFi.setHostname("Ferriswheel");
  Serial.println("Start connection");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(500); // waiting for the connection
  }
  Serial.println();
  Serial.println("Connected to the network");

  // set device's details (optional)
  device.setName("Ferriswheel");
  device.setSoftwareVersion("1.0.0");

  // handle switch state
  animationsSwitch.onStateCommand(onStateCommand);
  animationsSwitch.onBrightnessCommand(onBrightnessCommand);
  animationsSwitch.setName("Animations");
  animationsSwitch.setState(false);

  mqtt.begin(BROKER_ADDR, MQTT_USER, MQTT_PASSWORD);

  controller.setMqtt(&mqtt);
  #else
  controller.setAnimationsEnabled(true);
  #endif

  controller.setupTimer();

  controller.run();
}

// The code should never reach this far
void loop() {}
