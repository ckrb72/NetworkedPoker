#include <iostream>
#include <network/network.h>
#include <game/game.h>
#include <asio.hpp>

#include <thread>
#include <mutex>


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

    size_t len = socket.read_some(asio::buffer(buf), error);
    if(error == asio::error::eof)
    {

    }
    else if(error)
    {
        throw std::system_error(error);
    }

    std::cout.write(buf, len);

    exit(EXIT_FAILURE);
}