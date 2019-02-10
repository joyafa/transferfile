#pragma once

#include "event.h"
#include "eventtype.h"
#include <string>
#include <vector>

typedef struct tagFileInfo_s
{
	  std::string strFileName;
		std::string strMd5;
		uint64_t filelength;
		uint32_t pieces;			
}tagFileInfo_t;

class FileInfoRspEv : public iEvent
{
public:
	FileInfoRspEv(uint32_t code, const std::string& msg) :
		iEvent(EEVENTID_SEND_GENERAL_RSP, generateSeqNo()),
		code_(code),
		msg_(msg)
	{};

	uint32_t get_code() { return code_; };
	const std::string& get_msg() { return msg_; };
	//virtual std::ostream& dump(std::ostream& out) const;

private:
	uint32_t      code_;  
	std::string   msg_;  
};


class FileDataRspEv : public iEvent
{
public:
	FileDataRspEv(uint32_t code, const std::string& msg) :
		iEvent(EEVENTID_SEND_FILEDATA_RSP, generateSeqNo()),
		code_(code),
		msg_(msg)
	{};

	uint32_t get_code() { return code_; };
	const std::string& get_msg() { return msg_; };
	//virtual std::ostream& dump(std::ostream& out) const;

private:
	uint32_t      code_;
	std::string   msg_;
};
