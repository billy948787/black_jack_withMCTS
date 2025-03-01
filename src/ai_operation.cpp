#include "ai_operation.h"

#include <chrono>
#include <thread>

#include "game.h"
#include "mcts.h"
const int sleepTime = 2000;

const int simulations = 4000000;

AIOperation::AIOperation() {}

bool AIOperation::hit(std::vector<Poker> playerCards,
                      std::vector<Poker> dealerVisibleCards,
                      std::vector<Poker> cardPool) {
  auto mctsEngine =
      mcts::MCTS(simulations, playerCards, cardPool, dealerVisibleCards);

  auto bestNode = mctsEngine.run();

  if (bestNode->action == mcts::Action::HIT) {
    return true;
  } else {
    return false;
  }
}

std::map<std::string, bool> AIOperation::doubleOrSurrender(
    std::vector<Poker> playerCards, std::vector<Poker> dealerVisibleCards,
    std::vector<Poker> cardPool) {
  std::map<std::string, bool> result;

  auto mctsEngine =
      mcts::MCTS(simulations, playerCards, cardPool, dealerVisibleCards);

  auto bestNode = mctsEngine.run();

  if (bestNode->action == mcts::Action::DOUBLE) {
    result["double"] = true;
  } else if (bestNode->action == mcts::Action::SURRENDER) {
    result["surrender"] = true;
  } else {
    result["double"] = false;
    result["surrender"] = false;
  }
  return result;
}

bool AIOperation::insurance(std::vector<Poker> playerCards,
                            std::vector<Poker> dealerVisibleCards,
                            std::vector<Poker> cardPool) {
  auto mctsEngine =
      mcts::MCTS(simulations, playerCards, cardPool, dealerVisibleCards);

  auto bestNode = mctsEngine.run();

  if (bestNode->action == mcts::Action::INSURANCE) {
    return true;
  } else {
    return false;
  }
}

int AIOperation::stake(int, std::vector<Poker> dealerVisibleCards,
                       std::vector<Poker> cardPool) {
  Game game = Game::getInstance();
  int leastBet = game.getLeasetBet();

  return leastBet;
}
