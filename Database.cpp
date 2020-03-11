#include "Database.h"

Database::Database()
{
    /*
    try {
        sql::Driver *driver;
        sql::Connection *con;
        sql::Statement *stmt;
        sql::ResultSet *res;

        // Create a connection
        driver = get_driver_instance();
        string t = "tcp://";
        t += dbHostName;
        t += ":3306";
        con = driver->connect(t, dbUserName, dbPassword);
        / Connect to the MySQL test database
        con->setSchema(dbDatabaseName);

        stmt = con->createStatement();
        res = stmt->executeQuery("SELECT username FROM user;");
        while (res->next()) {
            // Access column data by alias or column name 
            cout << res->getString("username") << endl;
            // Access column data by numeric offset, 1 is the first column 
            cout << res->getString(1) << endl;
        }
        delete res;
        delete stmt;
        delete con;

    } catch (sql::SQLException &e) {
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }
    */
}

void Database::connect()
{
	sql::Driver *driver;
    // Create a connection
	driver = get_driver_instance();
	std::string t = "tcp://";
	t += dbHostName;
	t += ":3306";
	con = driver->connect(t, dbUserName, dbPassword);
    // Connect to the MySQL test database
	con->setSchema(dbDatabaseName);
}

int Database::parseEventString(std::string event)
{
	boost::algorithm::to_lower(event);
	if(event.compare("stop") == 0)
	{
		return 3;
	}
	else if(event.compare("start") == 0)
	{
		return 2;
	}
	else if(event.compare("completed") == 0)
	{
		return 1;
	} 
	else
	{
		return 0;
	}
}

std::string Database::urlDecode(std::string urlEncodedString)
{
	//%8E%7C%7C%1Bt%2E%9D%C7%AE%7E%AA9%1CB%7D%19%14a%D6%0C
	std::string output = "";
	for(int i=0;i<urlEncodedString.length();i++)
	{
		
	}
}

//Return Id of Torrent in Database
int Database::insertClientInfo(const std::vector<std::string*> &vectorOfArrays)
{
	std::string ipa;
	int port;
	int event;
	std::string infoHash;
	std::string peerId;
	int downloaded;
	int left;
	int uploaded;
	std::string torrentPass;
	for(size_t x=0;x<vectorOfArrays.size();x++)
	{
		if(vectorOfArrays.at(x)[1].compare("ip") == 0)
		{
			ipa = vectorOfArrays.at(x)[1];
			continue;
		}
		if(vectorOfArrays.at(x)[1].compare("port") == 0)
		{
			port = stoi(vectorOfArrays.at(x)[1]);
			continue;
		}
		if(vectorOfArrays.at(x)[1].compare("event") == 0)
		{
			event = parseEventString(vectorOfArrays.at(x)[1]);
			continue;
		}
		if(vectorOfArrays.at(x)[1].compare("info_hash") == 0)
		{
			infoHash = vectorOfArrays.at(x)[1];
			continue;
		}
		if(vectorOfArrays.at(x)[1].compare("peer_id") == 0)
		{
			peerId = vectorOfArrays.at(x)[1];
			continue;
		}
		if(vectorOfArrays.at(x)[1].compare("downloaded") == 0)
		{
			downloaded = stoi(vectorOfArrays.at(x)[1]);
			continue;
		}
		if(vectorOfArrays.at(x)[1].compare("left") == 0)
		{
			left = stoi(vectorOfArrays.at(x)[1]);
			continue;
		}
		if(vectorOfArrays.at(x)[1].compare("uploaded") == 0)
		{
			uploaded = stoi(vectorOfArrays.at(x)[1]);
			continue;
		}
		if(vectorOfArrays.at(x)[1].compare("urlKey") == 0)
		{
			torrentPass = vectorOfArrays.at(x)[1];
			continue;
		}
	}
	return insertAnnounceLog(ipa,port,event,infoHash,peerId,downloaded,left,uploaded,torrentPass);
}

