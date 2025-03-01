#include "game.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

#include "default_operation.h"
#define DEFAULT "\033[0;1m"
#define REDBACKGROUND "\033[41;1m"
#define GREENBACKGROUND "\033[42;1m"

const int sleepTime = 1000;

// make it singleton
Game *Game::_instance = nullptr;
Game &Game::getInstance() {
  if (_instance == nullptr) {
    _instance = new Game();
  }
  return *_instance;
}
// constructor
Game::Game() : _banker(nullptr), _leastBet(1000), _isRunning(true) {}

// game start
void Game::start(bool isTestMode) {
  if (isTestMode) {
    int wins = 0;
    int losses = 0;
    int draws = 0;
    int totalGames = 1000;
    int totalProfit = 0;

    Player defaultPlayer("Default", *new DefaultOperation());
    Player defaultPlayer2("Default2", *new DefaultOperation());
    Player aiPlayer("AI", *new AIOperation());

    _players.push_back(defaultPlayer);
    _players.push_back(aiPlayer);
    _players.push_back(defaultPlayer2);

    std::cout << "testing mcts..." << std::endl;
    std::cout << "[                                                  ] 0/"
              << totalGames << " (0%)" << std::endl;

    std::streambuf *originalCoutBuffer = std::cout.rdbuf();

    std::ofstream nullStream;
    // nullStream.open("/dev/null");  // Unix系統
    // 在Windows系統上用這個:
    nullStream.open("NUL");

    // 重定向標準輸出到空設備
    std::cout.rdbuf(nullStream.rdbuf());

    for (int i = 0; i < totalGames; i++) {
      if(_players[0].getMoney() < 100000){
        _players[0].addMoney(99999999);
      }

      if (i % 10 == 0 || i == totalGames - 1) {
        // 計算進度百分比
        int percentage = (i * 100) / totalGames;

        // 恢復標準輸出以顯示進度
        std::cout.rdbuf(originalCoutBuffer);

        // 繪製進度條
        std::cout << "\r[";
        for (int j = 0; j < 50; j++) {
          if (j < percentage / 2)
            std::cout << "=";
          else if (j == percentage / 2)
            std::cout << ">";
          else
            std::cout << " ";
        }
        std::cout << "] " << i << "/" << totalGames << " (" << percentage
                  << "%)     ";
        std::cout.flush();  // 確保立即顯示

        // 重新禁用輸出
        std::cout.rdbuf(nullStream.rdbuf());
      }

      _init();

      Dealer::shuffle(_cardPool);
      Dealer::deal(_players, _cardPool);
      Dealer::deal(_players, _cardPool);
      _banker->getPokers()[1].flipTheCard();
      _askForStake();
      _askForDoubleOrSurrender();
      _askInsuranceForAllPlayers();
      _drawForAllPlayers();
      _drawForBanker();
      _settle();
      Dealer::reduceCard(_players);

      aiPlayer = _players[1];
      defaultPlayer2 = _players[2];

      totalProfit += aiPlayer.getProfit();


      if (aiPlayer.getProfit() > defaultPlayer2.getProfit()) {
        wins++;
      } else if (aiPlayer.getProfit() < defaultPlayer2.getProfit()) {
        losses++;
      } else {
        draws++;
      }
    }

    // 恢復標準輸出
    std::cout.rdbuf(originalCoutBuffer);

    totalGames -= draws;

    // show the result
    std::cout << "\nMCTS winrate: " << (double)wins / totalGames * 100 << "%"
              << std::endl;

    std::cout << "profit: " << totalProfit << std::endl;
    return;
  }

  // normal game start
  std::cout << "Game start!"
            << "\n";
  // input the player count
  _inputPlayerCount();
  // input the round count
  _inputRoundCount();

  std::cout << "What is your name?"
            << "\n";
  std::string name;
  std::cin >> name;
  // create player
  _players.push_back(Player(name, *new ManualOperation()));

  for (int i = 1; i < _playerCount; i++) {
    _players.push_back(
        Player("Player" + std::to_string(i + 1) + "(AI)", *new AIOperation()));
  }

  _currentRound = 0;

  // game start
  while (_rounds-- > 0 && _isRunning) {
    std::cout << "Round " << ++_currentRound << " start!"
              << "\n";
    // init every round
    _init();
    // tell the player the banker
    std::cout << REDBACKGROUND << "*** The banker is " << _banker->getName()
              << " ***" << DEFAULT << "\n";
    // ask every player to stake
    _askForStake();
    //  shuffle the card
    Dealer::shuffle(_cardPool);
    std::cout << "Shuffling the card"
              << "\n";
    // deal the card to the players include banker
    Dealer::deal(_players, _cardPool);
    Dealer::deal(_players, _cardPool);
    // fold the banker's second card
    _banker->getPokers()[1].flipTheCard();
    // show all card's to the player
    _showAllCard();

    // ask every player to double surrender or do nothing
    _askForDoubleOrSurrender();

    // ask every player to take insurance or not
    _askInsuranceForAllPlayers();

    // ask every player to draw card
    _drawForAllPlayers();

    // ask the banker to draw card
    _drawForBanker();

    // settle the game
    _settle();

    // reduce the card
    Dealer::reduceCard(_players);
    // print the leaderboard
    _printLeaderboard();
    // kick out the player who can't afford the least bet or has no money
    _kickOut();
  }

  std::cout << "Game end!"
            << "\n";

  _printFinalLeaderboard();
}

