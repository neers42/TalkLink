#include <gtest/gtest.h>

#include "Domain/UserAggregateStore.hpp"

// UserAggregateStoreの操作メソッド(insert/erase/find)はUserRepositoryHandler/
// CommunicationHandlerのみがfriend経由でアクセスできるprivateな実装であり、
// テストから直接呼び出せない。公開APIであるsize()/empty()のみを検証する。

TEST(UserAggregateStoreTest, FreshlyConstructedStoreIsEmpty)
{
    UserAggregateStore store;
    EXPECT_TRUE(store.empty());
    EXPECT_EQ(store.size(), 0u);
}
