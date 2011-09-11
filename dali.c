#include "dali.h"
#include "config.h"
#include <avr/io.h>
#include <util/delay.h>

__attribute__((always_inline))
static inline void _delay_te(double te) {
  _delay_us(te * 416.67);
}

static void dali_set_level(uint8_t l) {
  if (DALI_OUT_INV ? l : !l) {
    DALI_OUT_PORT &= ~_BV(DALI_OUT_PIN);
  } else {
    DALI_OUT_PORT |= _BV(DALI_OUT_PIN);
  }
}

void dali_init() {
  DALI_OUT_DDR |= _BV(DALI_OUT_PIN);

  dali_set_level(1);

  TCCR0B |= _BV(CS02); // 256 prescaler
}

void dali_send_bit(uint8_t bit) {
  dali_set_level(!bit);
  _delay_te(1);
  dali_set_level(bit);
  _delay_te(1);
}

void dali_send_byte(uint8_t byte) {
  uint8_t i = 7;
  do {
    dali_send_bit((byte >> i) & 1);
  } while (i--);
}

void dali_send_start() {
  dali_send_bit(1);
}

void dali_send_end() {
  dali_set_level(1);
  _delay_te(4);
}

void dali_send_cmd2(uint8_t addr, uint8_t cmd) {
  dali_send_start();
  dali_send_byte(addr);
  dali_send_byte(cmd);
  dali_send_end();
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
