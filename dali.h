#ifndef DALI_H
#define DALI_H

#include <stdint.h>

void dali_init();
void dali_send_cmd2(uint8_t addr, uint8_t cmd);

#endif
