#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ak/ak.h"

#include "app.h"
#include "app_data.h"
#include "app_dbg.h"

#include "task_list.h"
#include "task_rf433.h"

q_msg_t gw_task_rf433_mailbox;

#define RF433_CODE_FILE_NAME						"/home/pi/rf433_devices.txt"
#define RF433_WARNNING_STATUS_FILE_NAME				"/home/pi/rf433_warnning_status.txt"
#define RF433_WARNNING_STATUS_READABLE_FILE_NAME	"/home/pi/rf433_warnning_status_readable.txt"

static fsm_t fsm_fr433;
static void rf433_idle(ak_msg_t* msg);
static void rf433_record(ak_msg_t* msg);
static void rf433_running(ak_msg_t* msg);

static rf_433_devs_code_t rf_433_devs_code;
static FILE* rf_433_devs_code_file;
static int read_record_code(rf_433_devs_code_t* records_list);
static int write_record_code(rf_433_devs_code_t* records_list);

static uint8_t record_code_type = RECORD_CODE_TYPE_NONE;

static rf_433_warnning_status_t devices_warnning_status;
static FILE* devices_warnning_status_file;
static FILE* devices_warnning_status_readable_file;
static int read_warnning_status(rf_433_warnning_status_t* status);
static int write_warnning_status(rf_433_warnning_status_t* status);

void* gw_task_rf433_entry(void*) {
	task_mask_started();
	wait_all_tasks_started();

	APP_DBG("gw_task_rf433_entry\n");

	FSM(&fsm_fr433, rf433_idle);

	/* check device code exist */
	memset(&rf_433_devs_code, 0, sizeof(rf_433_devs_code_t));

	if (read_record_code(&rf_433_devs_code) >= 0) {
		if (rf_433_devs_code.door_closed != 0			||
				rf_433_devs_code.door_opened != 0		||
				rf_433_devs_code.smoke_warnning != 0	||
				rf_433_devs_code.water_warnning != 0) {

			FSM_TRAN(&fsm_fr433, rf433_running);

			/* reset warnning status */
			devices_warnning_status.door_closed = 1;
			devices_warnning_status.smoke = 0;
			devices_warnning_status.water = 0;

			write_warnning_status(&devices_warnning_status);
		}
	}

	while (1) {

		while (msg_available(GW_TASK_RF433_ID)) {
			/* get messge */
			ak_msg_t* msg = rev_msg(GW_TASK_RF433_ID);

			/* handler message */
			fsm_dispatch(&fsm_fr433, msg);

			/* free message */
			free_msg(msg);
		}

		usleep(100);
	}

	return (void*)0;
}

int read_record_code(rf_433_devs_code_t* records_list) {
	int ret = -1;

	rf_433_devs_code_file = fopen(RF433_CODE_FILE_NAME, "r");

	if (rf_433_devs_code_file == NULL) {
		APP_DBG("can not open %s file\n", RF433_CODE_FILE_NAME);
	}
	else {
		ret = 0;
		APP_DBG("opened %s file\n", RF433_CODE_FILE_NAME);

		fread(records_list, 1, sizeof(rf_433_devs_code_t), rf_433_devs_code_file);

		fclose(rf_433_devs_code_file);
	}

	return ret;
}

int write_record_code(rf_433_devs_code_t* records_list) {
	int ret = -1;

	rf_433_devs_code_file = fopen(RF433_CODE_FILE_NAME, "w");

	if (rf_433_devs_code_file == NULL) {
		APP_DBG("can not open %s file\n", RF433_CODE_FILE_NAME);
	}
	else {
		ret = 0;
		APP_DBG("opened %s file\n", RF433_CODE_FILE_NAME);

		fwrite(records_list, 1, sizeof(rf_433_devs_code_t), rf_433_devs_code_file);

		fclose(rf_433_devs_code_file);
	}

	return ret;
}

int read_warnning_status(rf_433_warnning_status_t* status) {
	int ret = -1;

	devices_warnning_status_file = fopen(RF433_WARNNING_STATUS_FILE_NAME, "w+r");

	if (devices_warnning_status_file == NULL) {
		APP_DBG("can not open %s file\n", RF433_WARNNING_STATUS_FILE_NAME);
	}
	else {
		ret = 0;
		APP_DBG("opened %s file\n", RF433_WARNNING_STATUS_FILE_NAME);

		fread(status, 1, sizeof(rf_433_warnning_status_t), devices_warnning_status_file);

		fclose(devices_warnning_status_file);
	}

	return ret;
}

