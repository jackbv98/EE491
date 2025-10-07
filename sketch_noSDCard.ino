#include <Adafruit_NeoPixel.h>
#include "image_data.h"  

#define DATA_PIN_1 2
#define DATA_PIN_2 4
#define DATA_PIN_3 5
#define HALL_PIN   15

Adafruit_NeoPixel strip1(NUM_LEDS, DATA_PIN_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(NUM_LEDS, DATA_PIN_2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3(NUM_LEDS, DATA_PIN_3, NEO_GRB + NEO_KHZ800);

volatile uint32_t last_pulse_us = 0;
volatile uint32_t period_us = 1000000;
volatile bool pulse_flag = false;

uint16_t current_div = 0;
uint32_t slice_us = 1000000UL / DIVISIONS;
uint32_t next_deadline_us = 0;

void IRAM_ATTR hall_isr() {
  uint32_t now = micros();
  if (last_pulse_us != 0) {
    period_us = now - last_pulse_us;
  }
  last_pulse_us = now;
  pulse_flag = true;
}

void setup() {
  strip1.begin(); strip2.begin(); strip3.begin();

  strip1.show(); strip2.show(); strip3.show();
  pinMode(HALL_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(HALL_PIN), hall_isr, FALLING);
}

void loop() {
  if (pulse_flag) {
    pulse_flag = false;
    slice_us = period_us / DIVISIONS;
    current_div = 0;
    next_deadline_us = micros();
  }

  uint32_t now = micros();
  if ((int32_t)(now - next_deadline_us) >= 0) {
    display_division(current_div);
    current_div = (current_div + 1) % DIVISIONS;
    next_deadline_us += slice_us;
  }
}

void display_division(uint16_t div) {
  // Each division has STRIPS*NUM_LEDS*3 bytes
  uint32_t base_idx = div * STRIPS * NUM_LEDS * 3;
  for (int s = 0; s < STRIPS; s++) {
    for (int l = 0; l < NUM_LEDS; l++) {
      uint32_t idx = base_idx + (s * NUM_LEDS + l) * 3;
      uint8_t r = pgm_read_byte(&image_data[idx]);
      uint8_t g = pgm_read_byte(&image_data[idx+1]);
      uint8_t b = pgm_read_byte(&image_data[idx+2]);
      if (s == 0) strip1.setPixelColor(l, strip1.Color(r, g, b)); 
      if (s == 1) strip2.setPixelColor(l, strip2.Color(r, g, b));
      if (s == 2) strip3.setPixelColor(l, strip3.Color(r, g, b));
    }
  }
  strip1.show(); strip2.show(); strip3.show();
}
