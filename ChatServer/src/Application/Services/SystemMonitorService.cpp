#include "Application/Services/SystemMonitorService.hpp"
#include "Application/Services/TaskManagementService.hpp"
#include "Application/Services/UserManagementService.hpp"
#include <fstream>
#include <sstream>
#include <string>

namespace {
// 常駐し続けるバックグラウンドループタスク用のmax_execution_time。
// デフォルトの30秒を使うと、TaskManagementServiceの過負荷監視
// (check_system_overload)から「長時間実行タスク」として常に警告されて
// しまうため、実質無期限とみなせる十分大きな値を設定する。
constexpr auto kPermanentTaskTimeout = std::chrono::hours(24 * 365);
}

SystemMonitorService::SystemMonitorService(
    std::shared_ptr<TaskManagementService> task_management_service,
    std::shared_ptr<UserManagementService> user_management_service)
    : _task_management_service(std::move(task_management_service)),
      _user_management_service(std::move(user_management_service))
{
    if (!_task_management_service || !_user_management_service) {
        throw std::runtime_error("Failed to initialize SystemMonitorService dependencies");
    }

    LOG_INFO("[SystemMonitorService] SystemMonitorService initialized successfully");
}

SystemMonitorService::~SystemMonitorService()
{
    stop_monitoring();
    LOG_INFO("[SystemMonitorService] SystemMonitorService shutting down");
}

void SystemMonitorService::start_monitoring()
{
    if (_monitoring_active.exchange(true)) {
        return; // 既に起動済み
    }
    LOG_INFO("[SystemMonitorService] System monitoring started");

    // ループがthisを安全に参照し続けられるよう、shared_ptrでthisの寿命を延長する
    // (UserManagementService::start_heartbeat_monitoringと同様のライフタイム管理)
    auto self = shared_from_this();
    _task_management_service->schedule_task_with_timeout(
        TaskPriority::LOW,
        TaskCategory::SYSTEM,
        "System monitoring task",
        kPermanentTaskTimeout,
        [self]() {
            self->run_monitoring_loop();
        }
    );
}

void SystemMonitorService::stop_monitoring()
{
    if (_monitoring_active.exchange(false)) {
        _monitor_cv.notify_all();
        LOG_INFO("[SystemMonitorService] System monitoring stopped");
    }
}

void SystemMonitorService::run_monitoring_loop()
{
    const auto interval = get_monitor_config().check_interval;

    while (_monitoring_active.load()) {
        std::unique_lock<std::mutex> lock(_monitor_mutex);
        const bool stopped = _monitor_cv.wait_for(
            lock, interval, [this]() { return !_monitoring_active.load(); });
        if (stopped) {
            break;
        }
        lock.unlock();

        try {
            const auto status = get_system_status();
            LOG_INFO("[SystemMonitorService] Health check - connections: {}, cpu: {:.1f}%, mem: {:.1f}%",
                     status.active_connections, status.cpu_usage_percent, status.memory_usage_percent);

            const auto warning_threshold = get_monitor_config().high_usage_warning_threshold_percent;
            if (status.cpu_usage_percent >= warning_threshold || status.memory_usage_percent >= warning_threshold) {
                LOG_WARNING("[SystemMonitorService] High resource usage detected - cpu: {:.1f}%, mem: {:.1f}% (threshold: {:.1f}%)",
                            status.cpu_usage_percent, status.memory_usage_percent, warning_threshold);
            }

            const auto max_connections = get_monitor_config().max_concurrent_connections;
            if (status.active_connections >= max_connections) {
                // 新規接続の受付自体はUserManagementService::can_accept_new_connectionが
                // chat.confのmax_connectionsで別途判定している。ここでは監視用の警告のみ。
                LOG_WARNING("[SystemMonitorService] Active connections at or above configured capacity: {}/{}",
                            status.active_connections, max_connections);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("[SystemMonitorService] Exception during health check: {}", e.what());
        }
    }

    LOG_DEBUG("[SystemMonitorService] Monitoring loop exited");
}

SystemResourceStatus SystemMonitorService::get_system_status() const
{
    SystemResourceStatus status;
    status.active_connections = static_cast<uint32_t>(_user_management_service->get_connected_user_count());
    status.cpu_usage_percent = calculate_cpu_usage_percent();
    status.memory_usage_percent = calculate_memory_usage_percent();
    return status;
}

void SystemMonitorService::update_monitor_config(const SystemMonitorConfig& config)
{
    std::lock_guard<std::mutex> lock(_config_mutex);
    _config = config;
    LOG_INFO("[SystemMonitorService] Monitor configuration updated");
}

const SystemMonitorConfig& SystemMonitorService::get_monitor_config() const
{
    std::lock_guard<std::mutex> lock(_config_mutex);
    return _config;
}

double SystemMonitorService::calculate_cpu_usage_percent() const
{
    try {
        std::ifstream stat_file("/proc/stat");
        if (!stat_file.is_open()) {
            return 0.0;
        }

        std::string cpu_label;
        uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0;
        std::string first_line;
        std::getline(stat_file, first_line);
        std::istringstream iss(first_line);
        iss >> cpu_label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

        const uint64_t total = user + nice + system + idle + iowait + irq + softirq + steal;
        const uint64_t idle_total = idle + iowait;

        std::lock_guard<std::mutex> lock(_cpu_sample_mutex);
        if (!_has_cpu_sample) {
            _prev_cpu_total_jiffies = total;
            _prev_cpu_idle_jiffies = idle_total;
            _has_cpu_sample = true;
            return 0.0; // 初回は前回値が無いため算出不可
        }

        const uint64_t delta_total = total - _prev_cpu_total_jiffies;
        const uint64_t delta_idle = idle_total - _prev_cpu_idle_jiffies;
        _prev_cpu_total_jiffies = total;
        _prev_cpu_idle_jiffies = idle_total;

        if (delta_total == 0) {
            return 0.0;
        }

        return (1.0 - static_cast<double>(delta_idle) / static_cast<double>(delta_total)) * 100.0;

    } catch (const std::exception& e) {
        LOG_ERROR("[SystemMonitorService] Failed to read CPU usage: {}", e.what());
        return 0.0;
    }
}

double SystemMonitorService::calculate_memory_usage_percent() const
{
    try {
        std::ifstream meminfo_file("/proc/meminfo");
        if (!meminfo_file.is_open()) {
            return 0.0;
        }

        uint64_t mem_total_kb = 0;
        uint64_t mem_available_kb = 0;
        std::string line;
        while (std::getline(meminfo_file, line)) {
            std::istringstream iss(line);
            std::string key;
            uint64_t value = 0;
            iss >> key >> value;
            if (key == "MemTotal:") {
                mem_total_kb = value;
            } else if (key == "MemAvailable:") {
                mem_available_kb = value;
            }
            if (mem_total_kb != 0 && mem_available_kb != 0) {
                break;
            }
        }

        if (mem_total_kb == 0) {
            return 0.0;
        }

        return (1.0 - static_cast<double>(mem_available_kb) / static_cast<double>(mem_total_kb)) * 100.0;

    } catch (const std::exception& e) {
        LOG_ERROR("[SystemMonitorService] Failed to read memory usage: {}", e.what());
        return 0.0;
    }
}