int write_warnning_status(rf_433_warnning_status_t* status) {
	int ret = -1;

	devices_warnning_status_file = fopen(RF433_WARNNING_STATUS_FILE_NAME, "w");

	if (devices_warnning_status_file == NULL) {
		APP_DBG("can not open %s file\n", RF433_WARNNING_STATUS_FILE_NAME);
	}
	else {
		ret = 0;
		APP_DBG("opened %s file\n", RF433_WARNNING_STATUS_FILE_NAME);

		fwrite(status, 1, sizeof(rf_433_devs_code_t), devices_warnning_status_file);

		fclose(devices_warnning_status_file);
	}

	/* update readable file */
	devices_warnning_status_readable_file = fopen(RF433_WARNNING_STATUS_READABLE_FILE_NAME, "w");

	if (devices_warnning_status_readable_file == NULL) {
		APP_DBG("can not open %s file\n", RF433_WARNNING_STATUS_READABLE_FILE_NAME);
	}
	else {
		ret = 0;
		APP_DBG("opened %s file\n", RF433_WARNNING_STATUS_READABLE_FILE_NAME);

		fprintf(devices_warnning_status_readable_file, "%d\n", status->door_closed);
		fprintf(devices_warnning_status_readable_file, "%d\n", status->smoke);
		fprintf(devices_warnning_status_readable_file, "%d\n", status->water);

		fclose(devices_warnning_status_readable_file);
	}

	return ret;
}

void rf433_idle(ak_msg_t* msg) {
	switch (msg->header->sig) {
	case GW_RF433_ENTRY_RECORD_CODE_REQ:
		APP_DBG("GW_RF433_ENTRY_RECORD_CODE_REQ\n");
		record_code_type = RECORD_CODE_TYPE_NONE;
		FSM_TRAN(&fsm_fr433, rf433_record);
		break;

	default:
		break;
	}
}

void rf433_record(ak_msg_t* msg) {
	switch (msg->header->sig) {
	case GW_RF433_RECV_NOTIFY: {
		uint32_t code;
		get_data_dynamic_msg(msg, (uint8_t*)&code, sizeof(uint32_t));

		switch (record_code_type) {
		case RECORD_CODE_TYPE_DOOR_OPENED:
			APP_DBG("record code:%d\n", code);
			read_record_code(&rf_433_devs_code);
			rf_433_devs_code.door_opened = code;
			write_record_code(&rf_433_devs_code);
			record_code_type = RECORD_CODE_TYPE_NONE;
			break;

		case RECORD_CODE_TYPE_DOOR_CLOSED:
			APP_DBG("record code:%d\n", code);
			read_record_code(&rf_433_devs_code);
			rf_433_devs_code.door_closed = code;
			write_record_code(&rf_433_devs_code);
			record_code_type = RECORD_CODE_TYPE_NONE;
			break;

		case RECORD_CODE_TYPE_SMOKE_WARNNING:
			APP_DBG("record code:%d\n", code);
			read_record_code(&rf_433_devs_code);
			rf_433_devs_code.smoke_warnning = code;
			write_record_code(&rf_433_devs_code);
			record_code_type = RECORD_CODE_TYPE_NONE;
			break;

		case RECORD_CODE_TYPE_WATER_WARNNING:
			APP_DBG("record code:%d\n", code);
			read_record_code(&rf_433_devs_code);
			rf_433_devs_code.water_warnning = code;
			write_record_code(&rf_433_devs_code);
			record_code_type = RECORD_CODE_TYPE_NONE;
			break;

		default:
			break;
		}
	}
		break;

	case GW_RF433_EXIT_RECORD_CODE_REQ:
		APP_DBG("GW_RF433_EXIT_RECORD_CODE_REQ\n");
		record_code_type = RECORD_CODE_TYPE_NONE;
		if (read_record_code(&rf_433_devs_code) >= 0) {
			if (rf_433_devs_code.door_closed != 0			||
					rf_433_devs_code.door_opened != 0		||
					rf_433_devs_code.smoke_warnning != 0	||
					rf_433_devs_code.water_warnning != 0) {

				FSM_TRAN(&fsm_fr433, rf433_running);
			}
		}
		break;

	case GW_RF433_RECORD_DOOR_OPENED_CODE_REQ:
		APP_DBG("GW_RF433_RECORD_DOOR_OPENED_CODE_REQ\n");
		record_code_type = RECORD_CODE_TYPE_DOOR_OPENED;
		break;

	case GW_RF433_RECORD_DOOR_CLOSED_CODE_REQ:
		APP_DBG("GW_RF433_RECORD_DOOR_CLOSED_CODE_REQ\n");
		record_code_type = RECORD_CODE_TYPE_DOOR_CLOSED;
		break;

	case GW_RF433_RECORD_SMOKE_WARNNING_CODE_REQ:
		APP_DBG("GW_RF433_RECORD_SMOKE_WARNNING_CODE_REQ\n");
		record_code_type = RECORD_CODE_TYPE_SMOKE_WARNNING;
		break;

	case GW_RF433_RECORD_WATER_WARNNING_CODE_REQ:
		APP_DBG("GW_RF433_RECORD_WATER_WARNNING_CODE_REQ\n");
		record_code_type = RECORD_CODE_TYPE_WATER_WARNNING;
		break;

	default:
		break;
	}
}

