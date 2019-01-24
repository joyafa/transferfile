/*************************************************************************
  > File Name: test.cpp
  > Author: joyafa
  > Mail: joyafa@163.com 
  > Created Time: 2019-01-12 23:18:55
 ************************************************************************/
#include "fmmap.h"
#include<iostream>
#include <sys/stat.h>
#include <unistd.h>

#include <fcntl.h>
using namespace std;

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



int main(int argc, char** argv)
{
	if (argc < 2)
	{
		cout << "test aa.cpp" << endl;
		exit(0);
	}
	size_t len = getfile_length(argv[1]);

	//read file
	CFileMap read_file(argv[1], O_RDONLY, len);

	/*for (;;)
	{
		char buf[100] = { 0 };
		size_t l = read_file.read(buf, 100);
		if (l == 0)
		{
			break;
		}
		cout << buf;
	}*/
	//write file 
	//create file
	int flags = O_CREAT | O_RDWR;
	int fd = open("fmmap.ccp.bak", flags);
	if (-1 == fd)
	{
		cout << "create file failed!" << endl;
		exit(1);
	}
	off_t of = lseek(fd, len - 1, SEEK_SET);
	if (of == -1)
	{
		cout << "lseek failed!" << endl;
		exit(1);
	}
	write(fd, "1", 1);
	lseek(fd, 0, SEEK_SET);
	CFileMap write_file(fd, flags, len);
	for (;;)
	{
		char buf[4*1024] = { 0 };
		size_t l = read_file.read(buf, len);
		if (l == 0)
		{
			cout << "Write file complete!" << endl;
			break;
		}
		write_file.write(buf, len);
	}

	return 1;
}
