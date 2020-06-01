#include "Logger.h"

Logger::Logger()
{
	outfile = new std::ofstream("/var/log/qtracker/tracker.log",std::ios_base::app);
}

/*Logger operator<<(const Logger& l,std::string s)
{

}
*/
void Logger::write(std::string s)
{
	outfile->write();
	outfile->flush();
	//std::ofstream outfile;
  	//outfile.open("/var/log/qtracker/tracker.log", std::ios_base::app); // append instead of overwrite
  //	outfile << s; 
}

Logger::~Logger()
{
	delete outfile;
}