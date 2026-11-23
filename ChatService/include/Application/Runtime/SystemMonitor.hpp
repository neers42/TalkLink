#ifndef SYSTEMMONITOR_HPP
#define SYSTEMMONITOR_HPP

#include "Utils/common.hpp"
#include <condition_variable>

// Forward declarations
class TaskScheduler;
class UserSessionUseCase;

// システムリソース状態
struct SystemResourceStatus {
    uint32_t active_connections;
    double cpu_usage_percent;
    double memory_usage_percent;
};

// システム監視の設定パラメータ
struct SystemMonitorConfig {
    std::chrono::seconds check_interval{30};
    uint32_t max_concurrent_connections{1000};
    double high_usage_warning_threshold_percent{90.0};
};

/**
 * @brief サーバー稼働中のCPU/メモリ使用率・接続数を継続的に監視するサービス。
 *
 * ServerBootstrapper(起動シーケンスの制御)とは別の責務として切り出している。
 * 監視自体は起動完了後もサーバーが動いている間ずっと続く、起動とは独立した関心事のため。
 */
class SystemMonitor : public std::enable_shared_from_this<SystemMonitor> {
public:
    SystemMonitor(
        std::shared_ptr<TaskScheduler> task_scheduler,
        std::shared_ptr<UserSessionUseCase> user_session_use_case
    );
    ~SystemMonitor();

    // 監視ループを開始/停止する
    void start_monitoring();
    void stop_monitoring();

    // 現在のシステムリソース状態を取得する
    SystemResourceStatus get_system_status() const;

    // 監視設定を更新/取得する
    void update_monitor_config(const SystemMonitorConfig& config);
    const SystemMonitorConfig& get_monitor_config() const;

private:
    std::shared_ptr<TaskScheduler> _task_scheduler;
    std::shared_ptr<UserSessionUseCase> _user_session_use_case;

    void run_monitoring_loop();
    double calculate_cpu_usage_percent() const;
    double calculate_memory_usage_percent() const;

    SystemMonitorConfig _config;
    mutable std::mutex _config_mutex;

    std::atomic<bool> _monitoring_active{false};
    std::mutex _monitor_mutex;
    std::condition_variable _monitor_cv;

    // CPU使用率計測用の前回サンプル(Linux /proc/stat ベース)
    mutable std::mutex _cpu_sample_mutex;
    mutable uint64_t _prev_cpu_total_jiffies{0};
    mutable uint64_t _prev_cpu_idle_jiffies{0};
    mutable bool _has_cpu_sample{false};
};

#endif // SYSTEMMONITOR_HPP
