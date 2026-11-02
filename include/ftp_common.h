// ftp_common.h
#ifndef FTP_COMMON_H
#define FTP_COMMON_H

#define PORT 2121
#define BUFFER_SIZE 16384 // 16 KB
#define MAX_FILENAME 256
#define MAX_CMD 16

#include <stddef.h>

int send_all(int sock, const void *buf, size_t len);
int recv_all(int sock, void *buf, size_t len);

#endif