=============================================
Secure FTP-style File Transfer System (C / POSIX Sockets)
=============================================

Author: Oscar Huang

-------------------------------
PROJECT OVERVIEW
-------------------------------

This project implements a lightweight FTP-style file transfer system written in C 
using POSIX Sockets. It demonstrates low-level network programming, custom binary 
protocol design, secure data transfer, and data integrity validation.

The system consists of two components:

• FTP Server - Handles authentication, file uploads, and file downloads
• FTP Client - Connects to the server and performs file transfer operations

Key capabilities:

• TCP client/server communication
• Custom binary application protocol
• Authentication system
• File upload and download
• CRC32 integrity verification
• AES-256 encrypted file transfer
• Reliable socket transmission helpers

The project is designed for systems programming, networking, and secure data
transfer.

-------------------------
SYSTEM ARCHITECTURE
-------------------------

Client
|
| TCP Socket
|
Server

Transfer Flow:

1. Client connects to server
2. Client sends AUTH command
3. Server validates credentials
4. Client uploads file
5. Server verifies CRC and stores file
6. Client requests download
7. Server encrypts file and sends chunks
8. Client verifies CRC and decrypts data

---------------------
CUSTOM FTP PROTOCOL
---------------------

Each message begins with a fixed header.

Header structure:

struct ftp_header_t
{
    uint32_t magic;
    uint16_t cmd;
    uint16_t reserved;
    uint32_t payload_size;
}

Magic value:
FTP_MAGIC = 0x46545031 ("FTP1")

Commands supported:

CMD_AUTH        Authenticate client
CMD_UPLOAD      Upload file to server
CMD_DOWNLOAD    Download file from server
CMD_LIST        (Reserved)
CMD_QUIT        Disconnect
CMD_OK          Success response
CMD_ERROR       Failure response

-------------------
DATA INTEGRITY
-------------------

Each data chunk transferred includes a CRC32 checksum.

Transfer format:

[DATA_CHUNK]
[CRC32]

The receiver computes CRC32 and compares it with the transmitted value to
detect corruption.

----------------
ENCRYPTION
----------------

File downloads from the server are encrypted using:

AES-256-CBC (OpenSSL EVP API)

Process:

Server:
plaintext -> AES encrypt -> send encrypted chunk

Client:
receive encrypted chunk -> AES decrypt -> write to file

This ensures confidentiality of transferred files.

-------------------------------
RELIABLE SOCKET TRANSMISSION
-------------------------------

TCP send() and recv() may transmit partial data.

To handle this, two helper functions are implemented:

send_all()
recv_all()

These functions guarantee that full buffer is transmitted or received before returning.

----------------
FILE STRUCTURE
----------------

include/
ftp_common.h
ftp_protocol.h
ftp_crypto.h
ftp_crc.h
ftp_transfer.h
ftp_auth.h

src/
ftp_server.c        Server entry point
ftp_client.c        Client implementation
ftp_net.c           Reliable socket helpers
ftp_protocol.c      Protocol header handling
ftp_transfer.c      File upload/download logic
ftp_crypto.c        AES encryption/decryption
ftp_crc.c           CRC32 implementation
ftp_auth.c          Authentication logic

-------------------
BUILD INSTRUCTIONS
-------------------

Requirements:

Linux
GCC
OpenSSL development libraries

Install dependencies (Ubuntu / WSL):

sudo apt install build-essential libssl-dev

Build project:

make

Clean build files:

make clean

---------------------
RUNNING THE PROGRAM
---------------------

Start server:

make run-server

Output:
FTP server listening on port 2121

Run client (in another terminal):

make run-client

The client will:

1. Authenticate
2. Upload test.bin
3. Download the file
4. Save it as downloaded.bin

------------------------
GENERATING A TEST FILE
------------------------

Use the following command to generate a random file:

dd if=/dev/urandom of=test.bin bs=1K count=10

------------------------
KEY LEARNING OBJECTIVES
------------------------

This project demonstrates practical experience with:

• TCP socket programming
• Binary protocol design
• Network byte order handling
• Secure data transfer
• Data integrity verification
• Modular C project architecture
• Makefile based builds
• OpenSSL EVP encryption APIs

---------------------
FUTURE IMPROVEMENTS
---------------------

Possible enhancements:

• Multi-client support (threads or epoll)
• File listing command
• Resume interrupted transfer
• TLS-secured connections
• Rate limiting and throttling
• Progress reporting
• Stronger authentication