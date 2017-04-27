#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../common/jsmn.h"
#include "../common/random.h"

#include "app_data.h"
#include "app_dbg.h"
#include "app.h"

/******************************************************************************
 *  define rf infomation
 *****************************************************************************/
uint16_t gateway_rf24_address = GW_ADDRESS;
uint8_t gateway_ID = 0;
uint8_t gateway_active_channel = DEFAULT_CHANNEL;
uint8_t gateway_mac_address[6];

uint8_t init_pending_flag = APP_FLAG_ON;
/***************************************************************************************************
 *
 * *************************************************************************************************/
child_address_t child_address_pool[5] = {{01, NOT_USED},
                                      {02, NOT_USED},
                                      {03, NOT_USED},
                                      {04, NOT_USED},
                                      {05, NOT_USED}};

/***************************************************************************************************
 *
 * *************************************************************************************************/
slist* response_msg_pool = NULL;

bool is_msg_equal(void *msg1, void *msg2){
    response_msg_t* ptrmsg1 = (response_msg_t*) msg1;
    response_msg_t* ptrmsg2 = (response_msg_t*) msg2;
    return ((ptrmsg1->SourceID == ptrmsg2->SourceID) && (ptrmsg1->seq_num == ptrmsg2->seq_num));
}

void free_response_msg (void* msg) {
    free (msg);
}

uint8_t init_seq_num() {
    return  random_16bit(1, 100);
}

/***************************************************************************************************
 *
 * *************************************************************************************************/
slist* device_list = NULL;

bool is_device_equal_address (void* dv, void* address) {
    uint16_t* ptradress = (uint16_t*)address;
    device_t* ptrdv = (device_t*)dv;
    return (ptrdv->address == *ptradress);
}

bool is_device_equal_id (void* dv, void* id) {
    uint8_t* ptrid = (uint8_t*)id;
    device_t* ptrdv = (device_t*)dv;
    return (ptrdv->ID == *ptrid);
}

void free_device (void *dv) {
    free (dv);
}

bool compare_address_greater(void *dv1, void *dv2) {
    device_t* ptrdv1 = (device_t*)dv1;
    device_t* ptrdv2 = (device_t*)dv2;
    return (ptrdv1->address <= ptrdv2->address);
}

/***************************************************************************************************
 *
 * *************************************************************************************************/
static const char* sensor_js_format = "{\"temperature\": %d, \"humidity\": %d, \"current1\": %d, \"current2\": %d, \"current3\": %d, \"current4\": %d}";
static const char* ret_js_format	= "{\"ret\": %d}";
static const char* sig_js_format	= "{\"sig\": %d}";

static int jsoneq(const char *json, jsmntok_t *tok, const char *s);

int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

int sensor_json_struct(uint8_t* js, sensor_packet_t* sensor) {
	jsmn_parser p;
	jsmntok_t t[16];

	jsmn_init(&p);

	int r = jsmn_parse(&p, (const char*)js, strlen((const char*)js), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		APP_DBG("Failed to parse JSON: %d\n", r);
		return r;
	}

	if (r < 1 || t[0].type != JSMN_OBJECT) {
		APP_DBG("Object expected\n");
		return r;
	}

	for (int i = 1; i < r; i++) {
		if (jsoneq((const char*)js, &t[i], "temperature") == 0) {
			sensor->remote_temperature = atoi(strndup((const char*)js + t[i+1].start, t[i+1].end-t[i+1].start));
			i++;
		} else if (jsoneq((const char*)js, &t[i], "humidity") == 0) {
			sensor->humindity = atoi(strndup((const char*)js + t[i+1].start, t[i+1].end-t[i+1].start));
			i++;
		} else if (jsoneq((const char*)js, &t[i], "current1") == 0) {
			sensor->air_cond_current[0] = atoi(strndup((const char*)js + t[i+1].start, t[i+1].end-t[i+1].start));
			i++;
		} else if (jsoneq((const char*)js, &t[i], "current2") == 0) {
			sensor->air_cond_current[1] = atoi(strndup((const char*)js + t[i+1].start, t[i+1].end-t[i+1].start));
			i++;
		} else if (jsoneq((const char*)js, &t[i], "current3") == 0) {
			sensor->air_cond_current[2] = atoi(strndup((const char*)js + t[i+1].start, t[i+1].end-t[i+1].start));
			i++;
		} else if (jsoneq((const char*)js, &t[i], "current4") == 0) {
			sensor->air_cond_current[3] = atoi(strndup((const char*)js + t[i+1].start, t[i+1].end-t[i+1].start));
			i++;
		}
	}

	return 0;
}

void sensor_struct_json(uint8_t* js, sensor_packet_t* sensor) {
	sprintf((char*)js, sensor_js_format,
			sensor->remote_temperature,	\
			sensor->humindity,			\
			sensor->air_cond_current[0],\
			sensor->air_cond_current[1],\
			sensor->air_cond_current[2],\
			sensor->air_cond_current[3]);
}

void ret_json_struct(uint8_t* js, int* ret) {
	sprintf((char*)js, ret_js_format, *ret);
	APP_DBG("ret_json_struct:%s\n", js);
}

void sig_json_struct(uint8_t* js, int* sig) {
	sprintf((char*)js, sig_js_format, *sig);
	APP_DBG("sig_json_struct:%s\n", js);
}

int sig_struct_json(uint8_t* js, int* sig) {
	jsmn_parser p;
	jsmntok_t t[16];

	jsmn_init(&p);

	int r = jsmn_parse(&p, (const char*)js, strlen((const char*)js), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		APP_DBG("Failed to parse JSON: %d\n", r);
		return r;
	}

	if (r < 1 || t[0].type != JSMN_OBJECT) {
		APP_DBG("Object expected\n");
		return r;
	}

	for (int i = 1; i < r; i++) {
		if (jsoneq((const char*)js, &t[i], "sig") == 0) {
			*sig = atoi(strndup((const char*)js + t[i+1].start, t[i+1].end-t[i+1].start));
			i++;
		}
	}

	return 0;
}
