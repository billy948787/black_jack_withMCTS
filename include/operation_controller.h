#ifndef OPERATION_CONTROLLER_H
#define OPERATION_CONTROLLER_H
#include "operation.h"

class OperationController {
 public:
  OperationController(Operation&);

  std::map<std::string, bool> doubleOrSurrender(std::vector<Poker>,
                                                std::vector<Poker>,
                                                std::vector<Poker>);
  bool hit(std::vector<Poker>, std::vector<Poker>, std::vector<Poker>);
  bool insurance(std::vector<Poker>, std::vector<Poker>, std::vector<Poker>);
  int stake(int, std::vector<Poker>, std::vector<Poker>);

  void changeOperation(Operation& operation);

 private:
  Operation* operation;
};

#endif