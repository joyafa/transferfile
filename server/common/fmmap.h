#pragma once
#include <sys/types.h>
#include <string>

class CFileMap
{
public:
	CFileMap(const char *pathname, int flags, size_t filelength = 0);
	CFileMap(const int fd, int flags, size_t filelength);

	virtual ~CFileMap();

	//seek
	off_t seek(off_t offset, int where);

	//read
	size_t read(char*& buf, size_t len);

	//write
	size_t write(const char* buf, size_t len);
private:
	int _fd;
	char* _cur;
	size_t _filelength;
	char* _pHandle;
};

