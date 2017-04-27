#ifndef __TASK_LIST_H__
#define __TASK_LIST_H__

#include "../ak/ak.h"
#include "../ak/message.h"

#include "if_console.h"
#include "if_rf24.h"
#include "if_rf433.h"

#include "task_console.h"
#include "task_snmp.h"
#include "task_rf433.h"
#include "task_mqtt.h"
#include "task_firmware.h"
#include "task_if.h"
#include "task_sensor.h"
#include "task_digital_device.h"
#include "task_query_address.h"
#include "task_register_device.h"
#include "task_init_gateway.h"

/* task list MUST BE increase order */
#define GW_TASK_TIMER_ID					0
#define GW_TASK_IF_CONSOLE_ID				1
#define GW_TASK_CONSOLE_ID					2
#define GW_TASK_IF_RF24_ID					3
#define GW_TASK_SNMP_ID						4
#define GW_TASK_IF_RF433_ID					5
#define GW_TASK_RF433_ID					6
#define GW_TASK_MQTT_ID						7
#define GW_TASK_FIRMWARE_ID					8
#define GW_TASK_IF_ID						9
#define GW_TASK_SENSOR_ID					10
#define GW_TASK_DIGITAL_DEVICE_ID           11
#define GW_TASK_QUERY_ADDRESS_ID            12
#define GW_TASK_REGISTER_DEVICE_ID          13
#define GW_TASK_INIT_GATEWAY_ID             14
/* size of task list table */
#define GW_TASK_LIST_LEN					15

extern ak_task_t task_list[];



#endif //__TASK_LIST_H__
