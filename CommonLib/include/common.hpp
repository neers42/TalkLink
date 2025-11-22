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
using json = nlohmann::json;

#endif // COMMON_HPP