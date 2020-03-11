#ifndef TORRENT_H
#define TORRENT_H

#include <vector>
#include "Peer.h"



class Torrent
{
public:
	Torrent();
	Torrent(int _trackerId, int _seeders, int _leechers);
	int trackerId;
	int seeders;
	int leechers;
	int interval = 3;
	std::vector<Peer*> peers;
};

#endif