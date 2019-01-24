//server send filet to server
//each package send 1k bytes
//1 send filename to server
//2 loop read file and send datas to server
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <pthread.h>
using namespace std;

//TODO: send how many data once time is best???
#define BUFFERSIZE  1024
void* process(void* arg);

//47.107.182.184
//./send aa.zip 192.168.1.1 1699
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        cout << "Usage:./server 1688" << endl;
        exit(1);
    }
    //create socket
    int listensock = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == listensock)
    {
        cout << "create socket failed: " << errno << endl;
        exit(1);
    }
    
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family  = AF_INET;
    addr.sin_port = htons(atoi(argv[1]) );
    addr.sin_addr.s_addr = INADDR_ANY;
    //bind port
    int ret = bind(listensock, (struct sockaddr* )&addr, sizeof(addr));
     if ( -1 == ret)
    {
        cout << "bind socket error: " << errno << endl;
        exit(1);
    } 
    //listen 
    ret = listen(listensock, 10);
    if(-1 == ret)
    {
        cout << "listen socket failed: " << errno << endl;
        exit(1);
    }
    
    cout << "server ready......" << endl;
    //accept
    while (true)
    {
        struct sockaddr_in clientaddr;
        bzero(&clientaddr, sizeof(clientaddr));
        socklen_t len = sizeof(struct sockaddr_in);
        int clientsock = accept(listensock, (struct sockaddr* )&clientaddr, &len);
        if (-1 == clientsock)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }
            else
            {
                close(listensock);
                cout << "accept client failed: " << errno << endl;
                exit(1);
            }
        }
        //create a thread  to receive client data
        pthread_t tid = 0;
        ret = pthread_create(&tid, NULL, process, (void* )&clientsock);             
        if (ret  != 0) 
        {
            perror("pthread_create");
            exit(1);
        }
        //thread separate
        pthread_detach(tid);
        //check  file
    }
    close(listensock);
    cout << "file recv completed!" << endl;
    
    return 1;
}


void* process(void* arg)
{
    int clientsock = *(int* )arg;
    char buf[BUFFERSIZE] = {0};
    //read file name
    int ret = recv(clientsock, buf, BUFFERSIZE, 0);
    if (-1 == ret)
    {
        cout << "recv filename failed:" << errno << endl;
        close(clientsock);
        return NULL;
    }
    cout << "recv file name:" << buf << endl;
    char* filename = buf;
    int file = creat(filename, O_CREAT);
    if (-1 == file)
    {
        cout << "create file: " << filename << " failed, error: " << errno << endl;
        close(clientsock);
        return NULL;
    }
    
    while (true)
    {
        int len = recv(clientsock, buf, BUFFERSIZE, 0);
        //write file
        if (len > 0)
        {
            cout << "recv data len:" << len << endl;
            write(file, buf, len);
        }
        else if(-1 == len)
        {
            if(EAGAIN == errno || EWOULDBLOCK == errno)
            {
                continue;
            }
        }
        else
        {
            break;
        }
    }
    close(clientsock);
    close(file);
    cout << "Receive file completed!!!" << endl;
    
    return NULL;
}
