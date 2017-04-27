#include "task_register_device.h"
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "app_if.h"
#include "app_data.h"
#include "app_dbg.h"

#include "task_list.h"
#include "task_list_if.h"
#include <time.h>
#include "random.h"

q_msg_t gw_task_register_device_mailbox;
int8_t register_gw_accept = -1;
static uint8_t get_ID_not_used ();
static uint16_t get_random_num_for_key ();
static uint16_t current_device_address;

void* gw_task_register_device_entry(void *) {
    task_mask_started();
    wait_all_tasks_started();

    APP_DBG("gw_task_register_device_entry\n");

    while (1) {
        while (msg_available(GW_TASK_REGISTER_DEVICE_ID)) {
            /* get messge */
            ak_msg_t* msg = rev_msg(GW_TASK_REGISTER_DEVICE_ID);

            /* handler message */
            switch (msg->header->sig) {                                              // sig of GW task
            case GW_DV_BROADCAST_DF_CHANNEL_US : {                                   // default channel
                slist* device_browser = device_list;
                uint8_t number_devices = 0;
                while (device_browser) {
                    number_devices++;
                    ak_msg_t* s_msg = get_common_msg();
                    DefaultChannel* gw_dv_dfl_channel = (DefaultChannel*) malloc(sizeof(DefaultChannel));
                    gw_dv_dfl_channel->SourceID = GW_ID;
                    gw_dv_dfl_channel->Channel = DEFAULT_CHANNEL;

                    gw_dv_dfl_channel->SourceAddress = GW_ADDRESS;
                    gw_dv_dfl_channel->seq_num = 0;

                    set_if_type(s_msg, IF_TYPE_RF24);
                    set_if_sig(s_msg, GW_DV_BROADCAST_DF_CHANNEL_US);
                    set_if_task_id(s_msg, DV_TASK_CHANGE_CHANNEL_ID);

                    set_data_common_msg(s_msg, (uint8_t*)gw_dv_dfl_channel, sizeof(DefaultChannel));
                    device_t* device_data = (device_t*) device_browser->data;
                    set_if_to_address(s_msg, (uint32_t) device_data->address);

                    set_msg_sig(s_msg, GW_IF_COMMON_MSG_OUT);
                    task_post(GW_TASK_IF_ID, s_msg);
                    delayMicroseconds (10);
                    device_browser = device_browser->next;
                }

                uint16_t t = number_devices * 300;
                delay (t);
                radio.setChannel (DEFAULT_CHANNEL);
                APP_DBG ("Default_Channel: %d\n", radio.getChannel());
            }
                break;

            case GW_DV_BROADCAST_GW_CHANNEL_US: {
                slist* device_browser = device_list;
                uint8_t number_devices = 0;
                while (device_browser) {
                    number_devices++;
                    ak_msg_t* s_msg = get_common_msg();
                    DefaultChannel* gw_dv_gw_channel = (DefaultChannel*) malloc(sizeof(DefaultChannel));
                    gw_dv_gw_channel->SourceID = GW_ID;
                    gw_dv_gw_channel->Channel = gateway_active_channel;

                    gw_dv_gw_channel->SourceAddress = GW_ADDRESS;
                    gw_dv_gw_channel->seq_num = init_seq_num();

                    set_if_type(s_msg, IF_TYPE_RF24);
                    set_if_sig(s_msg, GW_DV_BROADCAST_GW_CHANNEL_US);
                    set_if_task_id(s_msg, DV_TASK_CHANGE_CHANNEL_ID);

                    set_data_common_msg(s_msg, (uint8_t*)gw_dv_gw_channel, sizeof(DefaultChannel));
                    device_t* device_data = (device_t*)device_browser->data;
                    set_if_to_address(s_msg, (uint32_t) device_data->address);

                    set_msg_sig(s_msg, GW_IF_COMMON_MSG_OUT);
                    task_post(GW_TASK_IF_ID, s_msg);
                    delayMicroseconds (10);

                    device_browser = device_browser->next;
                }

                uint16_t t = number_devices * 300;
                delay (t);
                radio.setChannel (gateway_active_channel);
                APP_DBG ("Active_Channel: %d\n", radio.getChannel());
                if (register_gw_accept) {
                    ak_msg_t* s_msg = get_pure_msg();
                    set_msg_sig(s_msg, GW_DV_REGISTER_RES_AFTER_GW_CHANNEL_US);
                    task_post(GW_TASK_REGISTER_DEVICE_ID, s_msg);
                }
            }
                break;

            case DV_GW_REGISTER_REQ_US : {                                           // received register request
                APP_DBG ("DV_GW_REGISTER_REQ_US\n");
                RequestRegister dv_gw_req_register;
                get_data_common_msg(msg, (uint8_t*)&dv_gw_req_register, sizeof (RequestRegister));
                APP_PRINT ("$$$$$$$$$\nNEW_DEVICE_REGISTER\nCODE_PRODUCT: %d\nFIRMWARE: %d\n$ACCEPT?\n",
                           dv_gw_req_register.CodeProduct, dv_gw_req_register.FirmwareVer);

                struct timespec spec;
                clock_gettime(CLOCK_REALTIME, &spec);
                register_gw_accept = -1;
                uint8_t now = spec.tv_sec;
                uint8_t timeout = 0;

                //==========TEST==========
                register_gw_accept = 1;
                //==========END TEST======
                while ((register_gw_accept == -1) && !timeout) {
                    clock_gettime(CLOCK_REALTIME, &spec);
                    if (spec.tv_sec - now > 10) {
                        timeout = true;
                    }
                }
                if (timeout) {
                    register_gw_accept = 0;
                    APP_PRINT("UNACCEPTED\n");
                }
                else {
                    if (register_gw_accept == 0) {
                        APP_PRINT("UNACCEPTED\n");
                    }
                    else if (register_gw_accept == 1) {
                        APP_PRINT("ACCEPTED\n");
                        // TODO luu thong tin device tam thoi
                    }
                }
                current_device_address = dv_gw_req_register.SourceAddress;
                ak_msg_t* s_msg = get_pure_msg();
                set_msg_sig(s_msg, GW_DV_REGISTER_RES_US);
                task_post(GW_TASK_REGISTER_DEVICE_ID, s_msg);
            }
                break;

            case GW_DV_REGISTER_RES_US: {
                APP_DBG ("GW_DV_REGISTER_RES_US\n");
                ResponseRegister gw_dv_res_register;
                gw_dv_res_register.SourceID = GW_ID;
                gw_dv_res_register.DestID = get_ID_not_used();
                gw_dv_res_register.RandomNum = get_random_num_for_key();
                gw_dv_res_register.ChannelActive = gateway_active_channel;
                gw_dv_res_register.Accept = register_gw_accept;
                gw_dv_res_register.seq_num = 0;

                ak_msg_t* s_msg = get_common_msg();

                set_if_type(s_msg, IF_TYPE_RF24);
                set_if_sig (s_msg, GW_DV_REGISTER_RES_US);
                set_if_task_id(s_msg, DV_TASK_REGISTER_DEVICE_ID);
                set_if_to_address(s_msg, current_device_address);

                set_data_common_msg(s_msg, (uint8_t*)&gw_dv_res_register, sizeof (ResponseRegister));

                set_msg_sig(s_msg, GW_IF_COMMON_MSG_OUT);
                task_post(GW_TASK_IF_ID, s_msg);
                timer_set (GW_TASK_REGISTER_DEVICE_ID, GW_DV_BROADCAST_GW_CHANNEL_US, TIME_WAIT_SEND, TIMER_ONE_SHOT);
            }
                break;

            case GW_DV_REGISTER_RES_AFTER_GW_CHANNEL_US: {
                ak_msg_t* s_msg = get_pure_msg();

                set_if_type(s_msg, IF_TYPE_RF24);
                set_if_sig (s_msg, GW_DV_REGISTER_RES_AFTER_GW_CHANNEL_US);
                set_if_task_id(s_msg, DV_TASK_REGISTER_DEVICE_ID);
                set_if_to_address(s_msg, current_device_address);

                set_msg_sig(s_msg, GW_IF_PURE_MSG_OUT);
                task_post(GW_TASK_IF_ID, s_msg);
            }
                break;

            case DV_GW_REGISTER_FN_US: {
                APP_DBG ("DV_GW_REGISTER_FN_US\n");
                FinishRegister dv_gw_fn_register;
                get_data_common_msg(msg, (uint8_t*)&dv_gw_fn_register, sizeof (FinishRegister));

                device_t* new_device = (device_t*) malloc (sizeof (device_t));
                new_device->address = dv_gw_fn_register.SourceAddress;
                new_device->CodeProduct = dv_gw_fn_register.CodeProduct;
                new_device->FirmwareVer = dv_gw_fn_register.FirmwareVer;
                new_device->ID = dv_gw_fn_register.SourceID;
                for (uint8_t i = 0; i < MAX_SUB_DEVICES; ++i) {
                    if (dv_gw_fn_register.subdevice[i].ActiveStatus) {
                        new_device->subdevice[i].SubID = dv_gw_fn_register.subdevice[i].SubID;
                        new_device->subdevice[i].DeviceType = dv_gw_fn_register.subdevice[i].DeviceType;
                        new_device->subdevice[i].Direction = dv_gw_fn_register.subdevice[i].Direction;
                    }
                }

                device_list = slist_append(device_list, (void*) new_device);
                APP_DBG ("$$$$$$$$$\nNEW_DEVICE_REGISTERED\nDEVICE_ID: %d\nDEVICE_ADDRESS: %d\n",
                         new_device->ID, new_device->address);

                device_list = slist_sort(device_list, compare_address_greater);

            }
                break;

            default:
                break;
            }

            /* free message */
            free_msg(msg);
        }

        usleep(100);
    }

    return (void*)0;
}

uint8_t get_ID_not_used() {
    static uint8_t i = 1;
    uint8_t ID = i;
    i++;
    return ID;
}

uint16_t get_random_num_for_key () {
    return random_16bit (1, 1000);
}
