#include "mcts.h"

mcts::MCTS::MCTS(int simualtions, std::vector<Poker> pokers,
                 std::vector<Poker> knownCardPool,
                 std::vector<Poker> dealerVisibleCards)
    : _simulations(simualtions),
      _knownCardPool(knownCardPool),
      _dealerVisibleCards(dealerVisibleCards) {
  unsigned int numThreads = std::thread::hardware_concurrency();
  numThreads = numThreads > 0 ? numThreads : 4;
  _threadPool = std::make_unique<ThreadPool>(numThreads);

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
      node->visits += 1;
      backpropagation(node, (node->action == Action::SURRENDER) ? -1 : 0);
    } else {
      int result = playout(node);
      backpropagation(node, result);
    }
  }

  int maxVisits = 0;
  for (const auto& child : _root->children) {
    if (child) {
      if (child->visits > maxVisits) {
        maxVisits = child->visits;
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
      if (!child) continue;
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

  if (node->action == Action::STAND) return;

  if (Poker::getPokerValue(node->pokers) > 21) return;

  for (int i = 0; i < MAX_CHILDREN; ++i) {
    if (_dealerVisibleCards.front().getNumber() != "A" &&
        Action::INSURANCE == static_cast<Action>(i))
      continue;  // insurance is not available
    if (node->action == Action::INSURANCE &&
        Action::INSURANCE == static_cast<Action>(i))
      continue;  // insurance is not available
    if (node->action == Action::INSURANCE &&
        Action::SURRENDER == static_cast<Action>(i))
      continue;  // surrender is not available
    if (node->action == Action::INSURANCE &&
        Action::DOUBLE == static_cast<Action>(i))
      continue;  // double is not available
    if (node->action == Action::HIT && Action::DOUBLE == static_cast<Action>(i))
      continue;  // double is not available
    if (node->action == Action::HIT &&
        Action::SURRENDER == static_cast<Action>(i))
      continue;  // surrender is not available
    if (node->action == Action::HIT &&
        Action::INSURANCE == static_cast<Action>(i))
      continue;  // insurance is not available
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
  std::atomic<int> totalResult{0};

  // 決定要使用的線程數量
  unsigned int numThreads = std::thread::hardware_concurrency();
  numThreads = numThreads > 0 ? numThreads : 4;  // 如果無法檢測，預設為4
  numThreads = std::min(
      numThreads, (unsigned int)PLAYOUT_TIMES);  // 不要創建比模擬次數更多的線程

  std::vector<std::thread> threads;

  // 平均分配工作給每個線程
  int playoutsPerTask = PLAYOUT_TIMES / numThreads;
  int remainingPlayouts = PLAYOUT_TIMES % numThreads;

  // 創建任務和存儲 future 來獲取結果
  std::vector<std::future<int>> results;

  auto taskFunction = [this, node](int playoutCount) {
    int taskResult = 0;

    for (int i = 0; i < playoutCount; i++) {
      // 為每次模擬建立所需資料的副本
      auto cardPoolCopy = _knownCardPool;
      auto playerPokersCopy = node->pokers;  // 複製玩家的牌，避免修改原始數據

      // 洗牌
      std::shuffle(cardPoolCopy.begin(), cardPoolCopy.end(),
                   std::default_random_engine(std::random_device()()));

      auto dealerVisibleCardsCopy = _dealerVisibleCards;

      // 模擬莊家的牌
      if (dealerVisibleCardsCopy.size() == 1 && !cardPoolCopy.empty()) {
        dealerVisibleCardsCopy.push_back(cardPoolCopy.back());
        cardPoolCopy.pop_back();
      }

      // 根據動作模擬玩家的牌
      switch (node->action) {
        case Action::HIT:
          playerPokersCopy.push_back(cardPoolCopy.back());
          cardPoolCopy.pop_back();
          break;
        case Action::STAND:
          break;
        case Action::DOUBLE:
          playerPokersCopy.push_back(cardPoolCopy.back());
          cardPoolCopy.pop_back();
          break;
        case Action::SURRENDER:
          taskResult -= 1;
          continue;  // 投降直接計算下一次模擬
        case Action::INSURANCE:
          break;
      }

      int playerScore = Poker::getPokerValue(playerPokersCopy);
      int dealerScore = Poker::getPokerValue(dealerVisibleCardsCopy);

      // 莊家策略：抽牌直到硬17點或更高，軟17需繼續抽牌 (H17規則)
      while ((dealerScore < 17 || 
       (dealerScore == 17 && 
        [&dealerVisibleCardsCopy]() {
          // 內嵌判斷是否為軟17的邏輯
          bool hasAce = false;
          int sum = 0;
          for ( auto& card : dealerVisibleCardsCopy) {
            if (card.getNumber() == "A") {
              hasAce = true;
              sum += 1; // 先將A計為1點
            } else if (card.getNumber() == "J" || card.getNumber() == "Q" || 
                      card.getNumber() == "K") {
              sum += 10;
            } else {
              sum += std::stoi(card.getNumber());
            }
          }
          // 檢查是否為軟17：有A且將一張A計為11點後總和為17
          return hasAce && (sum + 10 == 17);
        }())) 
      && !cardPoolCopy.empty()) {
        dealerVisibleCardsCopy.push_back(cardPoolCopy.back());
        cardPoolCopy.pop_back();
        dealerScore = Poker::getPokerValue(dealerVisibleCardsCopy);
      }

      int result = 0;
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

      taskResult += result;
    }
    return taskResult / playoutCount;
  };

  // 提交任務到線程池
  for (unsigned int i = 0; i < numThreads - 1; i++) {
    results.emplace_back(_threadPool->enqueue(taskFunction, playoutsPerTask));
  }
  results.emplace_back(
      _threadPool->enqueue(taskFunction, playoutsPerTask + remainingPlayouts));

  // 收集結果
  for (auto& future : results) {
    totalResult += future.get();
  }

  return totalResult / PLAYOUT_TIMES;
}