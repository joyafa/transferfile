
/*
 * �ַ���Ϣ����ģ�飬��ʵ���ǰ��ⲿ�յ�����Ϣ��ת�����ڲ��¼���Ҳ���Ǵ���msg->event�Ľ������
 * ������ٵ���ÿ��event handler������event���ڴ�ÿ��event handler��Ҫsubscribe��event��Ż�
 * �����õ�.
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

