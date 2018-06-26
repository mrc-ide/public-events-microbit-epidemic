#include "MicroBit.h"
#include "microepi.h"
#include "serialno.h"

MicroBit uBit;
ManagedString NEWLINE("\r\n");
ManagedString REG("REG");
ManagedString COLON(":");
ManagedString VERSION_INFO("VER:Epi Master 1.0:");
ManagedString PARAM_ACK("PAR:");

unsigned short epi_id;
float param_R0;
unsigned char param_Rtype;
unsigned short param_exposure;
unsigned char param_rpower;

unsigned char current_stage;
bool busy = false;
int serial_no;

void sendSerial(ManagedString s) {
  int res = uBit.serial.send(s, SYNC_SPINWAIT);
  while (res == MICROBIT_SERIAL_IN_USE) {
    uBit.sleep(10);
    res = uBit.serial.send(s);
  }
}

void onData(MicroBitEvent) {
  if (!busy) {
    busy=true;
    PacketBuffer msg = uBit.radio.datagram.recv();
    uint8_t *mbuf = msg.getBytes();

    // If we're in the "recruit minions stage"...
    if (current_stage == MASTER_STAGE_RECRUITMENT) {

      // We receive a "Register Me" sort of message. Format:
      // MSG_ID [char]   Serial_No [int]

      if (msg[MSG_TYPE] == REG_MESSAGE) {

        // Fetch serial number, and look-up a more friendly version.
        int incoming_serial;
        memcpy(&incoming_serial, &mbuf[REG_SERIAL], SIZE_INT);
        short friendly_id = get_id_from_serial(incoming_serial);
        ManagedString m_id(incoming_serial);
        ManagedString f_id(friendly_id);

        // Report to serial port
        sendSerial(REG + COLON + m_id + COLON + f_id + NEWLINE);
  
        // Reply to minion by broadcasting friendly id and params:-
        // MSG_ID [char]  Minion_Serial_No [int]   Minion_Friendly_Id [Short] 
        // Master_Serial_No [int]   Master_Time [long]
        // Epidemic_ID [short]   R0 [float]  Rtype [Char]  Rpower [Char]
        // Exposure [Short]

        unsigned long the_time = uBit.systemTime();

        PacketBuffer reply(REG_ACK_SIZE);
        uint8_t *rbuf = reply.getBytes();
        reply[MSG_TYPE] = REG_ACK_MESSAGE;
        memcpy(&rbuf[REG_ACK_MINION_SERIAL], &mbuf[REG_SERIAL], SIZE_INT);
        memcpy(&rbuf[REG_ACK_ID], &friendly_id, SIZE_SHORT);
        memcpy(&rbuf[REG_ACK_MASTER_SERIAL], &serial_no, SIZE_INT);
        memcpy(&rbuf[REG_ACK_MASTER_TIME], &the_time, SIZE_LONG);
        memcpy(&rbuf[REG_ACK_EPID], &epi_id, SIZE_SHORT);
        memcpy(&rbuf[REG_ACK_R0], &param_R0, SIZE_FLOAT);
        memcpy(&rbuf[REG_ACK_RTYPE], &param_Rtype, SIZE_CHAR);
        memcpy(&rbuf[REG_ACK_RPOWER], &param_rpower, SIZE_SHORT);
        memcpy(&rbuf[REG_ACK_EXPOSURE], &param_exposure, SIZE_SHORT);
        uBit.radio.datagram.send(reply);
      }
    }
    busy = false;
  }
}

