#include "MicroBit.h"
#include "microepi.h"

MicroBit uBit;

ManagedString VERSION_INFO("VER:Epi Minion 1.0:");
ManagedString NEWLINE("\r\n");

unsigned char current_stage = MINION_STAGE_REGISTRY;
int serial_no; // My internal serial number
short my_id;   // My friendly serial number

// Parameters for the current epidemic

unsigned short epi_id;
float param_R0;
unsigned char param_Rtype;
unsigned short param_exposure;
unsigned char param_rpower;

// My epidemic status


// Receive a radio message.

void onData(MicroBitEvent) {

  PacketBuffer msg = uBit.radio.datagram.recv();
  uint8_t* ibuf = msg.getBytes();
  
  // We're in the Registry stage.
  if (current_stage == MINION_STAGE_REGISTRY) {

    // Below: we receive a reply message. But it might not be to us...
    // MSG_ID [char]  Serial_No [int]   Friendly_Id [Short] 
    // Epidemic_ID [short]   R0 [float]  Rtype [Char]  Rpower [Char]
    // Exposure [Short]

    if (ibuf[MSG_TYPE_OFS] == REG_ACK_MESSAGE) {
      int incoming_serial;
      // If the incoming serial matches ours, then this is a reply for us.
      memcpy(&incoming_serial, &ibuf[REG_ACK_SERIAL_OFS], SIZE_INT);
      if ((incoming_serial == serial_no) && (serial_no >= 0)) {
        memcpy(&my_id, &ibuf[REG_ACK_ID_OFS], SIZE_SHORT);
        if (my_id>=0) {
          memcpy(&epi_id, &ibuf[REG_ACK_EPID], SIZE_SHORT);
          memcpy(&param_R0, &ibuf[REG_ACK_R0], SIZE_FLOAT);
          memcpy(&param_Rtype, &ibuf[REG_ACK_RTYPE], SIZE_CHAR);
          memcpy(&param_rpower, &ibuf[REG_ACK_RPOWER], SIZE_CHAR);
          memcpy(&param_exposure, &ibuf[REG_ACK_EXPOSURE], SIZE_SHORT);
          current_stage = MINION_STAGE_EPIDEMIC;
          uBit.radio.setGroup(REGISTERED_GROUP);
          uBit.display.print('S');
        }
      }
    }

  } else if (current_stage == MINION_STAGE_EPIDEMIC) {
    if (msg[MSG_TYPE_OFS] == REG_RESET_MESSAGE) {
      current_stage = MINION_STAGE_REGISTRY;
      uBit.radio.setGroup(UNREGISTERED_GROUP);
      uBit.radio.setTransmitPower(7);
      uBit.display.print('U');
    }
  }
}

void minion_stage_registry() {
  PacketBuffer buf(REG_MSG_SIZE);
  uint8_t *ibuf = buf.getBytes();
  buf[MSG_TYPE_OFS] = REG_MESSAGE;
  serial_no = microbit_serial_number();
  memcpy(&ibuf[REG_SERIAL_OFS], &serial_no, SIZE_INT);
  uBit.radio.setTransmitPower(7);

  while (current_stage == MINION_STAGE_REGISTRY) {
    uBit.radio.datagram.send(buf);
    uBit.sleep(1000);
  }
}

// Going to listen to serial port anyway, just in case a minion micro:bit
// gets plugged into the serial port by mistake. Would be helpfull if the
// manager tells us that's what we've done, and giving serial number for 
// this minion might also be helpful.

void sendSerial(ManagedString s) {
  int res = uBit.serial.send(s, SYNC_SPINWAIT);
  while (res == MICROBIT_SERIAL_IN_USE) {
    uBit.sleep(10);
    res = uBit.serial.send(s);
  }
}


void receiveSerial(MicroBitEvent) {
  ManagedString msg = uBit.serial.readUntil(NEWLINE);
  uBit.serial.clearRxBuffer();

  if (msg.charAt(0) == VER_MESSAGE) {
    ManagedString SERIAL_NO(serial_no);
    sendSerial(VERSION_INFO+SERIAL_NO);
  }
}


int main() {
  uBit.init();
  uBit.serial.setRxBufferSize(32);
  uBit.serial.baud(115200);
  uBit.messageBus.listen(MICROBIT_ID_SERIAL,  MICROBIT_SERIAL_EVT_DELIM_MATCH, receiveSerial);
  uBit.serial.eventOn(NEWLINE);

  uBit.display.print('U');
  uBit.radio.setGroup(UNREGISTERED_GROUP);
  uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
  uBit.radio.enable();
  
  while (true) {
    minion_stage_registry();
    while (current_stage != MINION_STAGE_REGISTRY) {
      uBit.sleep(1000);
    }
  }

  while (current_stage == MINION_STAGE_EPIDEMIC) {
    uBit.sleep(1000);
  }

  uBit.radio.disable();

  release_fiber();
}


