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

std::vector<int> Database::getFiles(std::string infoHash)
{
    std::vector<int> vector;
    try
    {
        sql::Driver *driver;
        sql::Connection *con;
        sql::PreparedStatement *pstmt;
        sql::ResultSet *res;

        // Create a connection
        driver = get_driver_instance();
        std::string t = "tcp://";
        t += dbHostName;
        t += ":3306";
        con = driver->connect(t, dbUserName, dbPassword);
        // Connect to the MySQL test database
        con->setSchema(dbDatabaseName);

        pstmt = con->prepareStatement("SELECT fileId, leechers, seeders FROM files WHERE infoHash = ?");
        pstmt->setString(1, infoHash);
        res = pstmt->executeQuery();
        while (res->next())
        {
            int fileId = res->getInt("fileId");
            int leechers = res->getInt("leechers");
            int seeders = res->getInt("seeders");
            vector.push_back(fileId);
            vector.push_back(leechers);
            vector.push_back(seeders);
        }
        return vector;
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::getFiles ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return vector;
    }
}

bool Database::updateFile(int fileId)
{
    try
    {
        sql::Driver *driver;
        sql::Connection *con;
        sql::PreparedStatement *pstmt;
        sql::ResultSet *res;

        // Create a connection
        driver = get_driver_instance();
        std::string t = "tcp://";
        t += dbHostName;
        t += ":3306";
        con = driver->connect(t, dbUserName, dbPassword);
        // Connect to the MySQL test database
        con->setSchema(dbDatabaseName);

        pstmt = con->prepareStatement
        (
            "UPDATE files "
            "SET "
            "seeders = (SELECT COUNT(isActive) FROM filesUsers WHERE fileId = ?), "
            "leechers = (SELECT COUNT(isActive) FROM filesUsers WHERE completed = 0 AND fileId = ?), "
            "completed = (SELECT COUNT(completed) FROM filesUsers WHERE fileId = ?) "
            "WHERE fileId = ?;"
        );
        pstmt->setInt(1, fileId);
        pstmt->setInt(2, fileId);
        pstmt->setInt(3, fileId);
        pstmt->setInt(4, fileId);
        if (pstmt->executeUpdate() > 0)
        {
            std::cout << "Updated file" << std::endl;
            return true;
        }
        else
        {
            std::cout << "Failed to update file" << std::endl;
            return false;
        }
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::updateFile ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

int Database::parseEventString(std::string event)
{
	boost::algorithm::to_lower(event);
	if(event.compare("stopped") == 0)
	{
		return 3;
	}
	else if(event.compare("started") == 0)
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
		if(urlEncodedString[i]=='%')
		{
			continue;
		}
		output+=urlEncodedString[i];
	}
	return output;
}

//Return infoHash if completed
std::string Database::insertClientInfo(const std::vector<std::string*> &vectorOfArrays)
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
		if(vectorOfArrays.at(x)[0].compare("ip") == 0)
		{
			ipa = vectorOfArrays.at(x)[1];
			continue;
		}
		if(vectorOfArrays.at(x)[0].compare("port") == 0)
		{
			port = stoi(vectorOfArrays.at(x)[1]);
			continue;
		}
		if(vectorOfArrays.at(x)[0].compare("event") == 0)
		{
             std::cout << "event before: " << vectorOfArrays.at(x)[1] << std::endl;
			event = parseEventString(vectorOfArrays.at(x)[1]);
            std::cout << "event: " << event << std::endl;
			continue;
		}
		if(vectorOfArrays.at(x)[0].compare("info_hash") == 0)
		{
			infoHash = urlDecode(vectorOfArrays.at(x)[1]);
			continue;
		}
		if(vectorOfArrays.at(x)[0].compare("peer_id") == 0)
		{
			peerId = urlDecode(vectorOfArrays.at(x)[1]);
			continue;
		}
		if(vectorOfArrays.at(x)[0].compare("downloaded") == 0)
		{
			downloaded = stoi(vectorOfArrays.at(x)[1]);
			continue;
		}
		if(vectorOfArrays.at(x)[0].compare("left") == 0)
		{
			left = stoi(vectorOfArrays.at(x)[1]);
			continue;
		}
		if(vectorOfArrays.at(x)[0].compare("uploaded") == 0)
		{
			uploaded = stoi(vectorOfArrays.at(x)[1]);
			continue;
		}
		if(vectorOfArrays.at(x)[0].compare("urlKey") == 0)
		{
			torrentPass = vectorOfArrays.at(x)[1];
			continue;
		}
	}
    std::cout << torrentPass << std::endl;
    if (updateAnnounceLog(ipa,port,event,infoHash,peerId,downloaded,left,uploaded,torrentPass))
    {
        return infoHash;
    }
    else
    {
        return NULL;
    }
	
}

