#ifndef MESSAGEDELIVERYRESULT_HPP
#define MESSAGEDELIVERYRESULT_HPP

#include <cstdint>
#include <string>
#include <vector>

struct DeliveryResult
{
    bool success = false;
    int sent_count = 0;
    std::vector<std::string> unsent_uids;
    std::string error_message;
    std::uint64_t room_sequence = 0;
};

#endif // MESSAGEDELIVERYRESULT_HPP