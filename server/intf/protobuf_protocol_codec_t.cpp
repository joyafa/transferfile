#include "protobuf_protocol_codec_t.h"



protobuf_protocol_codec_t::protobuf_protocol_codec_t()
{
}


protobuf_protocol_codec_t::~protobuf_protocol_codec_t()
{
}

bool protobuf_protocol_codec_t::encode(iEvent* ev, uint8_t* buffer, uint32_t size)
{
	//throw std::logic_error("The method or operation is not implemented.");
}

iEvent* protobuf_protocol_codec_t::decode(uint16_t mid, uint8_t* buffer, uint32_t size)
{
	//throw std::logic_error("The method or operation is not implemented.");
}
