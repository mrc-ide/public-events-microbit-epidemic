#include "MicroBit.h"
#include "microepi.h"
#include <math.h>

MicroBit uBit;

ManagedString VERSION_INFO("VER:Epi Minion 1.0:");
ManagedString NEWLINE("\r\n");

unsigned char current_stage = MINION_STAGE_REGISTRY;
int serial_no;          // My internal serial number
unsigned short my_id;   // My friendly serial number
int master_serial;      // My master's serial number
uint64_t master_time0;  // Master's time when params received
uint64_t my_time0;      // My time when params received


// Parameters for the current epidemic

unsigned short epi_id;
float param_R0;
unsigned char param_Rtype;
unsigned short param_exposure;
unsigned char param_rpower;
unsigned char n_contacts;

unsigned short* exposure_tracker = new unsigned short[MAX_MINIONS];
char current_state;

void reset() {
  for (int i=0; i<MAX_MINIONS; i++) exposure_tracker[i]=0;
  current_state = STATE_SUSCEPTIBLE;
  current_stage = MINION_STAGE_REGISTRY;
  uBit.display.print('U');
  uBit.radio.setGroup(UNREGISTERED_GROUP);
}

float poi(double e) {
  int n = 0;
  double limit;
  double x;
  limit = exp(-e);
  x = uBit.random(INT_MAX) / INT_MAX;
  while (x > limit) {
    n++;
    x = x * (uBit.random(INT_MAX) / INT_MAX);
  }
  return n;
}


// Become infected...

void become_infected(bool set_contacts) {
  if (current_state == STATE_SUSCEPTIBLE) {
    uBit.display.print('I');
    if (set_contacts) {
      if (param_Rtype == 0) n_contacts = (int) param_R0;
      else n_contacts = (int) poi(param_R0);
    }
    current_state = STATE_INFECTIOUS;
  }
}

// We often have to check that the master serial number, and epidemic number
// match the epidemic that a minion was recruited for. To save some lines of
// code, and make it look pretty, here is a macro that does it. I think macro
// with a bit more code is better than a function call here...