void rf433_running(ak_msg_t* msg) {
	switch (msg->header->sig) {
	case GW_RF433_RECV_NOTIFY: {
		uint32_t code;
		get_data_dynamic_msg(msg, (uint8_t*)&code, sizeof(uint32_t));

		if (code == rf_433_devs_code.door_opened) {
			APP_PRINT("WARNNING: DOOR IS OPENED !\n");
			read_warnning_status(&devices_warnning_status);
			if (devices_warnning_status.door_closed != 0) {
				devices_warnning_status.door_closed = 0;
				write_warnning_status(&devices_warnning_status);
			}
		}
		else if (code == rf_433_devs_code.door_closed) {
			APP_PRINT("WARNNING: DOOR IS CLOSED !\n");
			read_warnning_status(&devices_warnning_status);
			if (devices_warnning_status.door_closed != 1) {
				devices_warnning_status.door_closed = 1;
				write_warnning_status(&devices_warnning_status);
			}
		}
		else if (code == rf_433_devs_code.smoke_warnning) {
			APP_PRINT("WARNNING: SMOKE !\n");
			read_warnning_status(&devices_warnning_status);
			if (devices_warnning_status.smoke != 1) {
				devices_warnning_status.smoke = 1;
				write_warnning_status(&devices_warnning_status);
			}
			timer_set(GW_TASK_RF433_ID, GW_RF433_SMOKE_TIMER_WARNNING, GW_RF433_TIMER_SMOKE_WARNNING_CHECK_INTERVAL, TIMER_ONE_SHOT);
		}
		else if (code == rf_433_devs_code.water_warnning) {
			APP_PRINT("WARNNING: WATER !\n");
			read_warnning_status(&devices_warnning_status);
			if (devices_warnning_status.water != 1) {
				devices_warnning_status.water = 1;
				write_warnning_status(&devices_warnning_status);
			}
			timer_set(GW_TASK_RF433_ID, GW_RF433_WATER_TIMER_WARNNING, GW_RF433_TIMER_WATER_WARNNING_CHECK_INTERVAL, TIMER_ONE_SHOT);
		}
	}
		break;

	case GW_RF433_SMOKE_TIMER_WARNNING:
		APP_DBG("GW_RF433_SMOKE_TIMER_WARNNING\n");
		read_warnning_status(&devices_warnning_status);
		if (devices_warnning_status.smoke != 0) {
			devices_warnning_status.smoke = 0;
			write_warnning_status(&devices_warnning_status);
		}
		break;

	case GW_RF433_WATER_TIMER_WARNNING:
		APP_DBG("GW_RF433_WATER_TIMER_WARNNING\n");
		read_warnning_status(&devices_warnning_status);
		if (devices_warnning_status.water != 0) {
			devices_warnning_status.water = 0;
			write_warnning_status(&devices_warnning_status);
		}
		break;

	case GW_RF433_ENTRY_RECORD_CODE_REQ:
		APP_DBG("GW_RF433_ENTRY_RECORD_CODE_REQ\n");
		record_code_type = RECORD_CODE_TYPE_NONE;
		FSM_TRAN(&fsm_fr433, rf433_record);
		break;

	default:
		break;
	}
}
