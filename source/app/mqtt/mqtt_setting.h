#ifndef __MQTT_SETTING_H__
#define __MQTT_SETTING_H__

#include <mosquittopp.h>
#include <stdint.h>

class mqtt_setting : public mosqpp::mosquittopp {
public:
	mqtt_setting(const char *id, const char *host, int port);
	~mqtt_setting();

	void setting_public(uint8_t* msg, uint32_t len);

	/* call back functions */
	void on_connect(int rc);
	void on_publish(int mid);
	void on_subscribe(int mid, int qos_count, const int *granted_qos);
	void on_message(const struct mosquitto_message *message);

private:
	const char* m_topic;
	char m_connect_ok_flag;
	int m_mid;
};

#endif //__MQTT_SETTING_H__
