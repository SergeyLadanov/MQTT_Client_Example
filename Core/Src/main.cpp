
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

    }
};




class MQTT_Subscriber : public TCP_Client::IObserver
{
private:
    typedef enum
    {
        STATE_NO_CON = 0,
        STATE_CONN_ACK
    }ConnStates;

    TCP_Client *htcp = nullptr;
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

    void Disconnect(void)
    {
        unsigned char local_buf[200];
        int buflen = sizeof(local_buf);
        int len = 0;
        len = MQTTSerialize_disconnect(local_buf, buflen);
        printf("disconnecting\n");
        htcp->Send((uint8_t *) local_buf, len);
        htcp->Disconnect();
    }

    int Subscribe(char *topic_name, int msgid)
    {
        if (State != STATE_CONN_ACK)
        {
            return -1;
        }
        unsigned char local_buf[200];
        int buflen = sizeof(local_buf);
        int repply_len = 0;
        int req_qos = 0;
         
        MQTTString topicString = MQTTString_initializer;
        topicString.cstring = topic_name;
        repply_len = MQTTSerialize_subscribe(local_buf, buflen, 0, msgid, 1, &topicString, &req_qos);
        htcp->Send((uint8_t *) local_buf, repply_len);
        return 0;
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
        int repply_len = 0;
        int rc = 0;



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


        int parse_result = MQTTPacket_read(local_buf, buflen, transport_getdata);
        

        switch(parse_result)
        {
            case CONNACK :
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

                /* subscribe */
                Subscribe((char *) "rgb_switch", 1);
                Subscribe((char *) "rgb_brightness", 2);
                Subscribe((char *) "rgb_color", 3);
            break;

            case SUBACK :
                unsigned short submsgid;
                int subcount;
                int granted_qos;

                rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, local_buf, buflen);

                printf("Subcount: %d\n", subcount);

                if (granted_qos != 0)
                {
                    printf("granted qos != 0, %d\n", granted_qos);
                }
                else
                {
                    printf("Success SUB ACK\n");

                }
            break;

            case PUBLISH :
                unsigned char dup;
                int qos;
                unsigned char retained;
                unsigned short msgid;
                int payloadlen_in;
                unsigned char* payload_in;
                //int rc;
                MQTTString receivedTopic;

                rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
                &payload_in, &payloadlen_in, local_buf, buflen);

                snprintf((char *) local_buf, receivedTopic.lenstring.len + 1,  receivedTopic.lenstring.data);

                printf("\r\n\r\n");
                printf("Rc: %d\n", rc);
                printf("Msg id: %d\n", msgid);
                printf("Qos: %d\n", qos);
                printf("Dup: %d\n", dup);
                printf("Payload len: %d\n", payloadlen_in);
                printf("retained: %d\n", retained);
                printf("Received topic: %s\n", local_buf);
                printf("message arrived %.*s\n", payloadlen_in, payload_in);

                if (!strncmp(receivedTopic.lenstring.data, "rgb_switch", receivedTopic.lenstring.len))
                {
                    printf("Switch control\r\n");
                    if (*payload_in == '1')
                    {
                        globalState = 1;
                    }

                    if (*payload_in == '0')
                    {
                        globalState = 0;
                    }
                }

                if (!strncmp(receivedTopic.lenstring.data, "rgb_brightness", receivedTopic.lenstring.len))
                {
                    printf("Brightness select\r\n");
                }

                if (!strncmp(receivedTopic.lenstring.data, "rgb_color", receivedTopic.lenstring.len))
                {
                    printf("Color select\r\n");
                }

                printf("\r\n\r\n");
            break;
        }

    }

    void OnTcpConnected(TCP_Client *obj) override
    {
        printf("Connected!\r\n");
        htcp = obj;

        MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

        unsigned char buf[200];
        int buflen = sizeof(buf);

        int len = 0;

        data.clientID.cstring = (char *) "me1";
        data.keepAliveInterval = 20;
        data.cleansession = 1;
        data.username.cstring = Username;
        data.password.cstring = Password;

        len = MQTTSerialize_connect(buf, buflen, &data);

        obj->Send((uint8_t *) buf, len);
    }

    void OnTcpDisconnected(TCP_Client *obj) override
    {
        printf("Disconnected!\r\n");
        State = STATE_NO_CON;
    }

    void TcpPollConnectionl(TCP_Client *obj) override
    {
        if (State == STATE_CONN_ACK)
        {
            // Publishing state
            unsigned char local_buf[200];
            int buflen = sizeof(local_buf);
            MQTTString topicString = MQTTString_initializer;
            int repply_len = 0;

            char payload[64];
            int payloadlen;

            payloadlen = sprintf(payload, "%d", globalState);

            /* loop getting msgs on subscribed topic */
            topicString.cstring = (char *) "rgb_state";
            /* Sending data */
            printf("publishing reading\n");
            repply_len = MQTTSerialize_publish(local_buf, buflen, 0, 0, 0, 0, topicString, (unsigned char*)payload, payloadlen);
            obj->Send((uint8_t *) local_buf, repply_len);
        }
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