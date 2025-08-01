#pragma once
#include <stdio.h>
#include <vector>
#include <cstring>
#include <WinSock2.h>

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
            std::vector<uint8_t> payload{};

        public:

            Message()
            {}

            Message(T message_type)
            :header(message_type, 0)
            {}

            Message(T message_type, std::vector<uint8_t> data)
            :header(message_type, data.size()), payload(data)
            {}

            // Accepts any POD type
            // Note, this assumes that it will be in the appropriate format for the network
            // i.e. if you want to add a float then make sure to call htonf on it before appending
            template <typename Q>
            void append(Q data)
            {
                uint32_t old_size = payload.size();
                payload.resize(old_size + sizeof(data));
                memcpy(&payload[old_size], &data, sizeof(data));
                header.payload_size += sizeof(data);
            }

            void append(const char* str)
            {
                uint32_t len = strlen(str) + 1;
                uint32_t old_size = payload.size();
                payload.resize(old_size + len);
                memcpy(&payload[old_size], str, len);
                header.payload_size += len;
            }

            void append(void* data, uint32_t size)
            {
                uint32_t old_size = payload.size();
                payload.resize(old_size + size);
                memcpy(&payload[old_size], data, size);
                header.payload_size += size;
            }

            inline T get_type() const
            {
                return header.message;
            }

            void print() const
            {
                std::cout << "Vec Size: " << payload.size() << std::endl;
                std::cout << "Payload Recorded Size: " << header.payload_size << std::endl;
            }
            
            std::vector<uint8_t> serialize() const
            {
                uint64_t packed_header = pack_header(header);
                std::vector<uint8_t> buffer(sizeof(packed_header) + payload.size());
                memcpy(buffer.data(), &packed_header, sizeof(packed_header));
                memcpy(&buffer[sizeof(packed_header)], payload.data(), payload.size());
                return buffer;
            }
    };

    template<typename T>
    Message<T> NextMesage(std::vector<uint8_t>& buffer)
    {
        Message<T> message;
        // Read header from network_buffer (and update pointer into network_buffer and stuff)
        
        // Read payload_size bytes from network_buffer into vector (need checks to make sure all data is in the network_buffer... if not the maybe wait or try to get it all)
        
        // Pack into msesage
        return message;
    }

}
