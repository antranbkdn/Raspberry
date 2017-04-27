/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   12/01/2017
 * @brief: debug ak function
 ******************************************************************************
**/
#ifndef __AK_DBG_H__
#define __AK_DBG_H__

#include <stdio.h>

#define AK_MSG_DBG_EN				0
#define AK_TIMER_DBG_EN				0

#if (AK_MSG_DBG_EN == 1)
#define AK_MSG_DBG(fmt, ...)       printf(fmt, ##__VA_ARGS__)
#else
#define AK_MSG_DBG(fmt, ...)
#endif

#if (AK_TIMER_DBG_EN == 1)
#define AK_TIMER_DBG(fmt, ...)       printf(fmt, ##__VA_ARGS__)
#else
#define AK_TIMER_DBG(fmt, ...)
#endif

#endif //__AK_DBG_H__
