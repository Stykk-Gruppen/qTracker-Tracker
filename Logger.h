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
	//Logger operator<<(const Logger& l,std::string);
private:
	std::ofstream *outfile;
};
#endif