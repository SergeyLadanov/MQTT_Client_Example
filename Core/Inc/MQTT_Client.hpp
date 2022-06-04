#ifndef __MQTT_CLIENT_HPP_
#define __MQTT_CLIENT_HPP_


#include "TCP_Client.hpp"

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
    TCP_Client Tcp;
    pthread_t KeepConnectionTask;
    bool KeepLooping = false;
    char Username[32];
    char Password[32];
    char Host[32];
    uint16_t Port = 0;
    IObserver *Observer = nullptr;
public:
    bool Subscribe(char *topic);
    bool Publish(char *topic, char* payload);
    bool Connect(char *host, uint16_t port, char *username, char *password);
    void BindObserver(IObserver *obj);
    void Stop(void);
private:
    void OnTcpReceived(TCP_Client *obj, uint8_t *buf, uint32_t len) override;
    void OnTcpConnected(TCP_Client *obj) override;
    void OnTcpDisconnected(TCP_Client *obj) override;
    void TcpPollConnectionl(TCP_Client *obj) override;
};


#endif