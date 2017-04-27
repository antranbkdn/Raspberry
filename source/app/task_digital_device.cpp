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
#include "task_snmp.h"
#include "task_mqtt.h"
#include "if_rf24.h"
#include "../common/fifo.h"
#include "../common/ui.h"


#include "task_digital_device.h"

q_msg_t gw_task_digital_device_mailbox;

#define WAIT_RESPONSE_MSG_FIFO_SIZE 10

response_msg_t wait_response_fifo_buffer[WAIT_RESPONSE_MSG_FIFO_SIZE];
fifo_t wait_response_msg_fifo;
static void print_device (void*);

void* gw_task_digital_device_entry(void*) {
    task_mask_started();
    wait_all_tasks_started();
    set_print_callback(&print_device);

    APP_DBG("gw_task_digital_device_entry\n");

    response_msg_t* fifo_response_msg = (response_msg_t*) malloc(sizeof (response_msg_t));
    fifo_init(&wait_response_msg_fifo, wait_response_fifo_buffer, WAIT_RESPONSE_MSG_FIFO_SIZE, sizeof(response_msg_t));

    while (1) {
        while (msg_available(GW_TASK_DIGITAL_DEVICE_ID)) {
            /* get messge */
            ak_msg_t* msg = rev_msg(GW_TASK_DIGITAL_DEVICE_ID);

            /* handler message */
            switch (msg->header->sig) {                                     // sig of GW task
            case GW_DV_DIGITAL_CTL_US : {                                   // control device
                APP_DBG ("GW_DV_DIGITAL_CTL_US\n");
                ak_msg_t* s_msg = get_common_msg();

                set_if_type(s_msg, IF_TYPE_RF24);                           // set type RF24 for task if
                set_if_sig(s_msg, GW_DV_DIGITAL_CTL_US);                    // set sig for DV task
                set_if_task_id(s_msg, DV_TASK_DIGITAL_DEVICE_ID);           // ID for DV task
                set_if_to_address(s_msg, msg->header->if_to_address);

                set_data_common_msg (s_msg, (uint8_t*)msg->header->payload, msg->header->len);

                set_msg_sig(s_msg, GW_IF_COMMON_MSG_OUT);                   // sig back to task if

                response_msg_t* wait_response_msg = (response_msg_t*) malloc(sizeof (response_msg_t));
                ControlDigital* ptrpayload = (ControlDigital*) msg->header->payload;
                wait_response_msg->SourceID = ptrpayload->DestID;
                wait_response_msg->seq_num = ptrpayload->seq_num;
                response_msg_pool = slist_append(response_msg_pool, (void*) wait_response_msg);

                task_post(GW_TASK_IF_ID, s_msg);

                fifo_put(&wait_response_msg_fifo, wait_response_msg);
                timer_set(GW_TASK_DIGITAL_DEVICE_ID, TIME_OUT, TIME_WAIT_RESPONSE, TIMER_ONE_SHOT);
            }
                break;

            case DV_GW_DIGITAL_STATE_RES_US : {                             // control device
                APP_DBG ("DV_GW_DIGITAL_STATE_RES_US\n");               
                StateDigitalDevice dv_gw_res_control;
                get_data_common_msg(msg, (uint8_t*)&dv_gw_res_control, sizeof (StateDigitalDevice));

                uint8_t ID_find = dv_gw_res_control.SourceID;
                slist* slist_found = slist_find(device_list, (void*) &ID_find, is_device_equal_id);
                if (slist_found) {
                    device_t* device_found = (device_t*)slist_found->data;

                    if (dv_gw_res_control.StateDigital ==  device_found->subdevice[dv_gw_res_control.SubID].state) {
                        APP_PRINT ("$CONTROL_OK\n");
                        if (print_device_callback) {
                            print_device_callback((void*)device_found);
                        }
                        mqttdigitaldevice->digital_device_public((uint8_t*)"$CONTROL_OK\n", sizeof("$CONTROL_OK\n"));
                    }
                    else if (dv_gw_res_control.StateDigital != device_found->subdevice[dv_gw_res_control.SubID].state) {
                        APP_PRINT ("$FAIL_CMNR\n");
                        mqttdigitaldevice->digital_device_public((uint8_t*)"$FAIL_CMNR\n", sizeof("$FAIL_CMNR\n"));
                    }
                }
                response_msg_t response_msg;
                response_msg.SourceID = dv_gw_res_control.SourceID;
                response_msg.seq_num = dv_gw_res_control.seq_num - 1;

                slist_found = slist_find(response_msg_pool, &response_msg, is_msg_equal);
                if (slist_found) {
                    response_msg_t* msg_found = (response_msg_t*) slist_found->data;
                    response_msg_pool = slist_remove_by_data_x(response_msg_pool, msg_found, &free_response_msg);
                }
            }
                break;

             case DV_GW_DIGITAL_STATE_UPDATE_US: {
                APP_DBG ("DV_GW_DIGITAL_STATE_UPDATE_US\n");
                StateDigitalDevice dv_gw_update;
                get_data_common_msg(msg, (uint8_t*) &dv_gw_update, sizeof (StateDigitalDevice));

                uint8_t ID_find = dv_gw_update.SourceID;
                slist* slist_found = slist_find (device_list, (void*) &ID_find, is_device_equal_id);
                if (slist_found) {
                    device_t* device_found = (device_t*) slist_found->data;
                    device_found->subdevice[dv_gw_update.SubID].state = dv_gw_update.StateDigital;
                    APP_PRINT ("$DEVICE_STATUS_UPDATE\nID_DEVICE: %d\nSUB_ID_UPDATED: %d\nSUB_DEVICE_STATUS: %d\n",
                                device_found->ID, dv_gw_update.SubID, device_found->subdevice[dv_gw_update.SubID].state);
                    char buffer[63];
                    sprintf(buffer, "$ID_DEV: %d\nSUB_ID: %d\nSUB_DEV_STT: %d\n",
                            device_found->ID, dv_gw_update.SubID, device_found->subdevice[dv_gw_update.SubID].state);
                    mqttdigitaldevice->digital_device_public((uint8_t*)buffer, sizeof(buffer));
                }
            }
                break;

            case TIME_OUT : {
                if(!fifo_is_empty (&wait_response_msg_fifo)) {
                    fifo_get(&wait_response_msg_fifo, fifo_response_msg);
                    slist* slist_found = slist_find(response_msg_pool, fifo_response_msg, is_msg_equal);
                    if (slist_found) {
                        APP_PRINT ("$NOT_RESPONDING_CMNR\n");
                        mqttdigitaldevice->digital_device_public((uint8_t*)"$NOT_RESPONDING_CMNR\n", sizeof("$NOT_RESPONDING_CMNR\n"));
                        response_msg_pool = slist_remove_by_data_x(response_msg_pool, fifo_response_msg, &free_response_msg);
                    }
                    timer_set(GW_TASK_DIGITAL_DEVICE_ID, TIME_OUT, TIME_WAIT_RESPONSE, TIMER_ONE_SHOT);
                }

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

void print_device(void* device){
    device_t* ptrdevice = (device_t*) device;
    APP_PRINT ("UI_CALL_BACK: ID: %d\tSUB_ID: %d\tSTATE: %d\n"
               , ptrdevice->ID, ptrdevice->subdevice[SUB_ID].SubID, ptrdevice->subdevice[SUB_ID].state);
}
