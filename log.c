#include <stdio.h>
#include <stdlib.h>
#include "log.h"

const char *ltype[] = { "TRC","","TNET","","","","","",
                        "DBG","DCFG","DNET","","","","","",
                        "INFO","","","","","","","",
                        "HOOK","","","","","","","",
                        "WARN","","WNET","","","","","",
                        "ERR","ECFG","ENET","","","","","EGAD" };

void die()
{
    exit(-1);
}

int wlogf(short code, char *msgfmt, ...)
{
    va_list args;
    int ret;

    va_start(args, msgfmt);
    ret = wvlogf(code, msgfmt, args);
    va_end(args);
    return ret;
}

int wvlogf(short code, char *msgfmt, va_list args)
{
    char buffer[512] = {0};
    int ret;

    snprintf(buffer, 512, "+%02d %s %s", code, ltype[code], msgfmt);
    ret = vfprintf(stdout, buffer, args);
    fflush(stdout);
    if (code >= 50)
    {
        die();
    }
    return ret;
}
