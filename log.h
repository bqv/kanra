#ifndef _LOG_H_
#define _LOG_H_

/*
 * 0x - Verbose
 * 1x - Debug 
 * 2x - Informational
 * 3x - Plugin 
 * 4x - Warning 
 * 5x - Error
 */

#include <stdarg.h>

#define TRACE       000
#define NETTRACE    002

#define DEBUG       010
#define CFGDEBUG    011
#define NETDEBUG    012

#define INFO        020

#define HOOK        030

#define WARN        040
#define NETWARN     042

#define ERROR       050
#define CFGERROR    051
#define NETERROR    052
#define CATASTROPHE 057

int wlogf(short code, char *msgfmt, ...);
int wvlogf(short code, char *msgfmt, va_list args);

#endif /*LOG_H*/
