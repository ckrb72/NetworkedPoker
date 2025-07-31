#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <vector>
#include <cstring>

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 2 * 1024     // 2MB of buffer

// TODO: Right now we assume T will always be 32 bits but nothing actually checks this which is bad so figure out how to enforce that

template <typename T>
struct NetworkMessageHeader
{
    T message;
    uint32_t payload_size;
};

template <typename T>
uint64_t pack_header(NetworkMessageHeader<T> header)
{
    uint64_t data = ((uint64_t)htonl((uint32_t)header.message) << 32) | (uint64_t)htonl(header.payload_size);
    return data;
}

template <typename T>
NetworkMessageHeader<T> unpack_header(uint64_t data)
{
    NetworkMessageHeader<T> header = {};
    header.message = static_cast<T>(ntohl(data >> 32));
    header.payload_size = (uint32_t)ntohl((uint32_t)data);
    return header;
}

template <typename T>
class NetworkMessage
{
    private:
        NetworkMessageHeader<T> header{};
        std::vector<uint8_t> message{};

    public:
        NetworkMessage(T message_type)
        :header(message_type, 0)
        {}

        NetworkMessage(T message_type, std::vector<uint8_t> data)
        :header(message_type, data.size()), message(data)
        {}

        // Accepts any POD type
        template <typename Q>
        void append(Q data)
        {
            uint32_t old_size = message.size();
            message.resize(old_size + sizeof(data));
            memcpy(&message[old_size], &data, sizeof(data));
            header.payload_size += sizeof(data);
        }

        void print()
        {
            std::cout << "Vec Size: " << message.size() << std::endl;
            std::cout << "Payload Recorded Size: " << header.payload_size << std::endl;
        }
};