#ifndef REDISHELPER_HPP
#define REDISHELPER_HPP
#include "common.hpp"
#include "hiredis/hiredis.h"
class RedisHelper
{
private:
    std::string _host;
    int _port;
    std::unique_ptr<redisContext, decltype(&redisFree)> _context;   //デリータでredisFreeを使用
public:
    RedisHelper();
    RedisHelper(const std::string& host, int port);
    ~RedisHelper();

    bool connect();     //接続
    bool disconnect();  //切断

    bool set_key_value(const std::string& key, const std::string& value); //書き込み
    std::string get_key_value(const std::string& key);    //読み込み

    bool hmset(const std::string& key, const std::unordered_map<std::string, std::string>& fields); //ハッシュマップのセット
    bool hmget(const std::string& key, const std::vector<std::string>& fields_to_get, std::unordered_map<std::string, std::string>& out_fields); //ハッシュマップの取得
    bool expire(const std::string& key, std::uint64_t expire, std::uint64_t& ttl); //キーの有効期限設定
    bool delete_key(const std::vector<std::string>& keys); //キーの削除
    bool is_key_exists(const std::string& key); //キーの存在確認
};




#endif // REDISHELPER_HPP