#include <iostream>
#include <network/network.h>
#include <game/game.h>
#include <asio.hpp>

#include <thread>
#include <mutex>

#define NETBUFLEN 1024
#define TICKRATE 20

// TODO: Create ring buffer class / abstraction
// Ring buffer
std::vector<uint8_t> network_buffer = std::vector<uint8_t>(NETBUFLEN);

int main()
{
    asio::io_context context;

    // Creates the endpoints for a given ip and port
    asio::ip::tcp::resolver resolver(context);
    asio::ip::tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "8080");
    
    asio::ip::tcp::socket socket(context);

    // Connect to the given endpoint with the given socket
    asio::connect(socket, endpoints);

    char buf[256] = {};
    std::error_code error;


    // This will be on it's own network thread. Game data will be shared between render and network threads
    // Every tick 
        // Read all data into ring buffer
        size_t len = socket.read_some(asio::buffer(buf), error);
        if(error == asio::error::eof)
        {

        }
        else if(error)
        {
            throw std::system_error(error);
        }

    std::cout << len << std::endl;

        // Get all message
        // while(Network::GetNextMessage(network_buffer))
            // Parse Message
            /*
                switch(message.get_type())
                {
                    case ServerAction::DISCONNECT:
                    break;

                    case ServerAction::MESSAGE:
                    break;

                    case ServerAction::CARD:
                    break;

                    case ServerAction::CONNECT:
                    break;

                    default:
                    break;
                }
            */
        //endwhile
        
        // Send all pending messages
        // while(messages_to_send())
            // Send message over
        //endwhile
    //endtick

    exit(EXIT_FAILURE);
}