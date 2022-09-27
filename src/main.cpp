#include <Arduino.h>
#include <FastLED.h>

#include "leds.hpp"
#include "sprinkle.hpp"

#include "animation.hpp"
#include "alternating.hpp"
#include "snake.hpp"
#include "island.hpp"
#include "move.hpp"

volatile uint8_t frame = 0;

void animationLoop(Animation& animation) {
  allBlack();
  while (true) {
    cli();
    bool update = frame > 0;
    if (update) {
      frame--;
    }
    sei();
    if (update) {
      animation.frame();
      if (animation.finished()) {
        return;
      }
    }
  }
}

void setupTimer() {
  #ifdef ARDUINO_AVR_MICRO
  // Arduino micro: Timer 3
  OCR3A = 156;
  TCCR3B = (1 << WGM32) | (1 << CS30) | (1 << CS32);
  TIMSK3 = (1 << OCIE3A);

  #define TIMER_VEC TIMER3_COMPA_vect
  #elif ARDUINO_AVR_UNO
  // Uses timer 2 as the prescaler > 64 with timer 0 did not work
  TCCR2A = (1<<WGM21);    //Set the CTC mode
  OCR2A = 156; //Value for ORC0A for 10ms

  TIMSK2 |= (1<<OCIE2A);   //Set the interrupt request

  TCCR2B |= (1 << CS22) | (1<<CS21) | (1 << CS20);    //Set the prescale 1/1024 clock

  #define TIMER_VEC TIMER2_COMPA_vect
  #else
  #error "Timer could not be set up for board"
  #endif
}

#define RUN_ANIMATION(animationType) { animationType animation; animationLoop(animation); } break;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(30);

  setupTimer();

  sei();

  while (true) {
    switch (random8(5))
    {
    case 0: RUN_ANIMATION(AlternatingBlink)
    case 1: RUN_ANIMATION(GlitterBlink)
    case 2: RUN_ANIMATION(SnakeAnimation)
    case 3: RUN_ANIMATION(IslandAnimation)
    case 4: RUN_ANIMATION(MoveAnimation)
    }
  }
}

ISR(TIMER_VEC) {
  frame++;
}

void fallingStacks() {
  constexpr uint8_t stackDivisor = 30;
  constexpr uint8_t stackLength = NUM_LEDS >= stackDivisor ? NUM_LEDS / stackDivisor : 1;
  constexpr uint8_t stackCount = stackLength < 4 ? 4 - stackLength : 1;
  // TODO: In theory this should be a clean divisor of NUM_LEDs, this is not guaranteed yet.
  constexpr uint8_t stackLedCount = stackLength * stackCount;
  CRGB colors[stackLedCount];
  for (uint8_t i = 0; i < stackLedCount; i++) {
    colors[i] = getRandomColor();
  }

  uint8_t offset = random8(NUM_LEDS);

  for (uint8_t i = 0; i < NUM_LEDS; i += stackLength) {
    for (uint8_t j = 0; j < NUM_LEDS - i; j += stackLength) {
      for (uint8_t k = 0; k < stackLength; k++) {
        uint8_t led = (j + offset + k) % NUM_LEDS;
        if (j > 0) {
          uint8_t oldLed = led;
          if (led < stackLength) {
            oldLed += NUM_LEDS;
          }
          oldLed -= stackLength;
          leds[oldLed] = CRGB::Black;
        }
        leds[led] = colors[(i + k) % stackLedCount];
      }
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
    *getLedOffset(center_near, offset) = CRGB::Black;
    center_near--;
    if (center_far < NUM_LEDS) {
      *getLedOffset(center_far, offset) = CRGB::Black;
      center_far++;
    }
    FastLED.show();
    delay(100);
  }
}

template<size_t numColors>
void rotation(const uint32_t (&sectionColors)[numColors],
              const uint8_t sectionMultiply,
              const bool isSolid) {
  constexpr uint8_t rotationCount = 5;

  const uint8_t numSections = sectionMultiply * numColors;
  const uint8_t colorWidth = NUM_LEDS / numSections;

  const uint8_t colorOffset = random8(numColors);

  uint8_t offset = 0;

  for (int8_t section = numSections - 1; section >= 0; section--) {
    uint8_t len = colorWidth;
    if (section == 0) {
      len = NUM_LEDS - offset;
    }
    CRGB color = sectionColors[(colorOffset + section) % numColors];
    if (isSolid) {
      fill_solid(&leds[offset], len, color);
    } else {
      CRGB sndColor = sectionColors[(colorOffset + section + 1) % numColors];
      fill_gradient_RGB(&leds[offset], len, sndColor, color);
    }
    offset += colorWidth;
  }

  uint8_t rotation = rotationCount;
  while (rotation-- > 0) {
    for (uint8_t step = 0; step < NUM_LEDS; step++) {
      FastLED.show();
      CRGB temp = rotation == 0 ? CRGB::Black : leds[0];
      memmove(leds, &leds[1], sizeof(CRGB) * (NUM_LEDS - 1));
      leds[NUM_LEDS - 1] = temp;
      delay(50);
    }
  }
}

void rotation(const uint8_t mode, const uint8_t sectionMultiply, const bool isSolid) {
  switch (mode)
  {
  case 0:
    {
      const uint32_t c[] = {CRGB::White, CRGB::Black};
      rotation(c, sectionMultiply, isSolid);
    }
    break;
  case 1:
    {
      const uint32_t c[] = {CRGB::SkyBlue, CRGB::RoyalBlue, CRGB::Blue};
      rotation(c, sectionMultiply, isSolid);
    }
    break;
  default:
    {
      rotation(availableColors, sectionMultiply, isSolid);
    }
    break;
  }
}

void rotation() {
  const uint8_t mode = random8(3);
  const uint8_t sectionMultiply = random(3) + 1;
  const bool isSolid = randomBool();
  rotation(mode, sectionMultiply, isSolid);
}


typedef void (*t_movefunc)();

constexpr t_movefunc movefuncs[] = {
  &fallingStacks,
  &sprinkle,
  &rotation,
};
constexpr uint8_t numMoveFuncs = array_size(movefuncs);

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