#ifndef COMMON_H_
#define COMMON_H_

#include <iostream>
#include <string>
#include <mutex>
#include "stdlib.h"
#include "LogHandler.h"
#include "nlohmann/json.hpp"
#include <thread>
#include <chrono>
#include <memory>
#include <errno.h>
#include <random>
#include <sstream>
#include "ulid/ulid.hh"
#include "base64.h"
#include "base32.h"
//#include "spdlog/spdlog.h"


using json = nlohmann::json;

namespace random {
	//乱数生成
    inline unsigned char random_byte() {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_int_distribution<int> dist(0, 255);
        return static_cast<unsigned char>(dist(rng));
    }
}


#endif // !COMMON_H_

