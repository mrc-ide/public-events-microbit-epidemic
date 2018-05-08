/*

Send my serial number over Bluetooth to the master,
to be displayed via USB (Serial port).

*/

#include "MicroBit.h"

MicroBit uBit;

bool acknowledged = false;
const int serial_no = microbit_serial_number();
const ManagedString ack("ACK:");

void onData(MicroBitEvent) {
  ManagedString s = uBit.radio.datagram.recv();
  ManagedString cmp = ack+serial_no;
  acknowledged = (s == cmp);
  uBit.display.scroll(acknowledged);
}


int main() {
  
  // Init
  uBit.init();

  // Get my serial number
  ManagedString sn = ManagedString("REG:") + ManagedString(serial_no);
  // Broadcast over bluetooth, and listen for ACK.

  uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
  uBit.radio.enable();

  while (!acknowledged) {
    uBit.radio.datagram.send(sn);
    uBit.display.scroll("R");
    uBit.sleep(1000);
  }
  uBit.radio.disable();
  release_fiber();
}


/*
void loopUntilSent(ManagedString str) {
    int rtn = uBit.serial.send(str);
    while(rtn == MICROBIT_SERIAL_IN_USE) {
       uBit.sleep(0); // let other tasks run
       rtn = uBit.serial.send(str); 
    }
}
*/