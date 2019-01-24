#pragma once 
enum EErrorCode
{
      ERRC_SUCCESS              = 200,
	  ERRC_INVALID_MSG          = 400,
	  ERRC_INVALID_DATA         = 404,
	  ERRC_METHOD_NOT_ALLOWED   = 405,
	  ERRO_PROCCESS_FAILED      = 406,
	  ERRO_BIKE_IS_TOOK         = 407,
	  ERRO_BIKE_IS_RUNNING      = 408,
	  ERRO_BIKE_IS_DAMAGED      = 409
};

/* 事件ID */
enum EventID
{
    EEVENTID_COMMON_RSP                 = 0x00,
    EEVENTID_SEND_GENERAL_REQ           = 0x01, //发送文件基本信息,名称长度md5...
	EEVENTID_SENDD_FILEDATA_REQ         = 0x02,//发送文件内容
	EEVENTID_UNKOWN                     = 0xFFFF
};
