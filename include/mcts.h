#pragma once

#include <array>
#include <memory>
#include <algorithm>
#include <random>

#include "poker.h"

#define MAX_CHILDREN 5

namespace mcts {

enum Action {
  HIT,
  STAND,
  DOUBLE,
  SURRENDER,
  INSURANCE,
};

class Node {
 public:
  std::shared_ptr<Node> parent;

  std::array<std::shared_ptr<Node>, MAX_CHILDREN> children;

  int wins;

  int visits;

  std::vector<Poker> pokers;

  Action action;

  double getUCBValue() const;
};

class MCTS {
 public:
  MCTS(int simualtions,std::vector<Poker> pokers,std::vector<Poker> knownCardPool,
       std::vector<Poker> dealerVisibleCards) ;

  std::shared_ptr<Node> selection(std::shared_ptr<Node> root);

  void expansion(std::shared_ptr<Node> node);

  int playout(std::shared_ptr<Node> node);

  std::shared_ptr<Node> run();

  void backpropagation(std::shared_ptr<Node> node, int result);

 private:
  int _simulations;

  std::vector<Poker> _knownCardPool;

  std::vector<Poker> _dealerVisibleCards;

  std::shared_ptr<Node> _root;
};
}  // namespace mcts