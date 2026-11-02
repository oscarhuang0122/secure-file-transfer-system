// ftp_net.c
#include "ftp_common.h"
#include <sys/socket.h>

int send_all(int sock, const void *buf, size_t len)
{
    size_t total = 0;
    const char *p = buf;

    while (total < len)
    {
        ssize_t n = send(sock, p + total, len - total, 0);
        if (n <= 0) return -1;
        total += n;
    }
    return total;
}

int recv_all(int sock, void *buf, size_t len)
{
    size_t total = 0;
    char *p = buf;

    while (total < len)
    {
        ssize_t n = recv(sock, p + total, len - total, 0);
        if (n <= 0) return -1;
        total += n;
    }
    return total;
}