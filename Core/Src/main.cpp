
#include "main.hpp"
#include "MQTT_Client.hpp"
#include "MQTT_Publisher.hpp"


class Test_PubSub : public MQTT_Client::IObserver
{
private:
    float Temp = 20.0f;
    float Hum = 50.0f;
    char TempTopic[32];
    char HumTopic[32];
    char SubSwitcherTopic[32];
    char PubSwitcherTopic[32];
    bool SwitcherState = 0;
public:
    Test_PubSub(const char * temp_topic = nullptr, const char *hum_topic = nullptr, const char *sub_switch_topic = nullptr, const char *pub_switch_topic = nullptr)
    {
        memset(TempTopic, 0, sizeof(TempTopic));
        memset(HumTopic, 0, sizeof(HumTopic));
        memset(SubSwitcherTopic, 0, sizeof(SubSwitcherTopic));
        memset(PubSwitcherTopic, 0, sizeof(PubSwitcherTopic));

        if (temp_topic != nullptr)
        {
            snprintf(TempTopic, sizeof(TempTopic), temp_topic);
        }
        
        if (hum_topic != nullptr)
        {
            snprintf(HumTopic, sizeof(HumTopic), hum_topic);
        }

        if (sub_switch_topic != nullptr)
        {
            snprintf(SubSwitcherTopic, sizeof(SubSwitcherTopic), sub_switch_topic);
        }

        if (pub_switch_topic != nullptr)
        {
            snprintf(PubSwitcherTopic, sizeof(PubSwitcherTopic), pub_switch_topic);
        }
        
    }
private:
    void MQTT_OnConnected(MQTT_Client *obj) override
    {
        if (strlen(SubSwitcherTopic))
        {
            obj->Subscribe(SubSwitcherTopic, 1);
        }
    }

    void MQTT_OnDisconnected(MQTT_Client *obj) override
    {

    }

    void MQTT_OnReceived(MQTT_Client *obj, char *topic,  char *payload, uint16_t len) override
    {
        if (!strcmp(topic, SubSwitcherTopic))
        {
            char Repply[32];
            printf("\r\n\r\nNew state: %s\r\n\r\n", payload);
            
            if (payload[0] == '1')
            {
                SwitcherState = true;
            }

            if (payload[0] == '0')
            {
                SwitcherState = false;
            }

            if (strlen(PubSwitcherTopic))
            {
                snprintf(Repply, sizeof(Repply), "%d", SwitcherState);
                obj->Publish(PubSwitcherTopic, 1, Repply);
            }

        }
    }

    void MQTT_PollConnection(MQTT_Client *obj) override
    {
        char Buf[32];

        Hum = Hum + 1;

        if (strlen(TempTopic))
        {
            snprintf(Buf, sizeof(Buf), "%2.3f", Temp);
            obj->Publish(TempTopic, 1, Buf);
        }

        if (strlen(HumTopic))
        {
            snprintf(Buf, sizeof(Buf), "%2.3f", Hum);
            obj->Publish(HumTopic, 1, Buf);
        }
        
        if (strlen(PubSwitcherTopic))
        {
            snprintf(Buf, sizeof(Buf), "%d", SwitcherState);
            obj->Publish(PubSwitcherTopic, 1, Buf);
        }
    }
};


static TLS_Client tcp_cl1;
static MQTT_Publisher mqtt_pub;


static Test_PubSub mqtt_dht("dht_temp", "dht_hum", "sub_switch", "pub_switch");
static MQTT_Client Test("TestID");
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

    tcp_cl1.BindObserver(&mqtt_pub);

    Test.BindObserver(&mqtt_dht);
    Test.Begin(host, port, username, password);

    while (counter)
    {
        sleep(1);
        tcp_cl1.Connect((const char*) host, port);
        counter--;
    }

    sleep(1);
    
    return 0;  
}