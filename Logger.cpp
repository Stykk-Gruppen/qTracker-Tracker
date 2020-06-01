#include "Logger.h"

Logger::Logger()
{
	outfile = new std::ofstream("/var/log/qtracker/tracker.log",std::ios_base::app);
}

void operator<<(const Logger& l,std::string s)
{
	*l.outfile << s;
	l.outfile->flush();
}


void Logger::timestamp(){
	time_t now = time(0);
   	char* dt = ctime(&now);
   	*outfile << dt << ":" << "\n";
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