void Game::_inputPlayerCount() {
  std::string input;
  std::cout << "How many players?(2-4)"
            << "\n";

  while (true) {
    std::cin >> input;
    bool isNumber = true;
    for (auto element : input) {
      if (element > 57 || element < 48) {
        std::cout << "Please enter valid number!\n";
        isNumber = false;
        break;
      }
    }

    if (isNumber) {
      if (std::stoi(input) < 2 || std::stoi(input) > 4) {
        std::cout << "Please enter valid number!\n";
        continue;
      } else {
        _playerCount = std::stoi(input);

        break;
      }
    }
  }
}

void Game::_inputRoundCount() {
  std::string input;
  std::cout << "How many games do you want to play?"
            << "\n";

  while (true) {
    std::cin >> input;
    bool isNumber = true;
    for (auto element : input) {
      if (element > 57 || element < 48) {
        std::cout << "Please enter valid number!\n";
        isNumber = false;
        break;
      }
    }

    if (isNumber) break;
  }
  _rounds = std::stoi(input);
}

// print the leaderboard
void Game::_printLeaderboard() {
  _updateLeaderboard();
  // change the color
  std::cout << GREENBACKGROUND << "The result is:" << DEFAULT << "\n";

  std::cout << _banker->getName() << "(banker)"
            << "have : " << _banker->getMoney() << "dollars!\n"
            << ((_banker->getProfit() > 0) ? GREENBACKGROUND : REDBACKGROUND)
            << "(" << ((_banker->getProfit() > 0) ? "+" : "")
            << _banker->getProfit() << ")" << DEFAULT << "\n";
  for (auto player : _players) {
    if (player._isBanker) continue;
    if (player._isOut) continue;

    std::cout << player.getName() << "have : " << player.getMoney()
              << "dollars!\n"
              << ((player.getProfit() > 0) ? GREENBACKGROUND : REDBACKGROUND)
              << "(" << ((player.getProfit() > 0) ? "+" : "")
              << player.getProfit() << ")" << DEFAULT << "\n";
  }

  std::cout << GREENBACKGROUND << "The leaderboard is:" << DEFAULT << "\n";
  int i = 1;
  for (auto player : _leaderboard) {
    std::cout << i++ << ":\n"
              << player.getName() << " Money: " << player.getMoney() << "\n";
  }
}
// get the player list and sort it with the money then update the leaderboard
void Game::_updateLeaderboard() {
  _leaderboard = _players;
  // sort players by money
  std::sort(_leaderboard.begin(), _leaderboard.end(),
            [](Player a, Player b) { return a.getMoney() > b.getMoney(); });
}

