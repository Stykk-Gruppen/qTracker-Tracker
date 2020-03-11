#ifndef DATABASE_H
#define DATABASE_H

#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include "config.cpp"
#include "Torrent.h"

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

class Database{
public:
	Database();
	//Return torrentId from DB.
	int insertClientInfo(const std::vector<std::string*> &vectorOfArrays);
	Torrent getTorrent(int torrentId);
	bool updateAnnounceLog(std::vector<std::string> vector);
	int getTorrentId(std::string infoHash);
	void connect();
private:
	bool insertAnnounceLog(std::vector<std::string> vector);
	bool getUserId(std::string torrentPass, std::string *userId);
	bool userCanLeech(std::string userId);
	bool makeFile(std::string infoHash);
	sql::Connection *con;
};

#endif