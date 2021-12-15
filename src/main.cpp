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

const uint8_t getLedIndex(int8_t index) {
  if (index < 0) {
    while (index < 0) {
      index += NUM_LEDS;
    }
    return index;
  } else {
    return index % NUM_LEDS;
  }
}

CRGB* getLed(int8_t index) {
  uint8_t absoluteIndex = getLedIndex(index);
  return &leds[absoluteIndex];
}

void move1() {
  CRGB color = getRandomColor();
  uint8_t offset = random8(NUM_LEDS);

  for (int i = 0; i < NUM_LEDS; i++) {
    for (int j = 0; j < NUM_LEDS - i; j++) {
      uint8_t led = (j + offset) % NUM_LEDS;
      if (j > 0) {
        leds[(led == 0 ? NUM_LEDS : led) - 1] = CRGB::Black;
      }
      leds[led] = color;
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
  while (center_near < 0xff) {
    *getLed(center_near + offset) = CRGB::Black;
    center_near--;
    if (center_far < NUM_LEDS) {
      *getLed(center_far + offset) = CRGB::Black;
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
      int8_t index = remaining_iterations + start + i;
      if (reverse) {
        index = NUM_LEDS - index;
      }
      CRGB* led = getLed(index);
      if (i == 0) {
        *led = CRGB::Red;
      } else {
        *led = CRGB::Wheat;
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

typedef void (*t_movefunc)();

constexpr t_movefunc movefuncs[] = {&move1, &move2, &move3, &move4};
constexpr uint8_t numMoveFuncs = sizeof(movefuncs) / sizeof(t_movefunc);

void move(uint8_t mode) {
  allBlack();
  if (mode < numMoveFuncs) {
    (*movefuncs[mode])();
  }
}

void loop() {
  allBlack();
  move(random8(numMoveFuncs));
}