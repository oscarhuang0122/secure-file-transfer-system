// ftp_transfer.h
#ifndef FTP_TRANSFER_H
#define FTP_TRANSFER_H

#include <stdint.h>

int handle_upload(int sock, uint32_t filesize, const char *filename);
int handle_download(int sock, const char *filename);

#endif