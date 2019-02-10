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
#include <sys/wait.h>
#include <thread>
#include <sys/eventfd.h>
#include "eventtype.h"
using namespace std;
using namespace FileTransport;


//TODO: �ļ���ô�����������;
// 2 server �յ�֮��,��Ҫ����md5....����յ�complete��Ϣʱ����Ҫ�Ƚϼ��
//3 �ദ����,��Ҫ���Ӧ������,����Ӧ����־.......
//��ȡ�ļ�����
size_t getfile_length(const char* filename)
{
	int fd = open(filename, O_RDONLY, 0664);
	if (-1 == fd)
	{
		cout << "open file failed!!" << endl;
		return 0;
	}
	//��������ļ�,��Ҫ�򿪺� _D__USE_FILE_OFFSET64
	size_t len = 0;
#ifndef __NUSESTAT__
	struct stat sbuf;
	bzero(&sbuf, sizeof(struct stat));
	int ret = fstat(fd, &sbuf);
	if (ret < 0)
	{
		perror("fstat");
	}
	else
	{
		len = sbuf.st_size;
	}
#else
	off_t beg = lseek(fd, 0, SEEK_SET);
	off_t end = lseek(fd, 0, SEEK_END);
	len = end - beg;
#endif
	close(fd);

	return len;
}


protocol_head_t& encode_head(protocol_head_t& head, WORD msgid, uint32_t bodyLength)
{
	//initial header
	head.tag_ = 0xFBFC;
	head.type_ = PB_PROTOCOL_TYPE;
	head.version_ = MAKEVERSION(1, 1);//1.1=>0x11
	head.msg_id_ = msgid;// function
	head.len_ = bodyLength;
	//head.msg_sn = 1;
	//head.reserve_ = 1;
	LOG_INFO("encode_head: tag:%02X, type:%d, version:%02X, msgid:%d, len:%d,sn:%d, resv:%d ", head.tag_, head.type_, head.version_, head.msg_id_, head.len_, head.msg_sn, head.reserve_);
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
	LOG_INFO("FileInfo:%s,md5:%s, len:%u, pieces:%d", strFileName.c_str(), strMd5.c_str(), fileLength, pieces);

	return str;
}


std::string encode_filecontent(int nPiece, int idx, const char* buf, uint32_t len)
{
	assert(len > 0 && nullptr != buf);

	//body
	send_filecontent_request send_content;
	send_content.set_idx(idx);
	send_content.set_pieces(nPiece);
	send_content.set_length(len);
	send_content.set_data(buf, len);
	uint32_t crc32 = CRC32((void *)buf, (size_t)len);
	send_content.set_crc32(crc32);
	std::string str;
	send_content.SerializeToString(&str);
	LOG_INFO("encode_filecontent: piece:%d,idx:%d, len:%u", nPiece, idx, len);

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
	protocol_head_codec_t head_codec;
	head_codec.encode(&head, (uint8_t*)buf, sizeof(protocol_head_t));
	memcpy(buf + sizeof(protocol_head_t), msg.data(), msg.size());
	int ret = nio_write(fd, buf, len);
	if (ret != len)
	{
		LOG_ERROR("Send send_file_info len:%d", ret);
	}
	LOG_INFO("Send send_file_info len:%d", ret);

	//TODO:check response
#if 1
	len = nio_recv(fd, buf, sizeof(head), &ret);
	LOG_INFO("nio_recv len:%d,ret:%d", len, ret);
	if (0 == ret && len == sizeof(head))
	{
		bzero(&head, sizeof(head));
		head_codec.decode((uint8_t*)buf, sizeof(head), &head);
		LOG_INFO("nio_recv head:%d,id:%d len:%d", len, head.msg_id_, head.len_);
		len = nio_recv(fd, buf, head.len_, &ret);
		rsponse_result result;
		std::istringstream ss;
		ss.rdbuf()->pubsetbuf((char*)buf, head.len_);
		result.ParseFromIstream(&ss);
		if (!result.has_code())
		{
			LOG_WARNNING("there is no code field in message rsponse_result.");
		}
		else
		{
			if (result.code())
			{
				LOG_ERROR("recv error.");
			}
			else
			{
				LOG_INFO("recv ok");
			}
		}
	}
	//TODO:check response
#endif
	delete[] buf;

	return ret;
}



void send_completeFrame(int fd)
{
	protocol_head_t head;
	bzero(&head, sizeof(head));
	encode_head(head, EEVENTID_SEND_COMPLETE_REQ, 0);
	int len = sizeof(protocol_head_t);
	char* buf = new char[len];
	bzero(buf, len);
	protocol_head_codec_t head_codec;
	head_codec.encode(&head, (uint8_t*)buf, sizeof(protocol_head_t));
	int ret = nio_write(fd, buf, len);
	LOG_INFO("Send file file complete");
}



