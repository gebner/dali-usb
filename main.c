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

usbMsgLen_t usbFunctionSetup(uchar setupData[8]) {
  usbRequest_t *rq = (void *) setupData;

  switch (rq->bRequest) {
    case 2: {
      cli();
      dali_send_cmd2(rq->wValue.bytes[0], rq->wValue.bytes[1]);
      sei();
      break;
    }
  }

  return 0;
}
