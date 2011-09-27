#include "dali.h"
#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define PRESCALE 256

__attribute__((always_inline))
static inline uint8_t te2cnt(double te) {
  double sec = te * 416.67e-6;
  double ticks = sec * F_CPU;
  return (uint8_t) (ticks / PRESCALE + 0.5);
}

#define STATE_OFF       0
#define STATE_BIT_PART1 1
#define STATE_BIT_PART2 2
#define STATE_STOPBITS  3

static volatile uint8_t state = STATE_OFF;
static volatile uint8_t current_bit;
static volatile uint16_t data;

__attribute__((always_inline))
static void dali_set_level(uint8_t l) {
  if (DALI_OUT_INV ? l : !l) {
    DALI_OUT_PORT &= ~_BV(DALI_OUT_PIN);
  } else {
    DALI_OUT_PORT |= _BV(DALI_OUT_PIN);
  }
}

static void dali_toggle_level() {
  DALI_OUT_PINR = _BV(DALI_OUT_PIN);
}

void dali_init() {
  DALI_OUT_DDR |= _BV(DALI_OUT_PIN);

  dali_set_level(1);

  TCCR0B = _BV(CS02); // 256 prescaler
}

void dali_send_cmd2(uint8_t addr, uint8_t cmd) {
  if (state != STATE_OFF) return;

  data = (addr << 8) | (cmd << 0);
  current_bit = 16;

  dali_set_level(0);

  TCNT0 = 0;
  OCR0A = te2cnt(1);
  state = STATE_BIT_PART1;
  TIFR |= _BV(OCF0A);
  TIMSK |= _BV(OCIE0A);
}

ISR(TIM0_COMPA_vect) {
  TCNT0 = 0;
  switch (state) {
    case STATE_BIT_PART2:
      if (current_bit) {
	current_bit--;
	dali_set_level(!(data & _BV(current_bit)));

	state = STATE_BIT_PART1;
      } else {
	dali_set_level(1);

	OCR0A = te2cnt(4);
	state = STATE_STOPBITS;
      }
      break;
    case STATE_BIT_PART1:
      dali_toggle_level();

      state = STATE_BIT_PART2;
      break;
    case STATE_STOPBITS:
      TIMSK &= ~_BV(OCIE0A);
      state = STATE_OFF;
      break;
  }
}

#define dali_in_level_raw() (DALI_IN_PORT & _BV(DALI_IN_PIN))
#define dali_in_level() (DALI_IN_INV ? !dali_in_level_raw() : dali_in_level_raw())

uint8_t dali_recv() {
  uint8_t _100us_cnt = (uint8_t) ((F_CPU/256) * 100e-6);

  // start bit
  while (dali_in_level());
  TCNT0 = 0;
  while (!dali_in_level());
  uint8_t actual_te_cnt = TCNT0;

  // data byte
  uint8_t recv_byte = 0;
  uint8_t i = 7;
  uint8_t last_level = dali_in_level_raw();
  do {
    while (1) {
      if (last_level != dali_in_level_raw()) {
	last_level = dali_in_level_raw();
	if (_100us_cnt <= TCNT0 - actual_te_cnt && TCNT0 - actual_te_cnt <= _100us_cnt) {
	  // center edge detected
	  TCNT0 = 0;
	  if (dali_in_level()) {
	    recv_byte |= 1 << i;
	  }
	  break;
	}
      }
    }
  } while (i--);

  return recv_byte;
}
