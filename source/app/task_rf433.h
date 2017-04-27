#ifndef __TASK_RF433_H__
#define __TASK_RF433_H__

#include <stdint.h>
#include "../ak/message.h"

#define RECORD_CODE_TYPE_NONE				0x00
#define RECORD_CODE_TYPE_DOOR_OPENED		0x01
#define RECORD_CODE_TYPE_DOOR_CLOSED		0x02
#define RECORD_CODE_TYPE_SMOKE_WARNNING		0x03
#define RECORD_CODE_TYPE_WATER_WARNNING		0x04

typedef struct {
	uint32_t door_opened;
	uint32_t door_closed;

	uint32_t smoke_warnning;

	uint32_t water_warnning;
} rf_433_devs_code_t;

typedef struct {
	uint8_t door_closed;
	uint8_t smoke;
	uint8_t water;
} rf_433_warnning_status_t;

extern void* gw_task_rf433_entry(void*);
extern q_msg_t gw_task_rf433_mailbox;

#endif //__TASK_RF433_H__
