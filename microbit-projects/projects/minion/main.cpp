#include "MicroBit.h"
#include "microepi.h"

MicroBit uBit;
short my_id = -1;
unsigned char current_stage = MINION_STAGE_REGISTRY;
int serial_no = -1;

void onData(MicroBitEvent) {
  PacketBuffer msg = uBit.radio.datagram.recv();
  uint8_t *ibuf = msg.getBytes();
  if (current_stage == MINION_STAGE_REGISTRY) {
    if (msg[0] == REG_ACK_MESSAGE) {
      int incoming_serial = -1;
      memcpy(&incoming_serial, &msg[1], 4);
      if (incoming_serial == serial_no) {
        memcpy(&my_id, &ibuf[5], 2);
        current_stage = MINION_STAGE_WAITING;
        uBit.radio.setGroup(REGISTERED_GROUP);
        uBit.display.scroll(my_id);
        uBit.display.print('W');
      }
    }
  }
}

void minion_stage_registry() {
  PacketBuffer buf(5);
  uint8_t *ibuf = buf.getBytes();
  buf[0] = REG_MESSAGE;
  serial_no = microbit_serial_number();
  memcpy(&ibuf[1], &serial_no, 4);

  while (current_stage == MINION_STAGE_REGISTRY) {
    uBit.radio.datagram.send(buf);
    uBit.sleep(1000);
  }
}

int main() {
  uBit.init();
  uBit.radio.setGroup(UNREGISTERED_GROUP);
  uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
  uBit.radio.enable();
  
  minion_stage_registry();

  uBit.radio.disable();

  release_fiber();
}


