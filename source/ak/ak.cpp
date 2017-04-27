/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   12/01/2017
 * @brief:  Main defination of active kernel
 ******************************************************************************
**/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

#include "ak.h"
#include "ak_dbg.h"
#include "message.h"

#include "../sys/sys_dbg.h"

#include "../app/task_list.h"

static uint32_t ak_thread_table_len = 0;

static pthread_mutex_t ak_thread_mt_started;
static uint32_t ak_thread_started = 0;

int main() {
	ak_thread_table_len = GW_TASK_LIST_LEN;
	ak_thread_started = ak_thread_table_len;

	task_init();

	AK_MSG_DBG("TASK LIST LEN: %d\n", ak_thread_table_len);

	for (uint32_t index = 0; index < ak_thread_table_len; index++) {
		/* init mailbox */
		q_msg_init(task_list[index].mailbox);

		/* create task */
		pthread_create(&(task_list[index].pthread), NULL, task_list[index].task, NULL);
		printf("ID:%08x\tCREATE: %s\n",(unsigned int)task_list[index].pthread, task_list[index].info);
	}

	for (uint32_t index = 0; index < ak_thread_table_len; index++) {
		pthread_join(task_list[index].pthread, NULL);
	}

	return 0;
}

void task_mask_started() {
	pthread_mutex_lock(&ak_thread_mt_started);
	ak_thread_started --;
	pthread_mutex_unlock(&ak_thread_mt_started);
}

void wait_all_tasks_started() {
	uint32_t num_tasks_started;

	pthread_mutex_lock(&ak_thread_mt_started);
	num_tasks_started = ak_thread_started;
	pthread_mutex_unlock(&ak_thread_mt_started);

	while (num_tasks_started > 0) {
		pthread_mutex_lock(&ak_thread_mt_started);
		num_tasks_started = ak_thread_started;
		pthread_mutex_unlock(&ak_thread_mt_started);

		usleep(10);
	}
}

ak_msg_t* get_pure_msg() {
	ak_msg_t* g_msg = (ak_msg_t*)malloc(sizeof(ak_msg_t));
	if (g_msg == NULL) {
		FATAL("AK", 0x01);
	}

	g_msg->header = (header_t*)malloc(sizeof(header_t));
	if (g_msg->header == NULL) {
		FATAL("AK", 0x02);
	}

	g_msg->header->ref_count = 1;
	g_msg->header->type = PURE_MSG_TYPE;
	g_msg->header->len = 0;
	g_msg->header->payload = NULL;

	return g_msg;
}

ak_msg_t* get_dymanic_msg() {
	ak_msg_t* g_msg = (ak_msg_t*)malloc(sizeof(ak_msg_t));
	if (g_msg == NULL) {
		FATAL("AK", 0x02);
	}

	g_msg->header = (header_t*)malloc(sizeof(header_t));
	if (g_msg->header == NULL) {
		FATAL("AK", 0x03);
	}

	g_msg->header->ref_count = 1;
	g_msg->header->type = DYNAMIC_MSG_TYPE;
	g_msg->header->len = 0;
	g_msg->header->payload = NULL;

	AK_MSG_DBG("[MSG] get msg:%08x\theader:%08x\n", g_msg, g_msg->header);

	return g_msg;
}

ak_msg_t* get_common_msg() {
	ak_msg_t* g_msg = (ak_msg_t*)malloc(sizeof(ak_msg_t));
	if (g_msg == NULL) {
		FATAL("AK", 0x04);
	}

	g_msg->header = (header_t*)malloc(sizeof(header_t));
	if (g_msg->header == NULL) {
		FATAL("AK", 0x05);
	}

	g_msg->header->ref_count = 1;
	g_msg->header->type = COMMON_MSG_TYPE;
	g_msg->header->len = 0;
	g_msg->header->payload = NULL;

	AK_MSG_DBG("[MSG] get msg:%08x\theader:%08x\n", g_msg, g_msg->header);

	return g_msg;
}

