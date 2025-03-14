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
			spdlog::error("Connection error: {}", context->errstr);
			redisFree(context);
		}
		else {
			spdlog::error("Connection error: can't allocate redis context");
		}
		//throw std::runtime_error("Connection error");
	}
	else {
		spdlog::info("Connected to redis server.");
	}
}

void RedisClient::close()
{
	redisFree(this->context);
	spdlog::info("Connection closed.");
}


