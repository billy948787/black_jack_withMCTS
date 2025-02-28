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
      std::make_shared<Player>("test", *new MockOperation());

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

  player = std::make_shared<Player>("test", *new MockOperation());

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

  player = std::make_shared<Player>("test", *new MockOperation());

  player->addPoker(pokerQ);
  player->addPoker(pokerJ);
  EXPECT_EQ(player->getPoint(), 20);

  player.reset();

  player = std::make_shared<Player>("test", *new MockOperation());
  player->addPoker(pokerK);
  player->addPoker(pokerA);

  EXPECT_EQ(player->getPoint(), 21);
}