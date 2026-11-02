// ftp_crypto.h
#ifndef FTP_CRYPTO_H
#define FTP_CRYPTO_H

#include <stdint.h>

int aes_encrypt(const uint8_t *plaintext, int plaintext_len, uint8_t *ciphertext);

int aes_decrypt(const uint8_t *ciphertext, int ciphertext_len, uint8_t *plaintext);

#endif