void Game::_printFinalLeaderboard() {
  _updateLeaderboard();
  std::cout << GREENBACKGROUND << "The final leaderboard is:" << DEFAULT
            << "\n";
  int i = 1;
  for (auto player : _leaderboard) {
    std::cout << i++ << ":\n"
              << player.getName() << (player._isOut ? "(out)" : "")
              << " Money: " << player.getMoney()
              << ((player.getTotalProfit() > 0) ? GREENBACKGROUND
                                                : REDBACKGROUND)
              << "("
              << (player.getTotalProfit() > 0
                      ? "+" + std::to_string(player.getTotalProfit())
                      : std::to_string(player.getTotalProfit()))
              << ")" << DEFAULT << "\n";
  }
}

void Game::_printAction(std::string action, bool isAI) {
  isAI ? std::cout << " has chosen to " << action << "\n" : std::cout << "";
}

void Game::_initCardPool() {
  // clear the pool first
  if (_cardPool.size() < 52)
    _cardPool.clear();
  else
    return;
  // add the cards to the pool
  for (int deck = 0; deck < 4; deck++) {
    for (int i = 0; i < 4; i++) {
      for (int j = 1; j <= 13; j++) {
        Poker poker;
        Suit suit;
        switch (i) {
          case 0:
            suit = spade;
            if (j == 1) {
              poker.setNumber("A");
              poker.setSuit(suit);
              _cardPool.push_back(poker);
              break;
            }
            if (j == 11) {
              poker.setNumber("J");
              poker.setSuit(suit);
              _cardPool.push_back(poker);
              break;
            }
            if (j == 12) {
              poker.setNumber("Q");
              poker.setSuit(suit);
              _cardPool.push_back(poker);
              break;
            }
            if (j == 13) {
              poker.setNumber("K");
              poker.setSuit(suit);
              _cardPool.push_back(poker);
              break;
            }

            poker.setNumber(std::to_string(j));
            poker.setSuit(suit);
            _cardPool.push_back(poker);
            break;

          case 1:
            suit = heart;
            if (j == 1) {
              poker.setNumber("A");
              poker.setSuit(suit);
              _cardPool.push_back(poker);
              break;
            }
            if (j == 11) {
              poker.setNumber("J");
              poker.setSuit(suit);
              _cardPool.push_back(poker);
              break;
            }
            if (j == 12) {
              poker.setNumber("Q");
              poker.setSuit(suit);
              _cardPool.push_back(poker);
              break;
            }
            if (j == 13) {
              poker.setNumber("K");
              poker.setSuit(suit);
              _cardPool.push_back(poker);
              break;
            }

            poker.setNumber(std::to_string(j));
            poker.setSuit(suit);
            _cardPool.push_back(poker);
            break;

          case 2:
            suit = diamond;
            if (j == 1) {
              poker.setNumber("A");
              poker.setSuit(suit);
              _cardPool.push_back(poker);
              break;
            }
            if (j == 11) {
              poker.setNumber("J");
              poker.setSuit(suit);
              _cardPool.push_back(poker);
              break;
            }
            if (j == 12) {
              poker.setNumber("Q");
              poker.setSuit(suit);
              _cardPool.push_back(poker);
              break;
            }
            if (j == 13) {
              poker.setNumber("K");
              poker.setSuit(suit);
              _cardPool.push_back(poker);
              break;
            }

            poker.setNumber(std::to_string(j));
            poker.setSuit(suit);
            _cardPool.push_back(poker);
            break;
          case 3:
            suit = club;
            if (j == 1) {
              poker.setNumber("A");
              poker.setSuit(suit);
              _cardPool.push_back(poker);
              break;
            }
            if (j == 11) {
              poker.setNumber("J");
              poker.setSuit(suit);
              _cardPool.push_back(poker);
              break;
            }
            if (j == 12) {
              poker.setNumber("Q");
              poker.setSuit(suit);
              _cardPool.push_back(poker);
              break;
            }
            if (j == 13) {
              poker.setNumber("K");
              poker.setSuit(suit);
              _cardPool.push_back(poker);
              break;
            }

            poker.setNumber(std::to_string(j));
            poker.setSuit(suit);
            _cardPool.push_back(poker);
            break;
        }
      }
    }
  }
}

