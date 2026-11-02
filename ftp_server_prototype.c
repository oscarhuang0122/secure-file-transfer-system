// ftp_server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 2121
#define BUFFER_SIZE 1024

#define USER "admin"
#define PASS "password"

void* client_thread(void* arg);
void handle_client(int client_socket);

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Socket failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);
    printf("FTP Server listening on port %d...", PORT);

    while(1)
    {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);

        if (client_socket < 0)
        {
            perror("Accept failed");
            continue;
        }

        char user[50], pass[50];
        recv(client_socket, user, sizeof(user), 0);
        recv(client_socket, pass, sizeof(pass), 0);

        if (strcmp(user, USER) != 0 || strcmp(pass, PASS) != 0)
        {
            send(client_socket, "AUTH_FAIL", 9, 0);
            close(client_socket);
            continue;
        }

        send(client_socket, "AUTH_OK", 7, 0);
        printf("Client connected.\n");

        int* pclient = malloc(sizeof(int));
        *pclient = client_socket;

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, pclient);
        pthread_detach(tid); // Clean up thread automatically
    }

    close(server_socket);
    return 0;
}

void handle_client(int client_socket)
{
    char buffer[BUFFER_SIZE];
    char command[10], filename[100];
    FILE *fp;
    int bytes_read;

    while(1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);

        sscanf(buffer, "%s %s", command, filename);

        if (strcmp(command, "UPLOAD") == 0)
        {
            fp = fopen(filename, "wb");
            if (!fp)
            {
                perror("File open error");
                return;
            }
        
            while ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
            {
                if (strcmp(buffer, "EOF") == 0)break;
                fwrite(buffer, 1, bytes_read, fp);
                if(bytes_read < BUFFER_SIZE)break;
            }

            fclose(fp);
            printf("File %s uploaded.\n", filename);
        }

        else if (strcmp(command, "DOWNLOAD") == 0)
        {
            fp = fopen(filename, "rb");
            if (!fp)
            {
                perror("File open error");
                return;
            }
            
            fseek(fp, 0L, SEEK_END);
            long filesize = ftell(fp);
            fseek(fp, 0L, SEEK_SET);

            // Send file size to client first
            sprintf(buffer, "%ld", filesize);
            send(client_socket, buffer, strlen(buffer), 0);

            // Wait for client to acknowledge
            recv(client_socket, buffer, strlen(buffer), 0); // Wait for OK
            
            while (bytes_read = fread(buffer, 1, BUFFER_SIZE, fp) > 0)
            {
                send(client_socket, buffer, bytes_read, 0);
            }

            strcpy(buffer, "EOF");
            send(client_socket, buffer, strlen(buffer), 0);
            fclose(fp);
            printf("File %s sent to client.\n", filename);
        }

        else if (strcmp(command, "LIST") == 0)
        {
            system("ls > file_list.txt");
            fp = fopen("file_list.txt", "r");
            if (!fp)
            {
                perror("Failed to open file list");
                return;
            }
            while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
            {
                send(client_socket, buffer, bytes_read, 0);
            }
            strcpy(buffer, "EOF");
            send(client_socket, buffer, strlen(buffer), 0);
            fclose(fp);
            remove("file_list.txt");
        }

        else if (strcmp(command, "QUIT") == 0)
        {
            break;
        }

        else
        {
            printf("Unknown command.\n");
        }
    }
}

void* client_thread (void* arg)
{
    int client_socket = *(int*)arg;
    free(arg);
    handle_client(client_socket);
    close(client_socket);
    printf("Client disconnected.\n");
    return NULL;
}