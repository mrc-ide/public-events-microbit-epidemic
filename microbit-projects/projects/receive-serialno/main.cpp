/* Recceive serial number over Bluetooth, and send to serial port. */

#include "MicroBit.h"

MicroBit uBit;


void onData(MicroBitEvent) {
  ManagedString msg = uBit.radio.datagram.recv();
  if (msg.substring(0,4)=="REG:") {
    ManagedString sn = msg.substring(4,msg.length()-4);
    ManagedString ack = "ACK:"+sn;
    uBit.radio.datagram.send(ack);
    sn = sn + "\n";
    uBit.serial.send(sn);
  }
}


int main() {
  uBit.init();
  uBit.radio.enable();
  uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
  while (true) {
    uBit.sleep(100);
  }
  uBit.radio.disable();
  release_fiber();
}
