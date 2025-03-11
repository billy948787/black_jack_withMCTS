#include "player.h"

#include <typeinfo>

#include "ai_operation.h"

Player::Player(std::string name, Operation& operation)
    : _money(100000),
      _isBanker(false),
      _doubled(false),
      _surrendered(false),
      _hasInsurance(false),
      _gainedFromLastRound(0),
      _operationController(operation),
      _bet(0),
      _isOut(false),
      _name(name) {
  if (typeid(operation) == typeid(AIOperation)) {
    _isAI = true;
  } else {
    _isAI = false;
  }
}

void Player::switchBanker() { _isBanker = !_isBanker; }

void Player::getInsurance() {
  addMoney(_bet);
  _gainedFromLastRound += _bet;
}

void Player::lossInsurance() {
  reduceMoney(_bet / 2);
  _gainedFromLastRound -= _bet / 2;
}

int Player::getMoney() { return _money; }

int Player::getBet() { return _bet; }

void Player::addMoney(int money) { this->_money += money; }

void Player::doubleDown() {
  _doubled = true;
  callBet(_bet);
}

void Player::reduceMoney(int money) { this->_money -= money; }

void Player::clearState() {
  _surrendered = false;
  _hasInsurance = false;
  _gainedFromLastRound = 0;
  clearBet();
  _doubled = false;
}

void Player::out() { this->_isOut = true; }

std::string Player::getName() { return _name; }

void Player::addPoker(Poker poker) { _pokers.push_back(poker); }

void Player::clearPoker() { _pokers.clear(); }

int Player::getProfit() { return _gainedFromLastRound; }

int Player::getTotalProfit() { return _money - 100000; }

void Player::callBet(int bet) {
  this->_bet += bet;
  reduceMoney(bet);
}

void Player::clearBet() { _bet = 0; }

void Player::winBet() {
  addMoney(_bet * 2);
  _gainedFromLastRound += _bet;
  clearBet();
}

void Player::winBet(std::string type) {
  if (type == "five card charlie") {
    addMoney(_bet * 3);
    _gainedFromLastRound += _bet * 2;
  } else if (type == "shun") {
    addMoney(_bet * 3);
    _gainedFromLastRound += _bet * 2;
  } else if (type == "takeBetBack") {
    addMoney(_bet);
    _gainedFromLastRound = 0;
  } else if (type == "blackjack") {
    // 黑傑克賠率3:2，玩家獲得賭注1.5倍的獎金+返還原始賭注
    addMoney(_bet * 2.5);  // 1.5倍獎金 + 1倍原始賭注
    _gainedFromLastRound += _bet * 1.5;
  }

  clearBet();
}

void Player::loseBet() {
  _gainedFromLastRound -= _bet;
  clearBet();
}

void Player::surrender() {
  _gainedFromLastRound = -(_bet / 2);
  _surrendered = true;
  addMoney(_bet / 2);
  clearBet();
}

std::vector<Poker>& Player::getPokers() { return _pokers; }

int Player::getPoint() {
  int point = 0;
  int count = 0;
  for (auto poker : _pokers) {
    if (!poker._isFaceUp) continue;
    if (poker.getNumber() == "A") {
      count++;
      continue;
    }
    if (poker.getNumber() == "J" || poker.getNumber() == "Q" ||
        poker.getNumber() == "K") {
      point += 10;
    } else {
      point += std::stoi(poker.getNumber());
    }
  }

  while (count > 0) {
    if (point + count * 11 > 21) {
      point += 1;
    } else {
      point += 11;
    }
    count--;
  }

  return point;
}

void Player::changeOperation(Operation& operation) {
  _operationController.changeOperation(operation);
}