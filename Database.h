#ifndef DATABASE_H
#define DATABASE_H

#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include "config.cpp"
#include "Torrent.h"
#include "AnnounceInfo.h"
#include <math.h>
#include "Logger.h"
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
	~Database();
	std::string insertClientInfo(const std::vector<std::string*> &vectorOfArrays);
	Torrent getTorrent(std::string infoHash);
	std::string getErrorMessage();

private:
	Logger *logger;
	sql::Driver* driver;
	sql::Connection* con;
	sql::PreparedStatement* pstmt;
	sql::ResultSet* res;
	std::string errorMessage = "";
	AnnounceInfo* annInfo;

	std::string decode(std::string str);
	std::string urlDecode(std::string);
	std::vector<int> getTorrentData(std::string infoHash);
	std::vector<Peer*> getPeers(std::string infoHash);
	int parseEventString(std::string);
	double calcBonusPoints(uint64_t torrentSizeBytes, int newSeedMinutes, int numberOfSeeders, int totalSeedTimeMinutes);
	bool getUserId();
	bool userCanLeech();
	bool getClientId(bool recursive);
	bool createClient();
	bool updateClient();
	bool updateUserBonusPoints(int newSeedMinutes);
	bool updateTorrent();
	bool torrentExists();
	bool ipaIsBanned();
	bool getIpaId(bool recursive);
	bool createIpAddress();
	bool updateUserTorrentTotals();
	bool createUserTorrentTotals();
	bool updateClientTorrents();
	bool createClientTorrent();
	bool setOldUploadAndDownload();
};

#endif
