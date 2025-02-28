#include "dealer.h"

#include <algorithm>
#include <chrono>
#include <random>

void Dealer::shuffle(std::vector<Poker>& cardPool) {
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

  std::shuffle(cardPool.begin(), cardPool.end(),
               std::default_random_engine(seed));
}

void Dealer::deal(std::vector<Player>& players, std::vector<Poker>& cardPool) {
  for (auto& player : players) {
    player.addPoker(cardPool.back());
    cardPool.pop_back();
  }
}

void Dealer::deal(Player& banker, std::vector<Poker>& cardPool, bool needFlip) {
  // get the card
  Poker poker = cardPool.back();
  // pop the last card from pool
  cardPool.pop_back();
  if (needFlip) {
    poker.flipTheCard();
  }
  banker.addPoker(poker);
}

void Dealer::reduceCard(std::vector<Player>& players) {
  for (auto& player : players) {
    player.getPokers().clear();
  }
}