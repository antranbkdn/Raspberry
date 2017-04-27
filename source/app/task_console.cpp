#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ak/ak.h"
#include "../ak/timer.h"

#include "../common/cmd_line.h"

#include "app.h"
#include "app_if.h"
#include "app_dbg.h"
#include "app_data.h"

#include "task_list.h"
#include "task_list_if.h"
#include "task_console.h"
#include "task_digital_device.h"
#include "task_mqtt.h"
#include "task_init_gateway.h"
#include "../common/api.h"


q_msg_t gw_task_console_mailbox;

static int32_t i_shell_ver(uint8_t* argv);
static int32_t i_shell_help(uint8_t* argv);
static int32_t i_shell_record(uint8_t* argv);
static int32_t i_shell_fw(uint8_t* argv);
static int32_t i_shell_m(uint8_t* argv);
static int32_t i_shell_digital1(uint8_t* argv);
static int32_t i_shell_digital11(uint8_t* argv);
static int32_t i_shell_register(uint8_t* argv);
static int32_t i_shell_mqtt1(uint8_t* argv);
static int32_t i_shell_mqtt11(uint8_t* argv);
static int32_t i_shell_init_gateway (uint8_t* argv);

static cmd_line_t lgn_cmd_table[] = {
    {(const int8_t*)"ver",		i_shell_ver,			(const int8_t*)"get kernel version"},
    {(const int8_t*)"help",		i_shell_help,			(const int8_t*)"help command info"},

    {(const int8_t*)"record",	i_shell_record,			(const int8_t*)"record rf433"},
    {(const int8_t*)"fw",		i_shell_fw,				(const int8_t*)"update firmware"},


    {(const int8_t*)"m",		i_shell_m,				(const int8_t*)"message test"},
    {(const int8_t*)"digital1",  i_shell_digital1,        (const int8_t*)"control digital"},
    {(const int8_t*)"digital11",  i_shell_digital11,        (const int8_t*)"control digital"},
    {(const int8_t*)"register", i_shell_register,       (const int8_t*)"register device"},
    {(const int8_t*)"mqtt1"  ,  i_shell_mqtt1,          (const int8_t*)"control digital over mqtt"},
    {(const int8_t*)"mqtt11"  ,  i_shell_mqtt11,          (const int8_t*)"control digital over mqtt"},
    {(const int8_t*)"init"  ,   i_shell_init_gateway,    (const int8_t*) "init gateway"},

    /* End Of Table */
    {(const int8_t*)0,(pf_cmd_func)0,(const int8_t*)0}
};

uint8_t cls_cmd_buffer[CMD_BUFFER_SIZE];

