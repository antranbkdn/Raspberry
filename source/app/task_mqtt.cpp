#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ak/ak.h"
#include "../ak/timer.h"

#include "app.h"
#include "app_dbg.h"
#include "app_data.h"
#include "app_if.h"


#include "mqtt_sensor.h"
#include "mqtt_setting.h"
#include "mqtt_control.h"
#include "mqtt_digital_device.h"

#include "task_list.h"
#include "task_mqtt.h"

mqtt_sensor* mqttsensor = NULL;
mqtt_setting* mqttsetting = NULL;
mqtt_control* mqttcontrol = NULL;
mqtt_digital_device* mqttdigitaldevice = NULL;

q_msg_t gw_task_mqtt_mailbox;

void* gw_task_mqtt_entry(void*) {
	task_mask_started();
	wait_all_tasks_started();

	APP_DBG("gw_task_mqtt_entry\n");

	/* config mqtt */
    mqttsensor	= new mqtt_sensor("sensor", MQTT_HOST, MQTT_PORT);
    mqttsetting	= new mqtt_setting("setting", MQTT_HOST, MQTT_PORT);
    mqttcontrol	= new mqtt_control("control", MQTT_HOST, MQTT_PORT);
    mqttdigitaldevice = new mqtt_digital_device ("digital_device", MQTT_HOST, MQTT_PORT);

	while(1){
		while (msg_available(GW_TASK_MQTT_ID)) {
			/* get messge */
			ak_msg_t* msg = rev_msg(GW_TASK_MQTT_ID);

			switch (msg->header->sig) {
			case GW_MQTT_SENSOR_RES:
				APP_DBG("GW_MQTT_SENSOR_RES\n");
				mqttsensor->sensor_public((uint8_t*)"sensor", strlen("sensor"));
				break;

			case GW_MQTT_SETTING_REQ: {
				APP_DBG("GW_MQTT_SETTING_REQ\n");
				int payload_len = msg->header->len;
				char* payload = (char*)malloc(payload_len);
				get_data_dynamic_msg(msg, (uint8_t*)payload, payload_len);
				APP_DBG("SETTING:%s\n", payload);
				free(payload);
			}
				break;

			case GW_MQTT_CONTROL_REQ: {
				APP_DBG("GW_MQTT_CONTROL_REQ\n");
				int sig = 100;
				int payload_len = msg->header->len;
				uint8_t* payload = (uint8_t*)malloc(payload_len);
				get_data_dynamic_msg(msg, (uint8_t*)payload, payload_len);

				APP_DBG("CONTROL:%s\n", payload);
				sig_struct_json(payload, &sig);
				APP_DBG("SIG: %d\n", sig);

				free(payload);
			}
				break;

			case GW_MQTT_SETTING_RES:
				APP_DBG("GW_MQTT_SETTING_RES\n");
				break;

			case GW_MQTT_CONTROL_RES:
				APP_DBG("GW_MQTT_CONTROL_RES\n");
                break;

            case GW_MQTT_DIGITAL_DEVICE: {
                APP_DBG ("GW_MQTT_DIGITAL_DEVICE\n");
                ControlDigital sv_gw_ctl_dgt;
                get_data_dynamic_msg(msg, (uint8_t*)&sv_gw_ctl_dgt, sizeof (ControlDigital));

                uint8_t ID_find = sv_gw_ctl_dgt.DestID;
                slist* slist_found = slist_find(device_list, (void*)&ID_find, is_device_equal_id);
                if (slist_found) {
                    device_t* device_found = (device_t*)slist_found->data;
                    device_found->subdevice[SUB_ID].state = sv_gw_ctl_dgt.StateDigital;

                    ak_msg_t* s_msg = get_common_msg();

                    ControlDigital* gw_dv_ctl_dgt = (ControlDigital*) malloc(sizeof(ControlDigital));
                    gw_dv_ctl_dgt->SourceID = sv_gw_ctl_dgt.SourceID;
                    gw_dv_ctl_dgt->DestID   = sv_gw_ctl_dgt.DestID;
                    gw_dv_ctl_dgt->StateDigital = sv_gw_ctl_dgt.StateDigital;
                    gw_dv_ctl_dgt->seq_num = init_seq_num();

                    set_if_to_address(s_msg, (uint32_t)device_found->address);

                    s_msg->header->payload = gw_dv_ctl_dgt;
                    s_msg->header->len = sizeof (ControlDigital);

                    set_msg_sig(s_msg, GW_DV_DIGITAL_CTL_US);
                    task_post (GW_TASK_DIGITAL_DEVICE_ID, s_msg);
                }
            }
                break;
			default:
				break;
			}

			/* free message */
			free_msg(msg);
		}

		usleep(100);
	}

	return (void*)0;
}
