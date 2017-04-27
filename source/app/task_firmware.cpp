#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ak/ak.h"
#include "../ak/timer.h"

#include "../common/firmware.h"

#include "app.h"
#include "app_if.h"
#include "app_dbg.h"
#include "app_data.h"

#include "task_list.h"
#include "task_list_if.h"
#include "task_firmware.h"
#include "task_if.h"

q_msg_t gw_task_firmware_mailbox;

#define FILE_NAME "/home/pi/facc_firmware/facc_application.bin"

static uint32_t bin_index;
firmware_header_t current_firmware_info;
firmware_header_t file_firmware_info;

void* gw_task_firmware_entry(void*) {
	task_mask_started();
	wait_all_tasks_started();

	APP_DBG("gw_task_firmware_entry\n");

	while (1) {
		while (msg_available(GW_TASK_FIRMWARE_ID)) {
			/* get messge */
			ak_msg_t* msg = rev_msg(GW_TASK_FIRMWARE_ID);

			/* handler message */
			switch (msg->header->sig) {
			case GW_FIRMWARE_UPDATE_COMPLETED: {
				APP_DBG("GW_FIRMWARE_UPDATE_COMPLETED\n");
			}
				break;

			case GW_FIRMWARE_PACKED_TIMEOUT: {
				APP_DBG("GW_FIRMWARE_PACKED_TIMEOUT\n");
			}
				break;

			case GW_FIRMWARE_OTA_REQ: {
				APP_DBG("GW_FIRMWARE_OTA_REQ\n");
				ak_msg_t* s_msg = get_pure_msg();
				set_msg_sig(s_msg, GW_IF_PURE_MSG_OUT);

				set_if_type(s_msg, IF_TYPE_RF24);
				set_if_task_id(s_msg, AC_TASK_FIRMWARE_ID);
				set_if_sig(s_msg, AC_FIRMWARE_CURRENT_INFO_REQ);

				task_post(GW_TASK_IF_ID, s_msg);
			}
				break;

			case GW_FIRMWARE_CURRENT_INFO_RES: {
				APP_DBG("GW_FIRMWARE_CURRENT_INFO_RES\n");
				get_data_common_msg(msg, (uint8_t*)&current_firmware_info, sizeof(firmware_header_t));
				APP_DBG("current firmware checksum: %04x\n", current_firmware_info.checksum);
				APP_DBG("current firmware bin_len: %d\n", current_firmware_info.bin_len);

				firmware_get_info(&file_firmware_info, FILE_NAME);
				APP_DBG("file firmware checksum: %04x\n", file_firmware_info.checksum);
				APP_DBG("file firmware bin_len: %d\n", file_firmware_info.bin_len);

				if (current_firmware_info.checksum == file_firmware_info.checksum		\
						&& current_firmware_info.bin_len == file_firmware_info.bin_len) {
					APP_DBG("USER_NO_NEED_TO_UPDATE\n");
				}
				else {
					ak_msg_t* s_msg = get_common_msg();
					set_msg_sig(s_msg, GW_IF_COMMON_MSG_OUT);

					set_if_type(s_msg, IF_TYPE_RF24);
					set_if_task_id(s_msg, AC_TASK_FIRMWARE_ID);
					set_if_sig(s_msg, AC_FIRMWARE_UPDATE_REQ);
					set_if_data_common_msg(s_msg, (uint8_t*)&file_firmware_info, sizeof(firmware_header_t));

					task_post(GW_TASK_IF_ID, s_msg);
				}
			}
				break;

			case GW_FIRMWARE_UPDATE_RES_OK: {
				APP_DBG("GW_FIRMWARE_UPDATE_RES_OK\n");
				bin_index = 0;
				APP_DBG("USER_START_TRANSFER_FIRMWARE\n");

				ak_msg_t* s_msg = get_pure_msg();
				set_msg_sig(s_msg, GW_FIRMWARE_TRANFER_REQ);
				task_post(GW_TASK_FIRMWARE_ID, s_msg);
			}
				break;

			case GW_FIRMWARE_TRANFER_REQ: {
				uint8_t data_temp[AK_COMMON_MSG_DATA_SIZE];
				uint32_t remain;
				uint8_t len;
				memset(data_temp, 0, AK_COMMON_MSG_DATA_SIZE);
				remain = file_firmware_info.bin_len - bin_index;
				if (file_firmware_info.bin_len - bin_index <= AK_COMMON_MSG_DATA_SIZE) {
					len = (uint8_t)remain;
				}
				else {
					len = AK_COMMON_MSG_DATA_SIZE;
				}

				firmware_read(data_temp, bin_index, len, FILE_NAME);
				bin_index += AK_COMMON_MSG_DATA_SIZE;

				if (bin_index <= file_firmware_info.bin_len) {
					timer_set(GW_TASK_FIRMWARE_ID, GW_FIRMWARE_PACKED_TIMEOUT, GW_TIMER_FIRMWARE_PACKED_TIMEOUT_INTERVAL, TIMER_ONE_SHOT);
				}

				APP_DBG("TRANSFERED:%d\n", bin_index);

				ak_msg_t* s_msg = get_common_msg();

				set_if_type(s_msg, IF_TYPE_RF24);
				set_if_task_id(s_msg, AC_TASK_FIRMWARE_ID);
				set_if_sig(s_msg, AC_FIRMWARE_TRANSFER_REQ);
				set_if_data_common_msg(s_msg, data_temp, AK_COMMON_MSG_DATA_SIZE);

				set_msg_sig(s_msg, GW_IF_COMMON_MSG_OUT);
				task_post(GW_TASK_IF_ID, s_msg);
			}
				break;

			case GW_FIRMWARE_TRANSFER_RES_OK: {
				timer_remove_attr(GW_TASK_FIRMWARE_ID, GW_FIRMWARE_PACKED_TIMEOUT);
				timer_set(GW_TASK_FIRMWARE_ID, GW_FIRMWARE_TRANFER_REQ, GW_FIRMWARE_TRANFER_PACKET_PENDING_INTERVAL, TIMER_ONE_SHOT);
			}
				break;

			case GW_FIRMWARE_AC_INTERNAL_UPDATE_REQ: {
				APP_DBG("GW_FIRMWARE_AC_INTERNAL_UPDATE_REQ\n");

				timer_remove_attr(GW_TASK_FIRMWARE_ID, GW_FIRMWARE_PACKED_TIMEOUT);

				ak_msg_t* s_msg = get_pure_msg();

				set_if_type(s_msg, IF_TYPE_RF24);
				set_if_task_id(s_msg, AC_TASK_FIRMWARE_ID);
				set_if_sig(s_msg, AC_FIRMWARE_INTERNAL_UPDATE_RES_OK);

				set_msg_sig(s_msg, GW_IF_PURE_MSG_OUT);
				task_post(GW_TASK_IF_ID, s_msg);
			}
				break;

			case GW_FIRMWARE_UPDATE_BUSY: {
				APP_DBG("GW_FIRMWARE_UPDATE_BUSY\n");
			}
				break;

			case GW_FIRMWARE_TRANSFER_CHECKSUM_ERR: {
				APP_DBG("GW_FIRMWARE_TRANSFER_CHECKSUM_ERR\n");
				timer_remove_attr(GW_TASK_FIRMWARE_ID, GW_FIRMWARE_PACKED_TIMEOUT);
			}
				break;

			default:
				break;
			}

			free_msg(msg);
		}

		usleep(100);
	}

	return (void*)0;
}
