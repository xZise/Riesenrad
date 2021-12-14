#include <Arduino.h>
#include <FastLED.h>

constexpr uint8_t NUM_LEDS = 30;
constexpr uint8_t DATA_PIN = 8;

constexpr uint32_t availableColors[] = {CRGB::Red, CRGB::Yellow, CRGB::Green, CRGB::Ivory};
constexpr uint8_t availableColorsLength = sizeof(availableColors) / sizeof(uint32_t);

CRGB leds[NUM_LEDS];

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  leds[0] = CRGB(0xf0, 0, 0);
  leds[1] = CRGB(0, 0xf0, 0);
  leds[2] = CRGB(0, 0, 0xf0);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(30);
}

void allBlack() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
}

const CRGB getRandomColor() {
  return CRGB(availableColors[random8(availableColorsLength)]);
}

void move1() {
  CRGB color = getRandomColor();

  for (int i = 0; i < NUM_LEDS; i++) {
    for (int j = 0; j < NUM_LEDS - i; j++) {
      if (j > 0) {
        leds[j - 1] = CRGB::Black;
      }
      leds[j] = color;
      FastLED.show();
      delay(50);
    }
    delay(700);
  }

  uint8_t center_far = NUM_LEDS / 2;
  uint8_t center_near;
  if (NUM_LEDS % 2 == 1) {
    center_near = center_far;
  } else {
    center_near = center_far - 1;
  }
  for (int i = 0; i < NUM_LEDS / 2; i++) {
    if (center_near >= 0) {
      leds[center_near] = CRGB::Black;
      center_near--;
    }
    if (center_far < NUM_LEDS) {
      leds[center_far] = CRGB::Black;
      center_far++;
    }
    FastLED.show();
    delay(100);
  }
}

void move2() {
  CRGB color = getRandomColor();
  CRGB bgColor = getRandomColor();
  if (color == bgColor) {
    bgColor = CRGB::Black;
  }
  for (int iteration = 0; iteration < 10; iteration++) {
    for (int led = 0; led < NUM_LEDS; led++) {
      CRGB ledColor = ((led % 2) == (iteration % 2)) ? bgColor : color;
      leds[led] = ledColor;
    }
    FastLED.show();
    delay(500);
  }
}

void move3() {
  constexpr uint8_t trail_length = 4;

  const int start = random(NUM_LEDS);
  const bool reverse = random(2) == 0;

  for (uint8_t remaining_iterations = NUM_LEDS; remaining_iterations > 0; remaining_iterations--) {
    uint8_t actualTrail = min(trail_length, remaining_iterations - 1);
    for (int i = 0; i < actualTrail; i++) {
      int led = (remaining_iterations + start + i) % NUM_LEDS;
      if (reverse) {
        led = NUM_LEDS - led - 1;
      }
      if (i == 0) {
        leds[led] = CRGB::Red;
      } else {
        // leds[led] = CRGB::Wheat;
        CRGB color = CRGB::Wheat;
        // color = color.fadeToBlackBy(0xff - i - 10);
        leds[led] = color;
        // fadeToBlackBy(&leds[led], 1, 255);
      }
    }

    if (remaining_iterations < 4) {
      fadeToBlackBy(leds, NUM_LEDS, 255 / remaining_iterations);
    }
    FastLED.show();
    delay(100);
    allBlack();
  }
  delay(100);
}

void move4() {
  constexpr uint8_t glitterSpecs = NUM_LEDS / 5;
  allBlack();
  for (int i = 0; i < 20; i++) {
    uint8_t newSpecs = glitterSpecs;
    for (int led = 0; led < NUM_LEDS; led++) {
      if (leds[led] != CRGB(0, 0, 0)) {
        if (random8() < 150) {
          leds[led] = CRGB::Black;
        } else {
          newSpecs--;
        }
      }
    }
    while (newSpecs-- > 0) {
      leds[random8(NUM_LEDS)] = CRGB::White;
    }
    FastLED.show();
    delay(100);
  }
}

void loop() {
  allBlack();
  switch (random(4)) {
    case 0:
      move1();
      break;
    case 1:
      move2();
      break;
    case 2:
      move3();
      break;
    case 3:
      move4();
      break;
  }
  // FastLED.show();
  // delay(1000);

  // CRGB temp = leds[0];
  // for (int i = 0; i < NUM_LEDS - 1; i++) {
  //   leds[i] = leds[i + 1];
  // }
  // leds[NUM_LEDS - 1] = temp;
}