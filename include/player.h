#ifndef PLAYER_H
#define PLAYER_H
#include <vector>

#include "poker.h"
#include "operation.h"
#include "operation_controller.h"

class Player
{
private:
  std::vector<Poker> _pokers;
  int _money;
  int _bet;
  int _gainedFromLastRound;

  bool _isBanker;
  bool _doubled;
  bool _surrendered;
  bool _isOut;
  bool _hasInsurance;
  bool _isAI;


  OperationController _operationController;

  std::string _name;

  friend class Game;
  friend class ManualOperation;
  friend class AIOperation;
  friend class Dealer;

public:
  Player(std::string, Operation &);
  void switchBanker();
  int getMoney();
  void addMoney(int);
  void reduceMoney(int);
  void callBet(int);
  void clearBet();
  void winBet();
  void winBet(std::string);
  void loseBet();
  void surrender();
  void clearState();
  void doubleDown();
  void getInsurance();
  void lossInsurance();
  void out();
  std::string getName();
  void addPoker(Poker);
  void clearPoker();
  void changeOperation(Operation &);
  int getProfit();
  int getPoint();
  int getBet();
  int getTotalProfit();
  std::vector<Poker> &getPokers();
};

#endif