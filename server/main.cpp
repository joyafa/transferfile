#include "intf/interface.h"
#include "intf/file_socket.h"
#include "service/DispatchMsgService.h"
#include <memory>
#include "event.h"
#include <iostream>
#include <sys/epoll.h>
#include "log.h"

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
	LOG_INIT("log");
	cout << "Server is ready..." << endl;
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
		LOG_ERROR("Bind socket port:%s failed!", argv[1]);
		exit(1);
	}
	LOG_INFO("Bind port:%s OK", argv[1]);
	//set no block
	int ret = set_socket_non_block(server_socket);
	if (ret < 0 )
	{
		LOG_ERROR("Set fd non block error!");
		exit(1);
	}
	//listen 
	bool bret = intf.add_server_socket(server_socket);
	if (!bret)
	{
		LOG_ERROR("add erver socket error!");
		exit(1);
	}
	////EPOLL_CTL_ADD
	//bret = intf.add_epoll_event(server_socket, EPOLLIN | EPOLLOUT | EPOLLET);
	//if (!bret)
	//{
	//	exit(1);
	//}
	//accept & message process
	cout << "Server is running..." << endl;
	intf.run();
	//main thread loop
	for (;;);
	
	return 1;
}