ak_msg_t* ak_memcpy_msg(ak_msg_t* src) {
	ak_msg_t* ret_msg = AK_MSG_NULL;

	if (src == AK_MSG_NULL) {
		FATAL("AK", 0x06);
	}

	switch (get_msg_type(src)) {
	case PURE_MSG_TYPE: {
		/* get pure message */
		ret_msg = get_pure_msg();

		/* copy message header */
		memcpy(ret_msg->header, src->header, sizeof(header_t));
	}
		break;

	case DYNAMIC_MSG_TYPE: {
		/* get dynamic message */
		ret_msg = get_dymanic_msg();

		/* copy message header */
		memcpy(ret_msg->header, src->header, sizeof(header_t));

		/* copy dynamic payload */
		set_data_dynamic_msg(ret_msg, (uint8_t*)src->header->payload, src->header->len);
	}
		break;

	case COMMON_MSG_TYPE: {
		/* get common message */
		ret_msg = get_common_msg();

		/* copy message header */
		memcpy(ret_msg->header, src->header, sizeof(header_t));

		/* copy common payload */
		set_data_common_msg(ret_msg, (uint8_t*)src->header->payload, src->header->len);
	}
		break;

	default:
		break;
	}

	return ret_msg;
}

void ak_free_msg(ak_msg_t* msg) {
	if (msg != NULL) {
		q_msg_free(msg);
	}
	else {
		FATAL("AK", 0x07);
	}
}

void set_msg_sig(ak_msg_t* msg, uint32_t sig) {
	if (msg != NULL) {
		msg->header->sig = sig;
	}
	else {
		FATAL("AK", 0x08);
	}
}

void set_msg_task_id(ak_msg_t* msg, uint32_t task_id) {
	if (msg != NULL) {
		msg->header->task_id = task_id;
	}
	else {
		FATAL("AK", 0x09);
	}
}

void set_if_task_id(ak_msg_t* msg, uint32_t id) {
	if (msg != NULL) {
		msg->header->if_task_id = id;
	}
	else {
		FATAL("AK", 0x0A);
	}
}

void set_if_type(ak_msg_t* msg, uint32_t type) {
	if (msg != NULL) {
		msg->header->if_type = type;
	}
	else {
		FATAL("AK", 0x0B);
	}
}

void set_if_sig(ak_msg_t* msg, uint32_t sig) {
	if (msg != NULL) {
		msg->header->if_sig = sig;
	}
	else {
		FATAL("AK", 0x0B);
	}
}

void set_if_to_address(ak_msg_t* msg, uint32_t to_address) {
    if (msg != NULL) {
        msg->header->if_to_address = to_address;
    }
    else {
        FATAL("AK", 0x0A);
    }
}

void set_if_data_common_msg(ak_msg_t* msg, uint8_t* data, uint32_t len) {
	set_data_common_msg(msg, data, len);
}

void set_data_common_msg(ak_msg_t* msg, uint8_t* data, uint32_t len) {
	if (msg != NULL) {
		if (msg->header->type == COMMON_MSG_TYPE) {
			msg->header->payload = (uint8_t*)malloc((size_t)len);
			if (msg->header->payload == NULL) {
				FATAL("AK", 0x0D);
			}
			else {
				if (len > AK_COMMON_MSG_DATA_SIZE) {
					FATAL("AK", 0x0E);
				}
				else {
					msg->header->len = len;
					memcpy(msg->header->payload, data, len);
					AK_MSG_DBG("[MSG] set payload:%08x\n", msg->header->payload);
					AK_MSG_DBG("[MSG] set payload len:%d\n", msg->header->len);
				}
			}
		}
		else {
			FATAL("AK", 0x05);
		}
	}
	else {
		FATAL("AK", 0x03);
	}
}