int Database::insertAnnounceLog(std::string ipa, int port, int event, std::string infoHash,
	std::string peerId, int downloaded, int left, int uploaded, std::string torrentPass)
{
	int userId = -1;
	if(getUserId(torrentPass, &userId) && userCanLeech(userId))
	{
		connect();
		sql::PreparedStatement *pstmt;
		pstmt = con->prepareStatement
		(
			"INSERT INTO announceLog (ipa, port, event, infoHash, peerId, downloaded, left, uploaded, userId, modifiedTime) Values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
			);
		pstmt->setString(1, ipa);
		pstmt->setInt(2, port);
		pstmt->setInt(3, event);
		pstmt->setString(4, infoHash);
		pstmt->setString(5, peerId);
		pstmt->setInt(6, downloaded);
		pstmt->setInt(7, left);
		pstmt->setInt(8, uploaded);
		pstmt->setInt(9, userId);
		if (pstmt->executeQuery())
		{
        	//mysql_num_rows(MYSQL_RES *result)
			makeFile(infoHash);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool Database::makeFile(std::string infoHash)
{
	connect();
	sql::PreparedStatement *pstmt;
	pstmt = con->prepareStatement("INSERT INTO files (infoHash) Values (?)");
	pstmt->setString(1, infoHash);
	return (pstmt->executeQuery()) ? true : false;
}

int Database::getTorrentId(std::string infoHash)
{
	connect();
	sql::PreparedStatement *pstmt;
	pstmt = con->prepareStatement("SELECT id FROM files WHERE infoHash = ?");
	pstmt->setString(1, infoHash);
	sql::ResultSet* res = pstmt->executeQuery();
	if (res->next())
	{
		return res->getInt("id");
	}
	else
	{
		return 0;
	}
}

Torrent Database::getTorrent(int torrentId)
{
    //Get torrent and its peers from db, by torrentId

    Torrent t(/*TrackerId*/1, /*seeders*/1, /*leechers*/1);
    /*for(int i = 0; i < 3; i++)
    {
        Peer p(i,i,i);
        t.peers.push_back(p);
    }*/

    //Lagt til for test. Må selvfølgelig loope gjennom db.
	Peer p1("-lt0D60-B8048EED9AE36nF2073D1", "5.79.98.209", 59130);
	t.peers.push_back(p1);
	Peer p2("-qB4170-f7j06WXZAX-I", "79.160.58.120", 8999);
	t.peers.push_back(p2);

	return t;
}

bool Database::getUserId(std::string torrentPass, int *userId)
{
	connect();
	sql::PreparedStatement *pstmt;
	pstmt = con->prepareStatement("SELECT id FROM user WHERE torrentPass = ?");
	pstmt->setString(1, torrentPass);
	sql::ResultSet* res = pstmt->executeQuery();
	if (res->next())
	{
		*userId = res->getInt("id");
		return true;
	}
	else
	{
		return false;
	}
	delete res;
	delete pstmt;
	delete con;
}

bool Database::userCanLeech(int userId)
{
	connect();
	sql::PreparedStatement *pstmt;
	pstmt = con->prepareStatement("SELECT canLeech FROM user WHERE id = ?");
	pstmt->setInt(1, userId);
	sql::ResultSet* res = pstmt->executeQuery();
	if (res->next())
	{
		return (res->getInt("canLeech")) ? true : false;
	}
	else
	{
		return false;
	}
	delete res;
	delete pstmt;
	delete con;
}

bool Database::updateAnnounceLog(std::string ipa, int port, int event, std::string infoHash,
	std::string peerId, int downloaded, int left, int uploaded, std::string torrentPass)
{
	connect();
	sql::PreparedStatement *pstmt;
	pstmt = con->prepareStatement
	(
		"UPDATE announceLog SET event = ?, downloaded = ?, left = ?, uploaded = ?, modifiedTime = NOW() WHERE infoHash = ? AND peerId = ?"
		);
	pstmt->setInt(1, event);
	pstmt->setInt(2, downloaded);
	pstmt->setInt(3, left);
	pstmt->setInt(3, uploaded);
	pstmt->setString(4, infoHash);
	pstmt->setString(5, peerId); 
	if (pstmt->executeQuery())
	{
		return true;
	}
	else
	{
		return insertAnnounceLog(ipa, port, event, infoHash,
			peerId, downloaded, left, uploaded, torrentPass);
	}
}

std::vector<Peer*> Database::getPeers(int torrentId)
{
	std::vector<Peer*> peers;
	connect();
	sql::PreparedStatement *pstmt;
	pstmt = con->prepareStatement("SELECT peerId, ipa, port FROM announceLog AS al, files AS f, filesUsers AS fu WHERE al.infoHash = f.infoHash AND f.fileId = fu.fileId AND fu.isActive = 1 ");
	sql::ResultSet* res = pstmt->executeQuery();
	while (res->next())
	{
		std::string peerId = res->getString("peerId");
		std::string ipa = res->getString("ipa");
		int port = res->getInt("port");
		peers.push_back(new Peer(peerId, ipa, port));
	}
	return peers;
}
