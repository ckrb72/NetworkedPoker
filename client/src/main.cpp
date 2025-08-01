#include <iostream>
#include "../../common/network.h"
#include "../../common/game.h"

#include <thread>
#include <mutex>


std::mutex netmtx;

bool done = false;

void network_main()
{
    std::cout << "Trying to do network stuff" << std::endl;
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

    const char* suits[] = { "Spades", "Clubs", "Hearts", "Diamonds" };

    int recvlen = 0;
    char recvbuf[DEFAULT_BUFLEN] = {};
    if((recvlen = recv(sock, recvbuf, DEFAULT_BUFLEN, 0)) > 0)
    {
        std::cout << recvlen << std::endl;
        uint64_t packed_header = *((uint64_t*)&recvbuf);
        Network::MessageHeader<ServerAction> header = Network::unpack_header<ServerAction>(packed_header);
        std::cout << "Message: " << (uint32_t)header.message << std::endl;
        std::cout << "Payload: " << header.payload_size << std::endl;

        switch(header.message)
        {
            case ServerAction::MESSAGE:
            {
                std::cout << "Server contacted you" << std::endl;
            }
            break;

            case ServerAction::CARD:
            {
                // Parse card
                Card card;
                card.rank = (Rank)recvbuf[8];
                card.suit = (Suit)recvbuf[9];

                // Print card
                std::cout << "You were dealt: " << (uint32_t)card.rank << " " << suits[card.suit] << std::endl;
            }
            break;

            default:
            break;
        }
    }


    closesocket(sock);
    freeaddrinfo(result);


    done = true;
}

int main()
{

    std::thread thread(network_main);

    WSADATA wsa_data;
    if(WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        std::cerr << "Failed to start winsocket" << std::endl;
        exit(EXIT_FAILURE);
    }

    while(!done)
    {

    }

    WSACleanup();
    exit(EXIT_FAILURE);
}