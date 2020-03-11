#ifndef PEER_H
#define PEER_H
#include <string>

class Peer
{
public:
	Peer();
	Peer(std::string _peer_id, std::string _ip, int _port);

	std::string peer_id;
	std::string ip;
	int port;
};

#endif