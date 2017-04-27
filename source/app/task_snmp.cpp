#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "app_data.h"
#include "app_dbg.h"

#include "task_list.h"
#include "task_snmp.h"
#include "if_rf24.h"

q_msg_t gw_task_snmp_mailbox;

#define RF_DATA_FILE_NAME				"/home/pi/airs_cond_data.txt"
#define RF_SETTING_FILE_NAME			"/home/pi/airs_cond_setting_data.txt"

static FILE* rf_data_file;
static FILE* rf_setting_file;

static void update_rf_data_file(sensor_packet_t* sensor_data);
static void update_rf_setting_file(app_setting_t* setting_data);

void* gw_task_snmp_entry(void*) {
	task_mask_started();
	wait_all_tasks_started();

	APP_DBG("gw_task_snmp_entry\n");

	while (1) {

		while (msg_available(GW_TASK_SNMP_ID)) {
			/* get messge */
			ak_msg_t* msg = rev_msg(GW_TASK_SNMP_ID);

			/* handler message */
			switch (msg->header->sig) {
			case GW_SNMP_AC_SENSOR_REP: {
				APP_DBG("GW_SNMP_AC_SENSOR_REP\n");

				sensor_packet_t sensor_data;
				get_data_common_msg(msg, (uint8_t*)&sensor_data, sizeof(sensor_packet_t));
				update_rf_data_file(&sensor_data);
			}
				break;

			case GW_SNMP_AC_SETTINGS_REP: {
				APP_DBG("GW_SNMP_AC_SETTINGS_REP\n");

				app_setting_t setting_data;
				get_data_common_msg(msg, (uint8_t*)&setting_data, sizeof(app_setting_t));
				update_rf_setting_file(&setting_data);
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

void update_rf_data_file(sensor_packet_t* sensor_data) {
	/* update data to file */
	rf_data_file = fopen(RF_DATA_FILE_NAME, "w");
	if (rf_data_file == NULL) {
		APP_DBG("can not open %s file\n", RF_DATA_FILE_NAME);
	}
	else {
		APP_DBG("opened %s file\n", RF_DATA_FILE_NAME);
	}

	APP_DBG("remote_temperature:%d\n",sensor_data->remote_temperature);
	APP_DBG("humindity:%d\n",sensor_data->humindity);
	APP_DBG("air_cond_current[0]:%d\n",sensor_data->air_cond_current[0]);
	APP_DBG("air_cond_current[1]:%d\n",sensor_data->air_cond_current[1]);
	APP_DBG("air_cond_current[2]:%d\n",sensor_data->air_cond_current[2]);
	APP_DBG("air_cond_current[3]:%d\n",sensor_data->air_cond_current[3]);

	fprintf(rf_data_file, "%d\n",sensor_data->remote_temperature);
	fprintf(rf_data_file, "%d\n",sensor_data->air_cond_current[0]);
	fprintf(rf_data_file, "%d\n",sensor_data->air_cond_current[1]);
	fprintf(rf_data_file, "%d\n",sensor_data->air_cond_current[2]);
	fprintf(rf_data_file, "%d\n",sensor_data->air_cond_current[3]);
	fprintf(rf_data_file, "%d\n",sensor_data->humindity);

	fclose(rf_data_file);
}

void update_rf_setting_file(app_setting_t* setting_data) {
	/* update data to file */
	rf_setting_file = fopen(RF_SETTING_FILE_NAME, "w");

	if (rf_setting_file == NULL) {
		APP_DBG("can not open %s file\n", RF_SETTING_FILE_NAME);
	}
	else {
		APP_DBG("opened %s file\n", RF_SETTING_FILE_NAME);
	}

	APP_DBG("%d\n", setting_data->time_air_range);
	APP_DBG("%d\n", setting_data->milestone_temp_cool);
	APP_DBG("%d\n", setting_data->milestone_temp_normal);
	APP_DBG("%d\n", setting_data->milestone_temp_hot);

	fprintf(rf_setting_file, "%d\n", setting_data->time_air_range);
	fprintf(rf_setting_file, "%d\n", setting_data->milestone_temp_cool);
	fprintf(rf_setting_file, "%d\n", setting_data->milestone_temp_normal);
	fprintf(rf_setting_file, "%d\n", setting_data->milestone_temp_hot);

	fclose(rf_setting_file);
}
