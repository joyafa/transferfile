#pragma once
#include <stdint.h>

typedef int file_socket_t;

int create_and_bind_socket(uint16_t port);

int set_socket_non_block(int sfd);

int nio_recv(int sockfd, char* buffer, int length, int* ret);

int nio_write(int fd, char* buf, int len);