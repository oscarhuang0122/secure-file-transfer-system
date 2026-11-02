// ftp_auth.c
#include "ftp_auth.h"
#include <string.h>

int authenticate(const char *user, const char *pass)
{
    return strcmp(user, "admin") == 0 && 
           strcmp(pass, "password") == 0;
}
