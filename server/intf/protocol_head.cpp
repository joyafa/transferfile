#include "protocol_head.h"
#include "protocol_codec.h"
#include <string.h>
#include <arpa/inet.h>
#include <assert.h>


bool protocol_head_codec_t::decode(uint8_t* buffer, uint32_t size, protocol_head_t* head)
{
    assert(buffer != NULL);
    assert(sizeof(head) <= size);

    head->tag_     = get_u16(buffer, size, HEAD_TAG_POS);
    head->version_ = get_byte(buffer, size, HEAD_VERSION_POS);
    head->type_    = get_byte(buffer, size, HEAD_TYPE_POS);
    head->len_     = get_u16(buffer, size, HEAD_LEN_POS);
    head->msg_id_  = get_u16(buffer, size, HEAD_MID_POS);
    head->msg_sn   = get_u64(buffer, size, HEAD_MSN_POS);
    head->reserve_ = get_u32(buffer, size, HEAD_RESERVE_POS);

    return true;
}

bool protocol_head_codec_t::encode(protocol_head_t* head, uint8_t* buffer, uint32_t size)
{
    assert(buffer != NULL);
    assert(sizeof(head) <= size);

    set_u16(buffer, size, HEAD_TAG_POS, head->tag_);
    set_byte(buffer, size, HEAD_VERSION_POS, head->version_);
    set_byte(buffer, size, HEAD_TYPE_POS, head->type_);
    set_u16(buffer, size, HEAD_LEN_POS, head->len_);
    set_u16(buffer, size, HEAD_MID_POS, head->msg_id_);
    set_u32(buffer, size, HEAD_RESERVE_POS, head->reserve_);
}

uint8_t protocol_head_codec_t::get_byte(uint8_t* buffer, uint32_t size, uint32_t offset)
{
    assert(buffer != NULL);
    assert(offset <= size);
	return *(buffer + offset);
}

void protocol_head_codec_t::set_byte(uint8_t* buffer, uint32_t size, uint32_t offset, uint8_t val)
{
    assert(buffer != NULL );
    assert(offset + sizeof(uint8_t) <= size);
    buffer[offset] = val;
}

uint16_t protocol_head_codec_t::get_u16(uint8_t* buffer, uint32_t size, uint32_t offset)
{
    assert(buffer != NULL);
    assert(offset + sizeof(uint16_t) <= size);
	uint16_t val = 0;
    memcpy(&val, buffer + offset, 2);
    return ntohs(val);
}

void protocol_head_codec_t::set_u16(uint8_t* buffer, uint32_t size, uint32_t offset, uint16_t val)
{
    assert(buffer != NULL);
    assert(offset + sizeof(uint16_t) <= size);
    uint16_t temp = htons(val);
    memcpy(buffer + offset, &temp, 2);
}

uint32_t protocol_head_codec_t::get_u32(uint8_t* buffer, uint32_t size, uint32_t offset)
{
    assert(buffer != NULL);
    assert(offset + sizeof(uint32_t) <= size);
    uint32_t val = 0;
    memcpy(&val, buffer + offset, 4);
    return ntohl(val);
}

void protocol_head_codec_t::set_u32(uint8_t* buffer, uint32_t size, uint32_t offset, uint32_t val)
{
    assert(buffer != NULL);
    assert(offset + sizeof(uint32_t) <= size);
    uint32_t temp = htonl(val);
    memcpy(buffer + offset, &temp, 4);
}

uint64_t protocol_head_codec_t::get_u64(uint8_t* buffer, uint32_t size, uint32_t offset)
{
    assert(buffer != NULL);
    assert(offset + sizeof(uint64_t) <= size);
	uint64_t val = this->get_u32(buffer, size, offset);
    val = val << 32;
    val |= this->get_u32(buffer, size, offset + 4);
    return val;
}

void protocol_head_codec_t::set_u64(uint8_t* buffer, uint32_t size, uint32_t offset, uint64_t val)
{
    assert(buffer != NULL);
    assert(offset + sizeof(uint64_t) <= size);
    uint32_t temp = htonl(static_cast<uint32_t>(val));
    memcpy(buffer + offset + 4, &temp, 4);

    val = val >> 32;
    temp = htonl(static_cast<uint32_t>(val));
    memcpy(buffer + offset, &temp, 4);
}


