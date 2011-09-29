#include "dali.h"
#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define SHORT_PRESCALE 256
#define LONG_PRESCALE  1024

__attribute__((always_inline))
static inline void set_prescaler(uint16_t factor) {
  switch (factor) {
    case 1:    TCCR0B =                         _BV(CS00); break;
    case 8:    TCCR0B =             _BV(CS01);             break;
    case 64:   TCCR0B =             _BV(CS01) | _BV(CS00); break;
    case 256:  TCCR0B = _BV(CS02);                         break;
    case 1024: TCCR0B = _BV(CS02) |             _BV(CS00); break;
  }
}

__attribute__((always_inline))
static inline uint8_t te2cnt(double te, uint16_t factor) {
  double sec = te * 416.67e-6;
  double ticks = sec * F_CPU;
  return (uint8_t) (ticks / factor + 0.5);
}

#define STATE_OFF            0
#define STATE_BIT_PART1      1
#define STATE_BIT_PART2      2
#define STATE_STOPBITS       3
#define STATE_SETTLING       4
#define STATE_RECV_STARTBIT  5
#define STATE_RECV_BIT       6

static volatile uint8_t state = STATE_OFF;
static volatile uint8_t current_bit, te_ticks;
static volatile uint16_t data;
volatile uint8_t dali_status, dali_recv_data;

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
  DALI_IN_PORT |= _BV(DALI_IN_PIN);

  dali_set_level(1);

  GIMSK |= _BV(PCIF);
  PCMSK = 0;
  TCCR0B = _BV(CS02); // 256 prescaler
}

uint8_t dali_send_cmd2(uint8_t addr, uint8_t cmd) {
  if (state != STATE_OFF) return 1;

  data = (addr << 8) | (cmd << 0);
  current_bit = 16;
  dali_recv_data = 0;

  dali_set_level(0);

  TCNT0 = 0;
  set_prescaler(SHORT_PRESCALE);
  OCR0A = te2cnt(1, SHORT_PRESCALE);
  state = STATE_BIT_PART1;
  dali_status = DALI_STATUS_SENDING;
  TIFR |= _BV(OCF0A);
  TIMSK |= _BV(OCIE0A);

  return 0;
}

ISR(TIM0_COMPA_vect) {
  switch (state) {
    case STATE_BIT_PART2:
      TCNT0 = 0;
      if (current_bit) {
	current_bit--;
	dali_set_level(!(data & _BV(current_bit)));

	state = STATE_BIT_PART1;
      } else {
	dali_set_level(1);

	OCR0A = te2cnt(4, SHORT_PRESCALE);
	state = STATE_STOPBITS;
      }
      break;
    case STATE_BIT_PART1:
      dali_toggle_level();

      TCNT0 = 0;
      state = STATE_BIT_PART2;
      break;
    case STATE_STOPBITS:
      PCMSK = _BV(DALI_IN_PIN);
      set_prescaler(LONG_PRESCALE);
      OCR0A = te2cnt(22 * 1.33, LONG_PRESCALE);
      state = STATE_SETTLING;
      break;
    case STATE_SETTLING:
    case STATE_RECV_STARTBIT:
    case STATE_RECV_BIT:
      PCMSK = 0;
      TIMSK &= ~_BV(OCIE0A);
      dali_status = state == STATE_SETTLING ? DALI_STATUS_RECV_NONE : DALI_STATUS_RECV_ERR;
      state = STATE_OFF;
      break;
  }
}

#define dali_in_level_raw() (DALI_IN_PINR & _BV(DALI_IN_PIN))
#define dali_in_level() (DALI_IN_INV ? !dali_in_level_raw() : dali_in_level_raw())

__attribute__((always_inline))
static inline int abs(int a) {
  return a < 0 ? -a : a;
}

ISR(PCINT0_vect) {
  switch (state) {
    case STATE_SETTLING:
      set_prescaler(SHORT_PRESCALE);
      OCR0A = te2cnt(2, SHORT_PRESCALE);
      TCNT0 = 0;
      state = STATE_RECV_STARTBIT;
      break;
    case STATE_RECV_STARTBIT:
      te_ticks = TCNT0;
      OCR0A = 3*te_ticks;
      TCNT0 = 0;
      current_bit = 7;
      state = STATE_RECV_BIT;
      break;
    case STATE_RECV_BIT: {
      if (abs(2*te_ticks - TCNT0) <= te_ticks / 2) {
	if (dali_in_level()) {
	  dali_recv_data |= _BV(current_bit);
	}
	if (current_bit-- == 0) {
	  PCMSK = 0;
	  TIMSK &= ~_BV(OCIE0A);
	  dali_status = DALI_STATUS_RECV_OK;
	  state = STATE_OFF;
	} else {
	  te_ticks = TCNT0/2;
	  TCNT0 = 0;
	}
      }
      break;
    }
  }
}
