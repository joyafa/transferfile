#include "binrary_codec.h"
#include <iostream>
#include <sstream>
#include "log.h"
#include "eventtype.h"
#include "event.h"

iEvent* binrary_protocol_codec_t::decode(uint16_t mid, uint8_t* buffer, uint32_t size)
{
    iEvent* ev= NULL;

    switch (mid )
    {
    case EEVENTID_SEND_GENERAL_REQ:
        ev = decode_file_generalinfo(buffer, size);
        break;
    case EEVENTID_SEND_FILEDATA_REQ:
        //ev = decode_file_writefile_data(buffer, size);
        break;
    default:
		LOG_WARNNING("mid %d is invalid.", mid);
        break;
    }

    return ev;
}

struct fileInfo
{
	char chFileName[128];
	char md5[32];//整个文件计算md5值.收到完后比较实用,确保文件已经正常接收完成
	//
	int  pices;//分片个数
	int  length;//
};
//子结构体,每次传输包的信息
struct sub
{
	unsigned int crc32;//crc32 校验当前包是否接收正确
	size_t datalen;//包数据长度
	char* data;//包数据
	//每次收到一个包都有办法定位到文件中的位置,可以写入,  
	size_t offset;//文件中的偏移位置就可以写入

};

//包偏移地址,收到之后就可以直接往这里写数据, 文件映射,找到对应位置进行映射
iEvent* binrary_protocol_codec_t::decode_file_generalinfo(uint8_t* buffer, uint32_t size)
{
	//filename
}

bool binrary_protocol_codec_t::encode(iEvent* ev, uint8_t* buffer, uint32_t size)
{
    bool ret = false;
    switch(ev->get_eid())
    {
   /* case EEVENTID_GET_MOBILE_CODE_RSP:
        ret = encode_common_rsp_ev(ev, buffer, size);
        break;*/
    }

    return ret;
}


//bool binrary_protocol_codec_t::encode_common_rsp_ev(CommonRspEv* rsp, uint8_t* buffer, uint32_t size)
//{
//    rsponse_result rsp_ret;
//    rsp_ret.set_code(rsp->get_code());
//    rsp_ret.set_msg(rsp->get_msg());
//    if (!rsp->get_data().empty())
//    {
//        rsp_ret.set_data(rsp->get_data());
//    }
//
//    std::ostringstream ss;
//    ss.rdbuf()->pubsetbuf(buffer, size);
//
//    return rsp_ret.SerializeToOstream(&ss);
//}
//

