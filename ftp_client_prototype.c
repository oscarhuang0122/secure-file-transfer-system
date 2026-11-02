// ftp_client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 2121
#define BUFFER_SIZE 1024

int main()
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char command[10], filename[100];
    FILE *fp;
    int bytes_read;

    char user[50], pass[50];
    printf("Username: ");
    scanf("%s", user);
    printf("Password: ");
    scanf("%s", pass);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("Socket error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connect failed");
        exit(1);
    }

    send(sock, user, strlen(user), 0);
    send(sock, pass, strlen(pass), 0);

    recv(sock, buffer, BUFFER_SIZE, 0);
    if (strcmp(buffer, "AUTH_FAIL") == 0)
    {
        printf("Authentication failed.\n");
        close(sock);
        exit(1);
    }

    printf("Connect to FTP server.\n");

    while(1)
    {
        printf("ftp> ");
        scanf("%s", command);

        if (strcmp(command, "UPLOAD") == 0)
        {
            scanf("%s", filename);
            snprintf(buffer, BUFFER_SIZE, "UPLOAD %s", filename);
            send(sock, buffer, strlen(buffer), 0);

            fp = fopen(filename, "rb");
            if(!fp)
            {
                perror("File open error");
                continue;
            }

            fseek(fp, 0L, SEEK_END);
            long filesize = ftell(fp);
            fseek(fp, 0L, SEEK_SET);

            long total_sent = 0;

            while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
            {
                send(sock, buffer, bytes_read, 0);
                total_sent += bytes_read;

                int percent = (int)((total_sent * 100 / filesize));
                int barwidth = 50;
                int pos  = (percent * barwidth) / 100;

                printf("\rUploading %s: [", filename);
                for (int i = 0; i < barwidth; ++i)
                {
                    if (i < pos) printf("=");
                    else if (i == pos) printf(">");
                    else printf(" ");
                }
                printf("] %d%%", percent);
                fflush(stdout);
            }
            printf("\n");

            strcpy(buffer, "EOF");
            send(sock, buffer, strlen(buffer), 0);
            fclose(fp);
        }

        else if (strcmp(command, "DOWNLOAD") == 0)
        {
            scanf("%s", filename);
            if (access(filename, F_OK) == 0)
            {
                char choice;
                printf("File %s is exists. Overwrite? (y/n): ", filename);
                scanf("%c", &choice);
                if (choice != 'y' && choice != 'Y')continue;
            }

            snprintf(buffer, BUFFER_SIZE, "DOWNLOAD %s", filename);
            send(sock, buffer, strlen(buffer), 0);

            // Gey file size
            recv(sock, buffer, BUFFER_SIZE, 0);
            long filesize = atol(buffer);
            send(sock, "OK", 2, 0); // Confirm ready

            fp = fopen(filename, "wb");
            if(!fp)
            {
                perror("File open failed");
                continue;
            }

            long total_received = 0;

            while ((bytes_read = recv(sock, buffer, BUFFER_SIZE, 0)) > 0)
            {
                if (strcmp(buffer, "EOF") == 0) break;
                fwrite(buffer, 1, bytes_read, fp);
                total_received += bytes_read;

                int percent = (int)((total_received * 100) / filesize);
                int barwidth = 50;
                int pos = (percent * barwidth) / 100;

                printf("\rDownloading %s: [", filename);
                for (int i = 0; i < barwidth; ++i)
                {
                    if (i < pos) printf("=");
                    else if (i == pos) printf(">");
                    else printf(" ");
                }
                printf("] %ld bytes", total_received);
                fflush(stdout);

                if (bytes_read < BUFFER_SIZE)break;
            }

            printf("\n");
            fclose(fp);
            printf("Downloaded %s\n", filename);
        }

        else if (strcmp(command, "LIST") == 0)
        {
            send(sock, "LIST", 4, 0);
            while ((bytes_read = recv(sock, buffer, BUFFER_SIZE, 0)) > 0)
            {
                buffer[bytes_read] = '\0';
                if (strcmp(buffer, "EOF") == 0)break;
                printf("%s", buffer);
            }
            printf("\n");
        }

        else if (strcmp(command, "QUIT") == 0)
        {
            send(sock, "QUIT", 4, 0);
            break;
        }
        
        else
        {
            printf("Unknown command.\n");
        }
    }

    close(sock);
    return 0;
}