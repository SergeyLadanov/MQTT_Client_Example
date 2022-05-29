#ifndef __TCP_CLIENT_HPP_
#define __TCP_CLIENT_HPP_


#include <cstdint>
#include <sys/types.h>
#include <winsock.h>
#include <cstdio>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)//Windows includes
#include <winsock.h>
#else
#include <sys/socket.h> //Add support for sockets
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <pthread.h>

#ifdef __linux__
#define INVALID_SOCKET (-1)
#define SOCKET int
#define SOCKET_ERROR (-1)
#else
typedef int socklen_t;
#endif

class TCP_Client
{
public:

    class IObserver
    {
    public:
        virtual void OnTcpReceived(TCP_Client *obj, uint8_t *buf, uint32_t len) = 0;
        virtual void OnTcpConnected(TCP_Client *obj) = 0;
        virtual void OnTcpDisconnected(TCP_Client *obj) = 0;
        virtual void TcpPollConnectionl(TCP_Client *obj) = 0;
    };

private:
    static constexpr size_t BUFFER_SIZE = 1024;

    struct ClientArg
    {
        #if defined(_WIN32) || defined(_WIN64)
        SOCKET Fd;
        #else
        int Client_Fd;
        #endif
        IObserver *Observer;
        bool KeepLooping;
        pthread_mutex_t Mutex;

        TCP_Client *Self = nullptr;

        bool IsConnected()
        {
            return (Fd != INVALID_SOCKET);
        }
    };

    pthread_t ReceiveTask;
    pthread_t PollTask;

    ClientArg Hclient = {0, nullptr, false};
    

public:
    TCP_Client()
    {
        memset(&Hclient, 0, sizeof(ClientArg));
        Hclient.Self = this;
    }
    int Connect(const char *host, uint16_t port);
    bool IsConnected(void);
    int Send(uint8_t *data, uint32_t len);
    void BindObserver(IObserver *obj)
    {
        Hclient.Observer = obj;
    }
    void Disconnect(void)
    {
        Hclient.KeepLooping = false;
    }
private:
    static char* DomainIP(const char *domain);
    static void error(const char *msg);
};

#endif