#include "md5.h"
#include "crc32.h"
#include "iostream"
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>
#include "protocol_head.h"
#include "file_socket.h"
#include <unistd.h>
#include "protocol.pb.h"
#include <sstream>
#include <stdint.h>
#include <string>
#include "log.h"
#include "fmmap.h"
using namespace std;
using namespace FileTransport;

/* 事件ID */
enum EventID
{
	EEVENTID_COMMON_RSP = 0x00,
	EEVENTID_SEND_GENERAL_REQ = 0x01, //发送文件基本信息,名称长度md5...
	EEVENTID_SENDD_FILEDATA_REQ = 0x02,//发送文件内容
	EEVENTID_UNKOWN = 0xFFFF
};

const size_t each_pices_length = 100 * 1024 * 1024L; //100M 文件分片,文件片数,决定通过几个进程来发送
const size_t each_frame_length = 512;// 4 * 1024L; //4k 每帧数据长度,每次socket发送数据的长度
#define MAKEVERSION(a, b)  ((BYTE)(((BYTE)(a)) & 0x0f) | (BYTE)((BYTE)(((BYTE)(b)) & 0x0f) << 4))
//获取文件长nt count_file(char *filename) {}度
size_t getfile_length(const char* filename)
{
	int fd = open(filename, O_RDONLY);// | O_BINARY);
	if (-1 == fd)
	{
		cout << "open file failed!!" << endl;
		return 0;
	}
	off_t beg = lseek(fd, 0, SEEK_SET);
	off_t end = lseek(fd, 0, SEEK_END);
	size_t len = end - beg;
	close(fd);
	cout << len << endl;
	return len;
}

protocol_head_t& encode_head(protocol_head_t& head, WORD msgid, uint32_t len)
{
	//initial header
	head.tag_     = 0xFBFC;
	head.type_    = PB_PROTOCOL_TYPE;
	head.version_ = MAKEVERSION(1, 1);//1.1=>0x11
	head.msg_id_  = msgid;// function
	head.len_     = len;
	//head.msg_sn = 1;
	//head.reserve_ = 1;
}

//long selfid, unsigned short idx, unsigned short count, char type
std::string encode_fileinfo(const std::string& strFileName, const std::string& strMd5, uint32_t fileLength, uint32_t pieces)
{
	//body
	generalinfo_request send_fileinfo;
	send_fileinfo.set_filename(strFileName);
	send_fileinfo.set_md5(strMd5);
	send_fileinfo.set_filelength(fileLength);
	send_fileinfo.set_pieces(pieces);
	std::string str;
	send_fileinfo.SerializeToString(&str);

	return str;
}


uint32_t send_file_info(const std::string& strFileName, const std::string& strMd5, uint32_t fileLength, uint32_t pieces, int fd)
{
	std::string msg = encode_fileinfo(strFileName, strMd5, fileLength, pieces);
	protocol_head_t head;
	bzero(&head, sizeof(head));
	encode_head(head, EEVENTID_SEND_GENERAL_REQ, msg.size());
	int len = sizeof(protocol_head_t) + msg.size();
	char* buf = new char[len];
	bzero(buf, len);
	/*memcpy(buf, &head, sizeof(protocol_head_t));*/
	protocol_head_codec_t head_codec;
	head_codec.encode(&head, (uint8_t* )buf, len);
	memcpy(buf + sizeof(protocol_head_t), msg.data(), msg.size());
	int ret = nio_write(fd, buf, len);
	//TODO:check response
	nio_recv(fd, buf, sizeof(head), &ret);
	bzero(&head, sizeof(head));
	head_codec.decode((uint8_t* )buf, sizeof(head), &head);
	nio_recv(fd, buf, head.len_, &ret);
    //TODO:check response

	delete[] buf;

	return ret;
}

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		cout << "usage:send aaa.zip 192.168.11.11 9099" << endl;

		exit(0);
	}
	//step 1 file compress

	//step 2 cal md5, file size 
	std::string strFileName = argv[1];
	std::string strMd5 =  md5file(strFileName.c_str());
	
	size_t  fileLength = getfile_length(strFileName.c_str());
	if (fileLength <= 0)
	{
		LOG_ERROR("Send file length must not zero!");

		exit(1);
	}
	//step3 cal the number of process, according to file size
	int nProcess = fileLength % each_pices_length ? (fileLength / each_pices_length + 1) : (fileLength / each_pices_length);

	//step4 connect server
	int sockfd = init_client(argv[2], atoi(argv[3]));
	if (sockfd < 0)
	{
		LOG_ERROR("Connect server error!!");

		exit(1);
	}

	set_socket_non_block(sockfd);

	//step3 send file info
	send_file_info(strFileName, strMd5, fileLength, nProcess, sockfd);

#if 0
	//step4 fork sub process to send file 
	for (int i = 0; i < nProcess; ++i)
	{
		fork();
		//check parent & child 
	}
	//parent create evenfd for statices the send results
	check_is_send_over();
	cout << "文件发送完成!" << endl;

	//child process send pice
	//TODO:怎么把每块的起始位置发给子进程

	//创建这么多个进程.需要fork几次

	//eventfd怎么接收
#endif
	//每块数据分开大小发送,4*1024 或者是1个MCU
	CFileMap fm(strFileName.c_str(), O_RDONLY, fileLength);
	char *buf = nullptr;
	for (;;)
	{
		int len = fm.read(buf, each_frame_length);
		if (len > 0)
		{
			cout << buf;
			//TODO: 组包
			//std::string msg = encode_fileinfo(strFileName, strMd5, fileLength, pieces);
			protocol_head_t head;
			bzero(&head, sizeof(head));
			encode_head(head, EEVENTID_SEND_GENERAL_REQ, msg.size());
			int len = sizeof(protocol_head_t) + msg.size();
			//文件内容 body
			//senddata
			//TODO: 数据怎么从buf中放到protobuf中a,这里是不是有一次内存拷贝过程,能否省去
			nio_write(sockfd, buf, each_frame_length);
		}
		else
		{
			break;
		}
	}
	//event怎么通知, 应该还要通知发送的状态,发送了多少了, 
	//记录发送时间
	//eventfd notice parent process
	cout << "文件片: 发送完成" << endl;
	close(sockfd);

	return 1;
}

//client 思路,就是考虑多进程方式
//后面可以改造为reactor方式也是可以的
