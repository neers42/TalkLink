#ifndef DATABASEINTERFACE_H
#define DATABASEINTERFACE_H

class DataBaseInterface
{
public:
	virtual void connect() = 0;
	virtual void close() = 0;
	virtual ~DataBaseInterface() {};

};
#endif // !DATABASEINTERFACE_H

