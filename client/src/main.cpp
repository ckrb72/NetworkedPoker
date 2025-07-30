#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "../../common/network.h"

int main()
{
    WSADATA wsa_data;
    if(WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        std::cerr << "Failed to start winsocket" << std::endl;
        exit(EXIT_FAILURE);
    }

    struct addrinfo* result = nullptr;
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    const char* addr = "127.0.0.1";

    if(getaddrinfo(addr, DEFAULT_PORT, &hints, &result))
    {
        std::cerr << "Failed to getaddrinfo: " << WSAGetLastError() << std::endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    SOCKET sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(sock == INVALID_SOCKET)
    {
        std::cerr << "Failed to create socket" << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    if(connect(sock, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR)
    {
        std::cerr << "Failed to connect" << std::endl;
        closesocket(sock);
        freeaddrinfo(result);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    const char* sendbuf = "this is a test";

    if(send(sock, sendbuf, strlen(sendbuf) + 1, 0) == SOCKET_ERROR)
    {
        std::cerr << "Failed to send" << std::endl;
        closesocket(sock);
        freeaddrinfo(result);
        WSACleanup();
        exit(EXIT_FAILURE);
    }


    closesocket(sock);
    freeaddrinfo(result);
    WSACleanup();
    exit(EXIT_FAILURE);
}