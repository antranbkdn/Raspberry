#include "app.h"
#include "app_if.h"
#include "app_data.h"
#include "app_dbg.h"

#include "task_list.h"
#include "task_list_if.h"
#include "task_snmp.h"
#include "task_mqtt.h"
#include "if_rf24.h"
#include "../common/fifo.h"
#include "task_query_address.h"

static uint8_t get_child_address_not_used(void);

q_msg_t gw_task_query_address_mailbox;

void* gw_task_query_address_entry(void *) {
    task_mask_started();
    wait_all_tasks_started();
    APP_DBG ("gw_task_query_address_entry\n");

    while (1) {
        while (msg_available(GW_TASK_QUERY_ADDRESS_ID)) {
            ak_msg_t* msg = rev_msg(GW_TASK_QUERY_ADDRESS_ID);

        switch (msg->header->sig) {
        case DV_ADDRESS_QUE_US: {
            RequestAddress dv_gw_req_address;
            get_data_common_msg(msg, (uint8_t*)&dv_gw_req_address, sizeof (RequestAddress));
            APP_DBG ("$$$$$$$$\nRequest_Address_from: %d\n", dv_gw_req_address.SourceAddress);

            /* RESPONSE */
            ResponseAddress gw_dv_res_address;
            if (gw_dv_res_address.NewAddress = get_child_address_not_used()){
                gw_dv_res_address.Accept = 1;
            }
            else gw_dv_res_address.Accept = 0;
            gw_dv_res_address.seq_num = 0;

            ak_msg_t* s_msg = get_common_msg();

            set_if_type(s_msg, IF_TYPE_RF24);
            set_if_sig (s_msg, ADDRESS_RES_US);
            set_if_task_id(s_msg, DV_TASK_QUERY_ADDRESS);
            set_if_to_address(s_msg, dv_gw_req_address.SourceAddress);

            APP_DBG ("Prepare_sent_to: %d\n", dv_gw_req_address.SourceAddress);

            set_data_common_msg(s_msg, (uint8_t*) &gw_dv_res_address, sizeof (ResponseAddress));

            set_msg_sig (s_msg, GW_IF_COMMON_MSG_OUT_DIRECT);
            task_post(GW_TASK_IF_ID, s_msg);
        }
            break;

        default:
            break;
        }
        }
    }
}
uint8_t get_child_address_not_used(void) {
    // CRITICAL CONDITION?????
    for (int i = 0; i < 5; ++i) {
        if (!child_address_pool[i].is_used) {
            child_address_pool[i].is_used = USED;
            return child_address_pool[i].address;
        }
    }
    return 0;
}
