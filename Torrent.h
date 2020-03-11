#ifndef TORRENT_H
#define TORRENT_H

#include <vector>
#include "Peer.h"



class Torrent
{
public:
	Torrent();

	int trackerId;
	std::vector<Peer> peers;
};

#endif