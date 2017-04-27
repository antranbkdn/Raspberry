#ifndef __SYS_DBG_H__
#define __SYS_DBG_H__
#include <stdint.h>
#include <stdio.h>

#define SYS_PRINT_EN
#define SYS_DBG_EN

#if defined(SYS_PRINT_EN)
#define SYS_PRINT(fmt, ...)       printf(fmt, ##__VA_ARGS__)
#else
#define SYS_PRINT(fmt, ...)
#endif

#if defined(SYS_DBG_EN)
#define SYS_DBG(fmt, ...)       printf(fmt, ##__VA_ARGS__)
#else
#define SYS_DBG(fmt, ...)
#endif
#define FATAL(s, c) sys_dbg_fatal((const char*)s, (uint8_t)c)

extern void sys_dbg_fatal(const char* s, uint8_t c);

#endif //__SYS_DBG_H__
