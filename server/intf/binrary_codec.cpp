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
	char md5[32];//�����ļ�����md5ֵ.�յ����Ƚ�ʵ��,ȷ���ļ��Ѿ������������
	//
	int  pices;//��Ƭ����
	int  length;//
};
//�ӽṹ��,ÿ�δ��������Ϣ
struct sub
{
	unsigned int crc32;//crc32 У�鵱ǰ���Ƿ������ȷ
	size_t datalen;//�����ݳ���
	char* data;//������
	//ÿ���յ�һ�������а취��λ���ļ��е�λ��,����д��,  
	size_t offset;//�ļ��е�ƫ��λ�þͿ���д��

};

//��ƫ�Ƶ�ַ,�յ�֮��Ϳ���ֱ��������д����, �ļ�ӳ��,�ҵ���Ӧλ�ý���ӳ��
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

