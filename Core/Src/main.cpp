
#include "main.hpp"
#include "TCP_Client.hpp"
#include "MQTTPacket.h"


class MQTT_Publisher : public TCP_Client::IObserver
{
private:
    char Username[64];
    char Password[64];

public:
    MQTT_Publisher(char *username = nullptr, char *password = nullptr)
    {
        sprintf(Username, username);
        sprintf(Password, password);
    }
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
        char* payload = (char *) "20.0";
        int payloadlen = strlen(payload);
        int len = 0;

        

        data.clientID.cstring = (char *) "me";
        data.keepAliveInterval = 20;
        data.cleansession = 1;
        data.username.cstring = Username;
        data.password.cstring = Password;
        data.MQTTVersion = 4;

        len = MQTTSerialize_connect((unsigned char *)buf, buflen, &data);

        topicString.cstring = (char *) "sensor";
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
    }
};




class MQTT_Subscriber : public TCP_Client::IObserver
{
private:
    char Username[64];
    char Password[64];
public:
    MQTT_Subscriber(char *username = nullptr, char *password = nullptr)
    {
        sprintf(Username, username);
        sprintf(Password, password);
    }
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
    }
};


static TCP_Client tcp_cl1, tcp_cl2;
static MQTT_Publisher mqtt_pub;
static MQTT_Subscriber mqtt_sub;
static int counter = 200;

// Основная программа
int main(int argc, char *argv[])
{
	int err;
    char *host = (char *) "host";
    int port = 12608;
    char *username = (char *) "username";
    char *password = (char *) "password";

    #if defined(_WIN32) || defined(_WIN64)//Windows includes
    WSADATA wsaData;
    err = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (err != 0)
	{
		printf("Errore in WSAStartup\n");
        exit(-1);
	} 
    #endif

    if (argc > 1)
    {
        host = argv[1];
    }
        
    if (argc > 2)
    {
        port = atoi(argv[2]);
    }
        
    if (argc > 3)
    {
        username = argv[3];
    }
        
    if (argc > 4)
    {
        password = argv[4];
    }
        

    printf("Sending to hostname %s port %d\n", host, port);

    mqtt_pub = MQTT_Publisher(username, password);
    mqtt_sub = MQTT_Subscriber(username, password);

    tcp_cl1.BindObserver(&mqtt_pub);
    tcp_cl2.BindObserver(&mqtt_sub);
    tcp_cl2.Connect((const char*) host, port);

    while (counter)
    {
        sleep(1);
        tcp_cl1.Connect((const char*) host, port);
        counter--;
    }

    sleep(1);
    
    return 0;  
}