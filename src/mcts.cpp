#include "mcts.h"

mcts::MCTS::MCTS(int simualtions, std::vector<Poker> pokers,
                 std::vector<Poker> knownCardPool,
                 std::vector<Poker> dealerVisibleCards)
    : _simulations(simualtions),
      _knownCardPool(knownCardPool),
      _dealerVisibleCards(dealerVisibleCards) {
  _root = std::make_shared<Node>();

  _root->pokers = pokers;
  _root->wins = 0;
  _root->visits = 0;

  expansion(_root);
}

std::shared_ptr<mcts::Node> mcts::MCTS::run() {
  std::shared_ptr<Node> bestChild;

  for (int i = 0; i < _simulations; ++i) {
    auto node = selection(_root);

    if (node->visits != 0) {
      expansion(node);
    } else {
      int result = playout(node);
      backpropagation(node, result);
    }
  }

  double bestWinRate = -1.0;
  for (const auto& child : _root->children) {
    if (child && child->visits > 0) {  // 確保節點有被訪問過
      double winRate = static_cast<double>(child->wins) / child->visits;
      if (bestChild == nullptr || winRate > bestWinRate) {
        bestWinRate = winRate;
        bestChild = child;
      }
    }
  }

  return bestChild;
}

std::shared_ptr<mcts::Node> mcts::MCTS::selection(std::shared_ptr<Node> root) {
  std::shared_ptr<Node> node = root;
  while (true) {
    if (node->children[0] == nullptr) {
      return node;
    }

    std::shared_ptr<Node> bestChild = nullptr;
    for (const auto& child : node->children) {
      if (bestChild == nullptr ||
          child->getUCBValue() > bestChild->getUCBValue()) {
        bestChild = child;
      }
    }

    node = bestChild;
  }
}

double mcts::Node::getUCBValue() const {
  if (visits == 0) return std::numeric_limits<double>::max();

  const double explorationConstant = 1.414;

  return static_cast<double>(wins) / visits +
         explorationConstant * sqrt(log(parent->visits) / visits);
}

void mcts::MCTS::expansion(std::shared_ptr<Node> node) {
  if (node->action == Action::SURRENDER) return;

  if (node->action == Action::DOUBLE) return;

  for (int i = 0; i < MAX_CHILDREN; ++i) {
    auto child = std::make_shared<Node>();
    child->parent = node;
    child->action = static_cast<Action>(i);
    child->pokers = node->pokers;
    node->children[i] = child;
  }
}

void mcts::MCTS::backpropagation(std::shared_ptr<Node> node, int result) {
  while (node != nullptr) {
    node->visits++;
    node->wins += result;
    node = node->parent;
  }
}

int mcts::MCTS::playout(std::shared_ptr<Node> node) {
  auto cardPoolCopy = _knownCardPool;

  // shuffle the cardPool
  std::shuffle(cardPoolCopy.begin(), cardPoolCopy.end(),
               std::default_random_engine(std::random_device()()));

  auto dealerVisibleCardsCopy = _dealerVisibleCards;

  // simulate dealer's turn
  // because we only know one card on banker's hand
  if (dealerVisibleCardsCopy.size() == 1 && !cardPoolCopy.empty()) {
    dealerVisibleCardsCopy.push_back(cardPoolCopy.back());
    cardPoolCopy.pop_back();
  }

  // simulate banker card

  int point = Poker::getPokerValue(node->pokers);

  switch (node->action) {
    case Action::HIT:
      node->pokers.push_back(_knownCardPool.back());
      cardPoolCopy.pop_back();
      break;
    case Action::STAND:
      break;
    case Action::DOUBLE:
      node->pokers.push_back(_knownCardPool.back());
      cardPoolCopy.pop_back();
      break;
    case Action::SURRENDER:
      break;
    case Action::INSURANCE:

      break;
  }

  int playerScore = Poker::getPokerValue(node->pokers);
  int dealerScore = Poker::getPokerValue(dealerVisibleCardsCopy);

  // banker need to hit until 17
  while (dealerScore < 17 && !cardPoolCopy.empty()) {
    dealerVisibleCardsCopy.push_back(cardPoolCopy.back());
    cardPoolCopy.pop_back();
    dealerScore = Poker::getPokerValue(dealerVisibleCardsCopy);
  }

  int result = 0;

  if (node->action == Action::SURRENDER) {
    result = -1;
    return result;
  }

  if (playerScore > 21 && dealerScore > 21) {
    result = 0;

  } else if (playerScore > 21) {
    result = -1 * (node->action == Action::DOUBLE ? 2 : 1);

  } else if (dealerScore > 21) {
    result = 1 * (node->action == Action::DOUBLE ? 2 : 1);

  } else if (playerScore > dealerScore) {
    result = 1 * (node->action == Action::DOUBLE ? 2 : 1);

  } else if (playerScore < dealerScore) {
    result = -1 * (node->action == Action::DOUBLE ? 2 : 1);

  } else {
    result = 0;
  }
  return result;
}