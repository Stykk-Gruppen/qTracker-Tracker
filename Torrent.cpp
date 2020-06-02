#include "Torrent.h"


Torrent::Torrent()
{

}

Torrent::Torrent(int _trackerId, int _seeders, int _leechers): trackerId(_trackerId), seeders(_seeders), leechers(_leechers)
{
	
}

Torrent::~Torrent()
{
	for(auto peer : peers)
	{
		delete peer;
	}	
}