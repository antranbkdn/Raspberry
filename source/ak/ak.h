/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   12/01/2017
 * @brief:  Main defination of active kernel
 ******************************************************************************
**/
#ifndef __AK_H__
#define __AK_H__

#include <stdint.h>
#include <pthread.h>

#include "message.h"
#include "timer.h"

#define AK_VERSION						"1.0"

#define AK_RET_OK						(0x00)
#define AK_RET_NG						(0x01)

#define TASK_PRI_MAX_SIZE				(8)

#define TASK_PRI_LEVEL_1				(1)
#define TASK_PRI_LEVEL_2				(2)
#define TASK_PRI_LEVEL_3				(3)
#define TASK_PRI_LEVEL_4				(4)
#define TASK_PRI_LEVEL_5				(5)
#define TASK_PRI_LEVEL_6				(6)
#define TASK_PRI_LEVEL_7				(7)
#define TASK_PRI_LEVEL_8				(8)

#define AK_COMMON_MSG_DATA_SIZE			(64)

typedef void* (*pf_task)(void*);

typedef struct {
	/* public interface */
	uint32_t		id;
	uint32_t		pri;
	pf_task			task;

	q_msg_t*		mailbox;

	const char*		info;

	/* private interface */
	pthread_t		pthread;
} ak_task_t;

typedef enum {
	TIMER_ONE_SHOT,
	TIMER_PERIODIC
} timer_type_t;

typedef struct {
	uint8_t task_id;
	uint8_t sig;

    uint16_t address;
} ak_msg_pure_if_t;

typedef struct {
	uint8_t task_id;
	uint8_t sig;

	uint8_t len;
    uint8_t data[AK_COMMON_MSG_DATA_SIZE];

    uint16_t address;
} ak_msg_common_if_t;

/******************************************************************************
* message services
*
*******************************************************************************/
/**
 * @brief get_pure_msg
 * message only contain header info, it does not contain payload.
 * @return
 * ak_msg_t
 */
extern ak_msg_t* get_pure_msg();

/**
 * @brief get_dymanic_msg
 * message only contain header info, it does not contain payload.
 * @return
 * ak_msg_t
 */
extern ak_msg_t* get_dymanic_msg();

/**
 * @brief get_common_msg
 * message only contain header info, it does not contain payload.
 * @return
 * ak_msg_t
 */
extern ak_msg_t* get_common_msg();

/**
 * @brief ak_memcpy_msg
 * clone src message.
 * @param src
 * @return
 * the pointer that point to clone message.
 */
extern ak_msg_t* ak_memcpy_msg(ak_msg_t* src);

/**
 * @brief ak_free_msg
 * free ak message (include pure. dynamic and commmon message type).
 * @param msg
 */
extern void ak_free_msg(ak_msg_t* msg);

/**
 * @brief set_msg_sig
 * set signal for message.
 * @param msg
 * @param sig
 */
extern void set_msg_sig(ak_msg_t* msg, uint32_t sig);

/**
 * @brief set_msg_task_id
 * set tatget task id for message.
 * @param msg
 * @param task_id
 */
extern void set_msg_task_id(ak_msg_t* msg, uint32_t task_id);

/**
 * @brief set_if_task_id
 * set interface task id, this field is used for if task
 * @param msg
 * @param id
 */
extern void set_if_task_id(ak_msg_t* msg, uint32_t id);

/**
 * @brief set_if_type
 * set interface type, this field is used for if task
 * @param msg
 * @param type
 */
extern void set_if_type(ak_msg_t* msg, uint32_t type);

/**
 * @brief set_if_sig
 * set interface sig, this field is used for if task
 * @param msg
 * @param sig
 */
extern void set_if_sig(ak_msg_t* msg, uint32_t sig);

/**
 * @brief set_if_data_common_msg
 * set interface data message, this field is used for task if
 * @param msg
 * @param data
 * @param len
 */

extern void set_if_to_address(ak_msg_t* msg, uint32_t to_address);








extern void set_if_data_common_msg(ak_msg_t* msg, uint8_t* data, uint32_t len);

/**
 * @brief set_data_common_msg
 * set data message for common message
 * @param msg
 * @param data
 * @param len
 */
extern void set_data_common_msg(ak_msg_t* msg, uint8_t* data, uint32_t len);

