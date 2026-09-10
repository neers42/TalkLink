#ifndef IUSERREPOSITORY_HPP
#define IUSERREPOSITORY_HPP

#include <string>
#include <vector>
#include "Domain/DTO/UserInfo.hpp"

class IUserRepository
{
public:
    virtual ~IUserRepository() = default;

    virtual bool insert_user_info_data(
        const std::string& uid,
        const std::string& name,
        const std::string& mail_address,
        const std::string& icon_url,
        const std::string& created_at,
        const std::string& last_login
    ) = 0;

    virtual UserInfo get_user_info(
        const std::string& uid
    ) = 0;

    virtual bool update_last_login(
        const std::string& uid,
        const std::string& last_login
    ) = 0;

    virtual bool user_exists(
        const std::string& uid
    ) = 0;

    virtual std::string get_user_last_login(
        const std::string& uid
    ) = 0;

    virtual std::vector<UserInfo> search_user_infos(
        const std::string& query
    ) = 0;
};

#endif