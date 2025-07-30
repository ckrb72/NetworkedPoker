#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

// TCP connection handles important events like player actions and disconnects

// UDP connection handles things that don't matter as much like timers and such

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 256

int main()
{
    WSADATA wsa_data;
    if(WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        std::cerr << "Failed to init windows sockets" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Server" << std::endl;

    struct addrinfo* result = nullptr;
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    if(getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result) != 0)
    {
        std::cerr << "Failed to getaddrinfo" << std::endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    SOCKET listen_socket = INVALID_SOCKET;
    listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(listen_socket == INVALID_SOCKET)
    {
        std::cerr << "Failed to create socket" << std::endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    if(bind(listen_socket, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR)
    {
        std::cerr << "Failed to bind socket" << std::endl;
        closesocket(listen_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);

    if(listen(listen_socket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Failed to listen: " << WSAGetLastError() << std::endl;
        closesocket(listen_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    SOCKET client = accept(listen_socket, nullptr, nullptr);
    if(client == INVALID_SOCKET)
    {
        std::cerr << "Failed to accept: " << WSAGetLastError() << std::endl;
        closesocket(listen_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    char recvbuf[DEFAULT_BUFLEN];

    if(recv(client, recvbuf, DEFAULT_BUFLEN, 0) < 0)
    {
        std::cerr << "Failed to recv: " << WSAGetLastError() << std::endl;
        closesocket(listen_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    

    closesocket(client);

    closesocket(listen_socket);
    WSACleanup();
    return 0;
}