#include <Arduino.h>
#include <FastLED.h>
#include <new.h>

#include "leds.hpp"
#include "sprinkle.hpp"

#include "animation.hpp"
#include "alternating.hpp"
#include "snake.hpp"
#include "island.hpp"
#include "move.hpp"
#include "stacks.hpp"
#include "rotation.hpp"

#include "animationbuffer.hpp"

volatile uint8_t frame = 0;

void animationLoop(Animation& animation) {
  if (animation.clearOnStart()) {
    allBlack();
  }
  while (true) {
    cli();
    bool update = frame > 0;
    if (update) {
      frame--;
    }
    sei();
    // FIXME: This should be overhauled, as this leads to code which changed
    //        something in frame(), only to determine that it has finished.
    //        When an animation made one step there it would be only visible for
    //        a very short duration of time.
    //        Checking finished() before, does not help very much, as it would
    //        just check it 10ms later on the next iteration. A better solution
    //        is probably to add something to FrameAnimation, so that finished()
    //        changes on the last tick before the next frame is calculated.
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

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(30);

  setupTimer();

  sei();

  while (true) {
    switch (random8(8))
    {
    case 0: RUN_ANIMATION_ARGS(AlternatingBlink)
    case 1: RUN_ANIMATION_ARGS(GlitterBlink)
    case 2: RUN_ANIMATION_ARGS(SnakeAnimation)
    case 3: RUN_ANIMATION_ARGS(IslandAnimation)
    case 4: RUN_ANIMATION_ARGS(MoveAnimation)
    case 5: RUN_ANIMATION_ARGS(SprinkleAnimation)
    case 6: RUN_ANIMATION_ARGS(FallingStacks)
    case 7: RotationAnimation::createRandom(animationBuffer);
    }
    animationLoop(*animationBuffer.get());
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

typedef void (*t_movefunc)();

constexpr t_movefunc movefuncs[] = {
  &fallingStacks,
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
