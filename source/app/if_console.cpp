#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ak/ak.h"

#include "app.h"
#include "app_if.h"
#include "app_dbg.h"

#include "if_console.h"
#include "task_list.h"
#include "task_list_if.h"
#include "task_if.h"

#define FACC_LOGIN_OPT					(0x00)
#define INTERNAL_LOGIN_OPT				(0x01)

q_msg_t gw_task_if_console_mailbox;

static unsigned char cmd_buf[CMD_BUFFER_SIZE];
static int i_get_command(unsigned char* cmd_buf);

static uint32_t fw_option = INTERNAL_LOGIN_OPT;
const char* __internal	= "__internal\n";
const char* __facc		= "__facc\n";

void* gw_task_if_console_entry(void*) {
	task_mask_started();
	wait_all_tasks_started();

	APP_DBG("gw_task_if_console_entry\n");

	while (1) {
		if (i_get_command(cmd_buf) == 0) {
			/*check entry setting */
			if (strcmp((const char*)cmd_buf, __internal) == 0) {
				fw_option = INTERNAL_LOGIN_OPT;
			}
			/*check exit setting */
			else if (strcmp((const char*)cmd_buf, __facc) == 0) {
				fw_option = FACC_LOGIN_OPT;
			}
			else {
				/* send command to RF thread */
				switch (fw_option) {
				case FACC_LOGIN_OPT: {
					ak_msg_t* s_msg = get_common_msg();

					set_if_type(s_msg, IF_TYPE_RF24);
					set_if_task_id(s_msg, AC_TASK_SHELL_ID);
					set_if_sig(s_msg, AC_SHELL_REMOTE_CMD);

					set_data_common_msg(s_msg, cmd_buf, strlen((const char*)cmd_buf));

					set_msg_sig(s_msg, GW_IF_COMMON_MSG_OUT);
					task_post(GW_TASK_IF_ID, s_msg);
				}
					break;

				case INTERNAL_LOGIN_OPT: {
					ak_msg_t* s_msg = get_dymanic_msg();
					set_msg_sig(s_msg, GW_CONSOLE_INTERNAL_LOGIN_CMD);
					set_data_dynamic_msg(s_msg, cmd_buf, strlen((const char*)cmd_buf));
					task_post(GW_TASK_CONSOLE_ID, s_msg);
				}
					break;

				default:
					break;
				}
			}

			/* clean command buffer */
			memset(cmd_buf, 0, CMD_BUFFER_SIZE);
		}

		usleep(100);
	}
	return (void*)0;
}

int i_get_command(unsigned char* cmd_buf) {
	unsigned char c = 0;
	unsigned char index = 0;

	do {
		c = getchar();
		cmd_buf[index++] = c;
		if (index > CMD_BUFFER_SIZE ) {
			index = 0;
			printf("ERROR: buffer overload !\n");
			return (-1);
		}

		/* sleep 1us */
		usleep(1);

	} while (c != '\n');

	cmd_buf[index] = 0;
	return (0);
}
