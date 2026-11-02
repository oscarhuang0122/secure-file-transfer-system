// ftp_transfer.c
#include "ftp_transfer.h"
#include "ftp_protocol.h"
#include "ftp_common.h"
#include "ftp_crypto.h"
#include "ftp_crc.h"
#include <stdio.h>
#include <arpa/inet.h>

int handle_upload(int sock, uint32_t filesize, const char *filename)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp) return -1;

    char buffer[BUFFER_SIZE];
    uint32_t remaining = filesize;

    while (remaining > 0)
    {
        uint32_t chunk = remaining > BUFFER_SIZE ? BUFFER_SIZE : remaining;

        if (recv_all(sock, buffer, chunk) <= 0)
        {
            fclose(fp);
            return -1;
        }

        uint32_t net_crc;
        if (recv_all(sock, &net_crc, sizeof(net_crc)) <= 0)
        {
            fclose(fp);
            return -1;
        }

        uint32_t received_crc = ntohl(net_crc);
        uint32_t computed_crc = crc32_compute((uint8_t*)buffer, chunk);

        if (received_crc != computed_crc)
        {
            printf("CRC ERROR: upload corrupted\n");
            fclose(fp);
            return -1;
        }

        fwrite(buffer, 1, chunk, fp);
        remaining -= chunk;
    }

    fclose(fp);
    return 0;
}

int handle_download(int sock, const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) return -1;

    fseek(fp, 0, SEEK_END);
    uint32_t filesize = ftell(fp);
    rewind(fp);

    send_header(sock, CMD_DOWNLOAD, filesize);

    uint8_t buffer[BUFFER_SIZE];
    uint8_t encrypted[BUFFER_SIZE + 32];
    
    while (1)
    {
        size_t read_bytes = fread(buffer, 1, BUFFER_SIZE, fp);
        if (read_bytes == 0) break;

        int enc_size = aes_encrypt(buffer, read_bytes, encrypted);

        uint32_t crc = crc32_compute(encrypted, enc_size);

        uint32_t net_size = htonl(enc_size);
        uint32_t net_crc = htonl(crc);

        if (send_all(sock, &net_size, sizeof(net_size)) < 0)
            return -1;

        if (send_all(sock, encrypted, enc_size) < 0)
            return -1;

        if (send_all(sock, &net_crc, sizeof(net_crc)) < 0)
            return -1;
    }

    fclose(fp);
    return 0;
}