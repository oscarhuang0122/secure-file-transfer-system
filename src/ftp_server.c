// ftp_server.c
#include "ftp_common.h"
#include "ftp_protocol.h"
#include "ftp_transfer.h"
#include "ftp_auth.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

typedef enum
{
    STATE_WAIT_AUTH,
    STATE_READY
}client_state_t;

static const char *cmd_to_str(int cmd)
{
    switch (cmd)
    {
        case CMD_AUTH: return "AUTH";
        case CMD_UPLOAD: return "UPLOAD";
        case CMD_DOWNLOAD: return "DOWNLOAD";
        case CMD_LIST: return "LIST";
        case CMD_QUIT: return "QUIT";
        case CMD_OK: return "OK";
        case CMD_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

static int handle_client(int client_sock)
{
    client_state_t state = STATE_WAIT_AUTH;
    ftp_header_t hdr;
    char buffer[BUFFER_SIZE];

    while (1)
    {
        if (recv_header(client_sock, &hdr) < 0)
            break;

        printf("Received command: %s\n", cmd_to_str(hdr.cmd));
        
        switch (state)
        {
            case STATE_WAIT_AUTH:
            {
                if (hdr.cmd != CMD_AUTH)
                    return -1;
                
                if (recv_all(client_sock, buffer, hdr.payload_size) <= 0)
                    return -1;

                char user[32], pass[32];
                sscanf(buffer, "%31s %31s", user, pass);

                if (authenticate(user, pass))
                {
                    send_header(client_sock, CMD_OK, 0);
                    state = STATE_READY;
                }
                else
                {
                    send_header(client_sock, CMD_ERROR, 0);
                    return -1;
                }
                break;
            }

            case STATE_READY:
            {
                if (hdr.cmd == CMD_UPLOAD)
                {
                    if (recv_all(client_sock, buffer, hdr.payload_size) <= 0)
                        return -1;

                    char filename[MAX_FILENAME];
                    uint32_t filesize = 0;

                    if (sscanf(buffer, "%255s %u", filename, &filesize) != 2)
                        return -1;

                    if (handle_upload(client_sock, filesize, filename) == 0)
                    {
                        send_header(client_sock, CMD_OK, 0);
                        printf("Sending file: %s (%u bytes)\n", filename, filesize);
                    }
                    else
                        send_header(client_sock, CMD_ERROR, 0);
                }
                else if (hdr.cmd == CMD_DOWNLOAD)
                {
                    if (recv_all(client_sock, buffer, hdr.payload_size) <= 0)
                        return -1;

                    if (handle_download(client_sock, buffer) == 0)
                        send_header(client_sock, CMD_OK, 0);
                    else 
                        send_header(client_sock, CMD_ERROR, 0);
                }
                else if (hdr.cmd == CMD_QUIT)
                {
                    close(client_sock);
                    return 0;
                }
                break;
            }
        }
    }

    close(client_sock);
    return 0;
}

int main()
{
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_sock, 5);

    printf("FTP server listening on port %d\n", PORT);

    while (1)
    {
        int client_sock = accept(server_sock, NULL, NULL);
        if (client_sock >= 0)
        {
            handle_client(client_sock);
        }
    }

    close(server_sock);
    return 0;
}