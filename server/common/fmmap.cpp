#include "fmmap.h"
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "log.h"

CFileMap::CFileMap(const char *pathname, int flags, size_t filelength /*= 0*/)
	: _filelength(filelength)
	, _fd(-1)
	, _cur(nullptr)
	, _pHandle(nullptr)
{
	assert(nullptr != pathname);

	if (nullptr == pathname)
	{
		LOG_ERROR("File name is empty!");
		return;
	}

	if (0 == filelength)
	{
		LOG_ERROR("File length is zero!!");
		return;
	}
	//open file
	_fd = open(pathname, flags, 0664);
	if (-1 == _fd)
	{
		LOG_ERROR("open file failed!");

		return;
	}
	int prot = 0;
	if (O_RDONLY == flags)
	{
		prot = PROT_READ;
	}
	else if (O_WRONLY == flags)
	{
		prot = PROT_WRITE;
	}
	else if (O_RDWR == flags)
	{
		prot = PROT_WRITE | PROT_READ;
	}
	else
	{
		prot = PROT_WRITE | PROT_READ;
	}
	_cur = _pHandle = (char*)mmap(nullptr, _filelength, prot, MAP_SHARED, _fd, SEEK_SET);
}


CFileMap::CFileMap(const int fd, int flags, size_t filelength)
	: _fd(fd)
	, _filelength(filelength)
	, _pHandle(nullptr)
	, _cur(nullptr)
{
	int prot = 0;
	if (O_RDONLY == flags)
	{
		prot = PROT_READ;
	}
	else if (O_WRONLY == flags)
	{
		prot = PROT_WRITE;
	}
	else if (O_RDWR == flags)
	{
		prot = PROT_WRITE | PROT_READ;
	}
	else
	{
		prot = PROT_WRITE | PROT_READ;
	}
	_cur = _pHandle = (char*)mmap(nullptr, _filelength, prot, MAP_SHARED, _fd, SEEK_SET);
}

off_t CFileMap::seek(off_t offset, int where)
{
	switch (where)
	{
	case SEEK_SET:
		if (offset > _filelength)
		{
			return -1;
		}
		_cur = _pHandle + offset;
		break;
	case SEEK_CUR:
		if (_cur + offset - _pHandle > _filelength)
		{
			return -1;
		}
		_cur += offset;
		break;
	case SEEK_END:
	{

		off_t cur = off_t(_pHandle + _filelength - offset);
		if (cur < 0)
		{
			return -1;
		}
		_cur = _pHandle + _filelength - offset;
	}
	break;
	default://cur
		if (_cur + offset - _pHandle > _filelength)
		{
			return -1;
		}
		_cur += offset;
		break;
	}

	return off_t(_cur - _pHandle);
}


//TODO:read 最好还是要去掉这个buf的copy
size_t CFileMap::read(char*& buf, size_t len)
{
	if (nullptr == _cur || nullptr == _pHandle)
	{
		LOG_ERROR("file mmap create filed!");
		return 0;
	}
	buf = _cur;
	if (len > 0)
	{
		if (_cur + len > _pHandle + _filelength)
		{
			//renew length
			len = _pHandle + _filelength - _cur;
			if (len > 0)
			{
				_cur = _pHandle + _filelength;
			}
		}
		else
		{
			_cur += len;
		}
	}
	else
	{
		len = 0;// only copy zero data
	}

	return len;
}


size_t CFileMap::write(const char* buf, size_t len)
{
	if (nullptr == _cur || nullptr == _pHandle)
	{
		LOG_ERROR("file mmap create filed!");
		
		return 0;
	}
	if (buf && len > 0)
	{
		if (_cur + len > _pHandle + _filelength)
		{
			len = _pHandle + _filelength - _cur;
			if (len > 0)
			{
				memcpy(_cur, buf, len);
				_cur = _pHandle + _filelength;
			}
		}
		else
		{
			memcpy(_cur, buf, len);
			_cur += len;
		}
	}
	else
	{
		len = 0;// only copy zero data
	}

	return len;
}

CFileMap::~CFileMap()
{
	if (-1 != _fd)
	{
		close(_fd);
	}
	if (nullptr != _pHandle)
	{
		munmap(_pHandle, _filelength);
	}
}
