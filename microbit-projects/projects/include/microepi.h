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
// All messages start with a byte for the message type (MSG_TYPE_OFS = 0)

#define MSG_TYPE_OFS 0

// Messages to be handled over serial...

#define VER_MESSAGE '1'
#define PARAM_MESSAGE '2'
#define SEED_MESSAGE_MASTER '3'

// Messages over radio

#define REG_MESSAGE 4
  #define REG_SERIAL_OFS 1
  #define REG_MSG_SIZE 5

#define REG_ACK_MESSAGE 5
  #define REG_ACK_SERIAL_OFS 1
  #define REG_ACK_ID_OFS 5
  #define REG_ACK_EPID 7
  #define REG_ACK_R0 9
  #define REG_ACK_RTYPE 13
  #define REG_ACK_RPOWER 14
  #define REG_ACK_EXPOSURE 15
  #define REG_ACK_SIZE 17

#define SEED_MESSAGE_MINION 6
  #define SEED_MASTER_SERIAL 1
  #define SEED_EPI_ID 5
  #define SEED_VICTIM_ID 7
  #define SEED_MESSAGE_SIZE 9

#define REG_RESET_MESSAGE 7

#define SIZE_FLOAT 4
#define SIZE_INT 4
#define SIZE_SHORT 2
#define SIZE_CHAR 1


#endif