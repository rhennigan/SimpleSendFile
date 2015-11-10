//
// Created by rhennigan on 11/9/2015.
//

#ifndef DEMONSTRATIONSAUTOTESTING_SOCK_SRV_C_H
#define DEMONSTRATIONSAUTOTESTING_SOCK_SRV_C_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE (1024)
#define MSG_SIZE    (256)

void error(char *msg);

size_t socket_write(int socket_fd, char *message, size_t length);

size_t socket_read(int socket_fd, char *buffer, size_t length);

void transfer(int socket_fd);

#endif //DEMONSTRATIONSAUTOTESTING_SOCK_SRV_C_H
