#ifndef MICROBIT_EPI_H
#define MICROBIT_EPI_H

// This is a header of shared constants between the master
// and minion apps. It should consist only of pre-processor
// defintions; anything that requires runtime memory should
// be defined in the code for the app, because every byte is
// important!

// Save a bit of radio in the registration stage, by ignoring 
// registration messages once micro:bit is registered

#define UNREGISTERED_GROUP 1
#define REGISTERED_GROUP 2

// The different stages for the MASTER

#define MASTER_STAGE_WAIT_PARAMS 1
#define MASTER_STAGE_RECRUITMENT 2
#define MASTER_STAGE_EPIDEMIC    3

// The different stages for the MINION

#define MINION_STAGE_REGISTRY 10
#define MINION_STAGE_EPIDEMIC 11

// The different messages and offsets of the contents.
// All messages start with a byte for the message type (MSG_TYPE = 0)

#define MSG_TYPE 0

// Messages to be handled over serial...

#define VER_MESSAGE '1'
#define PARAM_MESSAGE '2'
#define SEED_MESSAGE_MASTER '3'

// Messages over radio

#define REG_MESSAGE 4
  #define REG_SERIAL 1
  #define REG_MSG_SIZE 5

#define REG_ACK_MESSAGE 5
  #define REG_ACK_MINION_SERIAL 1
  #define REG_ACK_ID 5
  #define REG_ACK_MASTER_SERIAL 7
  #define REG_ACK_MASTER_TIME 11
  #define REG_ACK_EPID 19
  #define REG_ACK_R0 21
  #define REG_ACK_RTYPE 25
  #define REG_ACK_RPOWER 26
  #define REG_ACK_EXPOSURE 27
  #define REG_ACK_SIZE 28

#define SEED_MINION_MSG 6
  #define SEED_MASTER_SERIAL 1
  #define SEED_EPI_ID 5
  #define SEED_VICTIM_ID 7
  #define SEED_N_CONS 9
  #define SEED_MSG_SIZE 10

#define REG_RESET_MESSAGE 7

#define INF_BCAST_MSG 8
  #define INF_BCAST_MASTER_SERIAL 1
  #define INF_BCAST_EPI_ID 5
  #define INF_BCAST_SOURCE_ID 7
  #define INF_BCAST_MSG_SIZE 9

#define INF_CAND_MSG 9
  #define INF_CAND_MASTER_SERIAL 1
  #define INF_CAND_EPI_ID 5
  #define INF_CAND_SOURCE_ID 7
  #define INF_CAND_VICTIM_ID 9
  #define INF_CAND_MSG_SIZE 11

#define INF_CONF_MSG 10
  #define INF_CONF_MASTER_SERIAL 1
  #define INF_CONF_EPI_ID 5
  #define INF_CONF_SOURCE_ID 7
  #define INF_CONF_VICTIM_ID 9
  #define INF_CONF_MSG_SIZE 11


#define SIZE_LONG 8
#define SIZE_FLOAT 4
#define SIZE_INT 4
#define SIZE_SHORT 2
#define SIZE_CHAR 1
#define INT_MAX 2147483647

#define SUSCEPTIBLE 1
#define INFECTED 2
#define RECOVERED 3
#endif