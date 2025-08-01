#include <iostream>
#include <game/game.h>
#include <network/network.h>
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

    Network::Message<ServerAction> server_message(ServerAction::MESSAGE);
    server_message.append("This is a message from the server");
    asio::write(socket, asio::buffer(server_message.serialize()));

    Network::Message<ServerAction> server_message2(ServerAction::MESSAGE);
    server_message2.append("This is a second message from the server");
    asio::write(socket, asio::buffer(server_message2.serialize()));

    Network::Message<ServerAction> server_message3(ServerAction::DISCONNECT);
    asio::write(socket, asio::buffer(server_message3.serialize()));

    return 0;
}