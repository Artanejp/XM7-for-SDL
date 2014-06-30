/*
 * Log functions
 * (C) 2014-06-30 K.Ohta
 * Licence : Apache
 */

#include "agar_logger.h"

static int syslog_flag = 0;
static int log_cons = 0;
static int log_onoff = 0;
extern "C" 
{
   
void XM7_OpenLog(int syslog, int cons)
     {
	int flags = 0;
	
	log_onoff = 1;
	if(syslog != 0) {
	   syslog_flag = -1;
#if defined(_SYS_SYSLOG_H) || defined(_SYSLOG_H)
	   if(cons != 0) { 
	      flags = LOG_CONS;
	   }
	   openlog("XM7", flags | LOG_PID | LOG_NOWAIT, LOG_USER);
#endif
	} else {
	   syslog_flag = 0;
	}
	log_cons = cons;
     }
   
   
void XM7_DebugLog(int level, char *fmt, ...)
     {
	va_list ap;
	struct tm *timedat;
	time_t nowtime;
	char strbuf[4096];
	char strbuf2[256];
	char strbuf3[24];
	struct timeval tv;
	int level_flag = LOG_USER;
	
	if(log_onoff == 0) return;
	
	if(level == XM7_LOG_DEBUG) {
	   level_flag |= LOG_DEBUG;
	} else if(level == XM7_LOG_INFO) { 
	   level_flag |= LOG_INFO;
	} else if(level == XM7_LOG_WARN) {
	   level_flag |= LOG_WARNING;
	} else {
	   level_flag |= LOG_DEBUG;
	}
	
	
	va_start(ap, fmt);	
	vsnprintf(strbuf, 4095, fmt, ap);
	nowtime = time(NULL);
	gettimeofday(&tv, NULL);
	if(log_cons != 0) { // Print only
	   timedat = localtime(&nowtime);
	   strftime(strbuf2, 255, "XM7: %Y-%m-%d %H:%M:%S", timedat);
	   snprintf(strbuf3, 23, ".%06d", tv.tv_usec);
	   fprintf(stdout, "%s%s %s\n", strbuf2, strbuf3, strbuf);
	} 
	if(syslog_flag != 0) { // SYSLOG
	   syslog(level_flag, "uS=%06d %s", tv.tv_usec, strbuf);
	}
	va_end(ap);
     }

void XM7_SetLogStatus(int sw)
     {
	if(sw == 0) {
	   log_onoff = 0;
	} else {
	   log_onoff = 1;
	}
     }
   
	
void XM7_CloseLog(void)
    {
	if(syslog_flag != 0) {
	     closelog();
	}
	syslog_flag = 0;
	log_cons = 0;
        log_onoff = 0;
     }
   
}
