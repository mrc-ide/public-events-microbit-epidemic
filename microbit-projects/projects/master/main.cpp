#include "MicroBit.h"
#include "microepi.h"
#include "serialno.h"

MicroBit uBit;

unsigned char current_stage = MASTER_STAGE_RECRUITMENT;
bool busy = false;

void onData(MicroBitEvent) {
  if (!busy) {
    busy=true;
    uBit.serial.send("Got data \r\n");
    PacketBuffer msg = uBit.radio.datagram.recv();
    uint8_t *ibuf = msg.getBytes();

    if (current_stage == MASTER_STAGE_RECRUITMENT) {
      if (msg[0] == REG_MESSAGE) {
        uBit.serial.send("REG_MESSAGE \r\n");
        int incoming_serial = -1;
        memcpy(&incoming_serial, &ibuf[1], 4);
        uBit.serial.send("ID %d \r\n",incoming_serial);
        short my_id = get_id_from_serial(incoming_serial);

        PacketBuffer reply(7);
        uint8_t *rbuf = reply.getBytes();
        reply[0] = REG_ACK_MESSAGE;
        memcpy(&rbuf[1], &msg[1], 4);
        memcpy(&rbuf[5], &my_id, 2);
        for (int i=1; i<=4; i++) reply[i] = msg[i];
        for (int i=5; i<=6; i++) reply[i] = ((unsigned char*)(&my_id))[i-5]; 
        uBit.radio.datagram.send(reply);
      }
    }
    busy = false;
  }
}

int main() {
  uBit.init();

/////////////////////////////
// MASTER_STAGE_RECRUITMENT 
//
// Entry: default at initialisation
// Display: "R"
// During: respond to incoming registry requests:-
//         (1) Translate serial into nicer ID.
//         (2) Reply with ACK + serial + ID
//         (3) Include that micro:bit in current game
// Exit: On A+B buttons pushed.
// Next Stage: 

  uBit.serial.send("Hello \r\n");
  uBit.radio.setGroup(UNREGISTERED_GROUP);
  uBit.display.setBrightness(64);
  uBit.display.print("R");
  uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
  uBit.radio.enable();
  while (current_stage == MASTER_STAGE_RECRUITMENT) {
    uBit.sleep(1000);
  }






  release_fiber();
}