#pragma once
#include "protocol_codec.h"
#include "fmmap.h"

class protobuf_protocol_codec_t :
	public protocol_codec_t
{
public:
	protobuf_protocol_codec_t();
	virtual ~protobuf_protocol_codec_t();

	virtual bool encode(iEvent* ev, uint8_t** buffer, uint32_t& size) override;
	virtual iEvent* decode(uint16_t mid, uint8_t* buffer, uint32_t size) override;
private:
	iEvent* decode_file_generalinfo(uint8_t* buffer, uint32_t size);
	iEvent* decode_file_writefile_data(uint8_t* buffer, uint32_t size);
	iEvent* CheckFileMd5();
	CFileMap* m_pFile;
	tagFileInfo_t m_fileInfo;
};

