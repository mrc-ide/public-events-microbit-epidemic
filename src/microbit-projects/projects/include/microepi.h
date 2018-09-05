/*
include/microepi.h is part of the micro:bit epidemic project.
It is included in both the master and minion projects to
be flashed on to micro:bits, and defines macros that
need to be consistent between the two.

The MIT License (MIT)

Copyright (c) 2018 Wes Hinsley
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


#ifndef MICROBIT_EPI_H
#define MICROBIT_EPI_H

// This is a header of shared constants between the master
// and minion apps.

#define MAX_MINIONS 100
#define MAX_TRANSMIT_POWER 7

// Save a bit of radio in the registration stage, by ignoring
// registration messages once micro:bit is registered

#define UNREGISTERED_GROUP 1
#define REGISTERED_GROUP 2

// The different stages for the MASTER

#define MASTER_STAGE_WAIT_PARAMS 1
#define MASTER_STAGE_RECRUITMENT 2
#define MASTER_STAGE_EPIDEMIC    3
#define MASTER_STAGE_POWER_OFF   4

// The different stages for the MINION

#define MINION_STAGE_REGISTRY 10
#define MINION_STAGE_EPIDEMIC 11
#define MINION_STAGE_POWER_OFF 12

// And different statuses of a minion

#define STATE_SUSCEPTIBLE 1
#define STATE_INFECTIOUS 2
#define STATE_RECOVERED 3

// The different messages and offsets of the contents.
// All messages start with a byte for the message type (MSG_TYPE = 0)

#define MSG_TYPE 0

// Messages to be handled over serial...

#define SER_VER_MSG '1'
#define SER_REG_MSG '2'
#define SER_PARAM_MSG '3'
#define SER_SEED_MSG '4'
#define SER_RESET_MSG '5'
#define SER_POWER_OFF_MSG '6'

// Messages over radio

#define REG_MSG 4
  #define REG_SERIAL 1
  #define REG_BUILD 5
  #define REG_MSG_SIZE 6

#define REG_ACK_MSG 5
  #define REG_ACK_MINION_SERIAL 1
  #define REG_ACK_ID 5
  #define REG_ACK_MASTER_SERIAL 7
  #define REG_ACK_MASTER_TIME 11
  #define REG_ACK_EPID 15
  #define REG_ACK_R0 17
  #define REG_ACK_RTYPE 21
  #define REG_ACK_POIMIN 22
  #define REG_ACK_POIMAX 23
  #define REG_ACK_RPOWER 24
  #define REG_ACK_EXPOSURE 25
  #define REG_ACK_BFLAGS 27
  #define REG_ACK_SIZE 28

#define SEED_MINION_MSG 6
  #define SEED_MASTER_SERIAL 1
  #define SEED_EPI_ID 5
  #define SEED_VICTIM_ID 7
  #define SEED_N_CONS 9
  #define SEED_MSG_SIZE 10

#define INF_BCAST_MSG 7
  #define INF_BCAST_MASTER_SERIAL 1
  #define INF_BCAST_EPI_ID 5
  #define INF_BCAST_SOURCE_ID 7
  #define INF_BCAST_MSG_SIZE 9

#define INF_CAND_MSG 8
  #define INF_CAND_MASTER_SERIAL 1
  #define INF_CAND_EPI_ID 5
  #define INF_CAND_SOURCE_ID 7
  #define INF_CAND_VICTIM_ID 9
  #define INF_CAND_MSG_SIZE 11

#define INF_CONF_MSG 9
  #define INF_CONF_MASTER_SERIAL 1
  #define INF_CONF_EPI_ID 5
  #define INF_CONF_SOURCE_ID 7
  #define INF_CONF_VICTIM_ID 9
  #define INF_CONF_MSG_SIZE 11

#define RESET_MSG 10
#define RESET_MSG_MASTER_SERIAL 1
#define RESET_MSG_SIZE 5

#define POWER_OFF_MSG 11
#define POWER_OFF_MASTER_SERIAL 1
#define POWER_OFF_MSG_SIZE 5

#define REP_INF_MSG 12
  #define REP_INF_MASTER_SERIAL 1
  #define REP_INF_EPI_ID 5
  #define REP_INF_INFECTOR_ID 7
  #define REP_INF_VICTIM_ID 9
  #define REP_INF_TIME 11
  #define REP_INF_NCONS 15
  #define REP_INF_MSG_SIZE 16

#define REP_RECOV_MSG 13
  #define REP_RECOV_MASTER_SERIAL 1
  #define REP_RECOV_EPI_ID 5
  #define REP_RECOV_VICTIM_ID 7
  #define REP_RECOV_TIME 9
  #define REP_RECOV_MSG_SIZE 13

#define CONF_REP_INF_MSG 14
  #define CONF_REP_INF_MASTER_SERIAL 1
  #define CONF_REP_INF_EPI_ID 5
  #define CONF_REP_INF_VICTIM_ID 7
  #define CONF_REP_INF_MSG_SIZE 9

#define CONF_REP_RECOV_MSG 15
  #define CONF_REP_RECOV_MASTER_SERIAL 1
  #define CONF_REP_RECOV_EPI_ID 5
  #define CONF_REP_RECOV_VICTIM_ID 7
  #define CONF_REP_RECOV_MSG_SIZE 9

#define SIZE_FLOAT 4
#define SIZE_INT 4
#define SIZE_SHORT 2
#define SIZE_CHAR 1
#define INT_MAX 2147483647

#define ALREADY_CONTACTED 32766
#define SEED_CONTACT_ID 32767

#endif