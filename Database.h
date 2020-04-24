#ifndef DATABASE_H
#define DATABASE_H

#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include "config.cpp"
#include "Torrent.h"
#include <math.h>

#include <boost/algorithm/string.hpp>

/*
  Include directly the different
  headers from cppconn/ and mysql_driver.h + mysql_util.h
  (and mysql_connection.h). This will reduce your build time!
*/
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

class Database
{
public:
	Database();
	std::string insertClientInfo(const std::vector<std::string*> &vectorOfArrays);
	Torrent getTorrent(std::string infoHash);
	std::string getErrorMessage();
private:
	std::string decode(std::string str);
	std::string urlDecode(std::string);
	std::vector<int> getTorrentData(std::string infoHash);
	std::vector<Peer*> getPeers(std::string infoHash);
	int parseEventString(std::string);
	int calcBonusPoints(int64_t torrentSizeBytes, int64_t newSeedMinutes, int64_t numberOfSeeders, int64_t totalSeedTimeMinutes);
	bool getUserId(std::string torrentPass, int *userId);
	bool userCanLeech(int userId);
	bool getClientId(std::string peerId, std::string ipa, int port, int userId, int *clientId, bool recursive);
	bool createClient(std::string peerId, std::string ipa, int port, int ipaId, int userId, int *clientId);
	bool updateClient(std::string peerId, std::string ipa, int port, int ipaId, int userId, int *clientId);
	bool updateTorrent(int torrentId, int event);
	bool createTorrent(int uploaderUserId, std::string infoHash, int *torrentId);
	bool torrentExists(std::string infoHash, int uploaderUserId, int *torrentId, bool recursive);
	bool ipaIsBanned(std::string ipa);
	bool getIpaId(std::string ipa, int userId, int *ipaId, bool recursive);
	bool createIpAddress(std::string ipa, int userId, int *ipaId);
	bool updateUserTorrentTotals(int clientId,int torrentId, int userId, uint64_t downloaded, uint64_t uploaded);
	bool createUserTorrentTotals(int torrentId, int userId, uint64_t downloaded, uint64_t uploaded);
	bool updateClientTorrents(std::string ipa, int port, int event, std::string infoHash,
		std::string peerId, uint64_t downloaded, uint64_t left, uint64_t uploaded,
		std::string torrentPass);
	bool createClientTorrent(int torrentId, int clientId, uint64_t downloaded, uint64_t left, uint64_t uploaded, int event);
	std::string errorMessage;
};

#endif