int Database::insertAnnounceLog(std::string ipa, int port, int event, std::string infoHash,
	std::string peerId, int downloaded, int left, int uploaded, int userId)
{
	if(userCanLeech(userId))
	{
		try
        {
            sql::Driver *driver;
            sql::Connection *con;
            sql::PreparedStatement *pstmt;
            sql::ResultSet *res;

            // Create a connection
            driver = get_driver_instance();
            std::string t = "tcp://";
            t += dbHostName;
            t += ":3306";
            con = driver->connect(t, dbUserName, dbPassword);
            // Connect to the MySQL test database
            con->setSchema(dbDatabaseName);

            pstmt = con->prepareStatement
            (
            "INSERT INTO announceLog (ipa, port, event, infoHash, peerId, downloaded, `left`, uploaded, userId, modifiedTime) Values (?, ?, ?, ?, ?, ?, ?, ?, ?, NOW())"
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
                std::cout << "Added new announceLog" << std::endl;
                createFile(infoHash);
                createFilesUsers(getTorrentId(infoHash), userId, downloaded, uploaded, left);
                return true;
            }
            else
            {
                std::cout << "Failed to add new announceLog" << std::endl;
                return false;
            }
        }
        catch (sql::SQLException &e)
        {
            std::cout << "Database::insertAnnounceLog ";
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
            return false;
        }
	}
	else
	{
		return false;
	}
}

