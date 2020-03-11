#ifndef PEER_H
#define PEER_H
#include <string>

class Peer
{
public:
	Peer();
	Peer(std::string _peer_id, std::string _ip, std::string _port);

	std::string peer_id;
	std::string ip;
	std::string port;
};

#endif