void get_data_common_msg(ak_msg_t* msg, uint8_t* data, uint32_t len) {
	if (msg != NULL) {
		if (msg->header->type == COMMON_MSG_TYPE) {
			if (msg->header->payload == NULL ||
					msg->header->len < len) {
				FATAL("AK", 0x0F);
			}
			else {
				memcpy(data, msg->header->payload, len);
				AK_MSG_DBG("[MSG] get payload:%08x\n", msg->header->payload);
			}
		}
		else {
			FATAL("AK", 0x10);
		}
	}
	else {
		FATAL("AK", 0x11);
	}
}

uint8_t get_data_len_common_msg(ak_msg_t* msg) {
	uint8_t ret = 0;
	if (msg != NULL) {
		if (msg->header->type == COMMON_MSG_TYPE) {
			ret = msg->header->len;
		}
		else {
			FATAL("AK", 0x12);
		}
	}
	else {
		FATAL("AK", 0x13);
	}
	return ret;
}

void set_data_dynamic_msg(ak_msg_t* msg, uint8_t* data, uint32_t len) {
	if (msg != NULL) {
		if (msg->header->type == DYNAMIC_MSG_TYPE) {
			msg->header->payload = (uint8_t*)malloc((size_t)len);
			if (msg->header->payload == NULL) {
				FATAL("AK", 0x14);
			}
			else {
				msg->header->len = len;
				memcpy(msg->header->payload, data, len);
				AK_MSG_DBG("[MSG] set payload:%08x\n", msg->header->payload);
				AK_MSG_DBG("[MSG] set payload len:%d\n", msg->header->len);
			}
		}
		else {
			FATAL("AK", 0x15);
		}
	}
	else {
		FATAL("AK", 0x16);
	}
}

void get_data_dynamic_msg(ak_msg_t* msg, uint8_t* data, uint32_t len) {
	if (msg != NULL) {
		if (msg->header->type == DYNAMIC_MSG_TYPE) {
			if (msg->header->payload == NULL ||
					msg->header->len < len) {
				FATAL("AK", 0x17);
			}
			else {
				memcpy(data, msg->header->payload, len);
				AK_MSG_DBG("[MSG] get payload:%08x\n", msg->header->payload);
			}
		}
		else {
			FATAL("AK", 0x18);
		}
	}
	else {
		FATAL("AK", 0x19);
	}
}

void task_post(uint32_t task_dst_id, ak_msg_t* msg) {

	if (task_dst_id >= ak_thread_table_len) {
		FATAL("AK", 0x1A);
	}

	q_msg_t* q_msg = task_list[task_dst_id].mailbox;

	if (msg != NULL) {
		msg->header->task_id = task_dst_id;
		q_msg_put(q_msg, msg);
	}
	else {
		FATAL("AK", 0x1B);
	}
}

bool msg_available(uint32_t task_id) {
	if (task_id >= GW_TASK_LIST_LEN) {
		FATAL("AK", 0x1C);
	}

	q_msg_t* q_msg = task_list[task_id].mailbox;

	return q_msg_available(q_msg);
}

ak_msg_t* rev_msg(uint32_t task_id) {
	if (task_id >= GW_TASK_LIST_LEN) {
		FATAL("AK", 0x1D);
	}

	q_msg_t* q_msg = task_list[task_id].mailbox;

	return q_msg_get(q_msg);
}

void msg_inc_ref_count(ak_msg_t* msg) {
	if (msg->header->ref_count++ > MAX_MSG_REF_COUNT) {
		FATAL("AK", 0x1E);
	}
}

void msg_dec_ref_count(ak_msg_t* msg) {
	if (msg->header->ref_count -- < 0) {
		FATAL("AK", 0x1F);
	}
}

uint32_t get_msg_ref_count(ak_msg_t* msg) {
	return msg->header->ref_count;
}

uint32_t get_msg_type(ak_msg_t* msg) {
	return msg->header->type;
}

void free_msg(ak_msg_t* msg) {

	msg_dec_ref_count(msg);

	if (get_msg_ref_count(msg) == 0) {
		if (msg) {
			q_msg_free(msg);
		}
		else {
			FATAL("AK", 0x20);
		}
	}
}