bool Database::createFile(std::string infoHash)
{
    try
    {
        sql::Driver *driver;
        sql::Connection *con;
        sql::PreparedStatement *pstmt;
        sql::ResultSet *res;

        // Create a connection
        driver = get_driver_instance();
        std::string t = "tcp://";
        t += dbHostName;
        t += ":3306";
        con = driver->connect(t, dbUserName, dbPassword);
        // Connect to the MySQL test database
        con->setSchema(dbDatabaseName);

        pstmt = con->prepareStatement("INSERT INTO files (infoHash) Values (?)");
        pstmt->setString(1, infoHash);
        return (pstmt->executeQuery()) ? true : false;
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::createFile ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

int Database::getTorrentId(std::string infoHash)
{
    try
    {
        sql::Driver *driver;
        sql::Connection *con;
        sql::PreparedStatement *pstmt;
        sql::ResultSet *res;

        // Create a connection
        driver = get_driver_instance();
        std::string t = "tcp://";
        t += dbHostName;
        t += ":3306";
        con = driver->connect(t, dbUserName, dbPassword);
        // Connect to the MySQL test database
        con->setSchema(dbDatabaseName);

        pstmt = con->prepareStatement("SELECT fileId FROM files WHERE infoHash = ?");
        pstmt->setString(1, infoHash);
        res = pstmt->executeQuery();
        if (res->next())
        {
            return res->getInt("fileId");
        }
        else
        {
            std::cout << "Failed to get torrent Id" << std::endl;
            return 0;
        }
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::getTorrentId ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

Torrent Database::getTorrent(std::string infoHash)
{
    //Get torrent and its peers from db, by torrentId
    std::vector<int> torrentData = getFiles(infoHash);

    Torrent t(/*TrackerId*/1, /*seeders*/torrentData[1], /*leechers*/torrentData[2]);

    t.peers = getPeers(torrentData[0]);
    /*
    //Lagt til for test. Må selvfølgelig loope gjennom db.
	Peer p1("-lt0D60-B8048EED9AE36nF2073D1", "5.79.98.209", 59130);
	t.peers.push_back(p1);
	Peer p2("-qB4170-f7j06WXZAX-I", "79.160.58.120", 8999);
	t.peers.push_back(p2);
    */

	return t;
}

bool Database::getUserId(std::string torrentPass, int *userId)
{
	try
    {
        sql::Driver *driver;
        sql::Connection *con;
        sql::PreparedStatement *pstmt;
        sql::ResultSet *res;

        // Create a connection
        driver = get_driver_instance();
        std::string t = "tcp://";
        t += dbHostName;
        t += ":3306";
        con = driver->connect(t, dbUserName, dbPassword);
        // Connect to the MySQL test database
        con->setSchema(dbDatabaseName);

    	pstmt = con->prepareStatement("SELECT id FROM user WHERE torrentPass = ?");
    	pstmt->setString(1, torrentPass);
    	res = pstmt->executeQuery();
    	if (res->next())
    	{
    		*userId = res->getInt("id");
            std::cout << "Got user Id" << std::endl;
            delete pstmt;
            delete res;
    		return true;
    	}
    	else
    	{
            std::cout << "Couldn't get user Id" << std::endl;
            delete pstmt;
            delete res;
    		return false;
    	}
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::getUserId ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }

}

bool Database::userCanLeech(int userId)
{
	try
    {
        sql::Driver *driver;
        sql::Connection *con;
        sql::PreparedStatement *pstmt;
        sql::ResultSet *res;

        // Create a connection
        driver = get_driver_instance();
        std::string t = "tcp://";
        t += dbHostName;
        t += ":3306";
        con = driver->connect(t, dbUserName, dbPassword);
        // Connect to the MySQL test database
        con->setSchema(dbDatabaseName);

        pstmt = con->prepareStatement("SELECT canLeech FROM user WHERE id = ?");
        pstmt->setInt(1, userId);
        res = pstmt->executeQuery();
        if (res->next())
        {
            std::cout << "User can leech" << std::endl;
            return (res->getInt("canLeech")) ? true : false;
        }
        else
        {
            std::cout << "User can't leech" << std::endl;
            return false;
        }
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::userCanLeech ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool Database::updateAnnounceLog(std::string ipa, int port, int event, std::string infoHash,
	std::string peerId, int downloaded, int left, int uploaded, std::string torrentPass)
{
    int userId = -1;
    if (getUserId(torrentPass, &userId))
    {
    	try
        {
            sql::Driver *driver;
            sql::Connection *con;
            sql::PreparedStatement *pstmt;
            sql::ResultSet *res;

            // Create a connection
            driver = get_driver_instance();
            std::string t = "tcp://";
            t += dbHostName;
            t += ":3306";
            con = driver->connect(t, dbUserName, dbPassword);
            // Connect to the MySQL test database
            con->setSchema(dbDatabaseName);

            pstmt = con->prepareStatement
            (
            "UPDATE announceLog SET event = ?, downloaded = ?, `left` = ?, uploaded = ?, modifiedTime = NOW() WHERE infoHash = ? AND peerId = ?"
            );
            pstmt->setInt(1, event);
            pstmt->setInt(2, downloaded);
            pstmt->setInt(3, left);
            pstmt->setInt(4, uploaded);
            pstmt->setString(5, infoHash);
            pstmt->setString(6, peerId); 
            if (pstmt->executeUpdate() > 0)
            {
                std::cout << "Updated announceLog" << std::endl;
                updateFilesUsers(getTorrentId(infoHash), userId, downloaded, uploaded, left, event);
                return true;
            }
            else
            {
                std::cout << "Failed to update announceLog. Will try to create one instead." << std::endl;
                return insertAnnounceLog(ipa, port, event, infoHash,
                    peerId, downloaded, left, uploaded, userId);
            }
        }
        catch (sql::SQLException &e)
        {
            std::cout << "Database::updateAnnounceLog ";
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
            return false;
        }
    }
    else
    {
        return false;
    }
}

std::vector<Peer*> Database::getPeers(int fileId)
{
	std::vector<Peer*> peers;
	try
    {
        sql::Driver *driver;
        sql::Connection *con;
        sql::PreparedStatement *pstmt;
        sql::ResultSet *res;

        // Create a connection
        driver = get_driver_instance();
        std::string t = "tcp://";
        t += dbHostName;
        t += ":3306";
        con = driver->connect(t, dbUserName, dbPassword);
        // Connect to the MySQL test database
        con->setSchema(dbDatabaseName);

        pstmt = con->prepareStatement("SELECT DISTINCT(peerId), ipa, port FROM announceLog AS al, files AS f, filesUsers AS fu WHERE al.infoHash = f.infoHash AND f.fileId = fu.fileId AND fu.isActive = 1 AND f.fileId = ?");
        pstmt->setInt(1, fileId);
        res = pstmt->executeQuery();
        while (res->next())
        {
            std::string peerId = res->getString("peerId");
            std::string ipa = res->getString("ipa");
            int port = res->getInt("port");
            peers.push_back(new Peer(peerId, ipa, port));
        }
        return peers;
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::getPeers ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return peers;
    }
}

bool Database::createFilesUsers(int fileId, int userId, int downloaded, int uploaded, int left)
{
    try
    {
        sql::Driver *driver;
        sql::Connection *con;
        sql::PreparedStatement *pstmt;
        sql::ResultSet *res;

        // Create a connection
        driver = get_driver_instance();
        std::string t = "tcp://";
        t += dbHostName;
        t += ":3306";
        con = driver->connect(t, dbUserName, dbPassword);
        // Connect to the MySQL test database
        con->setSchema(dbDatabaseName);

        pstmt = con->prepareStatement
        (
            "INSERT INTO filesUsers"
            "(fileId, userId, downloaded, uploaded, `left`)"
            "VALUES (?, ?, ?, ?, ?)"
        );
        pstmt->setInt(1, fileId);
        pstmt->setInt(2, userId);
        pstmt->setInt(3, downloaded);
        pstmt->setInt(4, uploaded);
        pstmt->setInt(5, left);
        std::cout << pstmt << std::endl;
        if( pstmt->executeQuery())
        {
            updateFile(fileId);
            std::cout << "Created fileUsers" << std::endl;
            return true;
        }
        else
        {
            std::cout << "Failed to create fileUsers" << std::endl;
            return false;
        }
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::createFilesUsers ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool Database::updateFilesUsers(int fileId, int userId, int downloaded, int uploaded, int left, int event)
{
    try
    {
        sql::Driver *driver;
        sql::Connection *con;
        sql::PreparedStatement *pstmt;
        sql::ResultSet *res;

        // Create a connection
        driver = get_driver_instance();
        std::string t = "tcp://";
        t += dbHostName;
        t += ":3306";
        con = driver->connect(t, dbUserName, dbPassword);
        // Connect to the MySQL test database
        con->setSchema(dbDatabaseName);

        pstmt = con->prepareStatement
        (
            "UPDATE filesUsers "
            "SET " 
            "isActive = IF(? < 3, 1, 0), "
            "announced = announced + 1, "
            "completed = IF(? = 0, 1, 0), "
            "downloaded = IF(downloaded > ?, downloaded + ?, ?), "
            "uploaded = IF(uploaded > ?, uploaded + ?, ?), "
            "`left` = ?, "
            "modifiedTime = NOW() "
            "WHERE fileId = ? AND userId = ?"
        );

        //Hvis man bare kunne brukte :downloaded osv..
        pstmt->setInt(1, event);
        pstmt->setInt(2, left);
        pstmt->setInt(3, downloaded);
        pstmt->setInt(4, downloaded);
        pstmt->setInt(5, downloaded);
        pstmt->setInt(6, uploaded);
        pstmt->setInt(7, uploaded);
        pstmt->setInt(8, uploaded);
        pstmt->setInt(9, left);
        pstmt->setInt(10, fileId);
        pstmt->setInt(11, userId);
        if (pstmt->executeUpdate() > 0)
        {
            std::cout << "Updated fileUsers" << std::endl;
            updateFile(fileId);
            return true;
        }
        else
        {
            std::cout << "Failed to update fileUsers. Will try to create one instead." << std::endl;
            return createFilesUsers(fileId, userId, downloaded, uploaded, left);
        } 
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::updateFilesUsers ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

