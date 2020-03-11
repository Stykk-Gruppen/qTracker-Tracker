#include "Server.h"
#include "bencode.hpp"
#include "Database.h"

using namespace bencode;

int main()
{
	Database *d = new Database();
	//int userId = 0;
	//d->getUserId("62293c115f1ffe3ca08fa4a31ee51a8d", &userId);
	//std::cout << userId;
	//auto data = bencode::decode("i666e");
	//auto value = boost::get<bencode::integer>(data);
	Server s(1337);
	s.central();
	return 0;
}