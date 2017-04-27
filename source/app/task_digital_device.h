#ifndef TASK_DIGITAL_DEVICE_H
#define TASK_DIGITAL_DEVICE_H

#include "../ak/message.h"

extern q_msg_t gw_task_digital_device_mailbox;
extern void* gw_task_digital_device_entry(void*);

typedef struct ControlDigital {
    uint8_t SourceID;
    uint8_t DestID;
    uint8_t SubID;
    uint8_t StateDigital;

    uint8_t seq_num;

} ControlDigital;

typedef struct StateDigitalDevice {
    uint8_t SourceID;
    uint8_t DestID;
    uint8_t SubID;
    uint8_t StateDigital;

    uint8_t seq_num;

} StateDigitalDevice;

#define TIME_WAIT_RESPONSE 500

#define DV_ID1  01
#define DV_ADDRESS1 01
#define SUB_ID  0

#define DV_ID2  02
#define DV_ADDRESS2 011
#define SUB_ID  0

#endif // TASK_DIGITAL_DEVICE_H
