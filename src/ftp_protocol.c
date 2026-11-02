// ftp_protocol.c
#include "ftp_protocol.h"
#include "ftp_common.h"
#include <arpa/inet.h>

int send_header(int sock, ftp_cmd_t cmd, uint32_t payload_size)
{
    ftp_header_t hdr;
    hdr.magic = htonl(FTP_MAGIC);
    hdr.cmd = htons(cmd);
    hdr.reserved = 0;
    hdr.payload_size = htonl(payload_size);

    return send_all(sock, &hdr, sizeof(hdr));
}

int recv_header(int sock, ftp_header_t *hdr)
{
    if (recv_all(sock, hdr, sizeof(*hdr)) <= 0)
        return -1;

    hdr->magic = ntohl(hdr->magic);
    hdr->cmd = ntohs(hdr->cmd);
    hdr->payload_size = ntohl(hdr->payload_size);

    if (hdr->magic != FTP_MAGIC)
        return -1;

    return 0;
}