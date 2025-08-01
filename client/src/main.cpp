#include <iostream>
#include <network/network.h>
#include <game/game.h>

#include <thread>
#include <mutex>
#include <queue>

// TODO: Create ring buffer class / abstraction
// Ring buffer
Network::RingBuffer net_buffer(128);
std::queue<Network::Message<ClientAction>> messages_to_send;

int main()
{
    asio::io_context context;

    // Creates the endpoints for a given ip and port
    asio::ip::tcp::resolver resolver(context);
    asio::ip::tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "8080");
    
    asio::ip::tcp::socket socket(context);

    // Connect to the given endpoint with the given socket
    asio::connect(socket, endpoints);


    // This will be on it's own network thread. Game data will be shared between render and network threads
    // Every tick 
    while(true)
    {
        char buf[256] = {};
        std::error_code error;
        size_t len = socket.read_some(asio::buffer(buf), error);
        if(len > 0) net_buffer.write_bytes(buf, len);

        if(error == asio::error::eof)
        {}
        else if(error)
        {
            throw std::system_error(error);
        }

        // Get all message
        Network::Message<ServerAction> message;
        while(Network::NextMessage(net_buffer, &message))
        {
            std::cout << "Read message" << std::endl;
            // Parse Message
            switch(message.get_type())
            {
                case ServerAction::DISCONNECT:
                    std::cout << "Disconnect" << std::endl;
                break;
                case ServerAction::MESSAGE:
                    std::cout << "Message" << std::endl;
                    std::cout << message.get_payload().data() << std::endl;
                break;
                case ServerAction::CARD:
                    std::cout << "Card" << std::endl;
                break;
                default:
                    std::cout << "Got Default Message" << std::endl;
                break;
            }
        }
        // Send all pending messages
        while(!messages_to_send.empty())
        {
            Network::Message<ClientAction> message = messages_to_send.front();
            messages_to_send.pop();
            // Send message over
            /*
                asio::send(server_socket, message.serialize());
            */
        }
    //endtick
    }
        

    exit(EXIT_FAILURE);
}