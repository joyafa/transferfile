#include "protobuf_protocol_codec_t.h"
#include "log.h"
#include "protocol.pb.h"
#include <string>
#include <assert.h>
#include <string.h>
#include <sstream>
#include "md5.h"
#include "crc32.h"
#include "protocol_head.h"
#include <fcntl.h>
#include "events_def.h"
#include <sys/types.h>
#include <unistd.h>

using namespace FileTransport;

protobuf_protocol_codec_t::protobuf_protocol_codec_t()
	:m_pFile(nullptr)
{
}


protobuf_protocol_codec_t::~protobuf_protocol_codec_t()
{
	if (m_pFile)
	{
		delete m_pFile;
	}
}


protocol_head_t& encode_head(protocol_head_t& head, WORD msgid, uint32_t len)
{
	//initial header
	head.tag_ = 0xFBFC;
	head.type_ = PB_PROTOCOL_TYPE;
	head.version_ = MAKEVERSION(1, 1);//1.1=>0x11
	head.msg_id_ = msgid;// function
	head.len_ = len;
	//head.msg_sn = 1;
	//head.reserve_ = 1;
	LOG_INFO("encode_head: tag:%02X, type:%d, version:%02X, msgid:%d, len:%d,sn:%d, resv:%d ", head.tag_, head.type_, head.version_, head.msg_id_, head.len_, head.msg_sn, head.reserve_);
}



bool protobuf_protocol_codec_t::encode(iEvent* ev, uint8_t** buffer, uint32_t& size)
{
	if (nullptr == ev)
	{
		return false;
	}
	uint32_t id = ev->get_eid();
	switch (id)
	{
	case EEVENTID_SEND_GENERAL_RSP:
	{
		FileInfoRspEv* p = dynamic_cast<FileInfoRspEv*>(ev);
		if (nullptr == p)
		{
			assert(0);
		}
		LOG_INFO("encode:id:%d code:%d, msg:%s", id, p->get_code(), p->get_msg().c_str());
		rsponse_result rsp_content;
		rsp_content.set_code(p->get_code());
		rsp_content.set_msg(p->get_msg());
		std::string str;
		rsp_content.SerializeToString(&str);
		protocol_head_t head;
		bzero(&head, sizeof(head));
		encode_head(head, id, str.size());
		size = sizeof(protocol_head_t) + str.size();
		*buffer = (uint8_t*)new char[size];
		bzero(*buffer, size);
		protocol_head_codec_t head_codec;
		head_codec.encode(&head, (uint8_t*)*buffer, sizeof(protocol_head_t));
		memcpy(*buffer + sizeof(protocol_head_t), str.data(), str.size());
	}
	break;
	case EEVENTID_SEND_FILEDATA_RSP:
		//encode_general_info();
	{
		FileDataRspEv* p = dynamic_cast<FileDataRspEv*>(ev);
		if (nullptr == p)
		{
			assert(0);
		}
		LOG_INFO("encode:id:%d code:%d, msg:%s", id, p->get_code(), p->get_msg().c_str());
		rsponse_result rsp_content;
		rsp_content.set_code(p->get_code());
		rsp_content.set_msg(p->get_msg());
		std::string str;
		rsp_content.SerializeToString(&str);
		protocol_head_t head;
		bzero(&head, sizeof(head));
		encode_head(head, id, str.size());
		size = sizeof(protocol_head_t) + str.size();
		*buffer = (uint8_t*)new char[size];
		bzero(*buffer, size);
		protocol_head_codec_t head_codec;
		head_codec.encode(&head, (uint8_t*)*buffer, sizeof(protocol_head_t));
		memcpy(*buffer + sizeof(protocol_head_t), str.data(), str.size());
	}
	break;
	default:
		break;
	}

	return true;
}

