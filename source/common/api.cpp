#include "api.h"
#include "slist.h"
#include "../app_data.h"
#include "../task_digital_device.h"
#include "../task_list.h"
#include "../task_list_if.h"
#include "../app_if.h"
#include "../app_dbg.h"

void digital_control(uint8_t ID, uint8_t SUBID, uint8_t State) {
    slist* slist_found = slist_find(device_list, (void*) &ID, is_device_equal_id);
    if (slist_found) {
        device_t* device_found = (device_t*)slist_found->data;
        device_found->subdevice[SUBID].SubID = SUBID;
        device_found->subdevice[SUBID].state = State;                           // de kiem tra response tu device co khop ko
        APP_DBG("Device_information:\nAddress: %d\nSubID: %d\nState SubID: %d\n",
                device_found->address, device_found->subdevice[SUBID].SubID, device_found->subdevice[SUB_ID].state);
        //==============================================================
        ak_msg_t* s_msg = get_common_msg();

        ControlDigital* gw_dv_ctl_dgt = (ControlDigital*)malloc(sizeof (ControlDigital));
        gw_dv_ctl_dgt->SourceID = GW_ID;
        gw_dv_ctl_dgt->DestID = device_found->ID;
        gw_dv_ctl_dgt->StateDigital = State;
        gw_dv_ctl_dgt->SubID = SUBID;
        gw_dv_ctl_dgt->seq_num = init_seq_num();

        set_if_to_address(s_msg, (uint32_t)device_found->address);


        set_data_common_msg(s_msg, (uint8_t*)gw_dv_ctl_dgt, sizeof (ControlDigital));

        set_msg_sig(s_msg, GW_DV_DIGITAL_CTL_US);
        task_post (GW_TASK_DIGITAL_DEVICE_ID, s_msg);
    }
}

void analog_control(uint8_t ID, uint8_t SUBID, uint8_t Value) {
    slist* slist_found = slist_find(device_list, (void*) &ID, is_device_equal_id);
    if (slist_found) {
        device_t* device_found = (device_t*)slist_found->data;
        device_found->subdevice[SUBID].SubID = SUBID;
        device_found->subdevice[SUBID].state = Value;                           // de kiem tra response tu device co khop ko
        APP_DBG("Device_information:\nAddress: %d\nSubID: %d\nState SubID: %d\n",
                device_found->address, device_found->subdevice[SUBID].SubID, device_found->subdevice[SUB_ID].state);
        //==============================================================
        ak_msg_t* s_msg = get_common_msg();

        ControlDigital* gw_dv_ctl_dgt = (ControlDigital*)malloc(sizeof (ControlDigital));
        gw_dv_ctl_dgt->SourceID = GW_ID;
        gw_dv_ctl_dgt->DestID = device_found->ID;
        gw_dv_ctl_dgt->StateDigital = Value;
        gw_dv_ctl_dgt->SubID = SUBID;
        gw_dv_ctl_dgt->seq_num = init_seq_num();

        set_if_to_address(s_msg, (uint32_t)device_found->address);


        set_data_common_msg(s_msg, (uint8_t*)gw_dv_ctl_dgt, sizeof (ControlDigital));

        set_msg_sig(s_msg, GW_DV_DIGITAL_CTL_US);
        task_post (GW_TASK_DIGITAL_DEVICE_ID, s_msg);
    }

}

void wait_device_register() {
    APP_DBG ("Wait_new_device\n");
    ak_msg_t* s_msg = get_pure_msg();
    set_msg_sig (s_msg, GW_DV_BROADCAST_DF_CHANNEL_US);
    task_post(GW_TASK_REGISTER_DEVICE_ID, s_msg);
}

void init_gateway() {
    ak_msg_t* s_msg = get_pure_msg();

    set_msg_sig(s_msg, GW_START_QUERRY_CHANNEL);
    task_post(GW_TASK_INIT_GATEWAY_ID, s_msg);
}
