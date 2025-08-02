#include <iostream>
#include <game/game.h>
#include <network/network.h>
#include "server_types.h"

Network::RingBuffer network_buffer(NETBUFLEN);

int main()
{
    Deck deck;
    deck.shuffle();

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

    Network::Message<ServerAction> card_message(ServerAction::CARD);
    card_message.append(deck.next());
    asio::write(socket, asio::buffer(card_message.serialize()));

    Network::Message<ServerAction> server_message3(ServerAction::DISCONNECT);
    asio::write(socket, asio::buffer(server_message3.serialize()));

    return 0;
}