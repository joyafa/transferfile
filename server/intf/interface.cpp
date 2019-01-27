// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.


#include "log.h"
#include "event.h"
#include "events_def.h"
#include "interface.h"
#include <functional>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE   1024
#define MAX_EVENTS 64

Interface::Interface(std::function< int(file_socket_t)>& equeue)
	: _equeue(equeue)
{
    epoll_fd_ = epoll_create(1);
    if (epoll_fd_ == -1)
    {
        LOG_ERROR("cannot create epoll_fd_!\n");
    }
};

bool Interface::add_epoll_event(file_socket_t socket, int events)
{
    struct epoll_event event;

    event.events  = events;//EPOLLIN | EPOLLOUT | EPOLLET;
    event.data.fd = socket;
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket, &event);
    if (ret == -1)
    {
        LOG_ERROR("can not add event to epoll_fd_!\n");
        return false;
    }

    return true;
}

bool Interface::add_server_socket(int socket)
{
	struct epoll_event event;
	memset(&event, 0, sizeof(event));

	_server_socket = socket;

	int ret = listen(_server_socket, SOMAXCONN);
	if (ret == -1)
	{
		LOG_ERROR("cannot listen at the given server_socket!\n");
		return false;
	}
	else
	{
		LOG_INFO("process %d listend success.", getpid());
	}

	epoll_fd_ = epoll_create(1);
	if (epoll_fd_ == -1)
	{
		LOG_ERROR("cannot create epoll_fd_!\n");
		return false;
	}

	if (!add_epoll_event(_server_socket, EPOLLIN | EPOLLOUT | EPOLLET))
	{
		return false;
	}

	return true;
}

bool Interface::accept_client(int sfd)
{
    struct sockaddr client_addr;
    socklen_t addrlen = sizeof(struct sockaddr);

    int client_fd = accept(sfd, &client_addr, &addrlen);
    if (client_fd == -1)
    {
       if (errno == EAGAIN || errno == EWOULDBLOCK)
       {
           return true;
       }
       else
       {
           LOG_ERROR("cannot accept new server_socket!\n");
           return false;
       }
    }

    int ret = set_socket_non_block(client_fd);
    if (ret == -1)
    {
        LOG_ERROR("cannot set flags!\n");
    }

    if (!add_epoll_event(client_fd, EPOLLET | EPOLLIN))
    {
        return false;
    }

    return true;
}


void Interface::run()
{
    struct epoll_event events[MAX_EVENTS] = {0};

    while (true)
    {
        //block
        int cnt = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);

        for (int i = 0; i < cnt; i++)
        {
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)
                || !(events[i].events & EPOLLIN))
            {
                LOG_ERROR("server_socket fd error!\n");
                ::close(events[i].data.fd);
                continue;

            }
            else if (events[i].data.fd == _server_socket)
            {
                accept_client(events[i].data.fd);
            }
            else
            {
				//TODO: 这里可以fork 一个子进程用来接收和处理客户端请求,并应答回去
				//if(0 == fork() ) //子进程
				//recv
				//......
				//send
				//将socket也丢给callback处理,加入queue
				_equeue(events[i].data.fd);
            }
        }
    }

    return;
}

bool Interface::close()
{
	::close(_server_socket);
    return true;
}
