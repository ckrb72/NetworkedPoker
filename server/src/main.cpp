#include <iostream>
#include <game/game.h>
#include <network/network.h>
#include <asio.hpp>
// TCP connection handles important events like player actions and disconnects

// UDP connection handles things that don't matter as much like timers and such


int main()
{
    // Context needed per application
    asio::io_context context;

    // Acceptor accepts connections
    asio::ip::tcp::acceptor acceptor(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 8080));

    // Accept the connection and use this socket to communicate
    asio::ip::tcp::socket socket(context);
    acceptor.accept(socket);

    Network::Message<ServerAction> server_message(ServerAction::CARD);
    server_message.append<Card>(Card{Rank::ACE, Suit::CLUBS});

    asio::write(socket, asio::buffer(server_message.serialize()));

    return 0;
}