// ftp_protocol.h
#ifndef FTP_PROTOCOL_H
#define FTP_PROTOCOL_H

#include <stdint.h>

#define FTP_MAGIC 0x46545031  // FTP1

typedef enum
{
    CMD_AUTH = 1,
    CMD_UPLOAD = 2,
    CMD_DOWNLOAD = 3,
    CMD_LIST = 4,
    CMD_QUIT = 5,
    CMD_OK = 6,
    CMD_ERROR = 7
}ftp_cmd_t;

typedef struct
{
    uint32_t magic;
    uint16_t cmd;
    uint16_t reserved;
    uint32_t payload_size;
}__attribute__((packed)) ftp_header_t;

int send_header(int sock, ftp_cmd_t cmd, uint32_t payload_size);
int recv_header(int sock, ftp_header_t *hdr);

#endif