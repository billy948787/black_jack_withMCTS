#ifndef OPERATION_H
#define OPERATION_H
#include <map>
#include <string>

#include "poker.h"

class Operation {
 public:
  virtual std::map<std::string, bool> doubleOrSurrender(std::vector<Poker>,
                                                        std::vector<Poker>,
                                                        std::vector<Poker>) = 0;
  virtual bool hit(std::vector<Poker>, std::vector<Poker>,
                   std::vector<Poker>) = 0;
  virtual bool insurance(std::vector<Poker>, std::vector<Poker>,
                         std::vector<Poker>) = 0;
  virtual int stake(int, std::vector<Poker>, std::vector<Poker>) = 0;
};

#endif