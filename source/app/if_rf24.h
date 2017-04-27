#ifndef __IF_RF24_H__
#define __IF_RF24_H__

#include "../ak/message.h"

#include <wiringPi.h>
#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <RF24Network/RF24Network_config.h>

#define RF24_BUFFER_SIZE		128
#define MAX_ADDRESS_POOL        64

extern q_msg_t gw_task_if_rf24_mailbox;
extern void* gw_task_if_rf24_entry(void*);

extern RF24 radio;
extern RF24Network network;

extern uint8_t gateway_active_channel;
#endif //__IF_RF24_H__
