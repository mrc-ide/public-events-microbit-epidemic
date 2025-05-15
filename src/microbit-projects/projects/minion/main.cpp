/*
minion/main.cpp is part of the micro:bit epidemic project.
This is the software to be run on the 100 (or fewer) 
minion micro:bits, which are the agents of the epidemic.

The MIT License (MIT)

Copyright (c) 2019 Wes Hinsley
MRC Centre for Global Infectious Disease Analysis
Department of Infectious Disease Epidemiology
Imperial College London

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include "MicroBit.h"
#include "microepi.h"
#include <math.h>

MicroBit uBit;

ManagedString VERSION_INFO("VER:Epi Minion 1.15:");
#define MINION_BUILD_NO 15
ManagedString NEWLINE("\n");
ManagedString END_SERIAL("#\n");


#define NO_BUTTONS 0
#define BUTTON_A 1
#define BUTTON_B 2
#define BUTTON_AB 3

#define ICONS_SIR 0
#define ICONS_IxR 1
#define ICONS_oIo 2

unsigned char current_stage; // Can be MINION_STAGE_REGISTRY, or MINION_STAGE_EPIDEMIC, or MINION_STAGE_POWEROFF.
int serial_no;               // My internal serial number
unsigned short my_id;        // My friendly id number
int master_serial;           // My master's serial number

int master_time0;           // Master (relative) time when params received
uint64_t my_time0;          // My time when params received
unsigned char inf_reported;    // Track whether the master acknowledged infection report
unsigned char recov_reported;  // Track whether the master acknowledged recovery report
int recovery_time;              // Remember when I recovered (for resilient resending)
int infection_time;             // Remember when I was infected (for resilient resending)
unsigned short who_infected_me; // Remember who infected me (for resilient resending)

// Parameters for the current epidemic

unsigned short epi_id;
float param_R0;
unsigned char param_Rtype;
unsigned char param_poimin;
unsigned char param_poimax;
unsigned short param_exposure;
unsigned char param_rpower;
unsigned char n_contacts;
unsigned char param_btrans;
unsigned char param_brec;
unsigned char param_icons;

unsigned short* exposure_tracker;
char current_state;
bool screen_on = true;

bool buttonsOk(char param) {
  bool go = (param==NO_BUTTONS);
  if (!go) go = (param == BUTTON_A) && (uBit.buttonA.isPressed()) && (!uBit.buttonB.isPressed());
  if (!go) go = (param == BUTTON_B) && (uBit.buttonB.isPressed()) && (!uBit.buttonA.isPressed());
  if (!go) go = (param == BUTTON_AB) && (uBit.buttonA.isPressed()) && (uBit.buttonB.isPressed());
  return go;
}

void ledStatus() {
  if (screen_on) {
    for (int i=0; i<4; i++) for (int j=0; j<5; j++)
      uBit.display.image.setPixelValue(i,j,0);
  if (current_state == STATE_SUSCEPTIBLE) {
      if (param_icons == ICONS_SIR) {
        for (int i=0; i<3; i++) for (int j=0; j<5; j+=2)
          uBit.display.image.setPixelValue(i,j,255);
        uBit.display.image.setPixelValue(0,1,255);
        uBit.display.image.setPixelValue(2,3,255);
      } else if (param_icons == ICONS_IxR) {
        for (int i=0; i<3; i++) for (int j=0; j<5; j+=4)
          uBit.display.image.setPixelValue(i,j,255);
        for (int j=1; j<=3; j++) uBit.display.image.setPixelValue(1,j,255);
      }

    } else if (current_state == STATE_INFECTIOUS) {
      if ((param_icons == ICONS_SIR) || (param_icons == ICONS_oIo)) {
        for (int i=0; i<3; i++) for (int j=0; j<5; j+=4)
          uBit.display.image.setPixelValue(i,j,255);
        for (int j=1; j<=3; j++) uBit.display.image.setPixelValue(1,j,255);
      } else if (param_icons == ICONS_IxR) {
        for (int i=0; i<=2; i++) {
          uBit.display.image.setPixelValue(1,i,255);
          uBit.display.image.setPixelValue(i,1,255);
        }
      }

    } else if (current_state == STATE_RECOVERED) {
      if ((param_icons == ICONS_SIR) || (param_icons == ICONS_IxR)) {
        for (int j=0; j<5; j++) uBit.display.image.setPixelValue(0,j,255);
        uBit.display.image.setPixelValue(1,0,255);
        uBit.display.image.setPixelValue(2,1,255);
        uBit.display.image.setPixelValue(1,2,255);
        uBit.display.image.setPixelValue(2,3,255);
        uBit.display.image.setPixelValue(2,4,255);
      }
    }
  } else {
    uBit.display.clear();
    uBit.display.image.setPixelValue(0,0,32);
  }
}

void reset() {
  uBit.display.image.setPixelValue(4,0,0);
  uBit.display.image.setPixelValue(4,1,0);
  uBit.display.image.setPixelValue(4,2,0);
  for (int i=0; i<MAX_MINIONS; i++) exposure_tracker[i]=0;
  current_state = STATE_SUSCEPTIBLE;
  current_stage = MINION_STAGE_REGISTRY;
  uBit.display.print('U');
  uBit.radio.setGroup(UNREGISTERED_GROUP);
  inf_reported = 0;
  recov_reported = 0;
}

int poi(double e) {
  int n = 0;
  double limit;
  double x;
  limit = exp(-e);
  x = (double)uBit.random(INT_MAX) / (double)INT_MAX;
  while (x > limit) {
    n++;
    x = x * ((double)uBit.random(INT_MAX) / (double)INT_MAX);
  }
  return n;
}

void reportInfected() {
  PacketBuffer omsg(REP_INF_MSG_SIZE);
  uint8_t *obuf = omsg.getBytes();
  obuf[MSG_TYPE] = REP_INF_MSG;
  memcpy(&obuf[REP_INF_MASTER_SERIAL], &master_serial, SIZE_INT);
  memcpy(&obuf[REP_INF_EPI_ID], &epi_id, SIZE_SHORT);
  memcpy(&obuf[REP_INF_INFECTOR_ID], &who_infected_me, SIZE_SHORT);
  memcpy(&obuf[REP_INF_VICTIM_ID], &my_id, SIZE_SHORT);
  memcpy(&obuf[REP_INF_TIME], &infection_time, SIZE_INT);
  memcpy(&obuf[REP_INF_NCONS], &n_contacts, SIZE_CHAR);
  uBit.radio.setTransmitPower(MAX_TRANSMIT_POWER);
  uBit.radio.datagram.send(omsg);
}

// Become infected...

void becomeInfected(bool set_contacts) {
  if (current_state == STATE_SUSCEPTIBLE) {
    if (set_contacts) {
      if (param_Rtype == 0) n_contacts = (unsigned char) param_R0;
      else if (param_Rtype == 1) {
        n_contacts = (unsigned char) poi(param_R0);
        if (n_contacts < param_poimin) n_contacts = param_poimin;
        if (n_contacts > param_poimax) n_contacts = param_poimax;
      }
    }

    infection_time = (int) ((uBit.systemTime() - my_time0) + master_time0);
    current_state = STATE_INFECTIOUS;
    ledStatus();
  }
}

void recover() {
  current_state = STATE_RECOVERED;
  ledStatus();
  uBit.display.image.setPixelValue(4,1,0);
  recovery_time = (int) ((uBit.systemTime() - my_time0) + master_time0);
}


// We often have to check that the master serial number, and epidemic number
// match the epidemic that a minion was recruited for. To save some lines of
// code, and make it look pretty, here is a macro that does it. I think macro
// with a bit more code is better than a function call here...

#define CHECK_RIGHT_SERIAL(__serial) \
  int check_master_serial; \
  memcpy(&check_master_serial, &ibuf[__serial], SIZE_INT); \
  if (check_master_serial == master_serial) {

#define END_CHECK_RIGHT_SERIAL }

#define CHECK_RIGHT_EPIDEMIC(__serial, __epi) \
  int check_master_serial; \
  memcpy(&check_master_serial, &ibuf[__serial], SIZE_INT); \
  if (check_master_serial == master_serial) { \
    unsigned short check_epid; \
    memcpy(&check_epid, &ibuf[__epi], SIZE_SHORT); \
    if (check_epid == epi_id) {

#define END_CHECK_RIGHT_EPIDEMIC }}

void reportRecovery() {
  PacketBuffer omsg(REP_RECOV_MSG_SIZE);
  uint8_t *obuf = omsg.getBytes();
  obuf[MSG_TYPE] = REP_RECOV_MSG;
  memcpy(&obuf[REP_RECOV_MASTER_SERIAL], &master_serial, SIZE_INT);
  memcpy(&obuf[REP_RECOV_EPI_ID], &epi_id, SIZE_SHORT);
  memcpy(&obuf[REP_RECOV_VICTIM_ID], &my_id, SIZE_SHORT);
  memcpy(&obuf[REP_RECOV_TIME], &recovery_time, SIZE_INT);
  uBit.radio.setTransmitPower(MAX_TRANSMIT_POWER);
  uBit.radio.datagram.send(omsg);
}
// Receive a radio message.

void onData(MicroBitEvent) {
  PacketBuffer imsg = uBit.radio.datagram.recv();
  uint8_t* ibuf = imsg.getBytes();

  if (current_stage == MINION_STAGE_REGISTRY) {

    // Below: we receive a reply message. But it might not be to us...
    // MSG_ID [char]  Serial_No [int]   Friendly_Id [Short]
    // Epidemic_ID [short]   R0 [float]  Rtype [Char]  Rpower [Char]
    // Exposure [Short]

    if (ibuf[MSG_TYPE] == REG_ACK_MSG) {
      int incoming_serial;

      memcpy(&incoming_serial, &ibuf[REG_ACK_MINION_SERIAL], SIZE_INT);

      // If the incoming serial matches ours...

      if (incoming_serial == serial_no) {
        memcpy(&my_id, &ibuf[REG_ACK_ID], SIZE_SHORT);

        // Keep track of our time, as well as master's time.
        my_time0 = uBit.systemTime();

        memcpy(&master_serial, &ibuf[REG_ACK_MASTER_SERIAL], SIZE_INT);
        memcpy(&master_time0, &ibuf[REG_ACK_MASTER_TIME], SIZE_INT);

        // Fetch the parameters out of the message.
        memcpy(&epi_id, &ibuf[REG_ACK_EPID], SIZE_SHORT);
        memcpy(&param_R0, &ibuf[REG_ACK_R0], SIZE_FLOAT);
        memcpy(&param_Rtype, &ibuf[REG_ACK_RTYPE], SIZE_CHAR);
        memcpy(&param_poimin, &ibuf[REG_ACK_POIMIN], SIZE_CHAR);
        memcpy(&param_poimax, &ibuf[REG_ACK_POIMAX], SIZE_CHAR);
        memcpy(&param_rpower, &ibuf[REG_ACK_RPOWER], SIZE_CHAR);
        memcpy(&param_exposure, &ibuf[REG_ACK_EXPOSURE], SIZE_SHORT);
        unsigned char bflags;
        memcpy(&bflags, &ibuf[REG_ACK_BFLAGS], SIZE_CHAR);
        param_btrans = bflags & 3;
        bflags = bflags / 4;
        param_brec = bflags & 3;
        bflags = bflags / 4;
        param_icons = bflags & 3;

        // Now moving into EPIDEMIC stage, and changing radio channel to
        // ignore the registration noise.

        current_stage = MINION_STAGE_EPIDEMIC;
        uBit.radio.setGroup(REGISTERED_GROUP);
        ledStatus();
        uBit.seedRandom();
      }
    }

  } else if (current_stage == MINION_STAGE_EPIDEMIC) {

  // I receive a RESET code:-

    if (ibuf[MSG_TYPE] == RESET_MSG) {
      CHECK_RIGHT_SERIAL(RESET_MSG_MASTER_SERIAL)

        // Re-broadcast reset, then change channel.
        PacketBuffer omsg(RESET_MSG_SIZE);
        uint8_t *obuf = omsg.getBytes();
        obuf[MSG_TYPE] = RESET_MSG;
        memcpy(&obuf[RESET_MSG_MASTER_SERIAL], &master_serial, SIZE_INT);
        uBit.radio.setTransmitPower(MAX_TRANSMIT_POWER);
        uBit.radio.datagram.send(omsg);
        uBit.radio.disable();
        reset();
        uBit.radio.enable();
      END_CHECK_RIGHT_SERIAL

    // I receive a POWER-OFF code:-

    } else if (ibuf[MSG_TYPE] == POWER_OFF_MSG) {
      CHECK_RIGHT_SERIAL(POWER_OFF_MASTER_SERIAL)
        // Re-broadcast reset, then change channel.
        PacketBuffer omsg(POWER_OFF_MSG_SIZE);
        uint8_t *obuf = omsg.getBytes();
        obuf[MSG_TYPE] = POWER_OFF_MSG;
        memcpy(&obuf[POWER_OFF_MASTER_SERIAL], &master_serial, SIZE_INT);
        uBit.radio.setTransmitPower(MAX_TRANSMIT_POWER);
        uBit.radio.datagram.send(omsg);
        current_stage = MINION_STAGE_POWER_OFF;
      END_CHECK_RIGHT_SERIAL

    } else if (ibuf[MSG_TYPE] == SCREEN_ON_MSG) {
      CHECK_RIGHT_SERIAL(SCREEN_ON_MASTER_SERIAL)
        // Re-broadcast screen-on once, and set screen status.
        if (ibuf[SCREEN_ON_REPEAT]==0) {
          PacketBuffer omsg(SCREEN_ON_MSG_SIZE);
          uint8_t *obuf = omsg.getBytes();
          obuf[MSG_TYPE] = SCREEN_ON_MSG;
          obuf[SCREEN_ON_REPEAT]=1;
          memcpy(&obuf[SCREEN_ON_MASTER_SERIAL], &master_serial, SIZE_INT);
          uBit.radio.setTransmitPower(MAX_TRANSMIT_POWER);
          uBit.radio.datagram.send(omsg);
        }
        screen_on = true;
        ledStatus();
      END_CHECK_RIGHT_SERIAL
     
    } else if (ibuf[MSG_TYPE] == SCREEN_OFF_MSG) {
      CHECK_RIGHT_SERIAL(SCREEN_OFF_MASTER_SERIAL)
        // Re-broadcast screen-off once, and set screen status.
        if (ibuf[SCREEN_OFF_REPEAT]==0) {
          PacketBuffer omsg(SCREEN_OFF_MSG_SIZE);
          uint8_t *obuf = omsg.getBytes();
          obuf[MSG_TYPE] = SCREEN_OFF_MSG;
          obuf[SCREEN_OFF_REPEAT]=1;
          memcpy(&obuf[SCREEN_OFF_MASTER_SERIAL], &master_serial, SIZE_INT);
          uBit.radio.setTransmitPower(MAX_TRANSMIT_POWER);
          uBit.radio.datagram.send(omsg);
        }
        screen_on = false;
        ledStatus();
      END_CHECK_RIGHT_SERIAL

    // Here, a susceptible minion receives a message to seed an infection.
    } else if ((ibuf[MSG_TYPE] == SEED_MINION_MSG) && (current_state == STATE_SUSCEPTIBLE)) {

      CHECK_RIGHT_EPIDEMIC(SEED_MASTER_SERIAL, SEED_EPI_ID)

        unsigned short victim_id;
        memcpy(&victim_id, &ibuf[SEED_VICTIM_ID], SIZE_SHORT);
        if (victim_id == my_id) {
          memcpy(&n_contacts, &ibuf[SEED_N_CONS], SIZE_CHAR);
          who_infected_me = SEED_CONTACT_ID;
          becomeInfected(n_contacts==0);

          // if n_contacts==0 on the way into becomeInfected(), then it picks
          // how many contacts using the epidemic parameters. If, awkwardly,
          // that returns zero, then we have a boring/efficient epidemic, and
          // instant recovery of the seed.

          if (n_contacts==0) recover();
        }

      END_CHECK_RIGHT_EPIDEMIC

      // Here, a minion (in any state) receives an infectious broadcast.
      // ie - you can be in any state to be a contact, but only susceptibles will then
      // become infectious and make their own contacts.

    } else if (ibuf[MSG_TYPE] == INF_BCAST_MSG) {

      CHECK_RIGHT_EPIDEMIC(INF_BCAST_MASTER_SERIAL, INF_BCAST_EPI_ID)

        if (buttonsOk(param_brec)) {
          uBit.display.image.setPixelValue(4,2,255-uBit.display.image.getPixelValue(4,2));

          // Increase our exposure counter for this (potential) infector.

          unsigned short source_id;
          memcpy(&source_id, &ibuf[INF_BCAST_SOURCE_ID], SIZE_SHORT);
          if (exposure_tracker[source_id] != ALREADY_CONTACTED) {
            exposure_tracker[source_id]++;

            if (exposure_tracker[source_id] >= param_exposure) {
              // If we've reached the exposure threshold from a single source,
              // we now might be one of their contacts - HOWEVER - this requres
              // confirmation from the infector, because there may be multiple replies
              // to the broadcast message, and we might be too late.

              // Using == param_exposure here, to ensure we only offer ourselves
              // as a contact once.

              // So, we send a message back to the infector, indicating we are
              // willing to be one of their infections.

              // Broadcast a candidate infection message with:
              // master_serial (int), epidemic id (short)
              // the infector's id (short), my id (short)

              // Random pause - spread out replies a bit.

              uBit.sleep(uBit.random(500));
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
          }
        }

      END_CHECK_RIGHT_EPIDEMIC

    // Here: we receive a "candidate infection" message, which is a reply to a previous "infection broadcast"
    // message we sent. We care about this message if we are infectious and wanting to confirm contacts.
    // When we have made sufficient contacts, we stop being infectious.

    } else if ((ibuf[MSG_TYPE] == INF_CAND_MSG) && (current_state == STATE_INFECTIOUS)) {

      CHECK_RIGHT_EPIDEMIC(INF_CAND_MASTER_SERIAL, INF_CAND_EPI_ID)

        unsigned short source_id;
        memcpy(&source_id, &ibuf[INF_CAND_SOURCE_ID], SIZE_SHORT);
        if (source_id == my_id) {

          unsigned short victim_id;
          memcpy(&victim_id, &ibuf[INF_CAND_VICTIM_ID], SIZE_SHORT);

          uBit.sleep(uBit.random(200));
          PacketBuffer omsg(INF_CONF_MSG_SIZE);
          uint8_t *obuf = omsg.getBytes();
          obuf[MSG_TYPE] = INF_CONF_MSG;
          memcpy(&obuf[INF_CONF_MASTER_SERIAL], &master_serial, SIZE_INT);
          memcpy(&obuf[INF_CONF_EPI_ID], &epi_id, SIZE_SHORT);
          memcpy(&obuf[INF_CONF_SOURCE_ID], &my_id, SIZE_SHORT);
          memcpy(&obuf[INF_CONF_VICTIM_ID], &victim_id, SIZE_SHORT);
          uBit.radio.setTransmitPower(MAX_TRANSMIT_POWER);
          uBit.radio.datagram.send(omsg);

        // Prevent the victim making the infector their first contact...

          exposure_tracker[victim_id] = ALREADY_CONTACTED;

          if (n_contacts>0) {
            n_contacts--;
            if (n_contacts == 0) recover();
          }
        }

        END_CHECK_RIGHT_EPIDEMIC

      // Now, suppose we've received a confirmation message (ie, the reply from
      // our INF_CAND_MSG). We are now a contact - and if we're susceptible, then
      // we now become infectious.

    } else if (ibuf[MSG_TYPE] == INF_CONF_MSG) {

      CHECK_RIGHT_EPIDEMIC(INF_CONF_MASTER_SERIAL, INF_CONF_EPI_ID)

        unsigned short victim_id;
        memcpy(&victim_id, &ibuf[INF_CONF_VICTIM_ID], SIZE_SHORT);
        if (victim_id == my_id) {
          memcpy(&who_infected_me, &ibuf[INF_CONF_SOURCE_ID], SIZE_SHORT);
          if (current_state == STATE_SUSCEPTIBLE) {
            becomeInfected(true);
            if (n_contacts == 0) recover();
          }
          exposure_tracker[who_infected_me] = ALREADY_CONTACTED;
        }

      END_CHECK_RIGHT_EPIDEMIC

    // If we're in Infectious / Recovered state, listen for the confirmation that
    // our infection report was received by the master.

    } else if ((ibuf[MSG_TYPE] == CONF_REP_INF_MSG) && (current_state != STATE_SUSCEPTIBLE)) {

      CHECK_RIGHT_EPIDEMIC(CONF_REP_INF_MASTER_SERIAL, CONF_REP_INF_EPI_ID)
        unsigned short victim_id;
        memcpy(&victim_id, &ibuf[CONF_REP_INF_VICTIM_ID], SIZE_SHORT);
        if (victim_id == my_id) {
          inf_reported = 1;
        }
      END_CHECK_RIGHT_EPIDEMIC

    // If we're in Recovered state, listen for the confirmation that
    // our recovery report was received by the master.

    } else if ((ibuf[MSG_TYPE] == CONF_REP_RECOV_MSG) && (current_state == STATE_RECOVERED)) {

      CHECK_RIGHT_EPIDEMIC(CONF_REP_RECOV_MASTER_SERIAL, CONF_REP_RECOV_EPI_ID)
        unsigned short victim_id;
        memcpy(&victim_id, &ibuf[CONF_REP_RECOV_VICTIM_ID], SIZE_SHORT);
        if (victim_id == my_id) {
          recov_reported = 1;
        }
      END_CHECK_RIGHT_EPIDEMIC
    }
  }
}



// If I'm infectious, then broadcast infection with radio power
// set to parameter rpower, for anyone who hears it. (This is called
// every second from main loop, if in the infectious state)

void broadcastInfection() {
  if (buttonsOk(param_btrans)) {
    uBit.sleep(uBit.random(200));
    if (screen_on)
      uBit.display.image.setPixelValue(4,1,255-uBit.display.image.getPixelValue(4,1));
    PacketBuffer omsg(INF_BCAST_MSG_SIZE);
    uint8_t *obuf = omsg.getBytes();
    obuf[MSG_TYPE] = INF_BCAST_MSG;
    memcpy(&obuf[INF_BCAST_MASTER_SERIAL], &master_serial, SIZE_INT);
    memcpy(&obuf[INF_BCAST_EPI_ID], &epi_id, SIZE_SHORT);
    memcpy(&obuf[INF_BCAST_SOURCE_ID], &my_id, SIZE_SHORT);
    uBit.radio.setTransmitPower(param_rpower);
    uBit.radio.datagram.send(omsg);
  }
}

// Send a message to the master requesting registration
// for an epidemic game. Called once a second from main
// loop when in the MINION_STAGE_REGISTRY stage.

void broadcastRegister() {
  if (screen_on)
    uBit.display.image.setPixelValue(4,0,255-uBit.display.image.getPixelValue(4,0));
  PacketBuffer omsg(REG_MSG_SIZE);
  uint8_t *obuf = omsg.getBytes();
  obuf[MSG_TYPE] = REG_MSG;
  serial_no = microbit_serial_number();
  memcpy(&obuf[REG_SERIAL], &serial_no, SIZE_INT);
  obuf[REG_BUILD] = MINION_BUILD_NO;
  uBit.radio.setTransmitPower(MAX_TRANSMIT_POWER);
  uBit.radio.datagram.send(omsg);
}

// Going to listen to serial port anyway, just in case a minion micro:bit
// gets plugged into the serial port by mistake. Would be helpful if the
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

  if (msg.charAt(0) == SER_VER_MSG) {
    ManagedString SERIAL_NO(serial_no);
    ManagedString COLON(":");
    ManagedString MB_VERSION(MB_GET_VERSION);
    sendSerial(VERSION_INFO + SERIAL_NO + COLON + MB_VERSION + END_SERIAL);
  }
}

int main() {
  uBit.init();
  uBit.serial.setRxBufferSize(32);
  uBit.serial.setTxBufferSize(32);
  MB_SET_SERIAL_BAUD(115200);
  uBit.messageBus.listen(MICROBIT_ID_SERIAL, MICROBIT_SERIAL_EVT_DELIM_MATCH, receiveSerial, MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY);
  uBit.serial.eventOn(NEWLINE);
  uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData, MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY);
  exposure_tracker = new unsigned short[MAX_MINIONS];

  reset();
  uBit.radio.enable();

  while ((current_stage == MINION_STAGE_REGISTRY) || (current_stage==MINION_STAGE_EPIDEMIC)) {

    while (current_stage == MINION_STAGE_REGISTRY) {
      uBit.sleep(1000);
      broadcastRegister();
    }
    uBit.display.image.setPixelValue(4,0,0);

    while (current_stage == MINION_STAGE_EPIDEMIC) {
      uBit.sleep(1000);
      if (current_state == STATE_INFECTIOUS) broadcastInfection();
      if ((current_state != STATE_SUSCEPTIBLE) && (inf_reported == 0)) reportInfected();
      if ((current_state == STATE_RECOVERED) && (recov_reported == 0)) reportRecovery();
    }
  }

  uBit.display.clear();
  uBit.messageBus.ignore(MICROBIT_ID_SERIAL, MICROBIT_SERIAL_EVT_DELIM_MATCH, receiveSerial);
  uBit.messageBus.ignore(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
  uBit.radio.disable();
  release_fiber();
}
