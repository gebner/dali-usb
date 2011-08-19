#include "dali.h"
#include "config.h"
#include <util/delay.h>

int main() {
  dali_init();

  while (1) {
    dali_send_cmd2(254, 120);
    _delay_ms(300);
  }
}
