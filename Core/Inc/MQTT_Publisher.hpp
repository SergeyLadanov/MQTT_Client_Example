#ifndef __MQTT_PUBLISHER_HPP_
#define __MQTT_PUBLISHER_HPP_

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
        if (username != nullptr)
        {
            sprintf(Username, username);
        }

        if (password != nullptr)
        {
            sprintf(Password, password);
        }
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

    }
};

#endif