#pragma once
#include <stdio.h>
#include <vector>
#include <cstring>
#include <WinSock2.h>
#include <asio.hpp>

#define DEFAULT_PORT 8080
#define NETBUFLEN 1024
#define TICKRATE 20

// TODO: Right now we assume T will always be 32 bits but nothing actually checks this which is bad so figure out how to enforce that

namespace network
{
    template <typename T>
    struct message_header
    {
        T message;
        uint32_t payload_size;
    };

    template <typename T>
    uint64_t pack_header(message_header<T> header)
    {
        uint64_t data = ((uint64_t)htonl((uint32_t)header.message) << 32) | (uint64_t)htonl(header.payload_size);
        return htonll(data);
    }

    template <typename T>
    message_header<T> unpack_header(uint64_t data)
    {
        uint64_t host_data = ntohll(data);
        message_header<T> header = {};
        header.message = static_cast<T>(ntohl(host_data >> 32));
        header.payload_size = (uint32_t)ntohl((uint32_t)host_data);
        return header;
    }

    template <typename T>
    class message
    {
        private:
            message_header<T> header{};
            std::vector<uint8_t> payload{};

        public:

            message()
            {}

            message(T message_type)
            :header(message_type, 0)
            {}

            message(T message_type, std::vector<uint8_t> data)
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

            inline const std::vector<uint8_t>& get_payload() const
            {
                return payload;
            }

            void print() const
            {
                std::cout << "Vec Size: " << payload.size() << std::endl;
                std::cout << "Payload Recorded Size: " << header.payload_size << std::endl;
            }
            
            // FIXME: Bug when message has no payload
            std::vector<uint8_t> serialize() const
            {
                uint64_t packed_header = pack_header(header);
                std::vector<uint8_t> buffer(sizeof(packed_header) + payload.size());
                memcpy(buffer.data(), &packed_header, sizeof(packed_header));

                if(payload.size() > 0)
                    memcpy(&buffer[sizeof(packed_header)], payload.data(), payload.size());
                
                return buffer;
            }
    };

    class ring_buffer
    {
        private:
            std::vector<uint8_t> buffer;

            // Read from tail, write to head
            uint32_t head, tail;
            uint32_t count;
        
        public:
            ring_buffer(uint32_t size)
            :buffer(size), head(0), tail(0), count(0)
            {}

            ring_buffer()
            :buffer(1024), head(0), tail(0)
            {}

            uint32_t read_bytes(void* container, uint32_t size)
            {
                uint32_t read_bytes = 0;

                // If we request more bytes than are in the buffer read nothing
                if((int32_t)(count - size) < 0) return 0;
                
                memcpy(container, &buffer[tail], size);
                tail = (tail + size) % buffer.size();
                read_bytes = size;
                count -= size;

                return read_bytes;
            }

            uint32_t write_bytes(void* data, uint32_t size)
            {
                uint32_t written_bytes = 0;

                // Don't place the bytes if there isn't enough space
                if(count + size > buffer.size()) return 0;

                memcpy(&buffer[head], data, size);
                head = (head + size) % buffer.size();
                written_bytes = size;
                count += size;

                return written_bytes;
            }

            inline uint32_t get_count() const
            {
                return count;
            }

            void print()
            {
                std::cout << "Size: " << buffer.size() << std::endl;\
                std::cout << "Count: " << count << std::endl;
                std::cout << "Head: " << head << std::endl;
                std::cout << "Tail: " << tail << std::endl;
                for(int i = 0; i < buffer.size(); i++)
                {
                    std::cout << "| " << (uint32_t)buffer[i] << " | ";
                }
                std::cout << std::endl;
            }
    };

    // Get next message from ring buffer
    // returns true if message was recieved, false otherwise
    template<typename T>
    bool next_message(ring_buffer& buffer, message<T>* message)
    {
        if(buffer.get_count() <= 0) return false;

        // Read header from network_buffer (and update pointer into network_buffer and stuff)
        uint64_t packed_header;
        if(buffer.read_bytes(&packed_header, sizeof(uint64_t)) == 0)
        {
            // Handle errors
            return false;
        }
        message_header header = unpack_header<T>(packed_header);
        
        // Read payload_size bytes from network_buffer into vector (need checks to make sure all data is in the network_buffer... if not the maybe wait or try to get it all)
        std::vector<uint8_t> payload(header.payload_size);

        if(header.payload_size > 0)
        {
            if(buffer.read_bytes(payload.data(), header.payload_size) == 0)
            {
                return false;
            }
        }
        
        // Pack into msesage
        *message = network::message<T>(header.message, payload);

        return true;
    }

}
