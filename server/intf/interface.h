#pragma once
#include <functional>
#include <vector>
#include "protocol_head.h"
#include "protocol_codec.h"
#include "file_socket.h"
#include "event.h"

class Interface
{
public:
	//paramer: queue: push message queue
    Interface(std::function< int(file_socket_t)>& equeue);
    bool add_server_socket(int socket);
    bool close();
    void run();
	bool add_epoll_event(file_socket_t socket, int events);

private:
    std::function< int (file_socket_t)>  _equeue;
	int _server_socket;
	file_socket_t  epoll_fd_;
	file_socket_t  channel_fd_[2];

    protocol_codec_t* codecs_[4]; // there is only 4 protocol codec.

private:
    bool handle_channel_event();

    bool accept_client(file_socket_t sfd);
    unsigned int read_client_data(file_socket_t fd, char* buffer, int size);
};
