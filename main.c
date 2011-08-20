#include "dali.h"
#include <util/delay.h>
#include "usbdrv.h"

int main() {
  dali_init();

  usbDeviceDisconnect();
  _delay_ms(100);
  usbInit();

  while (1) {
    usbPoll();
    dali_send_cmd2(254, 120);
    _delay_ms(300);
  }
}

usbMsgLen_t usbFunctionSetup(uchar setupData[8]) {
  usbRequest_t *rq = (void *) setupData;

  switch (rq->bRequest) {
    case 2: {
      dali_send_cmd2(rq->wValue.bytes[0], rq->wValue.bytes[1]);
      break;
    }
  }

  return 0;
}
