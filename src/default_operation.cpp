#include "default_operation.h"

std::map<std::string, bool> DefaultOperation::doubleOrSurrender(
    std::vector<Poker> playerCards, std::vector<Poker> dealerVisibleCards,
    std::vector<Poker> cardPool) {
  std::map<std::string, bool> result;
  result["double"] = false;
  result["surrender"] = false;

  int playerValue = Poker::getPokerValue(playerCards);

  // 只有兩張牌時才能加倍或投降
  if (playerCards.size() == 2) {
    std::string dealerCard = dealerVisibleCards[0].getNumber();
    int dealerValue = Poker::getPokerValue(dealerVisibleCards[0]);

    // 加倍策略：點數為9、10或11時考慮加倍
    if (playerValue >= 9 && playerValue <= 11) {
      // 莊家牌不是A或10點牌時加倍
      if (dealerValue >= 2 && dealerValue <= 6) {
        result["double"] = true;
      }
    }

    // 投降策略：高風險手牌(15-16)，莊家牌面強(9-A)時投降
    if (playerValue == 16) {
      if (dealerValue >= 9 || dealerCard == "A") {
        result["surrender"] = true;
      }
    } else if (playerValue == 15 && dealerValue == 10) {
      result["surrender"] = true;
    }
  }

  return result;
}

bool DefaultOperation::hit(std::vector<Poker> playerCards,
                           std::vector<Poker> dealerVisibleCards,
                           std::vector<Poker> cardPool) {
  int playerValue = Poker::getPokerValue(playerCards);

  // 檢查是否有A (軟牌)
  bool hasSoftHand = false;
  for (auto& card : playerCards) {
    if (card.getNumber() == "A") {
      hasSoftHand = true;
      break;
    }
  }

  // 莊家的明牌值
  int dealerValue = Poker::getPokerValue(dealerVisibleCards[0]);

  // 基本策略
  if (playerValue < 12) {
    // 點數太小，要牌
    return true;
  } else if (playerValue == 12) {
    // 點數12時，當莊家牌面為2-6時不要牌，其他情況要牌
    return !(dealerValue >= 2 && dealerValue <= 6);
  } else if (playerValue >= 13 && playerValue <= 16) {
    // 點數13-16時，當莊家牌面為2-6時不要牌，其他情況要牌
    return !(dealerValue >= 2 && dealerValue <= 6);
  } else if (hasSoftHand && playerValue <= 17) {
    // 軟牌且點數小於17時要牌
    return true;
  } else if (playerValue >= 17) {
    // 點數17或以上時不要牌
    return false;
  }

  // 預設不要牌
  return false;
}

bool DefaultOperation::insurance(std::vector<Poker> playerCards,
                                 std::vector<Poker> dealerVisibleCards,
                                 std::vector<Poker> cardPool) {
  // 只有當莊家明牌為A，且玩家點數為21(有blackjack)時才考慮買保險
  if (dealerVisibleCards[0].getNumber() == "A" &&
      Poker::getPokerValue(playerCards) == 21 && playerCards.size() == 2) {
    return true;
  }
  return false;
}

int DefaultOperation::stake(int money, std::vector<Poker> dealerVisibleCards,
                            std::vector<Poker> cardPool) {
  // 基礎下注策略 - 固定下注
  return 1000;
}