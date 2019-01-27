#pragma once
#include <sys/types.h>
#include <string>
#include <sys/mman.h>
class CFileMap
{
public:
	CFileMap(const char *pathname, int flags, size_t filelength = 0);
	CFileMap(const int fd, int flags, size_t filelength);

	virtual ~CFileMap();

	bool CheckFileMapStatus()const
	{
		return (MAP_FAILED != _pHandle);
	}
	//seek
	off_t seek(off_t offset, int where);

	//read, buf返回的是地址信息,不需要实际拷贝数据进来,节省一次内存拷贝过程
	size_t read(char*& buf, size_t len);

	//write
	size_t write(const char* buf, size_t len);
private:
	int _fd;
	char* _cur;
	size_t _filelength;
	char* _pHandle;
};

