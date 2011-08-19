#include "dali.h"
#include "config.h"
#include <avr/io.h>
#include <util/delay.h>

#define _delay_te(te) _delay_us((te) * 416.67)

void dali_init() {
  DALI_OUT_DDR |= _BV(DALI_OUT_PIN);
}

void dali_send_bit(uint8_t bit) {
  if (!bit) {
    DALI_OUT_PORT &= ~_BV(DALI_OUT_PIN);
    _delay_te(1);
    DALI_OUT_PORT |= _BV(DALI_OUT_PIN);
    _delay_te(1);
  } else {
    DALI_OUT_PORT |= _BV(DALI_OUT_PIN);
    _delay_te(1);
    DALI_OUT_PORT &= ~_BV(DALI_OUT_PIN);
    _delay_te(1);
  }
}

void dali_send_byte(uint8_t byte) {
  uint8_t i = 7;
  do {
    dali_send_bit((byte >> i) & 1);
  } while (i--);
}

void dali_send_start() {
  dali_send_bit(0);
}

void dali_send_end() {
  DALI_OUT_PORT |= _BV(DALI_OUT_PIN);
  _delay_te(4);
}

void dali_send_cmd2(uint8_t addr, uint8_t cmd) {
  dali_send_start();
  dali_send_byte(addr);
  dali_send_byte(cmd);
  dali_send_end();
}
