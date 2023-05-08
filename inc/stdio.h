#ifndef INC_STDIO_H__
#define INC_STDIO_H__
#include <inc/monitor.h>
#include <inc/stdarg.h>

void cprintf(const char *fmt, ...);
int vcprintf(const char *fmt, va_list ap);
void printfmt(void (*putch)(int, void *), void *putdat, const char *fmt, ...);
void vprintfmt(void (*putch)(int, void *), void *putdat, const char *fmt,
               va_list);

#endif