void Game::_askForStake() {
  for (auto &player : _players) {
    if (player._isBanker) continue;
    if (player._isOut) continue;

    std::cout << player.getName() << " : ";

    int stake = player._operationController.stake(
        player.getMoney(), _banker->getPokers(), _cardPool);

    _printAction("stake " + std::to_string(stake), player._isAI);

    player.callBet(stake);
  }
}

void Game::_decideTheBanker() {
  int highest = -1e9;
  int highestPlayers = 1;

  if (!_players[0]._isBanker) {
    _banker = &_players[0];
    _banker->switchBanker();
    return;
  }

  //clear the banker first 
  if (_banker != nullptr) {
    _banker->switchBanker();
    _banker = nullptr;
  }

  // the first round
  if (_currentRound == 1) {
    srand(time(NULL));
    _banker = &_players[rand() % _players.size()];
    _banker->switchBanker();
    return;
  }

  for (auto &player : _players) {
    if (player._isOut) continue;
    if (player.getProfit() > highest) {
      highestPlayers = 1;
      highest = player.getProfit();
      _banker = &player;
      continue;
    }
    if (player.getProfit() == highest) {
      highestPlayers++;
    }
  }
  // the highest(profit from last round) player > 1
  if (highestPlayers > 1) {
    int lowestMoney = 1e9;
    for (auto &player : _players) {
      if (player.getProfit() == highest) {
        if (player.getMoney() < lowestMoney) {
          lowestMoney = player.getMoney();
          _banker = &player;
        }
      }
    }
  }

  _banker->switchBanker();
}

void Game::_init() {
  _initCardPool();
  _decideTheBanker();
  // clear the player's state
  for (auto &player : _players) {
    player.clearState();
  }
}

void Game::_showAllCard() {
  std::cout << _banker->getName() << "(banker) "
            << "points : " << _banker->getPoint() << "\n";

  Poker::printPokers(_banker->getPokers());

  for (auto &player : _players) {
    if (player._isBanker) continue;
    if (player._isOut) continue;
    std::cout << player.getName() << " points : " << player.getPoint() << "\n";
    Poker::printPokers(player.getPokers());
  }
}

void Game::_askInsuranceForAllPlayers() {
  if (_banker->getPokers()[0].getNumber() != "A") return;
  for (auto &player : _players) {
    if (player._isBanker) continue;
    if (player._surrendered) continue;
    if (player._isOut) continue;

    std::cout << player.getName() << " : ";

    if (_banker->getPokers()[0].getNumber() == "A") {
      std::vector<Poker> dealerVisibleCards = {_banker->getPokers().front()};
      std::vector<Poker> knownCardPool = _cardPool;
      knownCardPool.push_back(_banker->getPokers()[1]);

      bool takeInsurance = player._operationController.insurance(
          player.getPokers(), dealerVisibleCards, knownCardPool);
      if (takeInsurance) {
        player._hasInsurance = true;

        _printAction("take the insurance", player._isAI);
      } else {
        _printAction("not take the insurance", player._isAI);
      }
    }
  }
}

void Game::_askForDoubleOrSurrender() {
  for (auto &player : _players) {
    if (player._isBanker) continue;
    if (player._isOut) continue;

    std::cout << player.getName() << " : ";

    std::vector<Poker> dealerVisibleCards = {_banker->getPokers().front()};
    std::vector<Poker> knownCardPool = _cardPool;
    knownCardPool.push_back(_banker->getPokers()[1]);

    std::map<std::string, bool> result =
        player._operationController.doubleOrSurrender(
            player.getPokers(), dealerVisibleCards, knownCardPool);

    if (result["double"]) {
      _printAction("double down", player._isAI);

      player._doubled = true;
    } else if (result["surrender"]) {
      player._surrendered = true;

      _printAction("surrender", player._isAI);
    } else {
      _printAction("do nothing", player._isAI);
    }
  }
}

