#pragma once
#include <game/game_common.h>
#include <vector>
#include <random>
#include <iostream>

namespace game
{
    class deck
    {
        private:
            std::vector<game::card> cards;
            uint8_t front;

        public:
            deck()
            :cards(52), front(0)
            {
                // Initialize Deck
                for(int i = 0; i < 4; i++)
                {
                    for(int j = 0; j < 13; j++)
                    {
                        cards[(i * 13) + j] = game::card{(game::rank)i, (game::suit)j};
                    }
                }

                shuffle();
            }

            void shuffle()
            {
                std::random_device dev;
                std::mt19937 rng(dev());

                // Fisher-Yates Shuffle
                for(int i = cards.size() - 1; i > 0; i--)
                {
                    std::uniform_int_distribution<std::mt19937::result_type> dist(0, i);
                    int j = dist(rng);

                    std::swap(cards[i], cards[j]);
                }
            }

            game::card next()
            {
                if(front >= cards.size())
                {
                    shuffle();
                    front = 0;
                }

                return cards[front++];
            }

    };
}
