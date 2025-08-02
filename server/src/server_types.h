#pragma once
#include <game/game.h>
#include <vector>
#include <random>
#include <iostream>

class Deck
{
    private:
        std::vector<Card> deck;
        uint8_t front;
    
    public:
        Deck()
        :deck(52), front(0)
        {
            // Initialize Deck
            for(int i = 0; i < 4; i++)
            {
                for(int j = 0; j < 13; j++)
                {
                    deck[(i * 13) + j] = Card{(Rank)i, (Suit)j};
                }
            }

            shuffle();
        }

        void shuffle()
        {
            std::random_device dev;
            std::mt19937 rng(dev());
            
            // Fisher-Yates Shuffle
            for(int i = deck.size() - 1; i > 0; i--)
            {
                std::uniform_int_distribution<std::mt19937::result_type> dist(0, i);
                int j = dist(rng);
                
                std::swap(deck[i], deck[j]);
            }
        }

        Card next()
        {
            if(front >= deck.size())
            {
                shuffle();
                front = 0;
            }
            
            return deck[front++];
        }

};