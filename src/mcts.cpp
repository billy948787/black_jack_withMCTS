#include "mcts.h"

mcts::MCTS::MCTS(int simualtions, std::vector<Poker> pokers,
                 std::vector<Poker> knownCardPool,
                 std::vector<Poker> dealerVisibleCards)
    : _simulations(simualtions),
      knownCardPool(knownCardPool),
      dealerVisibleCards(dealerVisibleCards) {
  unsigned int numThreads = std::thread::hardware_concurrency();
  numThreads = numThreads > 0 ? numThreads : 4;
  _threadPool = std::make_unique<ThreadPool>(numThreads);

  root = std::make_shared<Node>();

  root->pokers = pokers;
  root->value = 0;
  root->visits = 0;
  root->action = Action::HIT;
  root->parent = nullptr;

  for (int i = 0; i < MAX_CHILDREN; ++i) {
    std::shared_ptr<Node> child = std::make_shared<Node>();
    child->parent = root;
    child->action = static_cast<Action>(i);
    child->pokers = root->pokers;
    root->children[i] = child;
  }
}

std::shared_ptr<mcts::Node> mcts::MCTS::run() {
  std::shared_ptr<Node> bestChild;

  for (int i = 0; i < _simulations; ++i) {
    auto node = selection(root);

    if (node->visits != 0) {
      expansion(node);

      bool hasChildren = false;

      for (int j = 0; j < MAX_CHILDREN; ++j) {
        if (node->children[j] == nullptr) continue;
        double result = playout(node->children[j]);
        backpropagation(node->children[j], result);
        hasChildren = true;
        break;
      }

      if (!hasChildren) {
        backpropagation(node, 0);
      }
    } else {
      double result = playout(node);
      backpropagation(node, result);
    }
  }

  int maxVisits = 0;
  for (const auto& child : root->children) {
    if (child) {
      if (child->visits > maxVisits) {
        maxVisits = child->visits;
        bestChild = child;
      }
    }
  }

  std::cout << "bestChild: " << bestChild->action << std::endl;

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

  return static_cast<double>(value) / visits +
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

  for (int i = 0; i < MAX_CHILDREN; ++i) {
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
    child->pokers = node->pokers;

    // 根據動作更新牌狀態
    if (currentAction == Action::HIT && !knownCardPool.empty()) {
      // 模擬抽牌動作
      std::vector<Poker> tempPool = knownCardPool;
      std::shuffle(tempPool.begin(), tempPool.end(),
                   std::default_random_engine(std::random_device()()));
      child->pokers.push_back(tempPool.back());
    }

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
      numThreads, (unsigned int)PLAYOUT_TIMES);  // 不要創建比模擬次數更多的線程

  std::vector<std::thread> threads;

  // 平均分配工作給每個線程
  int playoutsPerTask = PLAYOUT_TIMES / numThreads;
  int remainingPlayouts = PLAYOUT_TIMES % numThreads;

  // 創建任務和存儲 future 來獲取結果
  std::vector<std::future<double>> results;

  auto taskFunction = [this, node](int playoutCount) {
    double taskResult = 0;

    for (int i = 0; i < playoutCount; i++) {
      // 為每次模擬建立所需資料的副本
      auto cardPoolCopy = knownCardPool;
      auto playerPokersCopy = node->pokers;  // 複製玩家的牌，避免修改原始數據

      // 洗牌
      std::shuffle(cardPoolCopy.begin(), cardPoolCopy.end(),
                   std::default_random_engine(std::random_device()()));

      auto dealerVisibleCardsCopy = dealerVisibleCards;

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

      double result = 0;
      // 檢查五張牌查理 (Five Card Charlie)
      if (playerScore <= 21 && playerPokersCopy.size() == 5) {
        result = 2 * (node->action == Action::DOUBLE ? 2 : 1);
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
          result = 2 * (node->action == Action::DOUBLE ? 2 : 1);
        } else {
          // 非順子的情況，繼續正常評估
          if (playerScore > 21) {  // 玩家爆牌
            result = -1 * (node->action == Action::DOUBLE ? 2 : 1);
          } else if (dealerScore > 21) {  // 莊家爆牌
            result = 1 * (node->action == Action::DOUBLE ? 2 : 1);
          } else if (playerScore == 21 &&
                     playerPokersCopy.size() == 2) {  // 玩家黑傑克
            result =
                1.5 *
                (node->action == Action::DOUBLE ? 2 : 1);  // 黑傑克支付1.5倍
          } else if (playerScore > dealerScore) {          // 玩家點數高
            result = 1 * (node->action == Action::DOUBLE ? 2 : 1);
          } else if (playerScore < dealerScore) {  // 莊家點數高
            result = -1 * (node->action == Action::DOUBLE ? 2 : 1);
          } else {  // 平局
            // 莊家和玩家都是黑傑克時才算真正的平局
            if (dealerScore == 21 && dealerVisibleCardsCopy.size() == 2 &&
                playerScore == 21 && playerPokersCopy.size() == 2) {
              result = 0;
            } else if (dealerScore == 21 &&
                       dealerVisibleCardsCopy.size() == 2) {
              // 莊家黑傑克，玩家普通21點
              result = -1 * (node->action == Action::DOUBLE ? 2 : 1);
            } else if (playerScore == 21 && playerPokersCopy.size() == 2) {
              // 玩家黑傑克，莊家普通21點
              result = 1.5 * (node->action == Action::DOUBLE ? 2 : 1);
            } else {
              result = 0;  // 真正的平局
            }
          }
        }
      } else {
        // 正常評估
        if (playerScore > 21) {  // 玩家爆牌
          result = -1 * (node->action == Action::DOUBLE ? 2 : 1);
        } else if (dealerScore > 21) {  // 莊家爆牌
          result = 1 * (node->action == Action::DOUBLE ? 2 : 1);
        } else if (playerScore == 21 &&
                   playerPokersCopy.size() == 2) {  // 玩家黑傑克
          result = 1.5 *
                   (node->action == Action::DOUBLE ? 2 : 1);  // 黑傑克支付1.5倍
        } else if (playerScore > dealerScore) {               // 玩家點數高
          result = 1 * (node->action == Action::DOUBLE ? 2 : 1);
        } else if (playerScore < dealerScore) {  // 莊家點數高
          result = -1 * (node->action == Action::DOUBLE ? 2 : 1);
        } else {  // 平局
          // 莊家和玩家都是黑傑克時才算真正的平局
          if (dealerScore == 21 && dealerVisibleCardsCopy.size() == 2 &&
              playerScore == 21 && playerPokersCopy.size() == 2) {
            result = 0;
          } else if (dealerScore == 21 && dealerVisibleCardsCopy.size() == 2) {
            // 莊家黑傑克，玩家普通21點
            result = -1 * (node->action == Action::DOUBLE ? 2 : 1);
          } else if (playerScore == 21 && playerPokersCopy.size() == 2) {
            // 玩家黑傑克，莊家普通21點
            result = 1.5 * (node->action == Action::DOUBLE ? 2 : 1);
          } else {
            result = 0;  // 真正的平局
          }
        }
      }

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

  return totalResult / PLAYOUT_TIMES;
}