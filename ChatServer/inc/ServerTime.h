#ifndef SERVERTIME_H_
#define SERVERTIME_H_

#include <chrono>
#include <iostream>
#include <string>
#include <ctime>
#include "StringFormat.h"
#include "common.h"


class ServerTime
{
public:
	static std::time_t current_time;

	static std::tm time_info;

	int millisecondsPart;

public:
	ServerTime();
	~ServerTime();

	//現在のサーバの時刻を表示
	static std::string get_current_time();

	
	
};

#endif // !SERVERTIME_H_


