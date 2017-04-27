#ifndef TASK_INIT_GATEWAY_H
#define TASK_INIT_GATEWAY_H
#include "../ak/message.h"
#include <pthread.h>

extern q_msg_t gw_task_init_gateway_mailbox;
extern void* gw_task_init_gateway_entry(void*);

typedef struct QueryChannel {
    uint8_t SourceChannel;
    uint8_t SourceMacAddress[6];
}QueryChannel;

typedef struct ResponseChannel {
    uint8_t SourceChannel;
    uint8_t SourceMacAddress[6];
    uint8_t ChannelIsUsed;
}ResponseChannel;

#define NOTUSED     0
#define USED        1


#endif // TASK_INIT_GATEWAY_H
