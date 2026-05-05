#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include "stdlib.h"
#include "LogHandler.hpp"
#include "nlohmann/json.hpp"
#include <thread>
#include <chrono>
#include <memory>
#include <atomic>
#include <errno.h>
#include <random>
#include <sstream>
#include <map>
#include "AppException.h"
#include "StringFormat.h"
#include "ServerTime.hpp"
#include <shared_mutex>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <tuple>
#include <unordered_map>
// ネットワーク関連のヘッダー
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
// SSL関連のヘッダー
#include <openssl/ssl.h>
using json = nlohmann::json;

// ソケット関連の型定義
typedef int socket_t;
#define INVALID_SOCKET_FD -1

#endif // COMMON_HPP