#include "../ak/timer.h"

#include "task_list.h"

ak_task_t task_list[] = {
	{	GW_TASK_TIMER_ID,		TASK_PRI_LEVEL_1,	timer_entry					,	&timer_mailbox					,	"timer service"		},
	{	GW_TASK_IF_CONSOLE_ID,	TASK_PRI_LEVEL_1,	gw_task_if_console_entry	,	&gw_task_if_console_mailbox		,	"terminal gate"		},
	{	GW_TASK_CONSOLE_ID,		TASK_PRI_LEVEL_1,	gw_task_console_entry		,	&gw_task_console_mailbox		,	"handle commands"	},
	{	GW_TASK_IF_RF24_ID,		TASK_PRI_LEVEL_1,	gw_task_if_rf24_entry		,	&gw_task_if_rf24_mailbox		,	"if rf24"			},
	{	GW_TASK_SNMP_ID,		TASK_PRI_LEVEL_1,	gw_task_snmp_entry			,	&gw_task_snmp_mailbox			,	"task snmp"			},
	{	GW_TASK_IF_RF433_ID,	TASK_PRI_LEVEL_1,	gw_task_if_rf433_entry		,	&gw_task_if_rf433_mailbox		,	"rf433 gate"		},
	{	GW_TASK_RF433_ID,		TASK_PRI_LEVEL_1,	gw_task_rf433_entry			,	&gw_task_rf433_mailbox			,	"task rf433"		},
	{	GW_TASK_MQTT_ID,		TASK_PRI_LEVEL_1,	gw_task_mqtt_entry			,	&gw_task_mqtt_mailbox			,	"task mqtt"			},
	{	GW_TASK_FIRMWARE_ID,	TASK_PRI_LEVEL_1,	gw_task_firmware_entry		,	&gw_task_firmware_mailbox		,	"task firmware"		},
	{	GW_TASK_IF_ID,			TASK_PRI_LEVEL_1,	gw_task_if_entry			,	&gw_task_if_mailbox				,	"task if"			},
    {	GW_TASK_SENSOR_ID,		TASK_PRI_LEVEL_1,	gw_task_sensor_entry		,	&gw_task_sensor_mailbox			,	"task sensor"		},
    {   GW_TASK_DIGITAL_DEVICE_ID, TASK_PRI_LEVEL_1,gw_task_digital_device_entry,   &gw_task_digital_device_mailbox ,   "task digital device"},
    {   GW_TASK_QUERY_ADDRESS_ID,  TASK_PRI_LEVEL_1,gw_task_query_address_entry,    &gw_task_query_address_mailbox  ,   "task query address" },
    {   GW_TASK_REGISTER_DEVICE_ID,TASK_PRI_LEVEL_1,gw_task_register_device_entry,  &gw_task_register_device_mailbox,   "task register"      },
    {   GW_TASK_INIT_GATEWAY_ID, TASK_PRI_LEVEL_1, gw_task_init_gateway_entry   ,   &gw_task_init_gateway_mailbox   ,   "task init gateway"  }
};

