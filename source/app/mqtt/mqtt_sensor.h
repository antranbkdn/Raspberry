#ifndef __MQTT_SENSOR_H__
#define __MQTT_SENSOR_H__

#include <mosquittopp.h>
#include <stdint.h>


class mqtt_sensor : public mosqpp::mosquittopp {
public:
	mqtt_sensor(const char *id, const char *host, int port);
	~mqtt_sensor();

	void sensor_public(uint8_t* msg, uint32_t len);

	/* call back functions */
	void on_connect(int rc);
	void on_publish(int mid);

private:
	const char* m_topic;
	char m_connect_ok_flag;
	int m_mid;
};

#endif //__MQTT_SENSOR_H__
