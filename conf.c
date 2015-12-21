#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "conf.h"
#include "log.h"

struct Config* parse_args(int argc, char *argv[])
{
    long port;

    struct Config *conf = malloc(sizeof(struct Config));
    assert(conf != NULL);

    wlogf(CFGDEBUG, "Found %d arguments...\n", argc-1);
    if (argc <= 1)
    {
        wlogf(CFGERROR, "No arguments given\n");
        return NULL;
    }
    wlogf(CFGDEBUG, "Hostname argument found: %s\n", argv[1]);
    strncpy(conf->hostname, argv[1], sizeof(conf->hostname));
    if (argc <= 2)
    {
        wlogf(WARN, "No port specified, defaulting to 6667\n");
        port = 6667;
    }
    else
    {
        wlogf(CFGDEBUG, "Port argument found: %s\n", argv[2]);
        if (argv[2][0] == '+')
        {
            wlogf(CFGDEBUG, "Using SSL\n", argv[2]);
            conf->ssl = true;
            port = strtol(argv[2]+1, NULL, 0);
        }
        else
        {
            wlogf(CFGDEBUG, "Not using SSL\n", argv[2]);
            port = strtol(argv[2], NULL, 0);
        }
    }
    if (port >= 65536)
    {
        wlogf(CFGERROR, "Port must be no greater than 65535\n");
        return NULL;
    }
    conf->port = port;
    wlogf(CFGDEBUG, "Config: {host:%s, port:%d, ssl:%s}\n",
          conf->hostname, conf->port, conf->ssl?"Yes":"No");
    return conf;
}
