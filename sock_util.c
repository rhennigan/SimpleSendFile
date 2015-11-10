//
// Created by rhennigan on 11/9/2015.
//

#include "sock_util.h"

//////////////////////////////////////////////////////////////////////////////
void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

//////////////////////////////////////////////////////////////////////////////
size_t socket_write(int socket_fd, char *message, size_t length) {
    size_t bytes = (size_t) write(socket_fd, message, length);
    if (bytes < 0) error("ERROR writing to socket");
    return bytes;
}

//////////////////////////////////////////////////////////////////////////////
size_t socket_read(int socket_fd, char *buffer, size_t length) {
    size_t bytes = (size_t) read(socket_fd, buffer, length);
    if (bytes < 0) error("ERROR reading from socket");
    return bytes;
}

//////////////////////////////////////////////////////////////////////////////
void transfer(int socket_fd) {
    char mesg_buf[256];
    char name_buf[256];
    char szst_buf[256];
    char data_buf[1024];

    // clear buffers
    memset(mesg_buf, 0, 256);
    memset(name_buf, 0, 256);
    memset(szst_buf, 0, 256);
    memset(data_buf, 0, 1024);

    // tell client connection is ready
    socket_write(socket_fd, "REC CON Null; RDY FNM;", 22);

    // get file name from client
    socket_read(socket_fd, name_buf, 255);
    printf("Filename: %s\n", name_buf);

    // send response and ask for file size
    sprintf(mesg_buf, "REC FNM %s; RDY LEN;", name_buf);
    socket_write(socket_fd, mesg_buf, strlen(mesg_buf));
    memset(mesg_buf, 0, 256);

    // get file size from client
    socket_read(socket_fd, szst_buf, 255);
    size_t file_size = (size_t) atoi(szst_buf);
    printf("File size: %zu\n", file_size);

    // send response and ask for file data
    sprintf(mesg_buf, "REC LEN %zu; RDY DAT;", file_size);
    socket_write(socket_fd, mesg_buf, strlen(mesg_buf));
    memset(mesg_buf, 0, 256);

    // create file for output
    FILE *output = fopen(name_buf, "w");
    if (output == NULL) error("ERROR opening file");

    // initialize transfer status variables
    size_t current = 0, remaining = file_size;
    size_t read_len;
    size_t n, i = 0, i0 = 0;

    // begin file transfer
    printf("|");
    while (remaining > 0) {
        read_len = remaining < 1024 ? remaining : 1024;
        n = socket_read(socket_fd, data_buf, read_len);
        current += n;
        remaining -= n;

        for (i = i0; i < (78 * current / file_size); i++) {
            printf("=");
            fflush(NULL);
            i0++;
        };

        fwrite(data_buf, sizeof(char), n, output);
        memset(data_buf, 0, 1024);
    }
    printf("|\n");

    // send confirmation of transfer
    sprintf(mesg_buf, "REC DAT %zu; RDY Null;", current);
    socket_write(socket_fd, mesg_buf, strlen(mesg_buf));
    memset(mesg_buf, 0, 256);
}





