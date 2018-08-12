#include "MicroBit.h"
#include "microepi.h"

// #define DEBUG_SERIAL
#ifdef DEBUG_SERIAL
  ManagedString DEB("DEB:");
#endif

MicroBit uBit;
ManagedString END_SERIAL("#\n");
ManagedString NEWLINE("\n");
ManagedString REG("REG");
ManagedString COLON(":");
ManagedString VERSION_INFO("VER:Epi Master 1.13:");
ManagedString RESTART_INFO("VER:Push reset button and rescan:");
ManagedString INF_MSG("INF:");
ManagedString RECOV_MSG("REC:");

unsigned short epi_id;
float param_R0;
unsigned char param_Rtype;
unsigned char param_poimin;
unsigned char param_poimax;
unsigned short param_exposure;
unsigned char param_rpower;
unsigned char param_btrans;
unsigned char param_brec;
unsigned char param_icons;
uint64_t time0;

unsigned char current_stage;
bool busy = false;
int serial_no;

// 64-byte buffer for incoming serial data.
char* ser_buffer = new char[64];
unsigned char ser_length = 0;

void sendSerial(ManagedString s) {
  int res = uBit.serial.send(s, SYNC_SPINWAIT);
  while (res == MICROBIT_SERIAL_IN_USE) {
    uBit.sleep(10);
    res = uBit.serial.send(s);
  }
}


// We often have to check that the master serial number, and epidemic number
// match the epidemic that a minion was recruited for. To save some lines of
// code, and make it look pretty, here is a macro that does it. I think macro
// with a bit more code is better than a function call here...

