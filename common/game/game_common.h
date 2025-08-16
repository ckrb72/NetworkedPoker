#pragma once
#include <cstdint>

namespace game
{
    enum suit : uint8_t
    {
        SPADES,
        CLUBS,
        HEARTS,
        DIAMONDS
    };

    enum rank : uint8_t
    {
        ONE,
        TWO,
        THREE,
        FOUR,
        FIVE,
        SIX,
        SEVEN,
        EIGHT,
        NINE,
        TEN,
        JACK,
        QUEEN,
        KING,
        ACE
    };

    struct card
    {
        rank rank;
        suit suit;
    };

    enum class server_action : uint32_t
    {
        NONE,
        MESSAGE,        // Send raw text to client
        CARD,           // Send card to client (via deal or other stuff)
        DISCONNECT      // Disconnect from client
    };

    enum class client_action : uint32_t
    {
        NONE,
        CONNECT,
        DISCONNECT,
        MOVE
    };
}