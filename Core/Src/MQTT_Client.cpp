#include "MQTT_Client.hpp"

#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define SUCCESS        0

bool MQTT_Client::Subscribe(char *topic)
{

}


bool MQTT_Client::Publish(char *topic, char* payload)
{

}


bool MQTT_Client::Begin(char *host, uint16_t port, char *username, char *password)
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

                printf("Conn status: %d\r\n", hdat->Tcp.IsConnected());
                
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


void MQTT_Client::OnTcpReceived(TCP_Client *obj, uint8_t *buf, uint32_t len)
{

}


void MQTT_Client::OnTcpConnected(TCP_Client *obj)
{
    printf("MQTT Connected!\r\n");
}


void MQTT_Client::OnTcpDisconnected(TCP_Client *obj)
{
    printf("MQTT Disconnected!\r\n");
}


void MQTT_Client::TcpPollConnectionl(TCP_Client *obj)
{

}