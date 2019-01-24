#pragma once
#include <stdint.h>
typedef uint32_t DWORD;    // DWORD = unsigned 32 bit value
typedef uint16_t WORD;     // WORD = unsigned 16 bit value
typedef uint8_t  BYTE;     // BYTE = unsigned 8 bit value
typedef uint64_t UINT64;   // UINT64 = unsigned 64 bit value

enum protocol_type_t
{
	JSON_PROTOCOL_TYPE = 0,  // jason
	PB_PROTOCOL_TYPE = 1,  // Google Protocol Buffer
	FB_PROTOCOL_TYPE = 2,  // FlatBuffers
	BINARY_PROTOCOL_TYPE = 3,  // binary format

	UNKNOW_PROTOCOL_TYPE = 0xFF
};
#pragma pack(4)
struct protocol_head_t
{
	WORD   tag_;        // 0xFBFC
	BYTE   version_;    // protocol version, high 4 bit is master version, low 4 bit is sub version
	BYTE   type_;      // protocol type
	WORD   len_;       // content length
	WORD   msg_id_;    // message id / function id
	UINT64 msg_sn;     // message serise number
	DWORD  reserve_;   // reserve bytes
};
#pragma pack()

#define HEAD_TAG_POS        0
#define HEAD_VERSION_POS    2
#define HEAD_TYPE_POS       3
#define HEAD_LEN_POS        4
#define HEAD_MID_POS        6
#define HEAD_MSN_POS        8
#define HEAD_RESERVE_POS    16


class protocol_head_codec_t
{
public:
    virtual bool decode(uint8_t* buffer, uint32_t size, protocol_head_t* head);
    virtual bool encode(protocol_head_t* head, uint8_t* buffer, uint32_t size);

private:
    uint8_t get_byte(uint8_t* buffer, uint32_t size, uint32_t offset);
    void set_byte(uint8_t* buffer, uint32_t size, uint32_t offset, uint8_t val);
    uint16_t get_u16(uint8_t* buffer, uint32_t size, uint32_t offset);
    void set_u16(uint8_t* buffer, uint32_t size, uint32_t offset, uint16_t val);
    uint32_t get_u32(uint8_t* buffer, uint32_t size, uint32_t offset);
    void set_u32(uint8_t* buffer, uint32_t size, uint32_t offset, uint32_t val);
    uint64_t get_u64(uint8_t* buffer, uint32_t size, uint32_t offset);
    void set_u64(uint8_t* buffer, uint32_t size, uint32_t offset, uint64_t val);
};
