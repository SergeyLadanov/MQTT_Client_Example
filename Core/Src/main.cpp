
#include "main.hpp"
#include "TCP_Client.hpp"
#include "MQTTPacket.h"



static int globalState = 0;

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
        printf("Poll connection!\r\n");
    }
};




class MQTT_Subscriber : public TCP_Client::IObserver
{
private:
    typedef enum
    {
        STATE_NO_CON = 0,
        STATE_CONN_ACK,
        STATE_CONN_SUBACK
    }ConnStates;

    ConnStates State = STATE_NO_CON;
    char Username[64];
    char Password[64];
public:
    MQTT_Subscriber(char *username = nullptr, char *password = nullptr)
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

        struct ReceivedData
        {
            uint8_t *Buf;
            int Offset;
            int Len;
        };

        static ReceivedData pkt = {nullptr, 0, 0};
        pkt.Buf = buf;
        pkt.Offset = 0;
        pkt.Len = len;

        unsigned char local_buf[200];
        int buflen = sizeof(local_buf);
        int req_qos = 0;
        int msgid = 1;
        MQTTString topicString = MQTTString_initializer;



        auto transport_getdata = [] (unsigned char* buf, int count)->int
        {
            int rc = 0;
            if (count <= pkt.Len)
            {
                rc = count;
            }
            else
            {
                rc = pkt.Len;
            }

            memcpy(buf, &pkt.Buf[pkt.Offset], rc);
            pkt.Offset += rc;

            return rc;
        };


        switch (State)
        {
            case STATE_NO_CON :
                /* wait for connack */
                if (MQTTPacket_read(local_buf, buflen, transport_getdata) == CONNACK)
                {
                    unsigned char sessionPresent, connack_rc;

                    if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, local_buf, len) != 1 || connack_rc != 0)
                    {
                        printf("Unable to connect, return code %d\n", connack_rc);
                        State = STATE_NO_CON;
                    }
                    else
                    {
                        printf("Success connack\n");
                        State = STATE_CONN_ACK;
                    }
                }

                /* subscribe */
                topicString.cstring = (char *) "rgb_switch";
                len = MQTTSerialize_subscribe(local_buf, buflen, 0, msgid, 1, &topicString, &req_qos);

                obj->Send((uint8_t *) local_buf, len);


            break;

            case STATE_CONN_ACK :

                if (MQTTPacket_read(local_buf, buflen, transport_getdata) == SUBACK) 	/* wait for suback */
                {
                    unsigned short submsgid;
                    int subcount;
                    int granted_qos;
                    int rc = 0;

                    rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, local_buf, buflen);
                    if (granted_qos != 0)
                    {
                        printf("granted qos != 0, %d\n", granted_qos);
                        State = STATE_NO_CON;
                    }
                    else
                    {
                        printf("Success SUB ACK\n");
                        State = STATE_CONN_SUBACK;
                    }
                }

                

            break;

            case STATE_CONN_SUBACK :
                /* loop getting msgs on subscribed topic */
                topicString.cstring = (char *) "rgb_state";

                /* transport_getdata() has a built-in 1 second timeout,
                your mileage will vary */
                if (MQTTPacket_read(local_buf, buflen, transport_getdata) == PUBLISH)
                {
                    unsigned char dup;
                    int qos;
                    unsigned char retained;
                    unsigned short msgid;
                    int payloadlen_in;
                    unsigned char* payload_in;
                    int rc;
                    MQTTString receivedTopic;

                    rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
                    &payload_in, &payloadlen_in, local_buf, buflen);
                    printf("message arrived %.*s\n", payloadlen_in, payload_in);

                    if (strcmp((const char *) payload_in, "1"))
                    {
                        globalState = 1;
                    }

                    
                    if (strcmp((const char *) payload_in, "0"))
                    {
                        globalState = 0;
                    }


                }

                char payload[64];
                int payloadlen = sprintf(payload, "%d", globalState);

                /* Sending data */
                printf("publishing reading\n");
                len = MQTTSerialize_publish(local_buf, buflen, 0, 0, 0, 0, topicString, (unsigned char*)payload, payloadlen);
                obj->Send((uint8_t *) local_buf, len);
            break;

        }


    }

    void OnTcpConnected(TCP_Client *obj) override
    {
        printf("Connected!\r\n");


        MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
        int rc = 0;
        int mysock = 0;
        unsigned char buf[200];
        int buflen = sizeof(buf);
        int msgid = 1;

        int req_qos = 0;

        int len = 0;

        data.clientID.cstring = (char *) "me1";
        data.keepAliveInterval = 20;
        data.cleansession = 1;
        data.username.cstring = Username;
        data.password.cstring = Password;

        len = MQTTSerialize_connect(buf, buflen, &data);

        obj->Send((uint8_t *) buf, len);

        // printf("disconnecting\n");
        // len = MQTTSerialize_disconnect(buf, buflen);
        // rc = transport_sendPacketBuffer(mysock, buf, len);

    // exit:
    //     transport_close(mysock);

    }

    void OnTcpDisconnected(TCP_Client *obj) override
    {
        printf("Disconnected!\r\n");
        State = STATE_NO_CON;
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
        //tcp_cl1.Connect((const char*) host, port);
        counter--;
    }

    sleep(1);
    
    return 0;  
}