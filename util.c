#include <string.h>
#include "util.h"

char *trim_crlf(char *str)
{
    size_t n = strlen(str);
    str[n-2] = '\0';
    return str;
}
