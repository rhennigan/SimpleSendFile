//
// Created by rhennigan on 11/9/2015.
//

#include "sock_util.h"

//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    // check argument count
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(EXIT_FAILURE);
    }

    // create socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) error("ERROR opening socket");

    // initialize server address and port
    struct sockaddr_in serv_addr, cli_addr;
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    uint16_t portno = (uint16_t) atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // bind the socket to address
    struct sockaddr *socket_addr = (struct sockaddr *) &serv_addr;
    int bind_result = bind(socket_fd, socket_addr, sizeof(serv_addr));
    if (bind_result < 0) error("ERROR on binding");

    // start listening for connections on the socket (max 5 at a time)
    listen(socket_fd, 5);

    // spawn a new process for incoming connections
    int newsockfd, pid;
    int clilen = sizeof(cli_addr);
    while (1) {
        // block while waiting for a connecting client
        newsockfd = accept(socket_fd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");

        // fork and let the child process handle the new connection
        pid = fork();
        if (pid < 0) error("ERROR on fork");
        if (pid == 0) { // this is the child process
            /* parent process still listens for incoming connections, so we
             * can close the original socket file descriptor */
            close(socket_fd);

            // receive the file
            transfer(newsockfd);
            printf("Transfer complete!\n\n");

            // the child process exits when done
            exit(EXIT_SUCCESS);

            // TODO: clean up zombie processes

        } else { // this is the parent process
            /* the child process handles the file transfer through this socket
             * so we close it and continue listening for new connections */
            close(newsockfd);
        }
    }
    // we should never get here
}