/**
 * @brief get_data_common_msg
 * set data message for common message
 * @param msg
 * @param data
 * @param len
 */
extern void get_data_common_msg(ak_msg_t* msg, uint8_t* data, uint32_t len);

/**
 * @brief get_data_len_common_msg
 * get data message for common message
 * @param msg
 * @return
 */
extern uint8_t get_data_len_common_msg(ak_msg_t* msg);

/**
 * @brief set_data_dynamic_msg
 * set data message for dynamic message
 * @param msg
 * @param data
 * @param len
 */
extern void set_data_dynamic_msg(ak_msg_t* msg, uint8_t* data, uint32_t len);

/**
 * @brief get_data_dynamic_msg
 * get data message for dynamic message
 * @param msg
 * @param data
 * @param len
 */
extern void get_data_dynamic_msg(ak_msg_t* msg, uint8_t* data, uint32_t len);

/**
 * @brief task_post
 * post message to target task
 * @param task_dst_id
 * @param msg
 */
extern void task_post(uint32_t task_dst_id, ak_msg_t* msg);

/**
 * @brief msg_available
 * check message exist in message queue
 * @param task_id
 * @return
 */
extern bool msg_available(uint32_t task_id);

/**
 * @brief rev_msg
 * get task message queue
 * @param task_id
 * @return
 */
extern ak_msg_t* rev_msg(uint32_t task_id);

/**
 * brief msg_inc_ref_count
 * @param msg
 */
extern void msg_inc_ref_count(ak_msg_t* msg);

/**
 * @brief msg_dec_ref_count
 * @param msg
 */
extern void msg_dec_ref_count(ak_msg_t* msg);

/**
 * @brief get_msg_ref_count
 * @param msg
 * @return
 */
extern uint32_t get_msg_ref_count(ak_msg_t* msg);

/**
 * @brief get_msg_type
 * @param msg
 * @return
 */
extern uint32_t get_msg_type(ak_msg_t* msg);

/**
 * @brief free_msg
 * @param msg
 */
extern void free_msg(ak_msg_t* msg);

/******************************************************************************
* task function
*
*******************************************************************************/
/**
 * @brief task_init
 * function is called before create threads
 */
extern void task_init();

/**
 * @brief task_mask_started
 * function is call when task entry, to mask that current task is started.
 */
extern void task_mask_started();

/**
 * @brief wait_all_tasks_started
 * function using to make sure that all task is initialed
 */
extern void wait_all_tasks_started();

/******************************************************************************
* timer services
*
*******************************************************************************/
/**
 * @brief timer_set
 * start timer
 * @param task_id
 * target task
 * @param sig
 * target signal
 * @param duty
 * @param timer_type
 * TIMER_ONE_SHOT
 * TIMER_PERIODIC
 * @return
 */
extern uint32_t timer_set(uint32_t task_id, uint32_t sig, uint32_t duty, timer_type_t timer_type);

/**
 * @brief timer_remove_attr
 * remove timer node
 * @param task_id
 * @param sig
 * @return
 */
extern uint32_t timer_remove_attr(uint32_t task_id, uint32_t sig);

/******************************************************************************
* fsm services
*
*******************************************************************************/
#define FSM(me, init_func)  ((fsm_t*)me)->state = (state_handler)init_func
#define FSM_TRAN(me, target)    ((fsm_t*)me)->state = (state_handler)target

typedef void (*state_handler)(ak_msg_t* msg);

typedef struct {
	state_handler state;
} fsm_t;

extern void fsm_dispatch(fsm_t* me, ak_msg_t* msg);

/******************************************************************************
* fsm services
*
*******************************************************************************/
#define TSM_FUNCTION_NULL		((tsm_func_f)0)
typedef void (*tsm_func_f)(ak_msg_t*);

typedef uint8_t tsm_state_t;

typedef struct {
	uint8_t sig;				/* signal */
	tsm_state_t next_state;		/* next state */
	tsm_func_f tsm_func;		/* state handler function */
} tsm_t;

typedef struct {
	tsm_state_t state;
	tsm_t** table;
} tsm_tbl_t;

#define TSM(t, s) tsm_init(t, s)

void tsm_init(tsm_tbl_t* tsm_tbl, tsm_state_t state);
void tsm_dispatch(tsm_tbl_t* tsm_tbl, ak_msg_t* msg);

#endif //__AK_H__
