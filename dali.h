#ifndef DALI_H
#define DALI_H

#include <stdint.h>

void dali_init();
uint8_t dali_send_cmd2(uint8_t addr, uint8_t cmd);
uint8_t dali_recv();

#define DALI_STATUS_IDLE	  0
#define DALI_STATUS_SENDING	  2

#define DALI_STATUS_RECV_MASK	  1
#define DALI_STATUS_RECV_NONE	  1
#define DALI_STATUS_RECV_OK	  3
#define DALI_STATUS_RECV_ERR	  5

volatile uint8_t dali_status, dali_recv_data;

#endif
