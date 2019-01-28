#include "protobuf_protocol_codec_t.h"
#include "log.h"
#include "protocol.pb.h"
#include <sstream>
using namespace FileTransport;



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
	iEvent* ev = NULL;

	switch (mid)
	{
	case EEVENTID_SEND_GENERAL_REQ:
		ev = decode_file_generalinfo(buffer, size);
		break;
	case EEVENTID_SENDD_FILEDATA_REQ:
		ev = decode_file_writefile_data(buffer, size);
		break;
	default:
		LOG_WARNNING("mid %d is invalid.", mid);
		break;
	}

	return ev;
}


iEvent * protobuf_protocol_codec_t::decode_file_generalinfo(uint8_t* buffer, uint32_t size)
{
	//body
	generalinfo_request send_fileinfo;
	std::istringstream ss;
	ss.rdbuf()->pubsetbuf((char* )buffer, size);
	send_fileinfo.ParseFromIstream(&ss);
	if (!send_fileinfo.has_filename())
	{
		LOG_WARNNING("there is no filename filed in message generalinfo_request.");
		return NULL;
	}
	const std::string strFileName = send_fileinfo.filename();
	if (!send_fileinfo.has_md5())
	{
		LOG_WARNNING("there is no md5 filed in message generalinfo_request.");
		return NULL;
	}
	const std::string strMd5 = send_fileinfo.md5();

	if (!send_fileinfo.has_filelength())
	{
		LOG_WARNNING("there is no filelength filed in message generalinfo_request.");
		return NULL;
	}
	uint64_t filelength = send_fileinfo.filelength();

	if (!send_fileinfo.has_pieces())
	{
		LOG_WARNNING("there is no pieces filed in message generalinfo_request.");
		return NULL;
	}
	uint32_t pieces = send_fileinfo.pieces();

	LOG_INFO("FileInfo:%s,md5:%s, len:%u, pieces:%d", strFileName.c_str(), strMd5.c_str(), filelength, pieces);
	std::cout << "FileInfo:" << strFileName << ",md5:" << strMd5 << ", fil length:" << filelength << ", pieces:" << pieces << std::endl;
	return nullptr; // new;
}


iEvent* protobuf_protocol_codec_t::decode_file_writefile_data(uint8_t* buffer, uint32_t size)
{
	LOG_INFO("decode_file_writefile_data");

	//body
	send_filecontent_request fileContent;
	std::istringstream ss;
	ss.rdbuf()->pubsetbuf((char*)buffer, size);
	fileContent.ParseFromIstream(&ss);
	if (!fileContent.has_idx())
	{
		LOG_WARNNING("there is no idx filed in message send_filecontent_request.");
		return NULL;
	}
	uint32_t idx = fileContent.idx();
	if (!fileContent.has_pieces())
	{
		LOG_WARNNING("there is no pieces filed in message send_filecontent_request.");
		return NULL;
	}
	uint32_t pieces = fileContent.pieces();

	if (!fileContent.has_length())
	{
		LOG_WARNNING("there is no length filed in message send_filecontent_request.");
		return NULL;
	}
	uint32_t length = fileContent.length();

	if (!fileContent.has_data())
	{
		LOG_WARNNING("there is no data filed in message send_filecontent_request.");
		return NULL;
	}
	//TODO:二进制数据，这里处理有问题吧
	const std::string data = fileContent.data();

	if (!fileContent.has_crc32())
	{
		LOG_WARNNING("there is no crc32 filed in message send_filecontent_request.");
		return NULL;
	}
	uint32_t crc32 = fileContent.crc32();
	//TODO:输出二进制这里有问题的
	LOG_INFO("FileContent:idx:%d,pieces:%d, len:%d, crc32:%u", idx, pieces, length, crc32);

	return nullptr; // new;
}
