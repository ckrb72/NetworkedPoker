#include <iostream>
#include <game/game_common.h>
#include <network/network.h>
#include "server_types.h"

network::ring_buffer network_buffer(NETBUFLEN);

int main()
{
    game::deck deck;
    deck.shuffle();

    // Context needed per application
    asio::io_context context;

    // Acceptor accepts connections
    asio::ip::tcp::acceptor acceptor(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 8080));

    // Accept the connection and use this socket to communicate
    asio::ip::tcp::socket socket(context);
    acceptor.accept(socket);

    network::message<game::server_action> server_message(game::server_action::MESSAGE);
    server_message.append("This is a message from the server");
    asio::write(socket, asio::buffer(server_message.serialize()));

    network::message<game::server_action> server_message2(game::server_action::MESSAGE);
    server_message2.append("This is a second message from the server");
    asio::write(socket, asio::buffer(server_message2.serialize()));

    network::message<game::server_action> card_message(game::server_action::CARD);
    card_message.append(deck.next());
    asio::write(socket, asio::buffer(card_message.serialize()));

    network::message<game::server_action> server_message3(game::server_action::DISCONNECT);
    asio::write(socket, asio::buffer(server_message3.serialize()));

    return 0;
}