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

const double SPECIAL_WIN_VALUE = 62.5 / 75;
const double NORMAL_WIN_VALUE = 50.0 / 75;
const double NORMAL_LOSE_VALUE = 12.5 / 75;
const double DOUBLE_WIN_VALUE = 75.0 / 75;
const double DOUBLE_LOSE_VALUE = 0.0 / 75;
const double DRAW_VALUE = 37.5 / 75;
const double SURRENDER_VALUE = 18.75 / 75;
const double INSURANCE_SUCCESS_VALUE = 37.5 / 75;
const double INSURANCE_FAIL_VALUE = 12.5 / 75;

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

  int drawCount;

  std::vector<Poker> pokers;
  std::vector<Poker> cardPool;

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

  std::vector<Poker> dealerVisibleCards;

  std::shared_ptr<Node> root;

 private:
  int _simulations;

  std::unique_ptr<ThreadPool> _threadPool;

  int _playoutTimes;

  std::mt19937 _rng;
};
}  // namespace mcts