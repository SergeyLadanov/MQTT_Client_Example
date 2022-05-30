
#include "main.hpp"
#include "TCP_Client.hpp"
#include "MQTTPacket.h"


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

        MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
        int rc = 0;
        char buf[200];
        int buflen = sizeof(buf);

        MQTTString topicString = MQTTString_initializer;
        char* payload = "20.0";
        int payloadlen = strlen(payload);
        int len = 0;

        

        data.clientID.cstring = "me";
        data.keepAliveInterval = 20;
        data.cleansession = 1;
        data.username.cstring = "username";
        data.password.cstring = "password";
        data.MQTTVersion = 4;

        len = MQTTSerialize_connect((unsigned char *)buf, buflen, &data);

        topicString.cstring = "sensor";
        len += MQTTSerialize_publish((unsigned char *)(buf + len), buflen - len, 0, 0, 0, 0, topicString, (unsigned char *)payload, payloadlen);

        len += MQTTSerialize_disconnect((unsigned char *)(buf + len), buflen - len);


        if (!obj->Send((uint8_t *)buf, len))
            printf("Successfully published\n");
        else
            printf("Publish failed\n");

        obj->Disconnect();
    }

    void OnTcpDisconnected(TCP_Client *obj) override
    {
        printf("Disconnected!\r\n");
    }

    void TcpPollConnectionl(TCP_Client *obj) override
    {
        printf("Poll connection!\r\n");
        //obj->Send((uint8_t *)"Test", 4);
    }
};


TCP_Client ClTest;
TCP_Observer Observer;
static int counter = 200;

// Основная программа
int main(void)
{
	int err;
    char *host = "host";
    int port = 12608;

    #if defined(_WIN32) || defined(_WIN64)//Windows includes
    WSADATA wsaData;
    err = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (err != 0)
	{
		printf("Errore in WSAStartup\n");
        exit(-1);
	} 
    #endif

    printf("Sending to hostname %s port %d\n", host, port);

    ClTest.BindObserver(&Observer);
    

    while (counter)
    {
        sleep(1);
        ClTest.Connect((const char*) host, port);
        counter--;
        // if (!counter)
        // {
        //     ClTest.Disconnect();
        // }
    }

    sleep(2);
    
    return 0;  
}