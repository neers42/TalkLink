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



using json = nlohmann::json;

namespace random {
	//—”¶¬
    inline unsigned char random_byte() {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_int_distribution<int> dist(0, 255);
        return static_cast<unsigned char>(dist(rng));
    }
}


#endif // !COMMON_H_

