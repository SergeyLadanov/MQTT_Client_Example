#include "MQTT_Client.hpp"
#include "MQTTPacket.h"
#include <cstring>

#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define SUCCESS        0

bool MQTT_Client::Subscribe(char *topic, int msgid)
{
    if (State != STATE_CONN_ACK)
    {
        return false;
    }
    unsigned char local_buf[200];
    int buflen = sizeof(local_buf);
    int repply_len = 0;
    int req_qos = 0;

    MQTTString topicString = MQTTString_initializer;
    topicString.cstring = topic;
    repply_len = MQTTSerialize_subscribe(local_buf, buflen, 0, msgid, 1, &topicString, &req_qos);
    
    if (!Data.Tcp.Send((uint8_t *) local_buf, repply_len))
    {
        return false;
    }

    return true;
}


bool MQTT_Client::Publish(char *topic, unsigned char retained, char* payload)
{
    int payloadlen = strlen(payload);
    return Publish(topic, retained, payload, payloadlen);
}


bool MQTT_Client::Publish(char *topic, unsigned char retained, char* payload, int payloadlen)
{
    if (State != STATE_CONN_ACK)
    {
        return false;
    }

    // Publishing state
    unsigned char local_buf[200];
    int buflen = sizeof(local_buf);
    MQTTString topicString = MQTTString_initializer;
    int repply_len = 0;


    /* loop getting msgs on subscribed topic */
    topicString.cstring = topic;
    /* Sending data */
    // printf("publishing reading\n");
    repply_len = MQTTSerialize_publish(local_buf, buflen, 0, 0, retained, 0, topicString, (unsigned char*)payload, payloadlen);
    if (!Data.Tcp.Send((uint8_t *) local_buf, repply_len))
    {
        return false;
    }

    return true;
}


bool MQTT_Client::Begin(const char *host, uint16_t port, const char *username, const char *password)
{
    int status = 0;
    snprintf(Data.Host, sizeof(Data.Host), host);
    Data.Port = port;
    snprintf(Username, sizeof(Username), username);
    snprintf(Password, sizeof(Password), password);

    pthread_mutex_init(&Data.Mutex, NULL);
    

    status = pthread_create(&KeepConnectionTask, NULL, [] (void *args)->void*
    {
        DataStruct *hdat = (DataStruct *) args;
        hdat->KeepLooping = true;
        uint32_t Delay = 10;
        

        do
        {
            Delay++;

            if (Delay >= 10)
            {
                pthread_mutex_lock(&hdat->Mutex);

                // printf("Conn status: %d\r\n", hdat->Tcp.IsConnected());
                
                if (!hdat->Tcp.IsConnected())
                {
                    hdat->Tcp.Connect((const char*) hdat->Host, hdat->Port);
                }

                pthread_mutex_unlock(&hdat->Mutex);
                Delay = 0;
            }
            sleep(1);
        }
        while (hdat->KeepLooping);

        pthread_mutex_destroy(&hdat->Mutex);
        if (hdat->Tcp.IsConnected())
        {
            hdat->Tcp.Disconnect();
        }

        return SUCCESS;
    }
    , &Data);

    return status;
}


void MQTT_Client::BindObserver(IObserver *obj)
{
    Observer = obj;
}


void MQTT_Client::Stop(void)
{
    pthread_mutex_lock(&Data.Mutex);
    Data.KeepLooping = false;
    pthread_mutex_unlock(&Data.Mutex);
}


void MQTT_Client::OnTcpReceived(TLS_Client *obj, uint8_t *buf, uint32_t len)
{
    // printf("Received: %s\r\n", (char *) buf);

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
                if (Observer != nullptr)
                {
                    Observer->MQTT_OnConnected(this);
                }
            }

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
            MQTTString receivedTopic;
            char TopicCstring[64];

            rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
            &payload_in, &payloadlen_in, local_buf, buflen);

            snprintf(TopicCstring, receivedTopic.lenstring.len + 1,  receivedTopic.lenstring.data);

            if (Observer != nullptr)
            {
                Observer->MQTT_OnReceived(this, TopicCstring, (char *) payload_in, payloadlen_in);
            }
        break;
    }
}


void MQTT_Client::OnTcpConnected(TLS_Client *obj)
{
    printf("MQTT Connected!\r\n");

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

    unsigned char buf[200];
    int buflen = sizeof(buf);

    int len = 0;

    data.clientID.cstring = Id;
    data.keepAliveInterval = 20;
    data.cleansession = 1;
    data.username.cstring = Username;
    data.password.cstring = Password;

    len = MQTTSerialize_connect(buf, buflen, &data);

    obj->Send((uint8_t *) buf, len);
}


void MQTT_Client::OnTcpDisconnected(TLS_Client *obj)
{
    printf("MQTT Disconnected!\r\n");
    if (Observer != nullptr)
    {
        Observer->MQTT_OnDisconnected(this);
    }
}


void MQTT_Client::TcpPollConnectionl(TLS_Client *obj)
{
    if (Observer != nullptr)
    {
        Observer->MQTT_PollConnection(this);
    }
}