#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "app_if.h"
#include "app_data.h"
#include "app_dbg.h"

#include "task_list.h"
#include "task_list_if.h"
#include "task_snmp.h"
#include "if_rf24.h"

#include "task_sensor.h"

q_msg_t gw_task_sensor_mailbox;

void* gw_task_sensor_entry(void*) {
	task_mask_started();
	wait_all_tasks_started();

	APP_DBG("gw_task_sensor_entry\n");

	/* start timer to query AC sensor data */
	//timer_set(GW_TASK_SENSOR_ID, GW_AC_SENSOR_REQ, GW_SENSOR_AC_TIMER_INTERVAL, TIMER_PERIODIC);

	while (1) {
		while (msg_available(GW_TASK_SENSOR_ID)) {
			/* get messge */
			ak_msg_t* msg = rev_msg(GW_TASK_SENSOR_ID);

			/* handler message */
			switch (msg->header->sig) {
			case GW_AC_SENSOR_REQ: {
				ak_msg_t* s_msg = get_pure_msg();

				set_if_type(s_msg, IF_TYPE_RF24);
				set_if_sig(s_msg, AC_SENSOR_REPORT_REQ);
				set_if_task_id(s_msg, AC_TASK_SENSOR_ID);

				set_msg_sig(s_msg, GW_IF_PURE_MSG_OUT);
				task_post(GW_TASK_IF_ID, s_msg);
			}
				break;

			case GW_AC_SENSOR_RES: {
				msg_inc_ref_count(msg);
				set_msg_sig(msg, GW_SNMP_AC_SENSOR_REP);
				task_post(GW_TASK_SNMP_ID, msg);
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
