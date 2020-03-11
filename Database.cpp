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

//Return Id of Torrent in Database
int Database::insertClientInfo(const std::vector<std::string*> &vectorOfArrays)
{
    //insertIntoDB
    return 1;
}

bool Database::insertAnnounceLog(std::vector<std::string> vector)
{
    if(getUserId(vector[10], &vector[8]) && userCanLeech(vector[8]))
    {
        connect();
        sql::PreparedStatement *pstmt;
        pstmt = con->prepareStatement
        (
            "INSERT INTO announceLog (ipa, port, event, infoHash, peerId, downloaded, left, uploaded, userId, modifiedTime) Values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
        );
        for (int i = 0; i < 10; i++)
        {
            pstmt->setString((i + 1), vector[i]);
        }
        return (pstmt->executeQuery()) ? true : false;
    }
    else
    {
        return false;
    }
}

bool Database::updateAnnounceLog(std::vector<std::string> vector)
{
    connect();
    sql::PreparedStatement *pstmt;
    pstmt = con->prepareStatement
    (
        "UPDATE announceLog SET event = ?, downloaded = ?, left = ?, uploaded = ?, modifiedTime = NOW() WHERE infoHash = ? AND peerId = ?"
    );
    pstmt->setString(1, vector[2]); //event
    pstmt->setString(2, vector[5]); //downloaded
    pstmt->setString(3, vector[6]); //left
    pstmt->setString(3, vector[7]); //uploaded
    pstmt->setString(4, vector[3]); //infoHash
    pstmt->setString(5, vector[4]); //peerId
    if (pstmt->executeQuery())
    {
        return true;
    }
    else
    {
        return insertAnnounceLog(vector);
    }
}

bool Database::makeFile(std::string infoHash)
{
    //Holder på her nå
    connect();
    return false;
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

bool Database::getUserId(std::string torrentPass, std::string *userId)
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

bool Database::userCanLeech(std::string userId)
{
    connect();
    sql::PreparedStatement *pstmt;
    pstmt = con->prepareStatement("SELECT canLeech FROM user WHERE id = ?");
    pstmt->setString(1, userId);
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


