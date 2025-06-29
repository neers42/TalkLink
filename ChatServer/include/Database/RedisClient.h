#ifndef REDISCLIENT_H_
#define REDISCLIENT_H_
#include "Utils/common.h"
#include "DataBaseInterface.h"
#include "spdlog/spdlog.h"
#include "hiredis/hiredis.h"
class RedisClient : public DataBaseInterface
{
public:
	//コンストラクタ
	RedisClient();
	//デストラクタ
	~RedisClient();

	const char* get_host() const {
		return host;
	}
	int get_port() const {
		return port;
	}

	void connect() override;
	void close() override;
	bool execute_query(const std::string& query) override;

private:
	const char* host = "127.0.0.1";
	int port = 6379;
	redisContext* context;

};

#endif // !REDISCLIENT_H_
