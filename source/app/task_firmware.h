#ifndef __TASK_FIRMWARE_H__
#define __TASK_FIRMWARE_H__

#include "../ak/message.h"

extern q_msg_t gw_task_firmware_mailbox;
extern void* gw_task_firmware_entry(void*);

#endif //__TASK_FIRMWARE_H__
