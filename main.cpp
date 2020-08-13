#include "Server.h"
#include "bencode.hpp"
#include "Database.h"

using namespace bencode;
/*! \mainpage QTracker-Tracker Documentation
 *
 * \section intro_sec Introduction
 *
* This document describes all the C++ classes used in our QTracker-Tracker.<br>
 * There are no ads on this page, but due to the unforunate name of the software,
 * adblockers will disable some functionality. Disable it to view the webpage as
 * intended.
 *
 */
int main()
{
	//Database *d = new Database();
	//auto data = bencode::decode("i666e");
	//auto value = boost::get<bencode::integer>(data);
	Server s(1337);
	s.central();
	return 0;
}