#define CHECK_RIGHT_EPIDEMIC(__serial, __epi) \
  int check_master_serial; \
  memcpy(&check_master_serial, &ibuf[__serial], SIZE_INT); \
  if (check_master_serial == serial_no) { \
    unsigned short check_epid; \
    memcpy(&check_epid, &ibuf[__epi], SIZE_SHORT); \
    if (check_epid == epi_id) {

#define END_CHECK_RIGHT_EPIDEMIC }}

// Handle incoming radio message
void onData(MicroBitEvent) {
  if (!busy) {
    busy=true;
    PacketBuffer imsg = uBit.radio.datagram.recv();
    uint8_t *ibuf = imsg.getBytes();

    // If we're in the "recruit minions stage"...
    if (current_stage == MASTER_STAGE_RECRUITMENT) {

      // We receive a "Register Me" sort of message. Format:
      // MSG_ID [char]   Serial_No [int]

      if (imsg[MSG_TYPE] == REG_MSG) {

        // Fetch serial number, and look-up a more friendly version.
        int incoming_serial;
        unsigned char build_no;
        memcpy(&incoming_serial, &ibuf[REG_SERIAL], SIZE_INT);
        memcpy(&build_no, &ibuf[REG_BUILD], SIZE_CHAR);
        ManagedString m_id(incoming_serial);
        ManagedString b_id(build_no);

        // Report to serial port, and wait for serial reply with friendly id msg.
        sendSerial(REG + COLON + m_id + COLON + b_id + END_SERIAL);

      }

    } else if (current_stage == MASTER_STAGE_EPIDEMIC) {

      // Receive information about a minion becoming infected.
      // Message will be: master_serial (int), epi_id (short)
      // infetor id (short), victim id (short), time (int) and
      // number of contacts to be made (char)

      // These are all sent to serial port in the form:
      // INF:infector_id:victim_id:time:ncontacts#\n

      if (ibuf[MSG_TYPE] == REP_INF_MSG) {
        CHECK_RIGHT_EPIDEMIC(REP_INF_MASTER_SERIAL, REP_INF_EPI_ID)

        unsigned short infector_id;
          memcpy(&infector_id, &ibuf[REP_INF_INFECTOR_ID], SIZE_SHORT);
          unsigned short victim_id;
          memcpy(&victim_id, &ibuf[REP_INF_VICTIM_ID], SIZE_SHORT);
          int inf_time;
          memcpy(&inf_time, &ibuf[REP_INF_TIME], SIZE_INT);
          unsigned char ncons;
          memcpy(&ncons, &ibuf[REP_INF_NCONS], SIZE_CHAR);
          ManagedString INF(infector_id);
          ManagedString VID(victim_id);
          ManagedString TIME(inf_time);
          ManagedString NCON(ncons);
          sendSerial(INF_MSG + INF + COLON + VID + COLON + TIME + COLON + NCON + END_SERIAL);

          // Message resiliance - broadcast receipt to victim.

          PacketBuffer omsg(CONF_REP_INF_MSG_SIZE);
          uint8_t *obuf = omsg.getBytes();
          obuf[MSG_TYPE] = CONF_REP_INF_MSG;
          memcpy(&obuf[CONF_REP_INF_MASTER_SERIAL], &serial_no, SIZE_INT);
          memcpy(&obuf[CONF_REP_INF_EPI_ID], &epi_id, SIZE_SHORT);
          memcpy(&obuf[CONF_REP_INF_VICTIM_ID], &victim_id, SIZE_SHORT);
          uBit.radio.datagram.send(omsg);

        END_CHECK_RIGHT_EPIDEMIC

      // Here, master receives a message giving details of a recovery
      // (ie, success acquiring all 'n' contacts.

      // Msg format: master_serial (int), epi_id (short),
      // victim_id (short), time (int)

      // Sent to serial port in the form:
      // REC:victim_id:time#\n

      } else if (ibuf[MSG_TYPE] == REP_RECOV_MSG) {
        CHECK_RIGHT_EPIDEMIC(REP_RECOV_MASTER_SERIAL, REP_RECOV_EPI_ID)
          unsigned short victim_id;
          memcpy(&victim_id, &ibuf[REP_RECOV_VICTIM_ID], SIZE_SHORT);
          int inf_time;
          memcpy(&inf_time, &ibuf[REP_RECOV_TIME], SIZE_INT);
          ManagedString VID(victim_id);
          ManagedString TIME(inf_time);
          sendSerial(RECOV_MSG + VID + COLON + TIME + END_SERIAL);

          // Message resiliance - broadcast receipt to victim.

          PacketBuffer omsg(CONF_REP_RECOV_MSG_SIZE);
          uint8_t *obuf = omsg.getBytes();
          obuf[MSG_TYPE] = CONF_REP_RECOV_MSG;
          memcpy(&obuf[CONF_REP_RECOV_MASTER_SERIAL], &serial_no, SIZE_INT);
          memcpy(&obuf[CONF_REP_RECOV_EPI_ID], &epi_id, SIZE_SHORT);
          memcpy(&obuf[CONF_REP_RECOV_VICTIM_ID], &victim_id, SIZE_SHORT);
          uBit.radio.datagram.send(omsg);

        END_CHECK_RIGHT_EPIDEMIC
      }
    }
    busy = false;
  }
}

// Handle messages coming from the manager app through the serial port.

void receiveSerial(MicroBitEvent) {
  ManagedString msg = uBit.serial.readUntil(NEWLINE);

  // It seems we occasionally get a new-line mid-message from pySerial...
  // So instead, we manually end messages with '#', and paste them together
  // if necessary.

  // Append the msg into our buffer, ommitting new lines

  for (int i=0; i<msg.length(); i++) {
    if (msg.charAt(i)!='\n') {
      ser_buffer[ser_length] = msg.charAt(i);
      ser_length++;
    }
  }

  // And if it ends with #, then process the message.
  if (ser_buffer[ser_length-1] == '#') {
    #ifdef DEBUG_SERIAL
      sendSerial(DEB);
      for (int i=0; i<ser_length; i++) sendSerial(ser_buffer[i]);
      sendSerial(END_SERIAL);
    #endif

    // Message dealt with when we're waiting for the user to setup
    // parameters:
    if (current_stage == MASTER_STAGE_WAIT_PARAMS) {

      // Report our software/micro:bit vresion and serial number.

      if (ser_buffer[0] == SER_VER_MSG) {
        ManagedString SERIAL_NO(serial_no);
        ManagedString MB_VERSION(uBit.systemVersion());
        sendSerial(VERSION_INFO + SERIAL_NO + COLON + MB_VERSION + END_SERIAL);


      // Receive parameter values, which cause us to move into
      // recruitment stage, and handle all the minions' requests
      // to play the game.

      } else if (ser_buffer[0] == SER_PARAM_MSG) {

        int start = 1;
        int param_no = 0;
        for (int i = 1; i<ser_length; i++) {
          if (ser_buffer[i]==',') {
            const char* bitp = &ser_buffer[start];
            if (param_no==0)      epi_id = (unsigned short) atoi(bitp);
            else if (param_no==1) param_R0 = (float) atof(bitp);
            else if (param_no==2) param_Rtype = (unsigned char)atoi(bitp);
            else if (param_no==3) param_poimin = (unsigned char)atoi(bitp);
            else if (param_no==4) param_poimax = (unsigned char)atoi(bitp);
            else if (param_no==5) param_rpower = (unsigned char) atoi(bitp);
            else if (param_no==6) param_exposure = (unsigned short) atoi(bitp);
            else if (param_no==7) param_btrans = (unsigned char) atoi(bitp);
            else if (param_no==8) param_brec = (unsigned char) atoi(bitp);
            else if (param_no==9) param_icons = (unsigned char) atoi(bitp);
            start = i+1;
            param_no++;
          }
        }
        time0 = uBit.systemTime();
        current_stage = MASTER_STAGE_RECRUITMENT;
      }
    } else {

       // Here, we get a request for version information while we're busy with
       // an epidemic, so return a helpful error message.

      if (ser_buffer[0] == SER_VER_MSG) {
        ManagedString SERIAL_NO(serial_no);
        ManagedString MB_VERSION(uBit.systemVersion());
        sendSerial(RESTART_INFO + SERIAL_NO + COLON + MB_VERSION + END_SERIAL);

       // This is the reply message to registration, which gives the serial number
       // and friendly id pair for a minion that's just registered itself.

       } else if (ser_buffer[0] == SER_REG_MSG) {
        int start = 1;
        int param_no = 0;
        int minion_serial_no;
        unsigned short friendly_id;
        for (int i = 1; i<ser_length; i++) {
          if (ser_buffer[i] == ',') {
            const char* bitp = &ser_buffer[start];
            if (param_no==0)      minion_serial_no = (int) atoi(bitp);
            else if (param_no==1) friendly_id = (unsigned short) atoi(bitp);
            start = i+1;
            param_no++;
          }
        }

        // Reply to minion by broadcasting friendly id and params:-
        // MSG_ID [char]  Minion_Serial_No [int]   Minion_Friendly_Id [Short]
        // Master_Serial_No [int]   Master_Time [long]
        // Epidemic_ID [short]   R0 [float]  Rtype [Char]  Rpower [Char]
        // Exposure [Short]

        int the_time = (int) (uBit.systemTime() - time0);

        PacketBuffer omsg(REG_ACK_SIZE);
        uint8_t *obuf = omsg.getBytes();
        obuf[MSG_TYPE] = REG_ACK_MSG;
        memcpy(&obuf[REG_ACK_MINION_SERIAL], &minion_serial_no, SIZE_INT);
        memcpy(&obuf[REG_ACK_ID], &friendly_id, SIZE_SHORT);
        memcpy(&obuf[REG_ACK_MASTER_SERIAL], &serial_no, SIZE_INT);
        memcpy(&obuf[REG_ACK_MASTER_TIME], &the_time, SIZE_INT);
        memcpy(&obuf[REG_ACK_EPID], &epi_id, SIZE_SHORT);
        memcpy(&obuf[REG_ACK_R0], &param_R0, SIZE_FLOAT);
        memcpy(&obuf[REG_ACK_RTYPE], &param_Rtype, SIZE_CHAR);
        memcpy(&obuf[REG_ACK_POIMIN], &param_poimin, SIZE_CHAR);
        memcpy(&obuf[REG_ACK_POIMAX], &param_poimax, SIZE_CHAR);
        memcpy(&obuf[REG_ACK_RPOWER], &param_rpower, SIZE_CHAR);
        memcpy(&obuf[REG_ACK_EXPOSURE], &param_exposure, SIZE_SHORT);
        char bcombine = param_btrans + (4* param_brec) + (16 * param_icons);
        memcpy(&obuf[REG_ACK_BFLAGS], &bcombine, SIZE_CHAR);

        uBit.radio.datagram.send(omsg);

      // Below, we receive a request to force a new infection.
      // Format: 3 , victim_id , no_contacts
      // If no_contacts == 0, then use the epidemic's R0 parameters.

      } else if (ser_buffer[0] == SER_SEED_MSG) {
        int start = 1;
        int param_no = 0;
        short seed_id = -1;
        char seed_contacts = -1;
        for (int i = 1; i<ser_length; i++) {
          if (ser_buffer[i] == ',') {
            const char* bitp = &ser_buffer[start];
            if (param_no==0) seed_id = (short) atoi(bitp);
            else if (param_no==1) seed_contacts = (char) atoi(bitp);
            start = i+1;
            param_no++;
          }
        }

        // Send out the seed message by radio.

        PacketBuffer buf(SEED_MSG_SIZE);
        uint8_t *ibuf = buf.getBytes();
        ibuf[MSG_TYPE] = SEED_MINION_MSG;
        memcpy(&ibuf[SEED_MASTER_SERIAL], &serial_no, SIZE_INT);
        memcpy(&ibuf[SEED_EPI_ID], &epi_id, SIZE_SHORT);
        memcpy(&ibuf[SEED_VICTIM_ID], &seed_id, SIZE_SHORT);
        memcpy(&ibuf[SEED_N_CONS], &seed_contacts, SIZE_CHAR);
        uBit.radio.setGroup(REGISTERED_GROUP);
        uBit.radio.datagram.send(buf);
        current_stage = MASTER_STAGE_EPIDEMIC;

      } else if (ser_buffer[0] == SER_RESET_MSG) {
        PacketBuffer buf(RESET_MSG_SIZE);
        uint8_t *ibuf = buf.getBytes();
        ibuf[MSG_TYPE] = RESET_MSG;
        memcpy(&ibuf[RESET_MSG_MASTER_SERIAL], &serial_no, SIZE_INT);
        uBit.radio.setGroup(REGISTERED_GROUP);
        uBit.radio.datagram.send(buf);
        current_stage = MASTER_STAGE_WAIT_PARAMS;

      } else if (ser_buffer[0] == SER_POWER_OFF_MSG) {
        PacketBuffer buf(POWER_OFF_MSG_SIZE);
        uint8_t *ibuf = buf.getBytes();
        ibuf[MSG_TYPE] = POWER_OFF_MSG;
        memcpy(&ibuf[POWER_OFF_MASTER_SERIAL], &serial_no, SIZE_INT);
        uBit.radio.setGroup(REGISTERED_GROUP);
        uBit.radio.datagram.send(buf);
        current_stage = MASTER_STAGE_POWER_OFF;
      }
    }

    // Reset buffer pointer for next time.

    ser_length = 0;
  }
  uBit.serial.eventOn(NEWLINE);
}

int main() {
  serial_no = microbit_serial_number();
  current_stage = MASTER_STAGE_WAIT_PARAMS;
  uBit.init();

  uBit.display.setBrightness(64);
  uBit.serial.setRxBufferSize(32);
  uBit.serial.setTxBufferSize(32);
  uBit.serial.baud(115200);
  uBit.messageBus.listen(MICROBIT_ID_SERIAL,  MICROBIT_SERIAL_EVT_DELIM_MATCH, receiveSerial);
  uBit.serial.eventOn(NEWLINE);

  while (current_stage != MASTER_STAGE_POWER_OFF) {
    // Wait for parameters through serial port, which will move our
    // stage from MASTER_STAGE_WAIT_PARAMS to MASTER_STAGE_RECRUITMENT

    uBit.radio.setGroup(UNREGISTERED_GROUP);
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
    while (current_stage == MASTER_STAGE_EPIDEMIC) uBit.sleep(1000);

    // Now we're either in MASTER_STAGE_POWER_OFF, or back to MASTER_STAGE_WAIT_PARAMS,
    // following a RESET/POWER_OFF message through the serial.
  }
  uBit.display.print("Z");
  uBit.radio.disable();

  release_fiber();
}