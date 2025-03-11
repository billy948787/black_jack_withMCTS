#ifndef AI_OPERATION_H
#define AI_OPERATION_H
#include <memory>

#include "mcts.h"
#include "operation.h"
class AIOperation : public Operation {
 public:
  std::map<std::string, bool> doubleOrSurrender(std::vector<Poker>,
                                                std::vector<Poker>,
                                                std::vector<Poker>) override;
  bool hit(std::vector<Poker>, std::vector<Poker>, std::vector<Poker>) override;
  bool insurance(std::vector<Poker>, std::vector<Poker>,
                 std::vector<Poker>) override;
  int stake(int, std::vector<Poker>, std::vector<Poker>) override;

  AIOperation();

 private:
};

#endif