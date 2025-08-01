#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <vector>
#include <cstring>

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 2 * 1024     // 2MB of buffer

// TODO: Right now we assume T will always be 32 bits but nothing actually checks this which is bad so figure out how to enforce that

namespace Network
{
    template <typename T>
    struct MessageHeader
    {
        T message;
        uint32_t payload_size;
    };

    template <typename T>
    uint64_t pack_header(MessageHeader<T> header)
    {
        uint64_t data = ((uint64_t)htonl((uint32_t)header.message) << 32) | (uint64_t)htonl(header.payload_size);
        return htonll(data);
    }

    template <typename T>
    MessageHeader<T> unpack_header(uint64_t data)
    {
        uint64_t host_data = ntohll(data);
        MessageHeader<T> header = {};
        header.message = static_cast<T>(ntohl(host_data >> 32));
        header.payload_size = (uint32_t)ntohl((uint32_t)host_data);
        return header;
    }

    template <typename T>
    class Message
    {
        private:
            MessageHeader<T> header{};
            std::vector<uint8_t> message{};

        public:
            Message(T message_type)
            :header(message_type, 0)
            {}

            Message(T message_type, std::vector<uint8_t> data)
            :header(message_type, data.size()), message(data)
            {}

            // Accepts any POD type
            // Note, this assumes that it will be in the appropriate format for the network
            // i.e. if you want to add a float then make sure to call htonf on it before appending
            template <typename Q>
            void append(Q data)
            {
                uint32_t old_size = message.size();
                message.resize(old_size + sizeof(data));
                memcpy(&message[old_size], &data, sizeof(data));
                header.payload_size += sizeof(data);
            }

            void append(const char* str)
            {
                uint32_t len = strlen(str) + 1;
                uint32_t old_size = message.size();
                message.resize(old_size + len);
                memcpy(&message[old_size], str, len);
                header.payload_size += len;
            }

            void append(void* data, uint32_t size)
            {
                uint32_t old_size = message.size();
                message.resize(old_size + size);
                memcpy(&message[old_size], data, size);
                header.payload_size += size;
            }

            void print()
            {
                std::cout << "Vec Size: " << message.size() << std::endl;
                std::cout << "Payload Recorded Size: " << header.payload_size << std::endl;
            }

            template <typename Q>
            friend uint32_t Send(SOCKET socket, Message<Q> message);
    };

    template <typename T>
    uint32_t Send(SOCKET socket, Message<T> message)
    {
        int sent_bytes = 0;
        int total_bytes = sizeof(message.header) + message.message.size();
        uint64_t packed_header = pack_header(message.header);

        std::vector<uint8_t> sendbuf(total_bytes);
        memcpy(&sendbuf[0], &packed_header, sizeof(packed_header));

        if(message.header.payload_size > 0)
            memcpy(&sendbuf[8], message.message.data(), message.message.size());

        // TODO: Might want to use ASIO or some encryption library for safer connections
        while((sent_bytes += send(socket, (const char*)&sendbuf[0], total_bytes, 0)) < total_bytes);
        return sent_bytes;
    }
}
