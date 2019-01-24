#pragma once
#include "protocol_head.h"
#include "protocol_codec.h"
#include "event.h"
class binrary_protocol_codec_t : public protocol_codec_t
{
public:
	virtual bool encode(iEvent* ev, uint8_t* buffer, uint32_t size);
	virtual iEvent* decode(uint16_t mid, uint8_t* buffer, uint32_t size);
private:
	iEvent* decode_file_generalinfo(uint8_t* buffer, uint32_t size);
	iEvent* decode_file_writefile_data(uint8_t* buffer, uint32_t size);

   //bool encode_common_rsp_ev(CommonRspEv* rsp, uint8_t* buffer, uint32_t size);
};
