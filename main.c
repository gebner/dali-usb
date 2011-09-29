#include "dali.h"
#include <util/delay.h>
#include "usbdrv.h"

int main() {
  dali_init();

  usbInit();
  usbDeviceDisconnect();
  _delay_ms(250);
  usbDeviceConnect();

  while (1) {
    usbPoll();
  }
}

__attribute__((always_inline))
static inline usbMsgLen_t min(usbMsgLen_t a, usbMsgLen_t b) {
  return a > b ? b : a;
}

static uchar buffer[2];

usbMsgLen_t usbFunctionSetup(uchar setupData[8]) {
  usbRequest_t *rq = (void *) setupData;

  usbMsgLen_t len = 0;
  usbMsgPtr = buffer;

  switch (rq->bRequest) {
    case 2:
      buffer[0] = dali_send_cmd2(rq->wValue.bytes[0], rq->wValue.bytes[1]);
      len = 1;
      break;
    case 3:
      buffer[0] = dali_status;
      buffer[1] = dali_recv_data;
      len = 2;
      break;
  }

  return min(len, rq->wLength.word);
}
