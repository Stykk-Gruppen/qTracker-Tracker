#include "Logger.h"

Logger::Logger()
{
	outfile = new std::ofstream("/var/log/qtracker/tracker.log",std::ios_base::app);
}

const Logger& operator<<(const Logger& l,std::string s)
{
	*l.outfile << s;
	l.outfile->flush();
	return l;
}


void Logger::timestamp(){
	time_t now = time(0);
   	char* dt = ctime(&now);
   	*outfile << "-----------TIMESTAMP : " << dt;
	outfile->flush();
}

void Logger::write(std::string s)
{
   	*outfile << s;
	outfile->flush();
}

Logger::~Logger()
{
	delete outfile;
}