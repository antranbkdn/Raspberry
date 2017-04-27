#ifndef API_H
#define API_H
#include <stdint.h>
extern void digital_control (uint8_t ID, uint8_t SUBID, uint8_t State);
extern void analog_control (uint8_t ID, uint8_t SUBID, uint8_t Value);

extern void wait_device_register (void);
extern void init_gateway (void);

#endif // API_H
