#include "DispatchMsgService.h"
#include "log.h"
#include "binrary_codec.h"
#include <algorithm>
#include <thread>
#include "protobuf_protocol_codec_t.h"

#define MAX_ITEM_IN_MSG_QUEUE 65536
#define BUF_SIZE 4096

DispatchMsgService::DispatchMsgService()
	: msg_queue_(PosixQueue<file_socket_t>(MAX_ITEM_IN_MSG_QUEUE))
	, svr_exit_(false)
{
	codecs_[JSON_PROTOCOL_TYPE] = NULL;
	codecs_[PB_PROTOCOL_TYPE] = new protobuf_protocol_codec_t;
	codecs_[FB_PROTOCOL_TYPE] = NULL;
	codecs_[BINARY_PROTOCOL_TYPE] = new binrary_protocol_codec_t;
}

DispatchMsgService::~DispatchMsgService()
{
}

bool DispatchMsgService::open()
{
    svr_exit_ = false;

    /* 异步处理则开启一个线程 */
    if (0 != pthread_create(&tid_, NULL, svc, this))
    {
		LOG_ERROR("open failed since create thread failed.");

        return false;
    }

    return true;
}

void DispatchMsgService::close()
{
    //delete msg_queue_;
    svr_exit_ = true;
}


int32_t DispatchMsgService::enqueue(file_socket_t fd)
{
    if (fd < 0)
    {
		perror("Invalid socket fd!");
        return -1;
    }

    return msg_queue_.enqueue(&fd, 0);
}


int DispatchMsgService::process(const file_socket_t fd)
{
	//TODO: 第一个版本先在这里处理,下一步进一步优化,移到线程池处理
	//read request header
	char buf[BUF_SIZE] = { 0 };
	int result = 1;
	int cnt = nio_recv(fd, buf, sizeof(protocol_head_t), &result);
	if (result <= 0) return 0;

	//decode head
	protocol_head_t proto_head;
	protocol_head_codec_t head_codec;
	head_codec.decode((uint8_t*)buf, BUF_SIZE, &proto_head);

	result = 1;
	cnt = nio_recv(fd, buf + sizeof(protocol_head_t), proto_head.len_, &result);
	if (result <= 0) return 0;

	if ((proto_head.type_ >= JSON_PROTOCOL_TYPE)
		&& (proto_head.type_ <= BINARY_PROTOCOL_TYPE)
		&& (codecs_[proto_head.type_] != NULL))
	{
		iEvent* ev = codecs_[proto_head.type_]->decode(proto_head.msg_id_, (uint8_t* )buf + sizeof(protocol_head_t), proto_head.len_);

		// TODO : encode rsp event to
		// TODO : send response,
		// nio_write

		//mmap file map
		//if (rsp)
		//{
		//	//TODO: buffer size  maybe too small
		//	bool ret = codecs_[proto_head.type_]->encode(rsp, (u8*)buf, BUF_SIZE);
		//	if (ret)
		//	{
		//		//rsp body
		//		//rsp header
		//		proto_head.msg_id_ = rsp->get_eid();
		//		//TODO:len buf 
		//		nio_write(events[i].data.fd, buf, BUF_SIZE);
		//	}

		//}

		//应答
	}
	else
	{
		LOG_ERROR("message (sn=%ld) type %d is error.", proto_head.msg_sn, proto_head.type_);
	}


}

void* DispatchMsgService::svc(void* argv)
{
    DispatchMsgService* dms = (DispatchMsgService*)argv;
    while(!dms->svr_exit_)
    {
        file_socket_t* fd = NULL;
        /* wait only 1 ms to dequeue */
        if (-1 == dms->msg_queue_.dequeue(fd, 1))
        {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        dms->process(*fd);
    }

    return NULL;
}


