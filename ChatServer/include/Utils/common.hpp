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
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "ulid/ulid.hh"
#include "AppException.h"
#include "StringFormat.h"
#include "ServerTime.hpp"
#include "ThreadPoolHandler.hpp"
#include "Network/SocketHandler.hpp"
#include "Network/TLSConnectionHandler.hpp"
#include <shared_mutex>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include "base64.hpp"
#include "base32.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <iomanip>
#include <jwt-cpp/jwt.h>
using json = nlohmann::json;

// ソケット関連の型定義
typedef int socket_t;
#define INVALID_SOCKET_FD -1

namespace random_utils {
	//乱数生成
    inline unsigned char random_byte() {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_int_distribution<int> dist(0, 255);
        return static_cast<unsigned char>(dist(rng));
    }
}


#endif // !COMMON_HPP

