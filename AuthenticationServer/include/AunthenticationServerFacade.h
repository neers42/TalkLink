#ifndef AUNTHENTICATIONSERVERFACADE_H
#define AUNTHENTICATIONSERVERFACADE_H
#include <iostream>
#include <string>
#include <vector>
#include "common.h"
#include "SocketHandler.h"
#include "ServerTime.h"

class AunthenticationServerFacade
{

private:

	public:
		AunthenticationServerFacade();
		~AunthenticationServerFacade();
		void run();
		
};

#endif // !AUNTHENTICATIONSERVERFACADE_H