void* gw_task_console_entry(void*) {
    task_mask_started();
    wait_all_tasks_started();

    APP_DBG("gw_task_console_entry\n");

    while (1) {

        while (msg_available(GW_TASK_CONSOLE_ID)) {
            /* get messge */
            ak_msg_t* msg = rev_msg(GW_TASK_CONSOLE_ID);

            /* handler message */
            switch (msg->header->sig) {

            case GW_CONSOLE_INTERNAL_LOGIN_CMD: {

                get_data_dynamic_msg(msg, cls_cmd_buffer, msg->header->len);

                switch (cmd_line_parser(lgn_cmd_table, cls_cmd_buffer)) {
                case CMD_SUCCESS:
                    break;

                case CMD_NOT_FOUND:
                    if (cls_cmd_buffer[0] != '\r' &&
                            cls_cmd_buffer[0] != '\n') {
                        APP_PRINT("ERROR: cmd unknown\n");
                    }
                    break;

                case CMD_TOO_LONG:
                    APP_PRINT("ERROR: cmd too long\n");
                    break;

                case CMD_TBL_NOT_FOUND:
                    APP_PRINT("ERROR: cmd table not found\n");
                    break;

                default:
                    APP_PRINT("ERROR: cmd error\n");
                    break;
                }
            }
                break;

            case GW_CONSOLE_AC_LOGIN_CMD: {
                uint8_t cmd_len = msg->header->len + 1;
                uint8_t* cmd = (uint8_t*)malloc(cmd_len);
                memset(cmd, 0 , cmd_len);

                get_data_dynamic_msg(msg, cmd, msg->header->len);

                APP_PRINT("%s", cmd);

                free(cmd);
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

int32_t i_shell_ver(uint8_t* argv) {
    (void)argv;
    APP_PRINT("version: %s\n", AK_VERSION);
    return 0;
}

int32_t i_shell_help(uint8_t* argv) {
    uint32_t idx = 0;
    switch (*(argv + 4)) {
    default:
        APP_PRINT("\nCOMMANDS INFORMATION:\n\n");
        while(lgn_cmd_table[idx].cmd != (const int8_t*)0) {
            APP_PRINT("%s\n\t%s\n\n", lgn_cmd_table[idx].cmd, lgn_cmd_table[idx].info);
            idx++;
        }
        break;
    }
    return 0;
}

#define CMD_LINE_OPT_RF433_NONE										(0x00)
#define CMD_LINE_OPT_GW_RF433_ENTRY_RECORD_CODE_REQ					(0x01)
#define CMD_LINE_OPT_GW_RF433_EXIT_RECORD_CODE_REQ						(0x02)
#define CMD_LINE_OPT_GW_RF433_RECORD_DOOR_OPENED_CODE_REQ				(0x03)
#define CMD_LINE_OPT_GW_RF433_RECORD_DOOR_CLOSED_CODE_REQ				(0x04)
#define CMD_LINE_OPT_GW_RF433_RECORD_SMOKE_WARNNING_CODE_REQ			(0x05)
#define CMD_LINE_OPT_GW_RF433_RECORD_WATER_WARNNING_CODE_REQ			(0x06)

int32_t i_shell_record(uint8_t* argv) {
    static uint8_t opt_shell_record = CMD_LINE_OPT_RF433_NONE;

    switch (*(argv + 7)) {

    /* entry record */
    case '0': {
        APP_PRINT("[RF433][RECORD] Do you want to entry record? y/n\n");
        opt_shell_record = CMD_LINE_OPT_GW_RF433_ENTRY_RECORD_CODE_REQ;
    }
        break;

        /* record door opened */
    case '1': {
        APP_PRINT("[RF433][RECORD] Do you want to record door opened? y/n\n");
        opt_shell_record = CMD_LINE_OPT_GW_RF433_RECORD_DOOR_OPENED_CODE_REQ;
    }
        break;

        /* record door closed */
    case '2': {
        APP_PRINT("[RF433][RECORD] Do you want to record door closed? y/n\n");
        opt_shell_record = CMD_LINE_OPT_GW_RF433_RECORD_DOOR_CLOSED_CODE_REQ;
    }
        break;

        /* record smoke warning */
    case '3': {
        APP_PRINT("[RF433][RECORD] Do you want to record smoke warnning? y/n\n");
        opt_shell_record = CMD_LINE_OPT_GW_RF433_RECORD_SMOKE_WARNNING_CODE_REQ;
    }
        break;

        /* record water warnning */
    case '4': {
        APP_PRINT("[RF433][RECORD] Do you want to record water warnning? y/n\n");
        opt_shell_record = CMD_LINE_OPT_GW_RF433_RECORD_WATER_WARNNING_CODE_REQ;
    }
        break;

        /* exit record */
    case '5': {
        APP_PRINT("[RF433][RECORD] Do you want to exit record? y/n\n");
        opt_shell_record = CMD_LINE_OPT_GW_RF433_EXIT_RECORD_CODE_REQ;
    }
        break;

    case 'n':
        opt_shell_record = CMD_LINE_OPT_RF433_NONE;
        break;

    case 'y': {
        switch (opt_shell_record) {
        case CMD_LINE_OPT_GW_RF433_ENTRY_RECORD_CODE_REQ: {
            APP_PRINT("[RF433][RECORD] ENTRY\n");
            ak_msg_t* s_msg = get_pure_msg();
            set_msg_sig(s_msg, GW_RF433_ENTRY_RECORD_CODE_REQ);
            task_post(GW_TASK_RF433_ID, s_msg);
        }
            break;

        case CMD_LINE_OPT_GW_RF433_EXIT_RECORD_CODE_REQ: {
            APP_PRINT("[RF433][RECORD] EXIT\n");
            ak_msg_t* s_msg = get_pure_msg();
            set_msg_sig(s_msg, GW_RF433_EXIT_RECORD_CODE_REQ);
            task_post(GW_TASK_RF433_ID, s_msg);
        }
            break;

        case CMD_LINE_OPT_GW_RF433_RECORD_DOOR_OPENED_CODE_REQ: {
            APP_PRINT("[RF433][RECORD] DOOR OPEND\n");
            ak_msg_t* s_msg = get_pure_msg();
            set_msg_sig(s_msg, GW_RF433_RECORD_DOOR_OPENED_CODE_REQ);
            task_post(GW_TASK_RF433_ID, s_msg);
        }
            break;

        case CMD_LINE_OPT_GW_RF433_RECORD_DOOR_CLOSED_CODE_REQ: {
            APP_PRINT("[RF433][RECORD] DOOR CLOSED\n");
            ak_msg_t* s_msg = get_pure_msg();
            set_msg_sig(s_msg, GW_RF433_RECORD_DOOR_CLOSED_CODE_REQ);
            task_post(GW_TASK_RF433_ID, s_msg);
        }
            break;

        case CMD_LINE_OPT_GW_RF433_RECORD_SMOKE_WARNNING_CODE_REQ: {
            APP_PRINT("[RF433][RECORD] SMOKE WARNNING\n");
            ak_msg_t* s_msg = get_pure_msg();
            set_msg_sig(s_msg, GW_RF433_RECORD_SMOKE_WARNNING_CODE_REQ);
            task_post(GW_TASK_RF433_ID, s_msg);
        }
            break;

        case CMD_LINE_OPT_GW_RF433_RECORD_WATER_WARNNING_CODE_REQ: {
            APP_PRINT("[RF433][RECORD] WATER WARNNING\n");
            ak_msg_t* s_msg = get_pure_msg();
            set_msg_sig(s_msg, GW_RF433_RECORD_WATER_WARNNING_CODE_REQ);
            task_post(GW_TASK_RF433_ID, s_msg);
        }
            break;

        default:
            APP_PRINT("[RF433][RECORD] unknow option\n");
            break;
        }
    }
        break;

        /* help info */
    default: {
        opt_shell_record = CMD_LINE_OPT_RF433_NONE;
        APP_PRINT("record command help:\n");
        APP_PRINT("\trecord 0: entry record mode\n");
        APP_PRINT("\trecord 5: exit record mode\n");
        APP_PRINT("\trecord 1: record door opend\n");
        APP_PRINT("\trecord 2: record door closed\n");
        APP_PRINT("\trecord 3: record smoke warnning\n");
        APP_PRINT("\trecord 4: record water warnning\n");
        APP_PRINT("\trecord y: record yes\n");
        APP_PRINT("\trecord n: record no\n");
    }
        break;
    }

    return 0;
}

int32_t i_shell_fw(uint8_t* argv) {
    (void)argv;
    ak_msg_t* s_msg = get_pure_msg();
    set_msg_sig(s_msg, GW_FIRMWARE_OTA_REQ);
    task_post(GW_TASK_FIRMWARE_ID, s_msg);
    return 0;
}

int32_t i_shell_m(uint8_t* argv) {
    switch (*(argv + 2)) {
    case '0': {
        APP_DBG("send pure test message\n");
        ak_msg_t* s_msg = get_pure_msg();

        set_if_type(s_msg, IF_TYPE_RF24);
        set_if_sig(s_msg, 0xAA);
        set_if_task_id(s_msg, AC_TASK_LIFE_ID);

        set_msg_sig(s_msg, GW_IF_PURE_MSG_OUT);
        task_post(GW_TASK_IF_ID, s_msg);
    }
        break;

    case '1': {
        APP_DBG("send common test message\n");
        ak_msg_t* s_msg = get_common_msg();
        uint8_t send_data[64];
        memset(send_data, 0xBB, 64);

        set_if_type(s_msg, IF_TYPE_RF24);
        set_if_sig(s_msg, 0xBB);
        set_if_task_id(s_msg, AC_TASK_LIFE_ID);
        set_data_common_msg(s_msg, send_data, 64);

        set_msg_sig(s_msg, GW_IF_COMMON_MSG_OUT);
        task_post(GW_TASK_IF_ID, s_msg);
    }
        break;

    case '3': {
        APP_DBG ("Chage to default channel\n");
        radio.setChannel (DEFAULT_CHANNEL);
    }
        break;

    case '4': {
        APP_DBG ("Chage to active channel\n");
        radio.setChannel (gateway_active_channel);
    }
        break;

    default:
        break;
    }
    return 0;
}

int32_t i_shell_digital1 (uint8_t* argv) {
//    uint8_t ID = 01;
    switch (*(argv + 9)) {
    case '0': {
//        //==============================================================
//        slist* slist_found = slist_find(device_list, (void*) &ID, is_device_equal_id);
//        if (slist_found) {
//            device_t* device_found = (device_t*)slist_found->data;
//            device_found->subdevice[SUB_ID].SubID = 0;
//            device_found->subdevice[SUB_ID].state = 0;
//            APP_DBG("Device_information:\nAddress: %d\nSubID: %d\nState SubID: %d\n",
//                    device_found->address, device_found->subdevice[SUB_ID].SubID, device_found->subdevice[SUB_ID].state);
//            //==============================================================
//            ak_msg_t* s_msg = get_common_msg();

//            ControlDigital* gw_dv_ctl_dgt = (ControlDigital*)malloc(sizeof (ControlDigital));
//            gw_dv_ctl_dgt->SourceID = GW_ID;
//            gw_dv_ctl_dgt->DestID = device_found->ID;
//            gw_dv_ctl_dgt->StateDigital = 0;
//            gw_dv_ctl_dgt->SubID = SUB_ID;
//            gw_dv_ctl_dgt->seq_num = init_seq_num();

//            set_if_to_address(s_msg, (uint32_t)device_found->address);


//            set_data_common_msg(s_msg, (uint8_t*)gw_dv_ctl_dgt, sizeof (ControlDigital));

//            set_msg_sig(s_msg, GW_DV_DIGITAL_CTL_US);
//            task_post (GW_TASK_DIGITAL_DEVICE_ID, s_msg);
//        }
        digital_control(01, 00, 0);
    }
        break;
    case '1': {
//        //==============================================================
//        slist* slist_found = slist_find(device_list, (void*) &ID, is_device_equal_id);
//        if (slist_found) {
//            device_t* device_found = (device_t*)slist_found->data;
//            device_found->subdevice[SUB_ID].state = 1;
//            APP_DBG("Device_information:\nAddress: %d\nSubID: %d\nState_SubID: %d\n",
//                    device_found->address, device_found->subdevice[SUB_ID].SubID, device_found->subdevice[SUB_ID].state);
//            //=============================================================
//            ak_msg_t* s_msg = get_common_msg();

//            ControlDigital* gw_dv_ctl_dgt = (ControlDigital*)malloc(sizeof (ControlDigital));
//            gw_dv_ctl_dgt->SourceID = GW_ID;
//            gw_dv_ctl_dgt->DestID = device_found->ID;
//            gw_dv_ctl_dgt->StateDigital = 1;
//            gw_dv_ctl_dgt->SubID = SUB_ID;
//            gw_dv_ctl_dgt->seq_num = init_seq_num();

//            set_if_to_address(s_msg, (uint32_t)device_found->address);


//            set_data_common_msg(s_msg, (uint8_t*)gw_dv_ctl_dgt, sizeof (ControlDigital));

//            set_msg_sig(s_msg, GW_DV_DIGITAL_CTL_US);
//            task_post (GW_TASK_DIGITAL_DEVICE_ID, s_msg);
//        }
        digital_control(01, 00, 1);
    }
        break;
    }
    return 0;
}

int32_t i_shell_digital11 (uint8_t* argv) {
//    uint8_t ID = 06;
    switch (*(argv + 10)) {
    case '0': {
//        //==============================================================
//        slist* slist_found = slist_find(device_list, (void*) &ID, is_device_equal_id);
//        if (slist_found) {
//            device_t* device_found = (device_t*)slist_found->data;
//            device_found->subdevice[SUB_ID].SubID = 0;
//            device_found->subdevice[SUB_ID].state = 0;
//            APP_DBG("Device_information:\nAddress: %d\nSubID: %d\nState SubID: %d\n",
//                    device_found->address, device_found->subdevice[SUB_ID].SubID, device_found->subdevice[SUB_ID].state);
//            //==============================================================
//            ak_msg_t* s_msg = get_common_msg();

//            ControlDigital* gw_dv_ctl_dgt = (ControlDigital*)malloc(sizeof (ControlDigital));
//            gw_dv_ctl_dgt->SourceID = GW_ID;
//            gw_dv_ctl_dgt->DestID = device_found->ID;
//            gw_dv_ctl_dgt->StateDigital = 0;
//            gw_dv_ctl_dgt->SubID = SUB_ID;
//            gw_dv_ctl_dgt->seq_num = init_seq_num();

//            set_if_to_address(s_msg, (uint32_t)device_found->address);


//            set_data_common_msg(s_msg, (uint8_t*)gw_dv_ctl_dgt, sizeof (ControlDigital));

//            set_msg_sig(s_msg, GW_DV_DIGITAL_CTL_US);
//            task_post (GW_TASK_DIGITAL_DEVICE_ID, s_msg);
//        }
        digital_control(011, 00, 0);
    }
        break;
    case '1': {
//        //==============================================================
//        slist* slist_found = slist_find(device_list, (void*) &ID, is_device_equal_id);
//        if (slist_found) {
//            device_t* device_found = (device_t*)slist_found->data;
//            device_found->subdevice[SUB_ID].state = 1;
//            APP_DBG("Device_information:\nAddress: %d\nSubID: %d\nState_SubID: %d\n",
//                    device_found->address, device_found->subdevice[SUB_ID].SubID, device_found->subdevice[SUB_ID].state);
//            //=============================================================
//            ak_msg_t* s_msg = get_common_msg();

//            ControlDigital* gw_dv_ctl_dgt = (ControlDigital*)malloc(sizeof (ControlDigital));
//            gw_dv_ctl_dgt->SourceID = GW_ID;
//            gw_dv_ctl_dgt->DestID = device_found->ID;
//            gw_dv_ctl_dgt->StateDigital = 1;
//            gw_dv_ctl_dgt->SubID = SUB_ID;
//            gw_dv_ctl_dgt->seq_num = init_seq_num();

//            set_if_to_address(s_msg, (uint32_t)device_found->address);


//            set_data_common_msg(s_msg, (uint8_t*)gw_dv_ctl_dgt, sizeof (ControlDigital));

//            set_msg_sig(s_msg, GW_DV_DIGITAL_CTL_US);
//            task_post (GW_TASK_DIGITAL_DEVICE_ID, s_msg);
//        }
        digital_control(011, 00, 1);
    }
        break;
    }
    return 0;
}

int32_t i_shell_register (uint8_t *argv) {
    switch (*(argv + 9)) {
    case '0': {
        APP_DBG ("Go_to_gateway_active_channel\n");
        ak_msg_t* s_msg = get_pure_msg();
        set_msg_sig(s_msg, GW_DV_BROADCAST_GW_CHANNEL_US);
        task_post(GW_TASK_REGISTER_DEVICE_ID, s_msg);
    }
        break;

    case '1': {
//        APP_DBG ("Wait_new_device\n");
//        ak_msg_t* s_msg = get_pure_msg();
//        set_msg_sig (s_msg, GW_DV_BROADCAST_DF_CHANNEL_US);
//        task_post(GW_TASK_REGISTER_DEVICE_ID, s_msg);
        wait_device_register();
    }
        break;
    default:
        break;
    }

    return 0;
}

int32_t i_shell_mqtt1 (uint8_t *argv) {
    switch (*(argv + 6)) {
    case '0': {
        ControlDigital sv_dv_ctl_dgt;
        sv_dv_ctl_dgt.DestID = 01;
        sv_dv_ctl_dgt.SourceID = GW_ID;
        sv_dv_ctl_dgt.SubID = SUB_ID;
        sv_dv_ctl_dgt.StateDigital = 0;
        sv_dv_ctl_dgt.seq_num = 0;

        uint8_t mqtt_buffer[MQTT_BUFFER_MAX_SIZE];
        memcpy(mqtt_buffer, (uint8_t*)&sv_dv_ctl_dgt, sizeof (ControlDigital));
        mqttdigitaldevice->digital_device_public(mqtt_buffer, MQTT_BUFFER_MAX_SIZE);
    }
        break;
    case '1': {
        ControlDigital sv_dv_ctl_dgt;
        sv_dv_ctl_dgt.DestID = 01;
        sv_dv_ctl_dgt.SourceID = GW_ID;
        sv_dv_ctl_dgt.SubID = SUB_ID;
        sv_dv_ctl_dgt.StateDigital = 1;
        sv_dv_ctl_dgt.seq_num = 0;

        uint8_t mqtt_buffer[MQTT_BUFFER_MAX_SIZE];
        memcpy(mqtt_buffer, (uint8_t*)&sv_dv_ctl_dgt, sizeof (ControlDigital));
        mqttdigitaldevice->digital_device_public(mqtt_buffer, MQTT_BUFFER_MAX_SIZE);

    }
        break;
    default:
        break;
    }
    return 0;
}

int32_t i_shell_mqtt11 (uint8_t *argv) {
    switch (*(argv + 7)) {
    case '0': {
        ControlDigital sv_dv_ctl_dgt;
        sv_dv_ctl_dgt.DestID = 06;
        sv_dv_ctl_dgt.SourceID = GW_ID;
        sv_dv_ctl_dgt.SubID = SUB_ID;
        sv_dv_ctl_dgt.StateDigital = 0;
        sv_dv_ctl_dgt.seq_num = 0;

        uint8_t mqtt_buffer[MQTT_BUFFER_MAX_SIZE];
        memcpy(mqtt_buffer, (uint8_t*)&sv_dv_ctl_dgt, sizeof (ControlDigital));
        mqttdigitaldevice->digital_device_public(mqtt_buffer, MQTT_BUFFER_MAX_SIZE);
    }
        break;
    case '1': {
        ControlDigital sv_dv_ctl_dgt;
        sv_dv_ctl_dgt.DestID = 06;
        sv_dv_ctl_dgt.SourceID = GW_ID;
        sv_dv_ctl_dgt.SubID = SUB_ID;
        sv_dv_ctl_dgt.StateDigital = 1;
        sv_dv_ctl_dgt.seq_num = 0;

        uint8_t mqtt_buffer[MQTT_BUFFER_MAX_SIZE];
        memcpy(mqtt_buffer, (uint8_t*)&sv_dv_ctl_dgt, sizeof (ControlDigital));
        mqttdigitaldevice->digital_device_public(mqtt_buffer, MQTT_BUFFER_MAX_SIZE);

    }
        break;
    default:
        break;
    }
    return 0;
}

int32_t i_shell_init_gateway (uint8_t* argv) {
     (void) argv;
//     ak_msg_t* s_msg = get_pure_msg();

//     set_msg_sig(s_msg, GW_START_QUERRY_CHANNEL);
//     task_post(GW_TASK_INIT_GATEWAY_ID, s_msg);
    init_gateway();
     return 0;
}
