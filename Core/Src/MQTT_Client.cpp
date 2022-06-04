#include "MQTT_Client.hpp"

bool MQTT_Client::Subscribe(char *topic)
{

}


bool MQTT_Client::Publish(char *topic, char* payload)
{

}


bool MQTT_Client::Connect(char *host, uint16_t port, char *username, char *password)
{
    
}


void MQTT_Client::BindObserver(IObserver *obj)
{

}


void MQTT_Client::Stop(void)
{

}


void MQTT_Client::OnTcpReceived(TCP_Client *obj, uint8_t *buf, uint32_t len)
{

}


void MQTT_Client::OnTcpConnected(TCP_Client *obj)
{

}


void MQTT_Client::OnTcpDisconnected(TCP_Client *obj)
{

}


void MQTT_Client::TcpPollConnectionl(TCP_Client *obj)
{

}