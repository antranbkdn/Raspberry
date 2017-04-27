#include "mqtt_sensor.h"

#include "app_dbg.h"

mqtt_sensor::mqtt_sensor(const char *id, const char *host, int port) : mosquittopp(id) {
	/* init private data */
	m_connect_ok_flag = -1;
	m_topic = "sensor";
	m_mid = 1;

	/* init mqtt */
	mosqpp::lib_init();

	/* connect */
	connect_async(host, port, 60);
	loop_start();
}

mqtt_sensor::~mqtt_sensor() {
	loop_stop();
	mosqpp::lib_cleanup();
}

void mqtt_sensor::on_connect(int rc) {
	if (rc == 0) {
		m_connect_ok_flag = 0;
		APP_DBG("[mqtt_sensor] on_connect OK\n");
	}
	else {
		APP_DBG("[mqtt_sensor] on_connect ERROR\n");
	}
}

void mqtt_sensor::sensor_public(uint8_t* msg, uint32_t len) {
	APP_DBG("[mqtt_sensor][sensor_public] msg:%s len:%d\n", msg, len);
	publish(&m_mid, m_topic, len, msg);
}

void mqtt_sensor::on_publish(int mid) {
	APP_DBG("[mqtt_sensor][on_publish] mid: %d\n", mid);
}
