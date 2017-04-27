#include "task_init_gateway.h"
#include "task_list.h"
#include "task_list_if.h"
#include "app.h"
#include "app_if.h"
#include "app_dbg.h"
#include "../common/mac_address.h"

q_msg_t gw_task_init_gateway_mailbox;
static const uint8_t num_channels = 120;
static const uint8_t num_reps = 100;         // so lan doi response
static uint8_t current_channel = num_channels;
static uint8_t current_channel_value = 0;
static uint8_t selected_channel_flag = APP_FLAG_OFF;

void* gw_task_init_gateway_entry(void *) {
    task_mask_started();
    wait_all_tasks_started();
    APP_DBG ("gw_task_init_gateway_entry\n");

    while (1) {
        while (msg_available(GW_TASK_INIT_GATEWAY_ID)) {
            ak_msg_t* msg = rev_msg(GW_TASK_INIT_GATEWAY_ID);

            switch (msg->header->sig) {
            case GW_START_QUERRY_CHANNEL: {
                APP_DBG ("GW_START_QUERRY_CHANNEL\n");
//                radio.begin();
//                radio.setAutoAck (false);
                current_channel_value = 0;
                uint8_t rep_counter = num_reps;
                while (current_channel) {
                    while (rep_counter--) {                     // listen 100 times
                        radio.setChannel(current_channel);
                        radio.startListening();
                        delayMicroseconds(225);
                        radio.stopListening();
                        current_channel_value += radio.testRPD();
                    }
                    if (current_channel_value) {                // kenh ko dung duoc, tiep tuc den kenh khac
                        current_channel--;
                        continue;
                    }
//                    radio.setAutoAck(true);
//                    radio.setChannel(1);
//                    delay(5);
                    radio.setChannel(current_channel);
                    radio.startListening();

//                    delay(5);
                    // den day thi kenh ok. query lai lan nua
                    network.begin (ZOMBIE_ADDRESS);
                    QueryChannel gw_gw_que_channel;
                    gw_gw_que_channel.SourceChannel = current_channel;
                    set_mac_address (gw_gw_que_channel.SourceMacAddress);

                    ak_msg_t* s_msg = get_common_msg();

                    set_if_sig(s_msg, GW_GW_CHANNEL_QUE_US);
                    set_if_task_id(s_msg, GW_TASK_INIT_GATEWAY_ID);
                    set_if_to_address(s_msg, GW_ADDRESS);
                    set_if_type(s_msg, IF_TYPE_RF24);

                    set_data_common_msg(s_msg, (uint8_t*) &gw_gw_que_channel, sizeof (QueryChannel));

                    set_msg_sig(s_msg, GW_IF_COMMON_MSG_OUT_DIRECT);
                    task_post (GW_TASK_IF_ID, s_msg);

                    selected_channel_flag = APP_FLAG_ON;

                    timer_set(GW_TASK_INIT_GATEWAY_ID, GW_GW_CHANNEL_RES_TIMEOUT, 1000, TIMER_ONE_SHOT);
                    current_channel--;
                    break;
                }
            }
                break;

            case GW_GW_CHANNEL_RES_US: {
                ResponseChannel gw_gw_res_channel;
                get_data_common_msg(msg, (uint8_t*) &gw_gw_res_channel, sizeof (ResponseAddress));

                APP_DBG ("received_response_from_channel: %d\nchannel_is_used: %d",
                         gw_gw_res_channel.SourceChannel,
                         gw_gw_res_channel.ChannelIsUsed);

                if (gw_gw_res_channel.ChannelIsUsed == USED) {
                    selected_channel_flag = APP_FLAG_OFF;
                    ak_msg_t* s_msg = get_pure_msg();
                    set_msg_sig(s_msg, GW_START_QUERRY_CHANNEL);
                }
            }
                break;

            case GW_GW_CHANNEL_RES_TIMEOUT: {
                if (selected_channel_flag == APP_FLAG_ON) {
                    gateway_active_channel = current_channel + 1;   // do code ngu
                    radio.setChannel (gateway_active_channel);
                    network.begin (GW_ADDRESS);
                    radio.startListening();
                    radio.printDetails();
                    APP_DBG ("selected_active_channel: %d\n", radio.getChannel());
                    current_channel = num_channels;
                }
            }
                break;

            case GW_GW_CHANNEL_QUE_US: {
                QueryChannel gw_gw_que_channel;
                get_data_common_msg(msg, (uint8_t*) &gw_gw_que_channel, sizeof (QueryChannel));
                APP_DBG ("received_query_from_channel: %d\n", gw_gw_que_channel.SourceChannel);

                /* RESPONSE */
                ResponseChannel gw_gw_res_channel;
                gw_gw_res_channel.SourceChannel = gateway_active_channel;
                set_mac_address(gw_gw_res_channel.SourceMacAddress);
                gw_gw_res_channel.ChannelIsUsed = USED;

                ak_msg_t* s_msg = get_common_msg();

                set_if_sig(s_msg, GW_GW_CHANNEL_RES_US);
                set_if_task_id(s_msg, GW_TASK_INIT_GATEWAY_ID);
                set_if_to_address(s_msg, GW_ADDRESS);
                set_if_type(s_msg, IF_TYPE_RF24);

                set_data_common_msg(s_msg, (uint8_t*) &gw_gw_que_channel, sizeof (ResponseChannel));

                set_msg_sig(s_msg, GW_IF_COMMON_MSG_OUT);
                task_post(GW_TASK_IF_ID, s_msg);
            }
                break;

            default:
                break;
            }
        }

    }
}
