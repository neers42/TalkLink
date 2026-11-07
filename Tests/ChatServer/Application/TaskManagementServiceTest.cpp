#include <gtest/gtest.h>
#include <future>
#include <chrono>
#include <atomic>

#include "Application/Services/TaskManagementService.hpp"
#include "ThreadPoolHandler.hpp"

namespace {

class TaskManagementServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        heartbeat_pool = std::make_shared<ThreadPoolHandler>(2);
        realtime_pool = std::make_shared<ThreadPoolHandler>(2);
        background_pool = std::make_shared<ThreadPoolHandler>(2);
        service = std::make_shared<TaskManagementService>(heartbeat_pool, realtime_pool, background_pool);
    }

    std::shared_ptr<ThreadPoolHandler> heartbeat_pool;
    std::shared_ptr<ThreadPoolHandler> realtime_pool;
    std::shared_ptr<ThreadPoolHandler> background_pool;
    std::shared_ptr<TaskManagementService> service;
};

} // namespace

TEST_F(TaskManagementServiceTest, ScheduledTaskActuallyExecutes)
{
    std::promise<void> ran;
    auto future = ran.get_future();

    service->schedule_task(
        TaskPriority::NORMAL,
        TaskCategory::SYSTEM,
        "test task",
        [&ran]() { ran.set_value(); }
    );

    ASSERT_EQ(future.wait_for(std::chrono::seconds(2)), std::future_status::ready);
}

TEST_F(TaskManagementServiceTest, TaskMetricsCountTotalExecuted)
{
    std::promise<void> ran;
    auto future = ran.get_future();

    const auto before = service->get_task_metrics().total_executed;

    service->schedule_task(
        TaskPriority::NORMAL,
        TaskCategory::SYSTEM,
        "test task",
        [&ran]() { ran.set_value(); }
    );
    ASSERT_EQ(future.wait_for(std::chrono::seconds(2)), std::future_status::ready);

    // execute_managed_task内のメトリクス更新がタスク関数の実行後に行われるため、
    // 反映されるまで短時間ポーリングする
    for (int i = 0; i < 100 && service->get_task_metrics().total_executed == before; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_EQ(service->get_task_metrics().total_executed, before + 1);
}

TEST_F(TaskManagementServiceTest, FailedTaskIsCountedAsFailed)
{
    std::promise<void> ran;
    auto future = ran.get_future();
    const auto before = service->get_task_metrics().failed_tasks;

    service->schedule_task(
        TaskPriority::NORMAL,
        TaskCategory::SYSTEM,
        "failing task",
        [&ran]() {
            ran.set_value();
            throw std::runtime_error("intentional failure for test");
        }
    );
    ASSERT_EQ(future.wait_for(std::chrono::seconds(2)), std::future_status::ready);

    for (int i = 0; i < 100 && service->get_task_metrics().failed_tasks == before; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_EQ(service->get_task_metrics().failed_tasks, before + 1);
}

TEST_F(TaskManagementServiceTest, CancelTaskOnUnknownIdReturnsFalse)
{
    EXPECT_FALSE(service->cancel_task("nonexistent-task-id"));
}

TEST_F(TaskManagementServiceTest, OverloadProtectionDisabledByDefault)
{
    EXPECT_FALSE(service->is_overloaded());
}

TEST_F(TaskManagementServiceTest, GetUserTaskCountForUnknownSocketIsZero)
{
    // 現在の実装ではtask_idに"_socket_<n>_"というパターンが埋め込まれることが
    // 無いため、どのソケットを指定しても常に0件になる(既知の未配線状態)
    EXPECT_EQ(service->get_user_task_count(123), 0u);
}
