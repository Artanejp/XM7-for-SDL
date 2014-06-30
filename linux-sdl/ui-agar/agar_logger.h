/*
 * Log functions
 * (C) 2014-06-30 K.Ohta
 * Licence : Apache
 */

#include <stdarg.h>
#include <stdio.h>

#include <syslog.h>

#include <time.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif
   extern void XM7_OpenLog(int syslog, int cons);
   extern void XM7_DebugLog(int level, char *fmt, ...);
   extern void XM7_CloseLog(void);
   extern void XM7_SetLogStatus(int sw);

#define XM7_LOG_ON 1
#define XM7_LOG_OFF 0
   
#define XM7_LOG_DEBUG 0
#define XM7_LOG_INFO 1
#define XM7_LOG_WARN 2
   
#ifdef __cplusplus
}
#endif
   