void Game::_drawForAllPlayers() {
  for (auto &player : _players) {
    if (player._isBanker) continue;
    if (player._isOut) continue;

    while (true) {
      if (player._surrendered) break;

      if (player._doubled) {
        player.doubleDown();

        Dealer::deal(player, _cardPool, false);

        std::cout << player.getName() << " :  has got these cards now:\n\n";

        std::cout << "Point : " << player.getPoint() << "\n";
        Poker::printPokers(player.getPokers());

        if (player.getPoint() == 21) {
          std::cout << player.getName() << " :  has reached 21 points\n";
          break;
        }

        if (player.getPoint() > 21) {
          std::cout << player.getName() << " :  has busted.\n";
          break;
        }

        break;
      }

      std::cout << player.getName() << " : ";
      std::vector<Poker> dealerVisibleCards = {_banker->getPokers().front()};
      std::vector<Poker> knownCardPool = _cardPool;
      knownCardPool.push_back(_banker->getPokers()[1]);

      bool toHit = player._operationController.hit(
          player.getPokers(), dealerVisibleCards, knownCardPool);

      if (toHit) {
        Dealer::deal(player, _cardPool, false);

        std::cout << " has got these cards now:\n\n";
        std::cout << "Point : " << player.getPoint() << "\n";
        Poker::printPokers(player.getPokers());

        if (player.getPoint() == 21) {
          std::cout << player.getName() << " :  has reached 21 points\n";
          break;
        }

        if (player.getPoint() > 21) {
          std::cout << player.getName() << " :  has busted.\n";
          break;
        }
      } else {
        _printAction("not hit", player._isAI);
        break;
      }
    }
  }
}

void Game::_drawForBanker() {
  // 翻開莊家的第二張牌
  _banker->getPokers()[1].flipTheCard();

  // 顯示莊家當前牌
  std::cout << _banker->getName() << "(banker)"
            << " : has got these cards now:\n\n";
  std::cout << "Point : " << _banker->getPoint() << "\n";
  Poker::printPokers(_banker->getPokers());

  // 莊家按H17規則抽牌：小於17點必須抽牌，軟17點也必須抽牌
  while ((_banker->getPoint() < 17) ||
         (_banker->getPoint() == 17 && _isSoft17(_banker->getPokers()))) {
    // 抽一張牌
    Dealer::deal(*_banker, _cardPool, false);

    // 顯示莊家當前牌
    std::cout << _banker->getName() << "(banker)"
              << " : has got these cards now:\n\n";
    std::cout << "Point : " << _banker->getPoint() << "\n";
    Poker::printPokers(_banker->getPokers());

    // 如果超過21點，顯示爆牌並結束
    if (_banker->getPoint() > 21) {
      std::cout << _banker->getName() << "(banker)"
                << " : has busted.\n";
      return;
    }
  }

  // 莊家已達到17點或以上（且不是軟17），停止抽牌
  std::cout << _banker->getName() << "(banker)"
            << " : stands with " << _banker->getPoint() << " points.\n";
}

// 判斷是否為軟17點 (有A且A算作11時總點數為17)
bool Game::_isSoft17(std::vector<Poker> &cards) {
  bool hasAce = false;
  int sum = 0;

  // 先計算所有牌的點數，Ace算作1點
  for (auto &card : cards) {
    if (card.getNumber() == "A") {
      hasAce = true;
      sum += 1;
    } else if (card.getNumber() == "J" || card.getNumber() == "Q" ||
               card.getNumber() == "K") {
      sum += 10;
    } else {
      sum += std::stoi(card.getNumber());
    }
  }

  // 如果有Ace且將其中一張Ace算作11點後總點數為17，則為軟17
  return hasAce && (sum + 10 == 17);
}

