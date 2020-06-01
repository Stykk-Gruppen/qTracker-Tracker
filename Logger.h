#ifndef LOGGER_H
#define LOGGER_H

#include <ctime>
#include <fstream>

class Logger
{
public:
	Logger();
	~Logger();
	void write(std::string);
	void timestamp();
	friend void operator<<(const Logger& l,std::string);
private:
	std::ofstream *outfile;
};
#endif