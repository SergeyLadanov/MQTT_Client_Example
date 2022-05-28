
#include "main.hpp"
#include <sys/types.h>
#include <winsock.h>
#include <cstdio>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>





char* domainIP(const char *domain)
{
    static char str_result[128] = {0};
    WSADATA wdata;
    struct hostent *remoteHost;
    int result = WSAStartup(MAKEWORD(2,2), &wdata);
    remoteHost = gethostbyname(domain);
    sprintf(str_result, inet_ntoa(*( struct in_addr*)remoteHost->h_addr_list[0]));
    return str_result;
}

struct sockaddr_in server_address;


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int Test(void)
{
	int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    portno = 9090;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
	{
		error("ERROR opening socket");
	}

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(domainIP("127.0.0.1"));
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		error("ERROR connecting");
	}
    printf("Please enter the message: ");
    memset(buffer,0, 256);
    fgets(buffer,255,stdin);
    n = send(sockfd, buffer, strlen(buffer), 0);
    if (n < 0)
	{
		error("ERROR writing to socket");
	}  
    memset(buffer,0, 256);
    n = recv(sockfd, buffer, 255,0);
    if (n < 0) 
	{
		error("ERROR reading from socket");
	}
    printf("%s\n", buffer);
    close(sockfd);
    return 0;
}




// Основная программа
int main(void)
{

	WSADATA wsaData;
	int err;

    err = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (err != 0)
	{
		error((char *) "Errore in WSAStartup");
	} 


	Test();
    return 0;  
}