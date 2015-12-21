#ifndef _CONF_H_
#define _CONF_H_

#include <stdbool.h>
#include <stdint.h>

#ifndef NI_MAXHOST
#define NI_MAXHOST 1024+1
#endif

struct Config
{
    char hostname[NI_MAXHOST];
    uint16_t port;
    bool ssl;
};

struct Config* parse_args(int argc, char *argv[]);

#endif /*CONF_H*/
