#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <memory>
#include <random>
#include <thread>

#include "poker.h"
#include "thread_pool.h"

#define MAX_CHILDREN 5

#define PLAYOUT_TIMES 100

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

  double value;

  int visits;

  std::vector<Poker> pokers;

  Action action;

  double getUCBValue() const;
};

class MCTS {
 public:
  MCTS(int simualtions, std::vector<Poker> pokers,
       std::vector<Poker> knownCardPool, std::vector<Poker> dealerVisibleCards);

  std::shared_ptr<Node> selection(std::shared_ptr<Node> root);

  void expansion(std::shared_ptr<Node> node);

  double playout(std::shared_ptr<Node> node);

  std::shared_ptr<Node> run();

  void backpropagation(std::shared_ptr<Node> node, double result);

  std::vector<Poker> knownCardPool;

  std::vector<Poker> dealerVisibleCards;

  std::shared_ptr<Node> root;

 private:
  int _simulations;

  std::unique_ptr<ThreadPool> _threadPool;
};
}  // namespace mcts