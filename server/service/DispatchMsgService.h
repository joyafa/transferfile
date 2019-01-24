
/*
 * 分发消息服务模块，其实就是把外部收到的消息，转化成内部事件，也就是存在msg->event的解码过程
 * 解码后，再调用每个event handler来处理event，于此每个event handler需要subscribe该event后才会
 * 被调用到.
 */

#ifndef BRK_SERVICE_DISPATCH_EVENT_SERVICE_H_
#define BRK_SERVICE_DISPATCH_EVENT_SERVICE_H_

#include <map>
#include <vector>
#include "event.h"
#include "eventtype.h"
#include "MsgQueue.h"
#include "file_socket.h"
#include "protocol_codec.h"
class DispatchMsgService
{
public:
    DispatchMsgService();
    virtual ~DispatchMsgService();

    virtual bool open();
    virtual void close();
    virtual int32_t enqueue(file_socket_t fd);
	virtual int process(const file_socket_t fd);
    static void* svc(void* argv);
    
protected:
    pthread_t tid_;
    PosixQueue<file_socket_t> msg_queue_;
    bool svr_exit_;
	protocol_codec_t* codecs_[4];
};


#endif

