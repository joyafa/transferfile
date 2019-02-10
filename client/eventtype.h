#pragma once 
enum EErrorCode
{
      ERRC_SUCCESS              = 200,
	  ERRC_INVALID_MSG          = 400,
	  ERRC_INVALID_DATA         = 404,
	  ERRC_METHOD_NOT_ALLOWED   = 405,
	  ERRO_PROCCESS_FAILED      = 406,
};

/* 事件ID */
enum EventID
{
	EEVENTID_SEND_GENERAL_REQ           = 0x00, //发送文件基本信息,名称长度md5...
	EEVENTID_SEND_GENERAL_RSP           = 0x01,
	EEVENTID_SEND_FILEDATA_REQ          = 0x02,//发送文件内容
	EEVENTID_SEND_FILEDATA_RSP          = 0x03,
	EEVENTID_SEND_COMPLETE_REQ          ,      //发送完成,没有包体
	EEVENTID_UNKOWN                     = 0xFFFF
};


const size_t EACH_PIECES_LENGTH = 100 * 1024 * 1024L; //100M 文件分片,文件片数,决定通过几个进程来发送
const size_t EACH_FRAME_LENGTH = 4 * 1024L; //4k 每帧数据长度,每次socket发送数据的长度
#define MAKEVERSION(a, b)  ((BYTE)(((BYTE)(a)) & 0x0f) | (BYTE)((BYTE)(((BYTE)(b)) & 0x0f) << 4))