#define CHECK_RIGHT_EPIDEMIC(__serial, __epi) \
  int check_master_serial; \
  memcpy(&check_master_serial, &ibuf[__serial], SIZE_INT); \
  if (check_master_serial == master_serial) { \
    unsigned short check_epid; \
    memcpy(&check_epid, &ibuf[__epi], SIZE_SHORT); \
    if (check_epid == epi_id) {

#define END_CHECK_RIGHT_EPIDEMIC }}

// Receive a radio message.

void onData(MicroBitEvent) {

  PacketBuffer imsg = uBit.radio.datagram.recv();
  uint8_t* ibuf = imsg.getBytes();

  if (current_stage == MINION_STAGE_REGISTRY) {

    // Below: we receive a reply message. But it might not be to us...
    // MSG_ID [char]  Serial_No [int]   Friendly_Id [Short]
    // Epidemic_ID [short]   R0 [float]  Rtype [Char]  Rpower [Char]
    // Exposure [Short]

    if (ibuf[MSG_TYPE] == REG_ACK_MESSAGE) {
      int incoming_serial;
      memcpy(&incoming_serial, &ibuf[REG_ACK_MINION_SERIAL], SIZE_INT);

      // If the incoming serial matches ours...

      if (incoming_serial == serial_no) {
        memcpy(&my_id, &ibuf[REG_ACK_ID], SIZE_SHORT);

        // Keep track of our time, as well as master's time.
        my_time0 = uBit.systemTime();

        memcpy(&master_serial, &ibuf[REG_ACK_MASTER_SERIAL], SIZE_INT);
        memcpy(&master_time0, &ibuf[REG_ACK_MASTER_TIME], SIZE_LONG);

        // Fetch the parameters out of the message.

        memcpy(&epi_id, &ibuf[REG_ACK_EPID], SIZE_SHORT);
        memcpy(&param_R0, &ibuf[REG_ACK_R0], SIZE_FLOAT);
        memcpy(&param_Rtype, &ibuf[REG_ACK_RTYPE], SIZE_CHAR);
        memcpy(&param_rpower, &ibuf[REG_ACK_RPOWER], SIZE_CHAR);
        memcpy(&param_exposure, &ibuf[REG_ACK_EXPOSURE], SIZE_SHORT);

        // Now moving into EPIDEMIC stage, and changing radio channel to
        // ignore the registration noise.

        current_stage = MINION_STAGE_EPIDEMIC;
        uBit.radio.setGroup(REGISTERED_GROUP);
        uBit.display.print('S');
        uBit.seedRandom();
      }
    }

  } else if (current_stage == MINION_STAGE_EPIDEMIC) {

    // I receive a RESET code:-

    if (ibuf[MSG_TYPE] == REG_RESET_MESSAGE) {
      current_stage = MINION_STAGE_REGISTRY;
      uBit.radio.setGroup(UNREGISTERED_GROUP);
      uBit.radio.setTransmitPower(MAX_TRANSMIT_POWER);
      uBit.display.print('U');
      reset();

    // Here, a minion receives a message to seed an infection.

    } else if (ibuf[MSG_TYPE] == SEED_MINION_MSG) {

      CHECK_RIGHT_EPIDEMIC(SEED_MASTER_SERIAL, SEED_EPI_ID)

        unsigned short victim_id;
        memcpy(&victim_id, &ibuf[SEED_VICTIM_ID], SIZE_SHORT);
        if (victim_id == my_id) {
          memcpy(&n_contacts, &ibuf[SEED_N_CONS], SIZE_CHAR);
          become_infected(n_contacts==0);
        }

      END_CHECK_RIGHT_EPIDEMIC

    // Here, a minion receives an infectious broadcast.

    } else if (ibuf[MSG_TYPE] == INF_BCAST_MSG) {

      CHECK_RIGHT_EPIDEMIC(INF_BCAST_MASTER_SERIAL, INF_BCAST_EPI_ID)

        // Increase our exposure counter for this (potential) infector.

        unsigned short source_id;
        memcpy(&source_id, &ibuf[INF_BCAST_SOURCE_ID], SIZE_SHORT);
        exposure_tracker[source_id]++;

        if (exposure_tracker[source_id] == param_exposure) {

          // If we've had enough exposure from a source, so consider
          // become infected - HOWEVER - this requres confirmation
          // from the infector, because there may be multiple replies
          // to the broadcast message, and we might be too late.

          // So, send a message back to the infector, indicating we are
          // willing to be one of their infections.

          // Broadcast a candidate infection message with:
          // master_serial (int), epidemic id (short)
          // the infector's id (short), my id (short)

          PacketBuffer omsg(INF_CAND_MSG_SIZE);
          uint8_t *obuf = omsg.getBytes();
          obuf[MSG_TYPE] = INF_CAND_MSG;
          memcpy(&obuf[INF_CAND_MASTER_SERIAL], &master_serial, SIZE_INT);
          memcpy(&obuf[INF_CAND_EPI_ID], &epi_id, SIZE_SHORT);
          memcpy(&obuf[INF_CAND_SOURCE_ID], &source_id, SIZE_SHORT);
          memcpy(&obuf[INF_CAND_VICTIM_ID], &my_id, SIZE_SHORT);
          uBit.radio.setTransmitPower(MAX_TRANSMIT_POWER);
          uBit.radio.datagram.send(omsg);
        }

      END_CHECK_RIGHT_EPIDEMIC

    // And the code for handling the incoming infection candidate message

    } else if (ibuf[MSG_TYPE] == INF_CAND_MSG) {

      CHECK_RIGHT_EPIDEMIC(INF_CAND_MASTER_SERIAL, INF_CAND_EPI_ID)

        unsigned short source_id;
        memcpy(&source_id, &ibuf[INF_CAND_SOURCE_ID], SIZE_SHORT);
        if (source_id == my_id) {

          // Note that we "play along" even if we've recovered; a recovered
          // minion still counts as a contact - just when we're recovered, we
          // don't start making any new contacts of our own.

          unsigned short victim_id;
          memcpy(&victim_id, &ibuf[INF_CAND_SOURCE_ID], SIZE_SHORT);
          PacketBuffer omsg(INF_CONF_MSG_SIZE);
          uint8_t *obuf = omsg.getBytes();
          obuf[MSG_TYPE] = INF_CONF_MSG;
          memcpy(&obuf[INF_CONF_MASTER_SERIAL], &master_serial, SIZE_INT);
          memcpy(&obuf[INF_CONF_EPI_ID], &epi_id, SIZE_SHORT);
          memcpy(&obuf[INF_CONF_SOURCE_ID], &my_id, SIZE_SHORT);
          memcpy(&obuf[INF_CONF_VICTIM_ID], &victim_id, SIZE_SHORT);
          uBit.radio.setTransmitPower(MAX_TRANSMIT_POWER);
          uBit.radio.datagram.send(omsg);
          if (n_contacts>0) {
            n_contacts--;
            if (n_contacts == 0) {
              current_state = STATE_RECOVERED;
              uBit.display.print('R');
            }
          }
        }

        END_CHECK_RIGHT_EPIDEMIC

      // Now, suppose we've received a confirmation message (ie, the reply from
      // our INF_CAND_MSG). The infector has confirmed we are one of their favourite
      // victims, so we are officially infectious.

    } else if (ibuf[MSG_TYPE] == INF_CONF_MSG) {

      CHECK_RIGHT_EPIDEMIC(INF_CONF_MASTER_SERIAL, INF_CONF_EPI_ID)

        unsigned short victim_id;
        memcpy(&victim_id, &ibuf[INF_CONF_VICTIM_ID], SIZE_SHORT);
        if (victim_id == my_id) {
          become_infected(true);
        }

      END_CHECK_RIGHT_EPIDEMIC

    }
  }
}

// If I'm infectious, then broadcast infection with radio power
// set to parameter rpower, for anyone who hears it. (This is called
// every second from main loop, if in the infectious state)

void broadcastInfection() {
  PacketBuffer omsg(INF_BCAST_MSG_SIZE);
  uint8_t *obuf = omsg.getBytes();
  obuf[MSG_TYPE] = INF_BCAST_MSG;
  memcpy(&obuf[INF_BCAST_MASTER_SERIAL], &master_serial, SIZE_INT);
  memcpy(&obuf[INF_BCAST_EPI_ID], &epi_id, SIZE_SHORT);
  memcpy(&obuf[INF_BCAST_SOURCE_ID], &my_id, SIZE_SHORT);
  uBit.radio.setTransmitPower(param_rpower);
  uBit.radio.datagram.send(omsg);
}

// Send a message to the master requesting registration
// for an epidemic game. Called once a second from main
// loop when in the MINION_STAGE_REGISTRY stage.

void broadcastRegister() {
  PacketBuffer omsg(REG_MSG_SIZE);
  uint8_t *obuf = omsg.getBytes();
  obuf[MSG_TYPE] = REG_MESSAGE;
  serial_no = microbit_serial_number();
  memcpy(&obuf[REG_SERIAL], &serial_no, SIZE_INT);
  uBit.radio.setTransmitPower(MAX_TRANSMIT_POWER);
  uBit.radio.datagram.send(omsg);
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
  uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
  uBit.radio.enable();

  reset();
 
  while (current_stage == MINION_STAGE_REGISTRY) {
    uBit.sleep(1000);
    broadcastRegister();
  }

  while (current_stage == MINION_STAGE_EPIDEMIC) {
    uBit.sleep(1000);
    if (current_state == STATE_INFECTIOUS) broadcastInfection();
  }

  uBit.radio.disable();

  release_fiber();
}
