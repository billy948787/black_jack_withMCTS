#include "operation_controller.h"

OperationController::OperationController(Operation& operation)
    : operation(&operation) {}

void OperationController::changeOperation(Operation& operation) {
  this->operation = &operation;
}

std::map<std::string, bool> OperationController::doubleOrSurrender(
    std::vector<Poker> playerCards, std::vector<Poker> dealerVisibleCards,
    std::vector<Poker> cardPool) {
  return operation->doubleOrSurrender(playerCards, dealerVisibleCards,
                                      cardPool);
}

bool OperationController::hit(std::vector<Poker> playerCards,
                              std::vector<Poker> dealerVisibleCards,
                              std::vector<Poker> cardPool) {
  return operation->hit(playerCards, dealerVisibleCards, cardPool);
}

bool OperationController::insurance(std::vector<Poker> playerCards,
                                    std::vector<Poker> dealerVisibleCards,
                                    std::vector<Poker> cardPool) {
  return operation->insurance(playerCards, dealerVisibleCards, cardPool);
}

int OperationController::stake(int money, std::vector<Poker> dealerVisibleCards,
                               std::vector<Poker> cardPool) {
  return operation->stake(money, dealerVisibleCards, cardPool);
}
