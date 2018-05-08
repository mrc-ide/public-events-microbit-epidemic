/* Recceive serial number over Bluetooth, and send to serial port. */

#include "MicroBit.h"
MicroBit uBit;

const ManagedString reg("REG:");
const ManagedString ack("ACK:");
const ManagedString nl("\r\n");

void onData(MicroBitEvent e) {
  ManagedString msg = uBit.radio.datagram.recv();
  if (msg.substring(0,4)==reg) {
    ManagedString sn = msg.substring(4,msg.length()-4);
    ManagedString reply = ack + ManagedString(sn);
    uBit.radio.datagram.send(reply);
    sn = sn + nl;
    uBit.serial.send(sn);
    
  }
}


int main() {
  uBit.init();
  uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
  uBit.radio.enable();
  uBit.serial.send("Send me REG:12345\n");
  while (true) {
    uBit.sleep(100);
  }
  uBit.radio.disable();
  release_fiber();
}
