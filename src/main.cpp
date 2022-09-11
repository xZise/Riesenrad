#include <Arduino.h>
#include <FastLED.h>

#include "leds.hpp"
#include "sprinkle.hpp"
#include "bitset.cpp"

#include "animation.hpp"
#include "alternating.hpp"

bool randomBool() {
  return (random8() >> 7) == 0;
}

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

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(30);

  // Arduino micro: Timer 3
  OCR3A = 156;
  TCCR3B = (1 << WGM32) | (1 << CS30) | (1 << CS32);
  TIMSK3 = (1 << OCIE3A);

  sei();

  // TCCR0A = (1<<WGM01);    //Set the CTC mode
  // OCR0A = 156; //Value for ORC0A for 10ms

  // TIMSK0 |= (1<<OCIE0A);   //Set the interrupt request
  // sei(); //Enable interrupt

  // TCCR0B |= (1<<CS02);    //Set the prescale 1/1024 clock
  // TCCR0B |= (1<<CS00);

  while (true) {
    switch (random8(2))
    {
    case 1: {
        AlternatingBlink animation;
        animationLoop(animation);
      }
      break;
    case 2: {
        GlitterBlink animation;
        animationLoop(animation);
      }
      break;
    }
  }
}

ISR(TIMER3_COMPA_vect) {
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

void snake() {
  constexpr uint8_t startLength = 3;
  constexpr uint8_t maxApples = 4;
  constexpr uint8_t maxLength = NUM_LEDS / 4 * 3;
  const CRGB head = CRGB::DarkGreen;
  const CRGB oddBody = CRGB::Green;
  const CRGB evenBody = CRGB::Turquoise;

  Bitset<NUM_LEDS> apples;
  bool reverse = randomBool();

  uint8_t length = startLength;
  uint8_t position = random8(NUM_LEDS);
  while (length < maxLength) {
    uint8_t missingApples = maxApples - apples.count();
    if (missingApples > 0) {
      uint8_t remainingLengthUnfed = maxLength - length - apples.count();
      if (remainingLengthUnfed < missingApples) {
        missingApples = remainingLengthUnfed;
      }
    }
    while (missingApples-- > 0) {
      uint8_t newApple;
      do {
        newApple = random8(NUM_LEDS);
        if (newApple >= position && newApple <= position + length) {
          continue;
        }
      } while(apples[newApple]);
      apples.set(newApple);
    }

    // draw current state
    for (uint8_t led = 0; led < NUM_LEDS; led++) {
      CRGB color;
      if (led == position) {
        color = head;
      } else {
        uint8_t index = led - position;
        // if led < position -> led (or index) "would be" the number of leds higher
        if (led < position) {
          index += NUM_LEDS;
        }
        if (index <= length) {
          color = (index >> 1) % 2 == 0 ? evenBody : oddBody;
        } else if (apples[led]) {
          color = CRGB::Red;
        } else {
          color = CRGB::Black;
        }
      }
      const uint8_t actualIndex = reverse ? NUM_LEDS - led - 1 : led;
      leds[actualIndex] = color;
    }
    FastLED.show();
    if (apples.reset(position)) {
      length++;
    }
    if (position > 0) {
      position -= 1;
    } else {
      position = NUM_LEDS - 1;
    }
    delay(50);
  }

  while (length-- > 0) {
    *getLedOffset(position, length + 1, reverse) = CRGB::Black;
    delay(50);
    FastLED.show();
  }
}

void move3() {
  constexpr uint8_t trail_length = NUM_LEDS / 10 + 1;

  const uint8_t start = random8(NUM_LEDS);
  const bool reverse = randomBool();

  for (uint8_t remaining_iterations = NUM_LEDS; remaining_iterations > 0; remaining_iterations--) {
    uint8_t actualTrail = min(trail_length, remaining_iterations - 1);
    for (uint8_t i = 0; i < actualTrail; i++) {
      uint8_t index = getLedOffsetIndex(remaining_iterations, start);
      index = getLedOffsetIndex(index, i, reverse);
      if (i == 0) {
        leds[index] = CRGB::Red;
      } else {
        leds[index] = CRGB::Wheat;
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

void islandsAlternating() {
  // Should be separated evenly?
  constexpr uint8_t numIslands = 9;
  constexpr uint8_t islandWidth = NUM_LEDS / numIslands;
  CRGB color = getRandomColor();

  // 6: [0 1 2 3 4 5]
  //     6 4 2 1 3 5
  // 5: [0 1 2 3 4]
  //     4 2 1 3 5

  uint8_t islandIndex = islandWidth / 2;
  int8_t delta = -1;
  for (uint8_t width = 0; width < islandWidth; width++) {
    uint8_t offset = islandIndex;
    while (offset < NUM_LEDS) {
      leds[offset] = color;
      offset += islandWidth;
    }

    islandIndex += delta;
    if (delta < 0) {
      delta--;
    } else {
      delta++;
    }
    delta = -delta;
    FastLED.show();
    delay(400);
  }
}

typedef void (*t_movefunc)();

constexpr t_movefunc movefuncs[] = {
  &fallingStacks,
  &move3,
  &islandsAlternating,
  &sprinkle,
  &rotation,
  &snake,
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