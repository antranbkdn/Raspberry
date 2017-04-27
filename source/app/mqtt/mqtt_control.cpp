#include <string.h>

#include "../ak/ak.h"

#include "mqtt_control.h"

#include "app.h"
#include "app_dbg.h"

#include "task_list.h"

mqtt_control::mqtt_control(const char *id, const char *host, int port) : mosquittopp(id) {
	/* init private data */
	m_connect_ok_flag = -1;
	m_topic = "control";
	m_mid = 1;

	/* init mqtt */
	mosqpp::lib_init();

	/* connect */
	connect_async(host, port, 60);
	loop_start();
}

mqtt_control::~mqtt_control() {
	loop_stop();
	mosqpp::lib_cleanup();
}

void mqtt_control::on_connect(int rc) {
	if (rc == 0) {
		m_connect_ok_flag = 0;
		APP_DBG("[mqtt_control] on_connect OK\n");
		subscribe(NULL, m_topic);
	}
	else {
		APP_DBG("[mqtt_control] on_connect ERROR\n");
	}
}

void mqtt_control::control_public(uint8_t* msg, uint32_t len) {
	APP_DBG("[mqtt_control][control_public] msg:%s len:%d\n", msg, len);
	publish(&m_mid, m_topic, len, msg);
}

void mqtt_control::on_publish(int mid) {
	APP_DBG("[mqtt_control][on_publish] mid: %d\n", mid);
}

void mqtt_control::on_subscribe(int mid, int qos_count, const int *granted_qos) {
	(void)granted_qos;
	APP_DBG("[mqtt_control][on_subscribe] mid:%d\tqos_count:%d\n", mid, qos_count);
}

void mqtt_control::on_message(const struct mosquitto_message *message) {
	APP_DBG("[mqtt_control][on_message] topic:%s\tpayloadlen:%d\n", message->topic, message->payloadlen);
	if (!strcmp(message->topic, m_topic)) {
		int payload_len = message->payloadlen + 1;
		char* payload = (char*)malloc(payload_len);
		memset(payload, 0 , payload_len);

		memcpy(payload, message->payload, message->payloadlen);

		APP_DBG("[mqtt_control][on_message] message->payload:%s\n", payload);

		/* post message to mqtt task */
		ak_msg_t* s_msg = get_dymanic_msg();
		set_msg_sig(s_msg, GW_MQTT_CONTROL_REQ);
		set_data_dynamic_msg(s_msg, (uint8_t*)payload, payload_len);
		task_post(GW_TASK_MQTT_ID, s_msg);

		/* free temp payload */
		free(payload);
	}
}
