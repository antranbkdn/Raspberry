#ifndef TASK_REGISTER_DEVICE_H
#define TASK_REGISTER_DEVICE_H
#include "../ak/message.h"
#include "app_data.h"

extern q_msg_t gw_task_register_device_mailbox;
extern void* gw_task_register_device_entry(void*);

extern int8_t register_gw_accept;


#define ACTIVE              (1)
#define INACTIVE            (0)

#define DV_TYPE_DIGITAL     (1)
#define DV_TYPE_ANALOG      (2)

#define DV_DIRECTION_OUT    (1)
#define DV_DIRECTION_IN     (2)

typedef struct SubDevice {
    uint8_t SubID;
    uint8_t DeviceType;
    uint8_t Direction;
    uint8_t ActiveStatus;
}SubDevice;

typedef struct RequestRegister {
    uint8_t SourceID;
    uint8_t DestID;
    SubDevice subdevice[MAX_SUB_DEVICES];
    uint8_t FirmwareVer;
    uint8_t CodeProduct;

    uint16_t SourceAddress;
    uint8_t seq_num;

}RequestRegister;

typedef struct ResponseRegister {
    uint8_t SourceID;
    uint8_t DestID;
    uint16_t RandomNum;
    uint16_t ChannelActive;
    uint8_t Accept;

    uint16_t SourceAddress;
    uint8_t seq_num;

}ResponseRegister;

typedef struct FinishRegister {
    uint8_t SourceID;
    uint8_t DestID;
    SubDevice subdevice[MAX_SUB_DEVICES];
    uint8_t FirmwareVer;
    uint8_t CodeProduct;
    uint16_t RandomNum;

    uint16_t SourceAddress;
    uint8_t seq_num;
} FinishRegister;

typedef struct KeyExchange {
    uint8_t SourceID;
    uint8_t DestID;
    uint16_t Key;

    uint16_t SourceAddress;
    uint8_t seq_num;

}KeyExchange;

typedef struct DefaultChannel {
    uint8_t SourceID;
    uint8_t Channel;

    uint16_t SourceAddress;
    uint8_t seq_num;

} DefaultChannel;
#define TIME_WAIT_SEND 1000

#endif // TASK_REGISTER_DEVICE_H
