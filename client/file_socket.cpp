#include "file_socket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "log.h"

int init_client(const char* ip, uint16_t port)
{
	int clientsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientsocket < 0) {
		printf("socket failed\n");
		return -1;
	}

	struct sockaddr_in serveraddr = { 0 };
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	serveraddr.sin_addr.s_addr = inet_addr(ip);

	int result = connect(clientsocket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (result != 0) {
		printf("connect failed\n");
		return -2;
	}

	return clientsocket;

}

int create_and_bind_socket(unsigned short port)
{
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0)
    {
        LOG_ERROR("create socket failed");
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0)
    {
	    LOG_ERROR("bind to port = %d failed.", port);
	    return -1;
    }

    // set SO_REUSEADDR
	int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));

    return sfd;
}

int set_socket_non_block(int sfd)
{
	int flags, res;

    flags = fcntl(sfd, F_GETFL);
    if (flags == -1)
    {
        LOG_ERROR("cannot get socket flags!\n");
        return -1;
    }

    flags |= O_NONBLOCK;
    res    = fcntl(sfd, F_SETFL, flags);
    if (res == -1)
    {
        LOG_ERROR("cannot set socket flags!\n");
        return -1;
    }

    return 0;
}

int nio_write(int fd, char* buf, int len)
{
    int write_pos = 0;
    int left_len = len;

    while (left_len > 0)
    {
        int writed_len = 0;
        if ((writed_len = write(fd, buf + write_pos, left_len)) <= 0)
        {
            if (errno == EAGAIN)
            {
               writed_len = 0;
            }
            else return -1;
        }
        left_len -= writed_len;
        write_pos += writed_len;
    }

    return len;
}

int nio_recv(int sockfd, char* buffer, int length, int* ret)
{
	int idx = 0;
	*ret = 0;

	while (1)
    {
		int count = recv(sockfd, buffer+idx, length - idx, 0);
		if (count == 0)
        {
			*ret = -1;
			::close(sockfd);
			break;
		}
        else if (count == -1)
		{
			*ret = 0;
			break;
		}
        else
        {
			idx += count;
			if (idx == length) break;
		}
	}

	return idx;
}



