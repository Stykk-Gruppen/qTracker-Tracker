#ifndef PEER_H
#define PEER_H

class Peer
{
public:
	Peer(int _peer_id, int _ip, int _port);

	int peer_id;
	int ip;
	int port;
};

#endif