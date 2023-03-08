/*
 * testSocketService.c
 *
 *  Created on: 2012-8-16
 *      Author: 皓月繁星
 */
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#include <WINSOCK2.H>   
#include <stdio.h>     

#define PORT           5150    
#define MSGSIZE        1024    

#pragma comment(lib, "ws2_32.lib")      

int main()
{
    WSADATA wsaData;
    SOCKET sListen;
    SOCKET sClient;
    SOCKADDR_IN local;
    SOCKADDR_IN client;
    char szMessage[MSGSIZE];
    int ret;
    int iaddrSize = sizeof(SOCKADDR_IN);
    WSAStartup(0x0202, &wsaData);

    sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    local.sin_family = AF_INET;
    local.sin_port = htons(PORT);
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sListen, (struct sockaddr*)&local, sizeof(SOCKADDR_IN));

    listen(sListen, 1);


    while (TRUE) {
        sClient = accept(sListen, (struct sockaddr*)&client, &iaddrSize);
        printf("Accepted client:%s:%d\n", inet_ntoa(client.sin_addr),
            ntohs(client.sin_port));
        ret = recv(sClient, szMessage, MSGSIZE, 0);
        szMessage[ret] = '\0';
        printf("Received [%d bytes]: '%s'\n", ret, szMessage);
    }
    return 0;
}