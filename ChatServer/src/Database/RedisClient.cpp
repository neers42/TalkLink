#include "RedisClient.h"

RedisClient::RedisClient()
{
}

RedisClient::~RedisClient()
{
	this->close();
}

void RedisClient::connect()
{
	this->context = redisConnect(this->get_host(), this->get_port());
	
	if (context == NULL || context->err) {
		if (context) {
			LOG_ERROR("Redis Connection error: {}.", context->errstr);
			//spdlog::error("Redis Connection error: {}", context->errstr);
			redisFree(context);
		}
		else {
			LOG_ERROR("Redis Connection error: can't allocate redis context.");
			//spdlog::error("Redis Connection error: can't allocate redis context");
		}
		//throw std::runtime_error("Connection error");
	}
	else {
		spdlog::info("Redis Database Connect success!.");
	}
}

void RedisClient::close()
{
	redisFree(this->context);
	spdlog::info("Redis Connection closed.");
}

bool RedisClient::execute_query(const std::string& query)
{
	return true;
}