iEvent* protobuf_protocol_codec_t::decode(uint16_t mid, uint8_t* buffer, uint32_t size)
{
	iEvent* ev = NULL;

	switch (mid)
	{
	case EEVENTID_SEND_GENERAL_REQ:
		ev = decode_file_generalinfo(buffer, size);
		break;
	case EEVENTID_SEND_FILEDATA_REQ:
		ev = decode_file_writefile_data(buffer, size);
		break;
	case EEVENTID_SEND_COMPLETE_REQ:
		LOG_INFO("Server receive file completed!");
		ev = CheckFileMd5();
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
	ss.rdbuf()->pubsetbuf((char*)buffer, size);
	send_fileinfo.ParseFromIstream(&ss);
	if (!send_fileinfo.has_filename())
	{
		LOG_WARNNING("there is no filename filed in message generalinfo_request.");
		return new FileInfoRspEv(1, "failed,no filename field");;
	}
	m_fileInfo.strFileName = send_fileinfo.filename();
	if (!send_fileinfo.has_md5())
	{
		LOG_WARNNING("there is no md5 filed in message generalinfo_request.");
		return new FileInfoRspEv(1, "failed,no md5 field");
	}
	m_fileInfo.strMd5 = send_fileinfo.md5();

	if (!send_fileinfo.has_filelength())
	{
		LOG_WARNNING("there is no filelength field in message generalinfo_request.");
		return new FileInfoRspEv(1, "failed,no filelength field");
	}
	m_fileInfo.filelength = send_fileinfo.filelength();

	if (!send_fileinfo.has_pieces())
	{
		LOG_WARNNING("there is no pieces field in message generalinfo_request.");
		return new FileInfoRspEv(1, "failed,no pieces field");
	}
	m_fileInfo.pieces = send_fileinfo.pieces();

	//TODO:创建文件
	//TODO:error 创建空文件,这里没有做这一步
	int fd = open(m_fileInfo.strFileName.c_str(), O_CREAT | O_RDWR);
	LOG_INFO("fd:%d len:%d", fd, m_fileInfo.filelength);
	lseek(fd, m_fileInfo.filelength - 1, SEEK_SET);
	write(fd, "1", 1);
	m_pFile = new CFileMap(fd, O_RDWR, m_fileInfo.filelength);

	LOG_INFO("FileInfo:%s,md5:%s, len:%u, pieces:%d", m_fileInfo.strFileName.c_str(), m_fileInfo.strMd5.c_str(), m_fileInfo.filelength, m_fileInfo.pieces);
	std::cout << "FileInfo:" << m_fileInfo.strFileName << ",md5:" << m_fileInfo.strMd5 << ", file length:" << m_fileInfo.filelength << ", pieces:" << m_fileInfo.pieces << std::endl;
	return new FileInfoRspEv(0, "send file info ok");
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
		return new FileDataRspEv(0, "send file content ok");
	}
	uint32_t idx = fileContent.idx();
	if (!fileContent.has_pieces())
	{
		LOG_WARNNING("there is no pieces filed in message send_filecontent_request.");
		return new FileDataRspEv(1, "no piece field");
	}
	uint32_t pieces = fileContent.pieces();

	if (!fileContent.has_length())
	{
		LOG_WARNNING("there is no length filed in message send_filecontent_request.");
		return new FileDataRspEv(1, "no length field");
	}
	uint32_t length = fileContent.length();

	if (!fileContent.has_data())
	{
		LOG_WARNNING("there is no data filed in message send_filecontent_request.");
		return new FileDataRspEv(1, "no data field");
	}
	//TODO:二进制数据，这里处理有问题吧
	const std::string data = fileContent.data();

	if (!fileContent.has_crc32())
	{
		LOG_WARNNING("there is no crc32 filed in message send_filecontent_request.");
		return new FileDataRspEv(1, "no crc32 field");
	}
	uint32_t crc32 = fileContent.crc32();
	//TODO:输出二进制这里有问题的
	LOG_INFO("FileContent:idx:%d,pieces:%d, len:%d, crc32:%u", idx, pieces, length, crc32);
	//LOG_INFO("file: %s", data.c_str());
	bool bOK = (crc32 == CRC32((void *)data.c_str(), (size_t)length));
	if (bOK)
	{
		LOG_INFO("decode_file_writefile_data check ok,write file!!");
		size_t ofset = pieces * EACH_PIECES_LENGTH + idx * EACH_FRAME_LENGTH;
		off_t of = m_pFile->seek(ofset, SEEK_SET);
		LOG_INFO("%p, offset:%u, seek:%d", m_pFile, ofset, of);
		m_pFile->write(data.c_str(), length);
		return new FileDataRspEv(0, "send data ok");
	}
	else
	{
		LOG_ERROR("decode_file_writefile_data check failed:%u != %u", crc32, CRC32((void *)data.c_str(), (size_t)length));
		return new FileDataRspEv(1, "send data failed!");
	}
}

iEvent* protobuf_protocol_codec_t::CheckFileMd5()
{
	if (m_pFile)
	{
		m_pFile->seek(0, SEEK_SET);
		char* buf = nullptr;
		m_pFile->read(buf, m_fileInfo.filelength);
		bool bRet = (m_fileInfo.strMd5 == md5(buf, m_fileInfo.filelength));
		if (bRet)
		{
			std::cout << "recv file completed!!!!!" << std::endl;
		}
		else
		{
			std::cout << "recv file failed!!!!!" << std::endl;
		}
	}

	return nullptr;
}
