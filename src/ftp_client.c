//ftp_client.c
#include "ftp_common.h"
#include "ftp_protocol.h"
#include "ftp_crypto.h"
#include "ftp_crc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

static int authenticate_client(int sock)
{ 
    char buffer[BUFFER_SIZE];

    printf("Sending AUTH\n");
    snprintf(buffer, sizeof(buffer), "admin password");

    send_header(sock, CMD_AUTH, strlen(buffer) + 1);
    send_all(sock, buffer, strlen(buffer) + 1);

    ftp_header_t hdr;
    if (recv_header(sock, &hdr) < 0)
        return -1;

    printf("Received response cmd: %d\n", hdr.cmd);
    return (hdr.cmd == CMD_OK) ? 0 : -1;
} 

static int upload_file(int sock, const char *filename)
{ 
    FILE *fp = fopen(filename, "rb");
    if (!fp) return -1;

    fseek(fp, 0, SEEK_END);
    uint32_t filesize = ftell(fp);
    rewind(fp);

    char meta[BUFFER_SIZE];
    snprintf(meta, sizeof(meta), "%s %u", filename, filesize);

    send_header(sock, CMD_UPLOAD, strlen(meta) + 1);
    send_all(sock, meta, strlen(meta) + 1);
    
    char buffer[BUFFER_SIZE];
    uint32_t remaining = filesize;
    
    printf("Uploading %u bytes...\n", filesize);

    while (remaining > 0)
    { 
        uint32_t chunk = remaining > BUFFER_SIZE ? BUFFER_SIZE : remaining;
        size_t bytes_read = fread(buffer, 1, chunk, fp);

        if (bytes_read == 0)
        { 
            fclose(fp);
            return -1;
        }

        uint32_t crc = crc32_compute((uint8_t*)buffer, bytes_read);
        uint32_t net_crc = htonl(crc);

        if (send_all(sock, buffer, bytes_read) < 0)
        { 
            fclose(fp);
            return -1;
        }

        if (send_all(sock, &net_crc, sizeof(net_crc)) < 0)
        { 
            fclose(fp);
            return -1;
        }

        remaining -= bytes_read;
    }

    printf("Upload complete\n");
    
    fclose(fp);

    ftp_header_t hdr;
    if (recv_header(sock, &hdr) < 0)
        return -1;
    
    return (hdr.cmd == CMD_OK) ? 0 : -1;
} 

static int download_file(int sock, const char *filename)
{ 
    send_header(sock, CMD_DOWNLOAD, strlen(filename) + 1);
    send_all(sock, filename, strlen(filename) + 1);

    ftp_header_t hdr;
    if (recv_header(sock, &hdr) < 0)
        return -1;

    FILE *fp = fopen("downloaded.bin", "wb");
    if (!fp) return -1;
    
    uint8_t buffer[BUFFER_SIZE + 32];
    uint8_t decrypted[BUFFER_SIZE];
    
    uint32_t remaining = hdr.payload_size;
    
    while (remaining > 0)
    {
        printf("Downloading chunk...\n");
        uint32_t net_size; 
        if (recv_all(sock, &net_size, sizeof(net_size)) <= 0)
            break;
            
        uint32_t enc_size = ntohl(net_size);
        if (recv_all(sock, buffer, enc_size) <= 0)
            break;
            
        uint32_t net_crc;
        if (recv_all(sock, &net_crc, sizeof(net_crc)) <= 0)
            break;
            
        uint32_t received_crc = ntohl(net_crc);
        uint32_t computed_crc = crc32_compute(buffer, enc_size);
        
        if (received_crc != computed_crc)
        { 
            printf("CRC ERROR: corrupted chunk\n");
            fclose(fp);
            return -1;
        } 
        
        int dec_size = aes_decrypt(buffer, enc_size, decrypted);
        fwrite(decrypted, 1, dec_size, fp);

        remaining -= dec_size;
    }

    printf("Download complete\n");
    
    fclose(fp);
    return 0;
}

int main()
{ 
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
    
    connect(sock, (struct sockaddr*)&server, sizeof(server));
    
    if (authenticate_client(sock) != 0)
    {
        printf("Authentication failed\n");
        return -1;
    }
    
    upload_file(sock, "test.bin");
    download_file(sock, "test.bin");

    send_header(sock, CMD_QUIT, 0);
    close(sock);

    return 0;
}