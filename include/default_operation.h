#pragma once

#include "operation.h"

class DefaultOperation : public Operation {
 public:
  std::map<std::string, bool> doubleOrSurrender(std::vector<Poker>,
                                                std::vector<Poker>,
                                                std::vector<Poker>) override;
  bool hit(std::vector<Poker>, std::vector<Poker>, std::vector<Poker>) override;
  bool insurance(std::vector<Poker>, std::vector<Poker>,
                 std::vector<Poker>) override;
  int stake(int, std::vector<Poker>, std::vector<Poker>) override;
};