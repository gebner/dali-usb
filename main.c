#include "dali.h"
#include <util/delay.h>
#include "usbdrv.h"
#include <avr/io.h>

int main() {
  dali_init();

  usbInit();
  usbDeviceDisconnect();
  _delay_ms(250);
  usbDeviceConnect();

  sei();

  while (1) {
    usbPoll();
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
