#include "intf/interface.h"
#include "intf/file_socket.h"
#include "service/DispatchMsgService.h"
#include <memory>
#include "event.h"
#include <iostream>
#include <sys/epoll.h>

using namespace std;
#define PORT 1688

void usage()
{
	cout << "Usage:server 9099" << endl;
}


int main(int argc, char** argv)
{
	//server 
	if (argc < 2)
	{
		usage();
		exit(1);
	}
	//conf
    std::shared_ptr<DispatchMsgService> dms(new DispatchMsgService);
	dms->open(); //启动线程

	//TODO:不能传指针, 会有问题的
	std::function<int (file_socket_t)> func = std::bind(&DispatchMsgService::enqueue, dms.get(), std::placeholders::_1);
	// create server socket and set to non block
	Interface intf(func);
	int server_socket = create_and_bind_socket(atoi(argv[1]));
	if (server_socket < 0)
	{
		exit(1);
	}
	//set no block
	int ret = set_socket_non_block(server_socket);
	if (ret < 0 )
	{
		exit(1);
	}
	//listen 
	bool bret = intf.add_server_socket(server_socket);
	if (!bret)
	{
		exit(1);
	}
	////EPOLL_CTL_ADD
	//bret = intf.add_epoll_event(server_socket, EPOLLIN | EPOLLOUT | EPOLLET);
	//if (!bret)
	//{
	//	exit(1);
	//}
	//accept & message process
	intf.run();

	//main thread loop
	for (;;);
	
	return 1;
}