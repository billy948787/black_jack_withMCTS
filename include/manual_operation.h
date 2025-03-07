#ifndef MANUAL_OPERATION_H
#define MANUAL_OPERATION_H
#include "operation.h"

class ManualOperation : public Operation {
 public:
  std::map<std::string, bool> doubleOrSurrender(std::vector<Poker>,
                                                std::vector<Poker>,
                                                std::vector<Poker>) override;
  bool hit(std::vector<Poker>, std::vector<Poker>, std::vector<Poker>) override;
  bool insurance(std::vector<Poker>, std::vector<Poker>,
                 std::vector<Poker>) override;
  int stake(int, std::vector<Poker>, std::vector<Poker>) override;
};

#endif