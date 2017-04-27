#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ak/ak.h"

#include "../sys/sys_dbg.h"

#include "app.h"
#include "app_if.h"
#include "app_data.h"
#include "app_dbg.h"

#include "task_list.h"
#include "task_list_if.h"
#include "task_snmp.h"
#include "if_rf24.h"

#include "task_if.h"

q_msg_t gw_task_if_mailbox;

void* gw_task_if_entry(void*) {
	task_mask_started();
	wait_all_tasks_started();

	APP_DBG("gw_task_if_entry\n");

	while (1) {
		while (msg_available(GW_TASK_IF_ID)) {
			/* get messge */
			ak_msg_t* msg = rev_msg(GW_TASK_IF_ID);

			/* handler message */
			if (msg->header->if_type == IF_TYPE_RF24) {
				switch (msg->header->sig) {
				case GW_IF_PURE_MSG_IN: {
					msg_inc_ref_count(msg);
					set_msg_sig(msg, msg->header->if_sig);
					task_post(msg->header->if_task_id, msg);
				}
					break;

				case GW_IF_PURE_MSG_OUT: {
					msg_inc_ref_count(msg);
					set_msg_sig(msg, GW_RF24_IF_PURE_MSG_OUT);
					task_post(GW_TASK_IF_RF24_ID, msg);
				}
					break;

				case GW_IF_COMMON_MSG_IN: {
					msg_inc_ref_count(msg);
					set_msg_sig(msg, msg->header->if_sig);
					task_post(msg->header->if_task_id, msg);
				}
					break;

				case GW_IF_COMMON_MSG_OUT: {
					msg_inc_ref_count(msg);
					set_msg_sig(msg, GW_RF24_IF_COMMON_MSG_OUT);
					task_post(GW_TASK_IF_RF24_ID, msg);
				}
					break;

                case GW_IF_COMMON_MSG_OUT_DIRECT: {
                    msg_inc_ref_count(msg);
                    set_msg_sig(msg, GW_RF24_IF_COMMON_MSG_OUT_DIRECT);
                    task_post(GW_TASK_IF_RF24_ID, msg);
                }
                    break;

				default:
					break;
				}
			}

			/* free message */
			free_msg(msg);
		}

		usleep(100);
	}

	return (void*)0;
}
