#include "mcts.h"

mcts::MCTS::MCTS(int simualtions, std::vector<Poker> pokers,
                 std::vector<Poker> knownCardPool,
                 std::vector<Poker> dealerVisibleCards)
    : _simulations(simualtions),
      dealerVisibleCards(dealerVisibleCards),
      _rng(std::random_device{}()) {
  unsigned int numThreads = std::thread::hardware_concurrency();
  numThreads = numThreads > 0 ? numThreads : 4;
  _threadPool = std::make_unique<ThreadPool>(numThreads);

  root = std::make_shared<Node>();

  root->cardPool = knownCardPool;
  root->pokers = pokers;
  root->value = 0;
  root->visits = 0;
  root->action = Action::HIT;
  root->parent = nullptr;

  _playoutTimes = knownCardPool.size();
}

std::shared_ptr<mcts::Node> mcts::MCTS::run() {
  std::shared_ptr<Node> bestChild;

  // 初始化子節點
  for (int i = 0; i < ACTION_COUNT; ++i) {
    if (dealerVisibleCards[0].getNumber() != "A" &&
        static_cast<Action>(i) == Action::INSURANCE) {
      continue;
    }

    std::shared_ptr<Node> child = std::make_shared<Node>();
    child->parent = root;
    child->action = static_cast<Action>(i);
    child->pokers = root->pokers;
    child->cardPool = root->cardPool;
    root->children[i] = child;
  }

  for (int i = 0; i < _simulations; ++i) {
    auto node = selection(root);

    if (node->visits != 0) {
      expansion(node);

      auto child = selection(node);

      if (child == node) {
        backpropagation(node, node->value);
      } else {
        double result = playout(child);
        backpropagation(child, result);
      }
    } else {
      double result = playout(node);
      backpropagation(node, result);
    }
  }

  int maxVisits = 0;
  for (const auto& child : root->children) {
    if (child == nullptr) continue;
    if (child->visits > maxVisits) {
      maxVisits = child->visits;
      bestChild = child;
    }
  }
  return bestChild;
}

std::shared_ptr<mcts::Node> mcts::MCTS::selection(std::shared_ptr<Node> root) {
  std::shared_ptr<Node> node = root;
  while (true) {
    bool hasChildren = false;

    std::shared_ptr<Node> bestChild = nullptr;

    double maxUCBValue = std::numeric_limits<double>::lowest();
    for (const auto& child : node->children) {
      if (!child) continue;
      if (bestChild == nullptr || child->getUCBValue() > maxUCBValue) {
        bestChild = child;
        maxUCBValue = child->getUCBValue();
        hasChildren = true;
      }
    }

    if (!hasChildren) {
      return node;
    }

    node = bestChild;
  }
}

double mcts::Node::getUCBValue() const {
  if (visits == 0) return std::numeric_limits<double>::max();

  const double explorationConstant = 1.414;

  return value / visits +
         explorationConstant * sqrt(log(parent->visits) / visits);
}

