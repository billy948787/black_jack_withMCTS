#include "poker.h"

#include <iostream>
#include <vector>
#define RED "\033[31;1m"
#define DEFAULT "\033[0;1m"
#define WHITE "\033[1;37m"

Poker::Poker(Suit suit, std::string number)
    : _isFaceUp(true), _suit(suit), _number(number) {}

Poker::Poker() : _isFaceUp(true) {}

void Poker::showAttribute() { std::cout << _suit << " " << _number << "\n"; }

Suit Poker::getSuit() { return _suit; }

std::string Poker::getNumber() { return _number; }

std::vector<std::string> Poker::getPattern() {
  return (_isFaceUp) ? _picture : _backPattern;
}

void Poker::setNumber(std::string number) { _number = number; }

void Poker::flipTheCard() { this->_isFaceUp = !this->_isFaceUp; }

void Poker::setSuit(Suit suit) {
  _suit = suit;
  _backPattern.push_back("-----------------");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("|***************|");
  _backPattern.push_back("-----------------");
  switch (_suit) {
    case spade:
      _picture.push_back("-----------------");
      (_number == "10") ? _picture.push_back("|10             |")
                        : _picture.push_back("|" + _number + "              |");
      _picture.push_back("|       *       |");
      _picture.push_back("|      ***      |");
      _picture.push_back("|     *****     |");
      _picture.push_back("|    *******    |");
      _picture.push_back("|   *********   |");
      _picture.push_back("|  ***********  |");
      _picture.push_back("| ************* |");
      _picture.push_back("|***************|");
      _picture.push_back("|***************|");
      _picture.push_back("|***************|");
      _picture.push_back("|***************|");
      _picture.push_back("| ************* |");
      _picture.push_back("|  ***********  |");
      _picture.push_back("|   *** * ***   |");
      _picture.push_back("|       *       |");
      _picture.push_back("|      ***      |");
      _picture.push_back("|     *****     |");
      _picture.push_back("|    *******    |");
      (_number == "10") ? _picture.push_back("|             10|")
                        : _picture.push_back("|              " + _number + "|");
      _picture.push_back("-----------------");
      break;
    case heart:
      _picture.push_back("-----------------");
      (_number == "10") ? _picture.push_back("|10             |")
                        : _picture.push_back("|" + _number + "              |");
      _picture.push_back("|               |");
      _picture.push_back("|   ***   ***   |");
      _picture.push_back("|  ***** *****  |");
      _picture.push_back("| ************* |");
      _picture.push_back("|***************|");
      _picture.push_back("|***************|");
      _picture.push_back("|***************|");
      _picture.push_back("| ************* |");
      _picture.push_back("| ************* |");
      _picture.push_back("|  ***********  |");
      _picture.push_back("|  ***********  |");
      _picture.push_back("|   *********   |");
      _picture.push_back("|   *********   |");
      _picture.push_back("|    *******    |");
      _picture.push_back("|     *****     |");
      _picture.push_back("|     *****     |");
      _picture.push_back("|      ***      |");
      _picture.push_back("|       *       |");
      (_number == "10") ? _picture.push_back("|             10|")
                        : _picture.push_back("|              " + _number + "|");
      _picture.push_back("-----------------");
      break;
    case diamond:
      _picture.push_back("-----------------");
      (_number == "10") ? _picture.push_back("|10             |")
                        : _picture.push_back("|" + _number + "              |");
      _picture.push_back("|               |");
      _picture.push_back("|               |");
      _picture.push_back("|       *       |");
      _picture.push_back("|      ***      |");
      _picture.push_back("|     *****     |");
      _picture.push_back("|    *******    |");
      _picture.push_back("|   *********   |");
      _picture.push_back("|  ***********  |");
      _picture.push_back("| ************* |");
      _picture.push_back("|***************|");
      _picture.push_back("| ************* |");
      _picture.push_back("|  ***********  |");
      _picture.push_back("|   *********   |");
      _picture.push_back("|    *******    |");
      _picture.push_back("|     *****     |");
      _picture.push_back("|      ***      |");
      _picture.push_back("|       *       |");
      _picture.push_back("|               |");
      (_number == "10") ? _picture.push_back("|             10|")
                        : _picture.push_back("|              " + _number + "|");
      _picture.push_back("-----------------");
      break;
    case club:
      _picture.push_back("-----------------");
      (_number == "10") ? _picture.push_back("|10             |")
                        : _picture.push_back("|" + _number + "              |");
      _picture.push_back("|               |");
      _picture.push_back("|      ***      |");
      _picture.push_back("|     *****     |");
      _picture.push_back("|     *****     |");
      _picture.push_back("|    *******    |");
      _picture.push_back("|    *******    |");
      _picture.push_back("|     *****     |");
      _picture.push_back("|      ***      |");
      _picture.push_back("|   **  *  **   |");
      _picture.push_back("| ***** * ***** |");
      _picture.push_back("|***************|");
      _picture.push_back("|***************|");
      _picture.push_back("|***************|");
      _picture.push_back("| ***** * ***** |");
      _picture.push_back("|   **  *  **   |");
      _picture.push_back("|      ***      |");
      _picture.push_back("|     *****     |");
      _picture.push_back("|    *******    |");
      (_number == "10") ? _picture.push_back("|             10|")
                        : _picture.push_back("|              " + _number + "|");
      _picture.push_back("-----------------");
      break;
  }
}

void Poker::printPokers(std::vector<Poker> pokers) {
  for (int i = 0; i < pokers[0].getPattern().size(); i++) {
    for (auto& poker : pokers) {
      if ((poker.getSuit() == heart || poker.getSuit() == diamond) &&
          poker._isFaceUp) {
        std::cout << RED << poker.getPattern()[i] << DEFAULT << "  ";
      } else {
        std::cout << WHITE << poker.getPattern()[i] << DEFAULT << "  ";
      }
    }
    std::cout << "\n";
  }
  std::cout << "\n";
}

void Poker::printPokers(Poker poker) {
  for (auto row : poker.getPattern()) {
    std::cout<< row << "\n";
  }
}