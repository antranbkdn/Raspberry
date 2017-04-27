#ifndef MQTT_DIGITAL_DEVICE_H
#define MQTT_DIGITAL_DEVICE_H

#include <mosquittopp.h>
#include <stdint.h>


class mqtt_digital_device : public mosqpp::mosquittopp {
public:
    mqtt_digital_device(const char *id, const char *host, int port);
    ~mqtt_digital_device();

    void digital_device_public(uint8_t* msg, uint32_t len);

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

#endif // MQTT_DIGITAL_DEVICE_H
