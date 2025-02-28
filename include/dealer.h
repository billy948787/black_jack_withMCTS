#ifndef DEALER_H
#define DEALER_H
#include <iostream>
#include <vector>

#include "player.h"
#include "poker.h"

class Dealer {
 public:
  static void shuffle(std::vector<Poker>&);
  static void deal(std::vector<Player>&, std::vector<Poker>&);
  static void deal(Player&, std::vector<Poker>&, bool);
  static void reduceCard(std::vector<Player>&);
};

#endif