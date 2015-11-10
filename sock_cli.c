//
// Created by rhennigan on 11/9/2015.
//

#include "sock_util.h"
#include <netdb.h>

//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    // check argument count
    if (argc < 4) {
        fprintf(stderr, "usage %s hostname port filename\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // read in command line arguments
    char *server_name = argv[1];
    int port_num = atoi(argv[2]);
    char *file_name = argv[3];

    // open a socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) error("ERROR opening socket");

    // get socket host
    struct hostent *server = gethostbyname(server_name);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }

    // initialize socket address config
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));

    // configure socket address and port
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr,
           server->h_addr,
           (size_t) server->h_length);

    serv_addr.sin_port = htons((uint16_t) port_num);

    // test configuration
    struct sockaddr *socket_addr = (struct sockaddr *) &serv_addr;
    int connect_result = connect(socket_fd, socket_addr, sizeof(serv_addr));
    if (connect_result < 0) error("ERROR connecting");

    // open image file for reading and get file size in bytes
    FILE *input = fopen(file_name, "r");
    if (input == NULL) {
        char err_msg[256];
        sprintf(err_msg, "ERROR loading %s", file_name);
        error(err_msg);
    }
    fseek(input, 0L, SEEK_END);
    long file_size = ftell(input);
    fseek(input, 0L, SEEK_SET);

    // initialize buffer to negotiate details of the file transfer
    char msg_buffer[MSG_SIZE];
    memset(msg_buffer, 0, MSG_SIZE);

    // get first message from server
    socket_read(socket_fd, msg_buffer, MSG_SIZE - 1);
    printf("%s\n", msg_buffer);
    memset(msg_buffer, 0, MSG_SIZE);
    // TODO: validate connection response
    // (this should be "REC CON Null; RDY FNM;")

    // send file name to server
    socket_write(socket_fd, file_name, strlen(file_name));

    // get response
    socket_read(socket_fd, msg_buffer, MSG_SIZE - 1);
    printf("%s\n", msg_buffer);
    memset(msg_buffer, 0, MSG_SIZE);
    // TODO: validate file_name response
    // (this should be "REC FNM <file_name>; RDY LEN;")

    // send file size to server
    sprintf(msg_buffer, "%lu", file_size);
    socket_write(socket_fd, msg_buffer, strlen(msg_buffer));
    memset(msg_buffer, 0, MSG_SIZE);

    // get response
    socket_read(socket_fd, msg_buffer, MSG_SIZE - 1);
    printf("%s\n", msg_buffer);
    memset(msg_buffer, 0, MSG_SIZE);
    // TODO: validate file_size response
    // (this should be "REC LEN <file_size>; RDY DAT;")

    // initialize buffer for writing image data to socket
    char buffer[BUFFER_SIZE];
    int bytes_transferred = 0;
    long bytes_remaining = file_size;
    long this_write_size = 0;
    long this_read_size = 0;
    size_t last_write_size = 0;

    // transfer the data to the server
    while (bytes_remaining > 0) {

        // the number of bytes we expect to write
        this_write_size = bytes_remaining < BUFFER_SIZE ?
                          bytes_remaining :
                          BUFFER_SIZE;

        // read from file and record the number of bytes we read
        this_read_size = fread(buffer,
                               sizeof(char),
                               (size_t) this_write_size,
                               input);

        // if these don't match, then something went wrong
        if (this_write_size != this_read_size)
            error("unexpected end of file");

        // write bytes to the socket and record how many were sent
        last_write_size = socket_write(socket_fd,
                                       buffer,
                                       (size_t) this_read_size);

        // check again to make sure the sizes match
        if (last_write_size != this_write_size) {
            printf("unable to write all data to socket\n");
            printf("%lu bytes written, %lu bytes expected\n",
                   last_write_size, this_write_size);
        }

        // update progress
        bytes_transferred += last_write_size;
        bytes_remaining -= last_write_size;
    }

    // get confirmation from server that transfer was successful
    socket_read(socket_fd, msg_buffer, MSG_SIZE - 1);
    printf("%s\n", msg_buffer);
    memset(msg_buffer, 0, MSG_SIZE);
    // TODO: validate transfer confirmation
    // (this should be "REC DAT <file_size>; RDY Null;")

    exit(EXIT_SUCCESS);
}
