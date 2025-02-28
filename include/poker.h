#ifndef POKER_H
#define POKER_H
#include <iostream>
#include <vector>
#include <string>

enum Suit { spade, heart, diamond, club };

class Poker {
 public:
  void showAttribute();
  void setSuit(Suit suit);
  void setNumber(std::string number);
  Poker(Suit, std::string);
  Poker();
  Suit getSuit();
  std::string getNumber();
  std::vector<std::string> getPattern();
  static void printPokers(std::vector<Poker> pokers);
  static void printPokers(Poker);
  void flipTheCard();

  static int getPokerValue(Poker poker) {
    if (poker.getNumber() == "A") return 11;
    if (poker.getNumber() == "J" || poker.getNumber() == "Q" ||
        poker.getNumber() == "K")
      return 10;
    return std::stoi(poker.getNumber());
  }

  static int getPokerValue(std::vector<Poker> pokers) {
    int point = 0;
    int count = 0;
    for (auto poker : pokers) {
      if (!poker._isFaceUp) continue;
      if (poker.getNumber() == "A") {
        count++;
        continue;
      }
      if (poker.getNumber() == "J" || poker.getNumber() == "Q" ||
          poker.getNumber() == "K") {
        point += 10;
      } else {
        point += std::stoi(poker.getNumber());
      }
    }

    while (count-- > 0) {
      if (point + 11 > 21) {
        point += 1;
      } else {
        point += 11;
      }
    }

    return point;
  }

 private:
  Suit _suit;
  std::string _number;
  bool _isFaceUp;
  std::vector<std::string> _picture;
  std::vector<std::string> _backPattern;
  friend class Player;
};

#endif