#include <gtest/gtest.h>

#include <memory>

#include "dealer.h"
#include "player.h"

class MockOperation : public Operation {  // Just for inject mock operation
  public:
    std::map<std::string, bool> doubleOrSurrender(std::vector<Poker>,
                                                  std::vector<Poker>,
                                                  std::vector<Poker>) override {
      return {{"double", false}, {"surrender", false}};
    }
    bool hit(std::vector<Poker>, std::vector<Poker>, std::vector<Poker>) override {
      return false;
    }
    bool insurance(std::vector<Poker>, std::vector<Poker>,
                  std::vector<Poker>) override {
      return false;
    }
    int stake(int, std::vector<Poker>, std::vector<Poker>) override {
      return 0;
    }
};

TEST(PlayerTest, TestPlayer) {
  std::shared_ptr<Player> player =
      std::make_shared<Player>("test", new MockOperation());

  Poker pokerA(spade, "A");
  Poker poker2(spade, "2");
  Poker poker3(spade, "3");
  Poker poker4(spade, "4");
  Poker poker5(spade, "5");
  Poker poker6(spade, "6");
  Poker poker7(spade, "7");
  Poker poker8(spade, "8");
  Poker poker9(spade, "9");
  Poker poker10(spade, "10");
  Poker pokerJ(spade, "J");
  Poker pokerQ(spade, "Q");
  Poker pokerK(spade, "K");

  // first poker set to test point
  player->addPoker(pokerA);
  EXPECT_EQ(player->getPoint(), 11);

  player->addPoker(poker2);
  EXPECT_EQ(player->getPoint(), 13);

  player->addPoker(poker3);
  EXPECT_EQ(player->getPoint(), 16);

  player->addPoker(poker4);
  EXPECT_EQ(player->getPoint(), 20);

  player->addPoker(poker5);
  EXPECT_EQ(player->getPoint(), 15);

  player->addPoker(poker6);
  EXPECT_EQ(player->getPoint(), 21);

  player.reset();
  EXPECT_EQ(player, nullptr);

  player = std::make_shared<Player>("test", new MockOperation());

  player->addPoker(pokerA);
  player->addPoker(pokerA);
  player->addPoker(pokerA);
  player->addPoker(pokerA);
  player->addPoker(pokerA);
  EXPECT_EQ(player->getPoint(), 15);

  player->addPoker(pokerA);
  EXPECT_EQ(player->getPoint(), 16);
  player->addPoker(pokerA);
  EXPECT_EQ(player->getPoint(), 17);
  player->addPoker(pokerA);
  EXPECT_EQ(player->getPoint(), 18);

  player->addPoker(poker3);
  EXPECT_EQ(player->getPoint(), 21);

  player.reset();

  player = std::make_shared<Player>("test", new MockOperation());

  player->addPoker(pokerQ);
  player->addPoker(pokerJ);
  EXPECT_EQ(player->getPoint(), 20);

  player.reset();

  player = std::make_shared<Player>("test", new MockOperation());
  player->addPoker(pokerK);
  player->addPoker(pokerA);

  EXPECT_EQ(player->getPoint(), 21);

  player.reset();

  player = std::make_shared<Player>("test", new MockOperation());

  player->addPoker(pokerA);
  player->addPoker(poker10);
  player->addPoker(pokerA);
  EXPECT_EQ(player->getPoint(), 12);
  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  // 測試A的多種組合情況
  player->addPoker(pokerA);  // A = 11
  player->addPoker(poker9);  // A + 9 = 20
  EXPECT_EQ(player->getPoint(), 20);

  player->addPoker(poker2);  // A + 9 + 2 = 12 (A變成1)
  EXPECT_EQ(player->getPoint(), 12);

  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  // 測試多張A的動態調整
  player->addPoker(pokerA);  // A = 11
  player->addPoker(pokerA);  // A + A = 2 (兩個都變成1)
  player->addPoker(poker9);  // A + A + 9 = 11
  EXPECT_EQ(player->getPoint(), 21);

  player->addPoker(poker5);  // A + A + 9 + 5 = 16
  EXPECT_EQ(player->getPoint(), 16);

  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  // 測試接近21點的邊界情況
  player->addPoker(poker10);  // 10
  player->addPoker(poker8);   // 10 + 8 = 18
  player->addPoker(pokerA);   // 10 + 8 + A = 19 (A = 1)
  player->addPoker(poker2);   // 10 + 8 + A + 2 = 21
  EXPECT_EQ(player->getPoint(), 21);

  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  // 測試超過21點的情況
  player->addPoker(poker10);  // 10
  player->addPoker(pokerJ);   // 10 + 10 = 20
  player->addPoker(poker3);   // 10 + 10 + 3 = 23 (爆牌)
  EXPECT_EQ(player->getPoint(), 23);

  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  // 測試多張A和面牌的組合
  player->addPoker(pokerA);  // A = 11
  player->addPoker(poker5);  // A + 5 = 16
  player->addPoker(pokerA);  // A + 5 + A = 17 (一個A為11，一個A為1)
  player->addPoker(pokerA);  // A + 5 + A + A = 18 (一個A為11，兩個A為1)
  player->addPoker(
      pokerA);  // A + 5 + A + A + A = 19 (所有A都為1，除了一個為11)
  player->addPoker(poker3);           // A + 5 + A + A + A + 3 = 22 (所有A都為1)
  EXPECT_EQ(player->getPoint(), 12);  // 超過21點后，所有A都應該算作1

  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  // 測試黑傑克 (A + 10)
  player->addPoker(pokerA);  // A
  player->addPoker(pokerK);  // A + K = 21
  EXPECT_EQ(player->getPoint(), 21);

  // 測試不同花色相同大小的牌
  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  player->addPoker(Poker(spade, "10"));  // 黑桃10
  player->addPoker(Poker(heart, "10"));  // 紅心10
  player->addPoker(Poker(club, "10"));   // 梅花10
  EXPECT_EQ(player->getPoint(), 30);

  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  // 測試所有四張A的組合
  player->addPoker(pokerA);  // A = 11
  EXPECT_EQ(player->getPoint(), 11);

  player->addPoker(pokerA);  // A + A = 12 (一個A為11，一個A為1)
  EXPECT_EQ(player->getPoint(), 12);

  player->addPoker(pokerA);  // A + A + A = 13 (一個A為11，兩個A為1)
  EXPECT_EQ(player->getPoint(), 13);

  player->addPoker(pokerA);  // A + A + A + A = 14 (一個A為11，三個A為1)
  EXPECT_EQ(player->getPoint(), 14);

  player->addPoker(poker8);           // A + A + A + A + 8 = 22 -> 所有A都變為1
  EXPECT_EQ(player->getPoint(), 12);  // 1 + 1 + 1 + 1 + 8 = 12

  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  // 測試面牌組合
  player->addPoker(pokerJ);  // J = 10
  player->addPoker(pokerQ);  // J + Q = 20
  player->addPoker(pokerK);  // J + Q + K = 30
  EXPECT_EQ(player->getPoint(), 30);

  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  // 測試精確的21點組合
  player->addPoker(poker7);  // 7
  player->addPoker(poker4);  // 7 + 4 = 11
  player->addPoker(pokerK);  // 7 + 4 + K = 21
  EXPECT_EQ(player->getPoint(), 21);

  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  // 測試A從11轉為1再轉回的情況
  player->addPoker(pokerA);  // A = 11
  player->addPoker(poker9);  // A + 9 = 20
  player->addPoker(poker3);  // A + 9 + 3 = 13 (A變為1)
  player->addPoker(poker7);  // A + 9 + 3 + 7 = 20
  EXPECT_EQ(player->getPoint(), 20);

  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  // 測試多張A交替的情況
  player->addPoker(pokerA);        // A = 11
  player->addPoker(poker5);        // A + 5 = 16
  player->addPoker(pokerA);  // A + 5 + A = 17 (第二張A為1)
  player->addPoker(poker2);        // A + 5 + A + 2 = 19
  player->addPoker(pokerA);   // A + 5 + A + 2 + A = 20 (第三張A為1)
  player->addPoker(poker2);        // A + 5 + A + 2 + A + 2 = 22 -> 所有A變為1
  EXPECT_EQ(player->getPoint(), 12);  // 1 + 5 + 1 + 2 + 1 + 2 = 12

  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  // 測試剛好爆牌的情況
  player->addPoker(poker10);  // 10
  player->addPoker(poker6);   // 10 + 6 = 16
  player->addPoker(poker5);   // 10 + 6 + 5 = 21
  player->addPoker(poker2);   // 10 + 6 + 5 + 2 = 23 (爆牌)
  EXPECT_EQ(player->getPoint(), 23);

  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  // 測試不同黑傑克組合
  player->addPoker(pokerA);  // A = 11
  player->addPoker(pokerJ);  // A + J = 21
  EXPECT_EQ(player->getPoint(), 21);

  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  player->addPoker(pokerK);  // K = 10
  player->addPoker(pokerA);  // K + A = 21
  EXPECT_EQ(player->getPoint(), 21);

  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  // 測試邊界情況：從爆牌到不爆牌
  player->addPoker(poker10);  // 10
  player->addPoker(poker9);   // 10 + 9 = 19
  player->addPoker(pokerA);   // 10 + 9 + A = 20 (A變為1而不是11來避免爆牌)
  EXPECT_EQ(player->getPoint(), 20);

  player.reset();
  player = std::make_shared<Player>("test", new MockOperation());

  // 測試連續添加小牌直到爆牌
  player->addPoker(poker5);  // 5
  player->addPoker(poker4);  // 5 + 4 = 9
  player->addPoker(poker3);  // 5 + 4 + 3 = 12
  player->addPoker(poker2);  // 5 + 4 + 3 + 2 = 14
  player->addPoker(poker6);  // 5 + 4 + 3 + 2 + 6 = 20
  player->addPoker(poker7);  // 5 + 4 + 3 + 2 + 6 + 7 = 27 (爆牌)
  EXPECT_EQ(player->getPoint(), 27);
}