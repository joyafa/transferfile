#include "DispatchMsgService.h"
#include "log.h"
#include "binrary_codec.h"
#include <algorithm>
#include <thread>
#include "protobuf_protocol_codec_t.h"
#include <string.h>

#define MAX_ITEM_IN_MSG_QUEUE 65536
#define BUF_SIZE 4096

DispatchMsgService::DispatchMsgService()
	: msg_queue_(PosixQueue<file_socket_t>(MAX_ITEM_IN_MSG_QUEUE))
	, svr_exit_(false)
{
	codecs_[JSON_PROTOCOL_TYPE]   = nullptr;
	codecs_[PB_PROTOCOL_TYPE]     = new protobuf_protocol_codec_t;
	codecs_[FB_PROTOCOL_TYPE]     = nullptr;
	codecs_[BINARY_PROTOCOL_TYPE] = nullptr;// new binrary_protocol_codec_t;
}

DispatchMsgService::~DispatchMsgService()
{
	for (int i = 0; i < sizeof(codecs_) / sizeof(codecs_[0]); ++i)
	{
		if (nullptr != codecs_[i])
		{
			delete codecs_[i];
			codecs_[i] = nullptr;
		}
	}
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

    return msg_queue_.enqueue(fd, 0);
}


int DispatchMsgService::process(const file_socket_t fd)
{
	//TODO: 第一个版本先在这里处理,下一步进一步优化,移到线程池处理
	//read request header
	char buf[BUF_SIZE] = { 0 };
	int result = 1;
	int cnt = nio_recv(fd, buf, sizeof(protocol_head_t), &result);
	if (result <= 0)
	{
		LOG_INFO("Client : %d exit!!!", fd);
		return 0;
	}
	LOG_INFO("recv datalen:%d", cnt);
	//decode head
	protocol_head_t proto_head;
	protocol_head_codec_t head_codec;
	head_codec.decode((uint8_t*)buf, sizeof(protocol_head_t), &proto_head);
	LOG_INFO("msgid:%d , data len:%d, pro:%d ", proto_head.msg_id_, proto_head.len_, proto_head.type_);
	//TODO: check head is good
	if (0xFBFC != proto_head.tag_)
	{
		LOG_ERROR("It is not a valid package head!!!");
		return 0;
	}
	char* pBuf = nullptr;
	if (proto_head.len_ > 0)
	{
		pBuf = new char[proto_head.len_];
		bzero(pBuf, proto_head.len_);
		result = 1;
		cnt = nio_recv(fd, pBuf, proto_head.len_, &result);
		if (result <= 0)
		{
			LOG_ERROR("nio_recv: %d", result);
			delete[] pBuf;
			return 0;
		}
		//TODO:郁闷死, 有时能发过来有时不行啊, 这个 子进程建立desocket 总是是不是收不到accept不到
		LOG_INFO("recv body: len: %d <==>%d", cnt, proto_head.len_);
	}
	if ((proto_head.type_ >= JSON_PROTOCOL_TYPE)
		&& (proto_head.type_ <= BINARY_PROTOCOL_TYPE)
		&& (codecs_[proto_head.type_] != NULL))
	{
		iEvent* rsp = codecs_[proto_head.type_]->decode(proto_head.msg_id_, (uint8_t* )pBuf, proto_head.len_);
		LOG_INFO("decode:%p", rsp);
		if (rsp)
		{
			char* rspBuff = nullptr;
			uint32_t len = 0;
			bool ret = codecs_[proto_head.type_]->encode(rsp, (uint8_t** )&rspBuff, len);
			LOG_INFO("encode:%d,%p,%d", ret, rspBuff,len);
			if (ret)
			{
				LOG_INFO("response len:%d, bodylen:%d,id:%d",len, ((protocol_head_t *)rspBuff)->len_, ((protocol_head_t *)rspBuff)->msg_id_);
				nio_write(fd, rspBuff, len);
				delete[] rspBuff;
			}
			delete rsp;
		}
	}
	else
	{
		LOG_ERROR("message (sn=%ld) type %d is error.", proto_head.msg_sn, proto_head.type_);
	}
	delete[] pBuf;
}


void* DispatchMsgService::svc(void* argv)
{
    DispatchMsgService* dms = (DispatchMsgService*)argv;
    while(!dms->svr_exit_)
    {
        file_socket_t fd = -1;
        /* wait only 1 ms to dequeue */
        if (-1 == dms->msg_queue_.dequeue(fd, 1))
        {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
		LOG_INFO("Get a client fd:%d data to work!", fd);
        dms->process(fd);
    }

    return NULL;
}


