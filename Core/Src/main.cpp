
#include "main.hpp"
#include "TCP_Client.hpp"


class TCP_Observer : public TCP_Client::IObserver
{
private:
    void OnTcpReceived(TCP_Client *obj, uint8_t *buf, uint32_t len) override
    {
        printf("Received: %s\r\n", (char *) buf);
    }

    void OnTcpConnected(TCP_Client *obj) override
    {
        printf("Connected!\r\n");
    }

    void OnTcpDisconnected(TCP_Client *obj) override
    {
        printf("Disconnected!\r\n");
    }

    void TcpPollConnectionl(TCP_Client *obj) override
    {
        printf("Poll connection!\r\n");
        obj->Send((uint8_t *)"Test", 4);
    }
};


TCP_Client ClTest;
TCP_Observer Observer;

// Основная программа
int main(void)
{
	int err;

    #if defined(_WIN32) || defined(_WIN64)//Windows includes
    WSADATA wsaData;
    err = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (err != 0)
	{
		printf("Errore in WSAStartup\n");
        exit(-1);
	} 
    #endif

    ClTest.BindObserver(&Observer);
    ClTest.Connect("localhost", 9090);

    while (ClTest.IsConnected())
    {
        sleep(1);
    }

    sleep(2);
    
    return 0;  
}