void mcts::MCTS::expansion(std::shared_ptr<Node> node) {
  // 終止條件：這些動作會結束回合
  if (node->action == Action::SURRENDER || node->action == Action::DOUBLE ||
      node->action == Action::STAND)
    return;

  // 爆牌情況
  if (Poker::getPokerValue(node->pokers) > 21) return;

  // 檢查是否在遊戲初始階段（只有前兩張牌）
  bool isInitialStage = node->pokers.size() == 2;

  // 對HIT動作的特殊處理
  if (node->action == Action::HIT) {
    int standNodeIndex = 0;  // 為STAND動作預留的子節點索引

    // 建立STAND子節點
    auto standChild = std::make_shared<Node>();
    standChild->parent = node;
    standChild->action = Action::STAND;
    standChild->cardPool = node->cardPool;
    standChild->pokers = node->pokers;
    standChild->value = 0;
    standChild->visits = 0;
    node->children[standNodeIndex] = standChild;

    // 為HIT動作創建多個子節點，每個代表不同的抽牌結果
    int hitNodesCount = HIT_SIMULATION_COUNT;  // 預留一個給STAND

    // 確保卡池不為空
    if (!node->cardPool.empty()) {
      for (int i = 0; i < hitNodesCount; ++i) {
        if (i + 1 >= MAX_CHILDREN) break;  // 防止越界

        auto child = std::make_shared<Node>();
        child->parent = node;
        child->action = Action::HIT;
        child->cardPool = node->cardPool;
        child->pokers = node->pokers;
        child->value = 0;
        child->visits = 0;

        // 隨機抽一張牌
        if (!child->cardPool.empty()) {
          std::uniform_int_distribution<int> dist(0,
                                                  child->cardPool.size() - 1);
          int cardIndex = dist(_rng);
          child->pokers.push_back(child->cardPool[cardIndex]);
          child->cardPool.erase(child->cardPool.begin() + cardIndex);
        }

        node->children[i + 1] = child;  // i+1因為索引0已用於STAND
      }
    }

    return;  // 已處理完HIT情況，直接返回
  }

  // 其他動作的原始處理邏輯
  for (int i = 0; i < ACTION_COUNT; ++i) {
    Action currentAction = static_cast<Action>(i);

    // 根據遊戲階段和當前狀態檢查動作合法性

    // 保險只能在莊家首牌為A且在初始階段使用
    if (currentAction == Action::INSURANCE) {
      if (dealerVisibleCards.front().getNumber() != "A" || !isInitialStage ||
          node->action == Action::INSURANCE || node->action == Action::HIT)
        continue;
    }

    // 雙倍下注只能在初始階段使用，且不能在HIT或INSURANCE之後
    if (currentAction == Action::DOUBLE) {
      if (!isInitialStage || node->action == Action::HIT ||
          node->action == Action::INSURANCE)
        continue;
    }

    // 投降只能在初始階段使用，且不能在HIT或INSURANCE之後
    if (currentAction == Action::SURRENDER) {
      if (!isInitialStage || node->action == Action::HIT ||
          node->action == Action::INSURANCE)
        continue;
    }

    // 建立子節點
    auto child = std::make_shared<Node>();
    child->parent = node;
    child->action = currentAction;
    child->cardPool = node->cardPool;
    child->pokers = node->pokers;
    child->value = 0;
    child->visits = 0;
    node->children[i] = child;
  }
}

void mcts::MCTS::backpropagation(std::shared_ptr<Node> node, double result) {
  while (node != nullptr) {
    node->visits++;
    node->value += result;
    node = node->parent;
  }
}

