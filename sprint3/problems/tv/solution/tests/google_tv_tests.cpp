#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "../src/tv.h"

// Тестовый стенд "Телевизор по умолчанию"
class TVByDefault : public testing::Test {
protected:
    TV tv_;
};
TEST_F(TVByDefault, IsOff) {
    EXPECT_FALSE(tv_.IsTurnedOn());
}
TEST_F(TVByDefault, DoesntShowAChannelWhenItIsOff) {
    EXPECT_FALSE(tv_.GetChannel().has_value());
}
// Включите этот тест и доработайте класс TV, чтобы тест выполнился успешно
#if 1
TEST_F(TVByDefault, CantSelectAnyChannel) {
    EXPECT_THROW(tv_.SelectChannel(10), std::logic_error);
    EXPECT_THROW(tv_.SelectLastViewedChannel(), std::logic_error);
    EXPECT_EQ(tv_.GetChannel(), std::nullopt);
    tv_.TurnOn();
    EXPECT_THAT(tv_.GetChannel(), testing::Optional(1));
}
#endif

// Тестовый стенд "Включенный телевизор"
class TurnedOnTV : public TVByDefault {
protected:
    void SetUp() override {
        tv_.TurnOn();
    }
};
TEST_F(TurnedOnTV, ShowsChannel1) {
    EXPECT_TRUE(tv_.IsTurnedOn());
    EXPECT_THAT(tv_.GetChannel(), testing::Optional(1));
}
TEST_F(TurnedOnTV, AfterTurningOffTurnsOffAndDoesntShowAnyChannel) {
    tv_.TurnOff();
    EXPECT_FALSE(tv_.IsTurnedOn());
    // Сравнение с nullopt в GoogleTest выполняется так:
    EXPECT_EQ(tv_.GetChannel(), std::nullopt);
}
TEST_F(TurnedOnTV, cannot_select_channel_out_of_) {
    EXPECT_THROW(tv_.SelectChannel(0), std::out_of_range);
    EXPECT_THROW(tv_.SelectChannel(100), std::out_of_range);
}
TEST_F(TurnedOnTV, CanSelectChannelFrom1To99) {
    tv_.SelectChannel(5);
    EXPECT_THAT(tv_.GetChannel(), testing::Optional(5));
    tv_.SelectChannel(10);
    tv_.SelectLastViewedChannel();
    EXPECT_THAT(tv_.GetChannel(), testing::Optional(5));
}
/* Реализуйте самостоятельно остальные тесты класса TV */
