#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ak/ak.h"

#include "../common/fifo.h"

#include "app.h"
#include "app_data.h"
#include "app_dbg.h"

#include "if_rf24.h"
#include "task_list.h"
#include "task_if.h"
#include "app_data.h"


#define SEND_FIFO_BUFFER_SIZE			2

q_msg_t gw_task_if_rf24_mailbox;

/* Setup for GPIO 22 CE and CE1 CSN with SPI Speed @ 1Mhz */
RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_1MHZ);

RF24Network network(radio);

static uint8_t rf24_buffer[RF24_BUFFER_SIZE];

static ak_msg_common_if_t send_common_fifo_buffer[SEND_FIFO_BUFFER_SIZE];
static fifo_t send_common_fifo;

static ak_msg_pure_if_t send_pure_fifo_buffer[SEND_FIFO_BUFFER_SIZE];
static fifo_t send_pure_fifo;

static ak_msg_common_if_t send_common_direct_fifo_buffer[SEND_FIFO_BUFFER_SIZE];
static fifo_t send_common_direct_fifo;

static uint8_t rf_pending_flag = APP_FLAG_OFF;

void* gw_task_if_rf24_entry(void*) {
    task_mask_started();
    wait_all_tasks_started();

    APP_DBG("gw_task_if_rf24_entry:%d\n", MAIN_BUFFER_SIZE);

    radio.begin();
    delay(5);

//    //======TEST=========
//    gateway_active_channel = 90;
//    //======END TEST=====

    network.begin(gateway_active_channel, gateway_rf24_address);
    radio.printDetails();

    fifo_init(&send_common_fifo, send_common_fifo_buffer, SEND_FIFO_BUFFER_SIZE, sizeof(ak_msg_common_if_t));
    fifo_init(&send_pure_fifo, send_pure_fifo_buffer, SEND_FIFO_BUFFER_SIZE, sizeof(ak_msg_pure_if_t));
    fifo_init (&send_common_direct_fifo, send_common_direct_fifo_buffer, SEND_FIFO_BUFFER_SIZE, sizeof(ak_msg_common_if_t));

    while (1) {
        network.update();

        /* check receive rf24 message */
        while(network.available()) {
            RF24NetworkHeader rf24_header;
            memset(rf24_buffer, 0, RF24_BUFFER_SIZE);

            network.read(rf24_header, rf24_buffer, RF24_BUFFER_SIZE);

            switch (rf24_header.type) {
            case RF24_DATA_COMMON_MSG_TYPE: {
                APP_DBG ("RF24_DATA_COMMON_MSG_TYPE\n");
                ak_msg_common_if_t* if_msg = (ak_msg_common_if_t*)rf24_buffer;
                ak_msg_t* s_msg = get_common_msg();

                set_if_type(s_msg, IF_TYPE_RF24);
                set_if_sig(s_msg, if_msg->sig);
                set_if_task_id(s_msg, if_msg->task_id);
                set_data_common_msg(s_msg, if_msg->data, if_msg->len);

                set_msg_sig(s_msg, GW_IF_COMMON_MSG_IN);
                task_post(GW_TASK_IF_ID, s_msg);

                rf_pending_flag = APP_FLAG_ON;
                timer_set(GW_TASK_IF_RF24_ID, GW_RF24_IF_TIMER_PACKET_DELAY, GW_RF24_IF_TIMER_PACKET_DELAY_INTERVAL, TIMER_ONE_SHOT);
            }
                break;

            case RF24_DATA_PURE_MSG_TYPE: {
                ak_msg_pure_if_t* if_msg = (ak_msg_pure_if_t*)rf24_buffer;
                ak_msg_t* s_msg = get_pure_msg();

                set_if_type(s_msg, IF_TYPE_RF24);
                set_if_sig(s_msg, if_msg->sig);
                set_if_task_id(s_msg, if_msg->task_id);

                set_msg_sig(s_msg, GW_IF_PURE_MSG_IN);
                task_post(GW_TASK_IF_ID, s_msg);

                rf_pending_flag = APP_FLAG_ON;
                timer_set(GW_TASK_IF_RF24_ID, GW_RF24_IF_TIMER_PACKET_DELAY, GW_RF24_IF_TIMER_PACKET_DELAY_INTERVAL, TIMER_ONE_SHOT);
            }
                break;

            case RF24_DATA_REMOTE_CMD_TYPE: {
                uint32_t str_len = strlen((const char*)rf24_buffer);
                ak_msg_t* s_msg = get_dymanic_msg();
                set_msg_sig(s_msg, GW_CONSOLE_AC_LOGIN_CMD);
                set_data_dynamic_msg(s_msg, rf24_buffer, str_len);
                task_post(GW_TASK_CONSOLE_ID, s_msg);
            }
                break;

            default:
                break;
            }
        }

        while (msg_available(GW_TASK_IF_RF24_ID)) {                         // kiem tra msg tu task khac gui den
            /* get messge */
            ak_msg_t* msg = rev_msg(GW_TASK_IF_RF24_ID);

            /* handler message */
            switch (msg->header->sig) {
            case GW_RF24_IF_PURE_MSG_OUT: {                                 // gui msg rf den device
                ak_msg_pure_if_t if_msg;
                memset(&if_msg, 0, sizeof(ak_msg_pure_if_t));

                if_msg.task_id = msg->header->if_task_id;
                if_msg.sig = msg->header->if_sig;
                if_msg.address = msg->header->if_to_address;

                if (rf_pending_flag == APP_FLAG_OFF) {                      // sang sang gui
                    RF24NetworkHeader send_header(msg->header->if_to_address, RF24_DATA_PURE_MSG_TYPE);
                    network.write(send_header, &if_msg, sizeof(ak_msg_pure_if_t));
                }
                else {
                    fifo_put(&send_pure_fifo, &if_msg);                     // khong san sang gui thi dua vao fifo
                }
            }
                break;

            case GW_RF24_IF_COMMON_MSG_OUT: {
                ak_msg_common_if_t if_msg;
                memset(&if_msg, 0, sizeof(ak_msg_common_if_t));

                if_msg.task_id = msg->header->if_task_id;
                if_msg.sig = msg->header->if_sig;
                if_msg.address = msg->header->if_to_address;

                if_msg.len = get_data_len_common_msg(msg);
                get_data_common_msg(msg, if_msg.data, if_msg.len);

                if (rf_pending_flag == APP_FLAG_OFF) {
                    APP_DBG ("send_to: %d from channel: %d\n", msg->header->if_to_address, radio.getChannel());
                    RF24NetworkHeader send_header(msg->header->if_to_address, RF24_DATA_COMMON_MSG_TYPE);
                    network.write(send_header, &if_msg, sizeof(ak_msg_common_if_t));
                }
                else {
                    fifo_put(&send_common_fifo, &if_msg);
                }
            }
                break;

            case GW_RF24_IF_COMMON_MSG_OUT_DIRECT: {
                ak_msg_common_if_t if_msg;
                memset(&if_msg, 0, sizeof(ak_msg_common_if_t));

                if_msg.task_id = msg->header->if_task_id;
                if_msg.sig = msg->header->if_sig;
                if_msg.address = msg->header->if_to_address;

                if_msg.len = get_data_len_common_msg(msg);
                get_data_common_msg(msg, if_msg.data, if_msg.len);

                if (rf_pending_flag == APP_FLAG_OFF) {
                    APP_DBG ("send_to: %d from channel: %d\n", msg->header->if_to_address, radio.getChannel());
                    RF24NetworkHeader send_header(msg->header->if_to_address, RF24_DATA_COMMON_MSG_TYPE);
                    network.write(send_header, &if_msg, sizeof(ak_msg_common_if_t), msg->header->if_to_address);
                }
                else {
                    fifo_put(&send_common_direct_fifo, &if_msg);
                }
            }
                break;

            case GW_RF24_IF_TIMER_PACKET_DELAY: {
                /* clear pending flag */
                rf_pending_flag = APP_FLAG_OFF;

                /* TODO: check data queue */
                if (!fifo_is_empty(&send_pure_fifo)) {
                    ak_msg_pure_if_t if_msg;
                    fifo_get(&send_pure_fifo, &if_msg);

                    RF24NetworkHeader ac_header(if_msg.address, RF24_DATA_PURE_MSG_TYPE);
                    network.write(ac_header, &if_msg, sizeof(ak_msg_pure_if_t));

                    rf_pending_flag = APP_FLAG_ON;
                    timer_set(GW_TASK_IF_RF24_ID, GW_RF24_IF_TIMER_PACKET_DELAY, GW_RF24_IF_TIMER_PACKET_DELAY_INTERVAL, TIMER_ONE_SHOT);
                }
                else if (!fifo_is_empty(&send_common_fifo)) {
                    ak_msg_common_if_t if_msg;
                    fifo_get(&send_common_fifo, &if_msg);

                    RF24NetworkHeader ac_header(if_msg.address, RF24_DATA_COMMON_MSG_TYPE);
                    network.write(ac_header, &if_msg, sizeof(ak_msg_common_if_t));

                    rf_pending_flag = APP_FLAG_ON;
                    timer_set(GW_TASK_IF_RF24_ID, GW_RF24_IF_TIMER_PACKET_DELAY, GW_RF24_IF_TIMER_PACKET_DELAY_INTERVAL, TIMER_ONE_SHOT);
                }

                else if (!fifo_is_empty(&send_common_direct_fifo)) {
                    ak_msg_common_if_t if_msg;
                    fifo_get(&send_common_direct_fifo, &if_msg);

                    APP_DBG ("send_to: %d from fifo with channel: %d\n", if_msg.address, radio.getChannel());
                    RF24NetworkHeader ac_header(if_msg.address, RF24_DATA_COMMON_MSG_TYPE);
                    network.write(ac_header, &if_msg, sizeof(ak_msg_common_if_t), if_msg.address);

                    rf_pending_flag = APP_FLAG_ON;
                    timer_set(GW_TASK_IF_RF24_ID, GW_RF24_IF_TIMER_PACKET_DELAY, GW_RF24_IF_TIMER_PACKET_DELAY_INTERVAL, TIMER_ONE_SHOT);
                }
            }
                break;

            default:
                break;
            }

            /* free message */
            free_msg(msg);
        }

        usleep(1000);
    }

    return (void*)0;
}