int main(int argc, char** argv)
{
	if (argc < 4)
	{
		cout << "usage:send aaa.zip 192.168.11.11 9099" << endl;

		exit(0);
	}
	LOG_INIT("log");
	//step 1 file compress

	//step 2 cal md5, file size 
	std::string strFileName = argv[1];
	std::string strMd5 = md5file(strFileName.c_str());
	LOG_INFO("Send file md5:%s", strMd5.c_str());
	size_t  fileLength = getfile_length(strFileName.c_str());
	if (fileLength <= 0)
	{
		LOG_ERROR("Send file length must not zero!");

		exit(1);
	}
	//step3 cal the number of process, according to file size
	int nProcess = fileLength % EACH_PIECES_LENGTH ? (fileLength / EACH_PIECES_LENGTH + 1) : (fileLength / EACH_PIECES_LENGTH);
	LOG_INFO("the number of processes:%d", nProcess);
	//step4 connect server
	int sockfd = 0;
#if 1
	sockfd = init_client(argv[2], atoi(argv[3]));
	if (sockfd < 0)
	{
		LOG_ERROR("Connect server error!!");

		exit(1);
	}
	LOG_INFO("connect:%s:%s ok!", argv[2], argv[3]);

	//set_socket_non_block(sockfd);

	//step5 send file info
	//TODO: �е�����,�ļ����� ԭ����ȡ���ǱȽϴ�ĳ��� size_t ,���ڵ��� uint32�����Է�,����???
	send_file_info(strFileName, strMd5, fileLength, nProcess, sockfd);
	cout << "Send file info ok!" << endl;
#endif
	//step6  map file
	CFileMap fm(strFileName.c_str(), O_RDONLY, fileLength);
	if (!fm.CheckFileMapStatus())
	{
		perror("Map file failed!!");
		exit(1);
	}
	//create eventfd
	int efd = eventfd(0, 0);
	if (-1 == efd)
	{
		LOG_ERROR("Create eventfd failed!!");
		exit(1);
	}
	//step5 fork sub process to send file 
	pid_t processid = 0;
	int nPiece = 0;
	for (nPiece = 0; nPiece < nProcess; ++nPiece)
	{
		processid = fork();
		LOG_INFO("process id is %d after fork", processid);
		if (processid < 0)
		{
			LOG_ERROR("fork child task failed.");
			break;
		}
		else if (processid == 0)  // this is child process
		{
			LOG_INFO("this is child process.");
			break;
		}
		else if (processid > 0)
		{
			LOG_INFO("this is parent process.");
		}
	}
	if (processid > 0) //parent
	{
		LOG_INFO("parent id is %d", getpid());
		LOG_INFO("file transport client start successful!");

		//parent create evenfd for statices the send results
		//eventfd��ô����
		//check_is_send_over();
		//cout << "Send file completed, close socket!!" << endl;
		//close(sockfd);
		// if child process exit with -1 then exit parent process.
		// if child process exit with not -1 then restart child process
		int status = 0;
		while (status == 0)
		{
			pid_t pid = wait(&status);  // parent process block here.
			int exitStatus = WEXITSTATUS(status);
			LOG_INFO("parent process wait return with: pid=%d, status=%d, exitStatus=%d.", pid, status, exitStatus);

			if ((WIFEXITED(status) != 0) && (pid > 0))
			{
				if (exitStatus == -1)
				{
					LOG_ERROR("child process %d cannot normal start so parent should be exit too.", pid);
					exit(-1);
				}
				else
				{
					processid = fork();  // restart an child process.
					LOG_INFO("restart child process");
					if (processid == 0) break;  // break while.
				}
			}

			if (WIFSIGNALED(status)) // child exited on an unhandled signal (maybe a bus error or seg fault)
			{
				LOG_INFO("restart child process");
				processid = fork();  // restart an child process.
				if (processid == 0) break;  // break while.
			}
		}
		//TODO:�����һ���������Ϣ,���շ��յ������Ϊ�ļ��Ѿ�������ϾͿ��Լ��md5
		//TODO:block read
		uint64_t u = 0;
		ssize_t s = read(efd, &u, sizeof(uint64_t));
		if (s != sizeof(uint64_t))
		{
			LOG_ERROR("parent process read data from eventfd failed!");
		}
		else
		{
			LOG_INFO("parent process read data from eventfd:%d", u);
		}
		//.....//TODO:������������eventfd������ֵ����һֱ,��Ϊ�������,����һ��������Ϣ
		if (u == nProcess)
		{
			send_completeFrame(sockfd);
			LOG_INFO("Close socket:%d ok!", sockfd);
			close(sockfd);
		}
		for (;;)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	if (0 == processid)//child process
	{
		LOG_INFO("this is child process pid=%d (parentid)=%d,piece=%d", getpid(), getppid(), nPiece);

		//child conenct server
		sockfd = init_client(argv[2], atoi(argv[3]));
		if (sockfd < 0)
		{
			LOG_ERROR("child process Connect server error!!");

			exit(1);
		}
		LOG_INFO("child process connect:%s:%s ok!", argv[2], argv[3]);

		//set_socket_non_block(sockfd);

		//child process send pieces
		//��ȡ�ļ��Ŀ�ʼλ��
		int ofset = nPiece * EACH_PIECES_LENGTH;
		LOG_INFO("this is child process pid=%d begin=%d", getpid(), ofset);
		//����ƫ��λ��,����ƫ��λ�ý��ж�ȡ�ļ�
		fm.seek(ofset, SEEK_SET);
		//ÿ�����ݷֿ���С����,4*1024 ������1��MCU
		char *buf = nullptr;
		for (int idx = 0;; ++idx)
		{
			int len = fm.read(buf, EACH_FRAME_LENGTH);
			if (len > 0)
			{
				std::string msg = encode_filecontent(nPiece, idx, buf, len);
				cout << "Process:" << getpid() << ", Piece:" << nPiece << " ,idx:" << idx << ", len=" << len << endl;
				protocol_head_t head;
				bzero(&head, sizeof(head));
				encode_head(head, EEVENTID_SEND_FILEDATA_REQ, msg.size());
				int packlen = sizeof(protocol_head_t) + msg.size();
				char* pSendBuff = new char[packlen];
				bzero(pSendBuff, packlen);
				//senddata
				//TODO: server recv has sm problem
				protocol_head_codec_t head_codec;
				head_codec.encode(&head, (uint8_t*)pSendBuff, sizeof(protocol_head_t));
				memcpy(pSendBuff + sizeof(protocol_head_t), msg.data(), msg.size());

				char* pBuffer = nullptr;
				while (true)
				{
					len = nio_write(sockfd, pSendBuff, packlen);
					LOG_INFO("Send file content Data len:%d", len);

					//TODO: recv response, header + body
					int ret = 0;
					bzero(&head, sizeof(head));
					len = nio_recv(sockfd, (char*)&head, sizeof(head), &ret);
					LOG_INFO("nio_recv: len:%d, ret:%d", len, ret);
					if (ret != 0 || len != sizeof(head))
					{
						cout << "Recv head error, send again..." << endl;
						//continue;
					}
					protocol_head_t headDecode;
					bzero(&headDecode, sizeof(headDecode));
					head_codec.decode((uint8_t*)&head, sizeof(head), &headDecode);
					LOG_INFO("nio_recv head:%d,id:%d len:%d", len, headDecode.msg_id_, headDecode.len_);
					if (nullptr == pBuffer)
					{
						pBuffer = new char[headDecode.len_];
					}
					len = nio_recv(sockfd, pBuffer, headDecode.len_, &ret);
					LOG_INFO("nio_recv:%d,id:%d len:%d", len, headDecode.msg_id_, headDecode.len_);
					rsponse_result result;
					std::istringstream ss;
					ss.rdbuf()->pubsetbuf((char*)pBuffer, headDecode.len_);
					result.ParseFromIstream(&ss);
					if (!result.has_code())
					{
						LOG_WARNNING("there is no code field in message rsponse_result.");
					}
					else
					{
						if (result.code())
						{
							LOG_ERROR("recv error.");
							cout << "recv error" << endl;
							continue;
						}
						else
						{
							LOG_INFO("recv ok");
							cout << "recv ok" << endl;
							break;;
						}
					}
					//TODO:
					break;
				}
				if (nullptr != pBuffer)
				{
					delete[] pBuffer;
				}
				if (nullptr != pSendBuff)
				{
					delete[] pSendBuff;
				}
			}
			else
			{
				break;
			}
		}
		close(sockfd);
		uint64_t u = 1;
		ssize_t s = write(efd, &u, sizeof(uint64_t));
		if (s != sizeof(uint64_t))
		{
			LOG_ERROR("Write eventfd error!");
		}
		LOG_INFO("child process exit");
	}

	return 1;
}

//event��ô֪ͨ, Ӧ�û�Ҫ֪ͨ���͵�״̬,�����˶�����, 
//��¼����ʱ��
//eventfd notice parent process
//cout << "File send over" << endl;

//client ˼·,���ǿ��Ƕ���̷�ʽ
//������Ը���Ϊreactor��ʽҲ�ǿ��Ե�
