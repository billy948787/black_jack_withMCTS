#include <iostream>

#include "game.h"
#define DEFAULT "\033[0;1m"

int main() {
  Game game = Game::getInstance();

  std::cout << DEFAULT << "Welcome to BlackJack\n";

  bool isTestMode = true;

  game.start(isTestMode);
}