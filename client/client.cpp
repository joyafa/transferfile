//client send filet to server
//each package send 1k bytes
//1 send filename to server
//2 loop read file and send datas to server
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <fcntl.h>
#include <time.h>
using namespace std;

//TODO: send how many data once time is best???
#define BUFFERSIZE  1024

//47.107.182.184
//send aa.zip 192.168.1.1 1699
int main(int argc, char** argv)
{
    if (argc < 4)
    {
        cout << "Usage: send a.zip 192.168.1.1 1699" << endl;
        exit(1);
    }
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sock)
    {
        cout << "create socket failed,errir:" << errno << endl;
        exit(1);
    }
    
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));//memset zero
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[3]));//host to net
    addr.sin_addr.s_addr = inet_addr(argv[2]);
    int ret = connect(sock, (struct sockaddr* )&addr, sizeof(struct sockaddr_in));
    if (-1 == ret)
    {
        cout << "connect server failed: " << errno << endl;
        close(sock);
        exit(1);
    }
    
    //send file name and md5
    //TODO: calc md5
    ostringstream oss;
    oss << argv[1] << ".bak";//TODO: size and md5
    ssize_t s = send(sock, oss.str().c_str(), oss.str().size(), 0);
    if (-1 == s || s != oss.str().size())
    {
        cout << "send data failed:" << errno << endl;
        exit(1);
    }
    cout << "send file name:" << oss.str() << endl;
    int file = open(argv[1], O_RDONLY );//| _O_BINARY);
    if (-1 == file)
    {
        cout << "open file: " << argv[1] << " failed!!!" << endl;
        close(sock);
        exit(1);
    }
    
    clock_t start = clock();

    while (true)//read file
    {
        char buf[BUFFERSIZE] = {0};        
        ssize_t sz = read(file, buf, BUFFERSIZE);
        if (sz > 0)
        {
            //send data
            cout << "send data len:" << sz << endl;
            ret = send(sock, buf, sz, 0);
            if (ret != sz)
            {
                cout << "Send data wrong, right is:" << sz <<" but only send:" << ret << endl;
                break;
            }
        }
        else
        {
            //send complete
            cout << "File send completed!!!!" << endl;
            break;
        }
    }
    cout << "Total time: " << (clock() - start) / 1000 << "ms" << endl;
    close(file);
    close(sock);
    
    return 1;
}
