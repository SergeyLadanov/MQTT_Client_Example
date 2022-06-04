#ifndef __MQTT_CLIENT_HPP_
#define __MQTT_CLIENT_HPP_


#include "TCP_Client.hpp"
#include <pthread.h>

class MQTT_Client : public TCP_Client::IObserver
{
public:
    class IObserver
    {
    public:
        virtual void MQTT_OnConnected(MQTT_Client *obj) = 0;
        virtual void MQTT_OnDisconnected(MQTT_Client *obj) = 0;
        virtual void MQTT_OnReceived(MQTT_Client *obj, char *topic,  char *payload, uint16_t len) = 0;
        virtual void MQTT_PollConnection(MQTT_Client *obj) = 0;
    };
private:

    struct DataStruct
    {
        TCP_Client Tcp;
        bool KeepLooping = false;
        char Host[32];
        uint16_t Port = 0;
        pthread_mutex_t Mutex;
    };

    char Username[32];
    char Password[32];
    pthread_t KeepConnectionTask;
    DataStruct Data;
    IObserver *Observer = nullptr;
public:
    MQTT_Client()
    {
        Data.Tcp.BindObserver(this);
    }
    bool Subscribe(char *topic);
    bool Publish(char *topic, char* payload);
    bool Begin(char *host, uint16_t port, char *username, char *password);
    void BindObserver(IObserver *obj);
    void Stop(void);
private:
    void OnTcpReceived(TCP_Client *obj, uint8_t *buf, uint32_t len) override;
    void OnTcpConnected(TCP_Client *obj) override;
    void OnTcpDisconnected(TCP_Client *obj) override;
    void TcpPollConnectionl(TCP_Client *obj) override;
};


#endif