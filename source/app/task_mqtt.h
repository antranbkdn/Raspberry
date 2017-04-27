#ifndef __TASK_MQTT_H__
#define __TASK_MQTT_H__

#include <mosquittopp.h>
#include "../ak/message.h"
#include "mqtt/mqtt_control.h"
#include "mqtt/mqtt_sensor.h"
#include "mqtt/mqtt_setting.h"
#include "mqtt/mqtt_digital_device.h"

#define MQTT_HOST "10.133.159.15"
#define MQTT_PORT 1883

extern q_msg_t gw_task_mqtt_mailbox;
extern void* gw_task_mqtt_entry(void*);

extern mqtt_sensor*	mqttsensor;
extern mqtt_setting*	mqttsetting;
extern mqtt_control*	mqttcontrol;
extern mqtt_digital_device* mqttdigitaldevice;

#define MQTT_BUFFER_MAX_SIZE        128

#endif //__TASK_MQTT_H__
