/*

Send my serial number over Bluetooth to the master.

*/

#include "MicroBit.h"

MicroBit uBit;
bool acknowledged = false;
const int serial_no = microbit_serial_number();


void onData(MicroBitEvent) {
  ManagedString s = uBit.radio.datagram.recv();
  acknowledged = (s == "ACK:"+serial_no);
}


int main() {
  // Init
  uBit.init();

  // Get my serial number
  ManagedString sn("REG:"+serial_no);

  // Broadcast over bluetooth, and listen for ACK.

  uBit.radio.enable();
  while (!acknowledged) {
    uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
    uBit.radio.datagram.send(sn);
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