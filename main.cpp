#include "Server.h"
#include "bencode.hpp"
#include "Database.h"

using namespace bencode;

int main()
{
	//Database *d = new Database();
	//auto data = bencode::decode("i666e");
	//auto value = boost::get<bencode::integer>(data);
	Server s(1337);
	s.central();
	return 0;
}