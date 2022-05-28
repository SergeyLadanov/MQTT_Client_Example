
#include "main.hpp"
#include <sys/types.h>
#include <winsock.h>
#include <cstdio>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>




// void Inet_pton(int af, const char *src, void *dst) {
//     int res = inet_pton(af, src, dst);
//     if (res == 0) {
//         printf("inet_pton failed: src does not contain a character"
//             " string representing a valid network address in the specified"
//             " address family\n");
//         exit(EXIT_FAILURE);
//     }
//     if (res == -1) {
//         perror("inet_pton failed");
//         exit(EXIT_FAILURE);
//     }
// }

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
        error("ERROR opening socket");
    server = gethostbyname("127.0.0.1");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    // memcpy((char *)server->h_addr, 
    //      (char *)&serv_addr.sin_addr.s_addr,
    //      server->h_length);

	serv_addr.sin_addr.s_addr = inet_addr(domainIP("localhost"));
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    printf("Please enter the message: ");
    memset(buffer,0, 256);
    fgets(buffer,255,stdin);
    n = send(sockfd, buffer, strlen(buffer), 0);
    if (n < 0) 
         error("ERROR writing to socket");
    memset(buffer,0, 256);
    n = recv(sockfd, buffer, 255,0);
    if (n < 0) 
         error("ERROR reading from socket");
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

    // int network_socket;
	// network_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	// // specify an address for the socket
    // ZeroMemory(&server_address, sizeof(server_address));
	// server_address.sin_family = AF_INET;
	// server_address.sin_port = htons(9090);
	// // server_address.sin_addr.s_addr = inet_addr(domainIP("yandex.ru"));
	// //server_address.sin_addr.s_addr = INADDR_LOOPBACK;
	// server_address.sin_addr.s_addr = inet_addr(domainIP("localhost"));

    


    // int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
	// // check for error with the connection
	// if (connection_status == -1){
	// 	printf("There was an error making a connection to the remote socket \n\n");
	// }
    // else
    // {
    //     printf("Success connection!\n\n");
    // }
	
	// // receive data from the server
	// char server_response[256];
	// recv(network_socket, server_response, sizeof(server_response), 0);

	// // print out the server's response
	// printf("The server sent the data: %s\n", server_response);

	// int n = send(network_socket, "Test", 5, 0);
    // if (n < 0) 
    //      error("ERROR writing to socket");

	// // and then close the socket
	// closesocket(network_socket);


	Test();
    return 0;  
}