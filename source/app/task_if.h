#ifndef __TASK_IF_H__
#define __TASK_IF_H__

#include "../ak/message.h"

#define IF_TYPE_RF24				(0x01)
#define IF_TYPE_MQTT                (0x02)

extern q_msg_t gw_task_if_mailbox;
extern void* gw_task_if_entry(void*);

#endif //__TASK_IF_H__
