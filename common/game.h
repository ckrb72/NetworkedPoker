#pragma once
#include <cstdint>

enum Suits : uint8_t
{
    SPADES,
    CLUBS,
    HEARTS,
    DIAMONDS
};

enum Rank : uint8_t
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

enum class ServerAction : uint32_t
{
    NONE,
    MESSAGE,        // Send raw text to client
    CARD,           // Send card to client (via deal or other stuff)
    DISCONNECT      // Disconnect from client
};

enum class ClientAction : uint32_t
{
    NONE,
    CONNECT,
    DISCONNECT,
    MOVE
};