void receiveSerial(MicroBitEvent) {
  ManagedString msg = uBit.serial.readUntil(NEWLINE);
  uBit.serial.clearRxBuffer();

  if (current_stage == MASTER_STAGE_WAIT_PARAMS) {
  
    if (msg.charAt(0) == VER_MESSAGE) {

      ManagedString SERIAL_NO(serial_no);
      sendSerial(VERSION_INFO+SERIAL_NO);

    } else if (msg.charAt(0) == PARAM_MESSAGE) {
      int start = 1;
      int param_no = 0;
      for (int i = 1; i<msg.length(); i++) {
        if (msg.charAt(i)=='\t') {
          ManagedString bit = msg.substring(start, i-start);
          const char* bitp = bit.toCharArray();
          if (param_no==0)      epi_id = (unsigned short) atoi(bitp);
          else if (param_no==1) param_R0 = (float) atof(bitp);
          else if (param_no==2) param_Rtype = (unsigned char)atoi(bitp);
          else if (param_no==3) param_rpower = (unsigned char) atoi(bitp);
          else if (param_no==4) param_exposure = (unsigned short) atoi(bitp);
        
          start = i+1;
          param_no++;
        }
      }
      current_stage = MASTER_STAGE_RECRUITMENT;
    }
  } else if (current_stage == MASTER_STAGE_RECRUITMENT) {
    if(msg.charAt(0) == SEED_MESSAGE_MASTER) {
      int start = 1;
      int param_no = 0;
      short seed_id = -1;
      char seed_contacts = -1;
      for (int i = 1; i<msg.length(); i++) {
        if (msg.charAt(i)=='\t') {
          ManagedString bit = msg.substring(start, i-start);
          const char* bitp = bit.toCharArray();
          if (param_no==0) seed_id = (short) atoi(bitp);
          else if (param_no==1) seed_contacts = (char) atoi(bitp);
          start = i+1;
          param_no++;
        }
      }
      PacketBuffer buf(SEED_MSG_SIZE);
      uint8_t *ibuf = buf.getBytes();
      ibuf[MSG_TYPE] = SEED_MSG_MINION;
      memcpy(&ibuf[SEED_MASTER_SERIAL], &serial_no, SIZE_INT);
      memcpy(&ibuf[SEED_EPI_ID], &epi_id, SIZE_SHORT);
      memcpy(&ibuf[SEED_VICTIM_ID], &seed_id, SIZE_SHORT);
      memcpy(&ibuf[SEED_N_CONS], &seed_contacts, SIZE_CHAR);
      uBit.radio.datagram.send(buf);
    }
  }
  uBit.serial.eventOn(NEWLINE);
}

int main() {
  serial_no = microbit_serial_number();
  current_stage = MASTER_STAGE_WAIT_PARAMS;
  uBit.init();
  uBit.radio.setGroup(UNREGISTERED_GROUP);
  uBit.display.setBrightness(64);
  uBit.serial.setRxBufferSize(32);
  uBit.serial.baud(115200);
  uBit.messageBus.listen(MICROBIT_ID_SERIAL,  MICROBIT_SERIAL_EVT_DELIM_MATCH, receiveSerial);
  uBit.serial.eventOn(NEWLINE);

  // Wait for parameters through serial port, which will move our
  // stage from MASTER_STAGE_WAIT_PARAMS to MASTER_STAGE_RECRUITMENT

  uBit.display.print("W");
  while (current_stage == MASTER_STAGE_WAIT_PARAMS) uBit.sleep(1000);

  // Now we're in MASTER_STAGE_RECRUITENT. Turn on the radio,
  // and start to answer requests from minions to play. Wait for
  // serial-port seeding information, which moves us to
  // MASTER_STAGE_EPIDEMIC mode.

  uBit.display.print("R");
  uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
  uBit.radio.enable();
  uBit.radio.setTransmitPower(7);
  while (current_stage == MASTER_STAGE_RECRUITMENT) uBit.sleep(1000);

  // Now we're in MASTER_STAGE_EPIDEMIC. The radio listens for infection
  // news reports, and forwards them to the serial port for processing.
  // This stage ends when we receive a kill signal from the serial port

  uBit.display.print("E");
  while (current_stage == MASTER_STAGE_RECRUITMENT) uBit.sleep(1000);

  // Now we're in MASTER_STAGE_DONE. Turn off everything and await a
  // restart signal.

  uBit.display.print("Z");
  uBit.radio.disable();

  release_fiber();
}