void Game::_settle() {
  for (auto &player : _players) {
    int point = player.getPoint();

    if (player._isBanker) continue;
    if (player._isOut) continue;

    if (player._hasInsurance) {
      if (_banker->getPokers().size() == 2 && _banker->getPoint() == 21) {
        _banker->reduceMoney(player.getBet());
        _banker->_gainedFromLastRound -= player.getBet();
        player.getInsurance();
      } else {
        _banker->addMoney(player.getBet() / 2);
        _banker->_gainedFromLastRound += player.getBet() / 2;
        player.lossInsurance();
      }
    }
    if (player._surrendered) {
      _banker->_gainedFromLastRound += player.getBet() / 2;
      _banker->addMoney(player.getBet() / 2);
      player.surrender();
      continue;
    }

    // check if the player has five card charlie
    if (point <= 21 && player.getPokers().size() == 5) {
      _banker->reduceMoney(player.getBet() * 2);
      _banker->_gainedFromLastRound -= player.getBet() * 2;
      player.winBet("five card charlie");
      continue;
    }
    // check if the player has shun
    if (point == 21 && player.getPokers().size() == 3) {
      std::map<int, bool> numberMap;
      for (auto poker : player.getPokers()) {
        if (poker.getNumber() == "6") {
          numberMap[6] = true;
        }
        if (poker.getNumber() == "7") {
          numberMap[7] = true;
        }
        if (poker.getNumber() == "8") {
          numberMap[8] = true;
        }
      }
      if (numberMap[6] && numberMap[7] && numberMap[8]) {
        _banker->reduceMoney(player.getBet() * 2);
        _banker->_gainedFromLastRound -= player.getBet() * 2;
        player.winBet("shun");
        continue;
      }
    }

    // not the special case
    if (_banker->getPoint() > 21) {
      if (player.getPoint() <= 21) {
        _banker->reduceMoney(player.getBet());
        _banker->_gainedFromLastRound -= player.getBet();
        player.winBet();
      } else {
        _banker->addMoney(player.getBet());
        _banker->_gainedFromLastRound += player.getBet();
        player.loseBet();
      }
      continue;
    }

    if (_banker->getPoint() <= 21) {
      // 玩家爆牌
      if (player.getPoint() > 21) {
        _banker->addMoney(player.getBet());
        _banker->_gainedFromLastRound += player.getBet();
        player.loseBet();
        continue;
      }

      // 判斷玩家是否有黑傑克 (Ace + 10值牌)
      bool playerHasBlackjack =
          (player.getPokers().size() == 2 && player.getPoint() == 21);
      bool bankerHasBlackjack =
          (_banker->getPokers().size() == 2 && _banker->getPoint() == 21);

      // 比較點數
      if (playerHasBlackjack && !bankerHasBlackjack) {
        // 玩家有黑傑克，莊家沒有
        _banker->reduceMoney(player.getBet() * 1.5);
        _banker->_gainedFromLastRound -= player.getBet() * 1.5;
        player.winBet("blackjack");
      } else if (player.getPoint() > _banker->getPoint()) {
        // 玩家點數大於莊家
        _banker->reduceMoney(player.getBet());
        _banker->_gainedFromLastRound -= player.getBet();
        player.winBet();
      } else if (player.getPoint() < _banker->getPoint()) {
        // 玩家點數小於莊家
        _banker->addMoney(player.getBet());
        _banker->_gainedFromLastRound += player.getBet();
        player.loseBet();
      } else {
        // 點數相同，處理推局
        player.winBet("takeBetBack");
      }
    }
  }
}
int Game::getLeasetBet() { return _leastBet; }

void Game::_kickOut() {
  for (int i = 0; i < _players.size(); i++) {
    // check if the player is out of the game or not
    if (_players[i]._isOut) continue;
    if (_players[i].getMoney() < _leastBet) {
      std::cout << _players[i].getName()
                << " has been kicked out!(can't afford the least bet)\n";
      _players[i].out();
    } else if (_players[i].getMoney() == 0) {
      std::cout << _players[i].getName() << " has been kicked out!(no money)\n";
      _players[i].out();
    }
  }

  int outCount = 0;

  for (auto player : _players) {
    if (player._isOut) outCount++;
  }

  if (outCount == _players.size() - 1) {
    _isRunning = false;
  }
}