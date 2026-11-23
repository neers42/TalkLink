#include <gtest/gtest.h>

#include "Domain/MessageIDGenerator.hpp"
#include <set>
#include <cstdlib>

TEST(MessageIDGeneratorTest, GeneratedIdsAreUniqueAndIncreasing)
{
    std::set<uint64_t> ids;
    uint64_t previous = 0;
    for (int i = 0; i < 1000; ++i) {
        uint64_t id = MessageIDGenerator::generate_message_id();
        EXPECT_GE(id, previous);
        ids.insert(id);
        previous = id;
    }
    EXPECT_EQ(ids.size(), 1000u);
}

TEST(MessageIDGeneratorTest, ResolveMachineIdMatchesEnvironmentVariable)
{
    // テスト実行環境ではCHAT_SERVICE_MACHINE_ID=1がgtest_discover_testsのENVIRONMENTで設定されている
    EXPECT_EQ(MessageIDGenerator::resolve_machine_id(), 1u);
}
