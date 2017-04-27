#ifndef TASK_QUERY_ADDRESS_H
#define TASK_QUERY_ADDRESS_H
#include "../ak/message.h"
#include "app_data.h"

extern q_msg_t gw_task_query_address_mailbox;
extern void* gw_task_query_address_entry(void*);

typedef struct RequestAddress {
    uint16_t SourceAddress;

    uint8_t seq_num;
} RequestAddress;

typedef struct ResponseAddress {
    uint8_t SourceID;
    uint16_t NewAddress;
    uint8_t Accept;

    uint8_t seq_num;
} ResponseAddress;


#endif // TASK_QUERY_ADDRESS_H
