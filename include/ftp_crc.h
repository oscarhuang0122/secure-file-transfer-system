// ftp_crc.h
#ifndef FTP_CRC_H
#define FTP_CRC_H

#include <stdint.h>
#include <stddef.h>

uint32_t crc32_compute(const uint8_t *data, size_t len);
 
#endif