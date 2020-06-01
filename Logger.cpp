#include "Logger.h"

Logger::Logger()
{
	outfile = new std::ofstream("/var/log/qtracker/tracker.log",std::ios_base::app);
}

void Logger::write(std::string s)
{
	//write timestamp
	time_t now = time(0);
   	char* dt = ctime(&now);
	outfile->write(dt,24);

	//write separator
	const char* c = " : ";
	outfile->write(c,3);

	//write content
	int size = s.size();
	const char *cstr = s.c_str();
	outfile->write(cstr,size);

	//write newline
	const char* n = "\n";
	outfile->write(n,1);

	outfile->flush();
}

Logger::~Logger()
{
	delete outfile;
}