double mcts::MCTS::playout(std::shared_ptr<Node> node) {
  double totalResult = 0.0;

  // 決定要使用的線程數量
  unsigned int numThreads = std::thread::hardware_concurrency();
  numThreads = numThreads > 0 ? numThreads : 4;  // 如果無法檢測，預設為4
  numThreads = std::min(
      numThreads, (unsigned int)_playoutTimes);  // 不要創建比模擬次數更多的線程

  std::vector<std::thread> threads;

  // 平均分配工作給每個線程
  int playoutsPerTask = _playoutTimes / numThreads;
  int remainingPlayouts = _playoutTimes % numThreads;

  // 創建任務和存儲 future 來獲取結果
  std::vector<std::future<double>> results;

  auto taskFunction = [this, node](int playoutCount) {
    double taskResult = 0;

    for (int i = 0; i < playoutCount; i++) {
      // 為每次模擬建立所需資料的副本
      auto cardPoolCopy = node->cardPool;
      auto playerPokersCopy = node->pokers;  // 複製玩家的牌，避免修改原始數據

      // 洗牌
      std::shuffle(cardPoolCopy.begin(), cardPoolCopy.end(), _rng);

      auto dealerVisibleCardsCopy = dealerVisibleCards;

      // 模擬莊家的牌
      if (dealerVisibleCardsCopy.size() == 1 && !cardPoolCopy.empty()) {
        dealerVisibleCardsCopy.push_back(cardPoolCopy.back());
        cardPoolCopy.pop_back();
      }

      // 根據動作模擬玩家的牌
      double result = 0;
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
          taskResult += SURRENDER_VALUE;
          continue;
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

      // 莊家有黑傑克
      if (dealerScore == 21 && dealerVisibleCardsCopy.size() == 2) {
        // 玩家有無保險
        if (node->action == Action::INSURANCE) {
          result = INSURANCE_SUCCESS_VALUE;
        } else {
          result = node->action == Action::DOUBLE ? DOUBLE_LOSE_VALUE
                                                  : NORMAL_LOSE_VALUE;
        }
      } else {
        // 檢查五張牌查理 (Five Card Charlie)
        if (playerScore <= 21 && playerPokersCopy.size() == 5) {
          result = node->action == Action::DOUBLE ? DOUBLE_WIN_VALUE
                                                  : SPECIAL_WIN_VALUE;
        }
        // 檢查順子 (6-7-8)
        else if (playerScore == 21 && playerPokersCopy.size() == 3) {
          // 檢查是否為 6-7-8 順子
          bool has6 = false, has7 = false, has8 = false;
          for (auto& poker : playerPokersCopy) {
            if (poker.getNumber() == "6")
              has6 = true;
            else if (poker.getNumber() == "7")
              has7 = true;
            else if (poker.getNumber() == "8")
              has8 = true;
          }
          if (has6 && has7 && has8) {
            result = node->action == Action::DOUBLE ? DOUBLE_WIN_VALUE
                                                    : SPECIAL_WIN_VALUE;
          } else {
            // 非順子的情況，繼續正常評估
            if (playerScore > 21) {  // 玩家爆牌
              result = node->action == Action::DOUBLE ? DOUBLE_LOSE_VALUE
                                                      : NORMAL_LOSE_VALUE;
            } else if (playerScore == 21 &&
                       playerPokersCopy.size() == 2) {  // 玩家黑傑克
              result = node->action == Action::DOUBLE ? DOUBLE_WIN_VALUE
                                                      : SPECIAL_WIN_VALUE;
            } else if (playerScore > dealerScore) {  // 玩家點數高
              result = node->action == Action::DOUBLE ? DOUBLE_WIN_VALUE
                                                      : NORMAL_WIN_VALUE;
            } else if (playerScore < dealerScore) {  // 莊家點數高
              result = node->action == Action::DOUBLE ? DOUBLE_LOSE_VALUE
                                                      : NORMAL_LOSE_VALUE;
            } else {
              result = DRAW_VALUE;
            }
          }
        } else {
          // 正常評估
          if (playerScore > 21) {  // 玩家爆牌
            result = node->action == Action::DOUBLE ? DOUBLE_LOSE_VALUE
                                                    : NORMAL_LOSE_VALUE;
          } else if (dealerScore > 21) {  // 莊家爆牌
            result = node->action == Action::DOUBLE ? DOUBLE_WIN_VALUE
                                                    : NORMAL_WIN_VALUE;
          } else if (playerScore == 21 &&
                     playerPokersCopy.size() == 2) {  // 玩家黑傑克
            result = node->action == Action::DOUBLE ? DOUBLE_WIN_VALUE
                                                    : SPECIAL_WIN_VALUE;
          } else if (playerScore > dealerScore) {  // 玩家點數高
            result = node->action == Action::DOUBLE ? DOUBLE_WIN_VALUE
                                                    : NORMAL_WIN_VALUE;
          } else if (playerScore < dealerScore) {  // 莊家點數高
            result = node->action == Action::DOUBLE ? DOUBLE_LOSE_VALUE
                                                    : NORMAL_LOSE_VALUE;
          } else {  // 平局
            result = DRAW_VALUE;
          }
        }

        if (node->action == Action::INSURANCE) {
          result -= INSURANCE_FAIL_VALUE;
        }
      }

      if (result < 0) result = 0;

      taskResult += result;
    }
    return taskResult;
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

  return totalResult / _playoutTimes;
}