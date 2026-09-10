#ifndef USERINFO_HPP
#define USERINFO_HPP

#include <string>

struct UserInfo
{
    std::string uid;
    std::string name;
    std::string mail_address;
    std::string icon_url;

    UserInfo()
        : uid(""), name(""), mail_address(""), icon_url("")
    {}

    UserInfo(
        const std::string& uid,
        const std::string& name,
        const std::string& mail_address,
        const std::string& icon_url
    )
        : uid(uid),
          name(name),
          mail_address(mail_address),
          icon_url(icon_url)
    {}

    UserInfo(
        const std::string& name,
        const std::string& mail_address,
        const std::string& icon_url
    )
        : uid(""),
          name(name),
          mail_address(mail_address),
          icon_url(icon_url)
    {}
};

#endif