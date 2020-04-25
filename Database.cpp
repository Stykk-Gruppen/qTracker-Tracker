#include "Database.h"

Database::Database()
{
    try
    {
        // Create a connection
        driver = get_driver_instance();
        std::string t = "tcp://";
        t += dbHostName;
        t += ":3306";
        con = driver->connect(t, dbUserName, dbPassword);
        // Connect to the MySQL test database
        con->setSchema(dbDatabaseName);
    }
    catch (sql::SQLException &e) {
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }
}

Database::~Database()
{
    delete con;
    delete pstmt;
    delete res;
    delete annInfo;
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
        std::cout << "event = Bump (0)" << std::endl;
        return 0;
    }
}

std::string Database::decode(std::string str)
{
    std::cout << std::endl << "-------- (Stian Decoder) --------" << std::endl << std::endl;
    std::string decoded = "";
    for (int i = 0; i < str.size(); i++)
    {
        if (str[i] != '%')
        {
            std::ostringstream os;
            os << std::hex << (int)str[i];
            decoded += os.str();
            std::cout << str[i] << " ---> " << os.str() << std::endl;
        }
        else
        {
            decoded += std::tolower(str[i+1]);
            decoded += std::tolower(str[i+2]);
            i = i + 2;
        }
    }
    std::cout << "Size: " << decoded.size() << std::endl;
    std::cout << decoded << std::endl;
    std::cout << std::endl << "-------- (Stian Decoder) --------" << std::endl << std::endl;
    return decoded;
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
    std::string infoHash;
    std::string peerId;
    std::string torrentPass;
    int port;
    int event = 0;
    int64_t downloaded;
    int64_t left;
    int64_t uploaded;
    
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
            std::cout << "event = " << vectorOfArrays.at(x)[1];
            event = parseEventString(vectorOfArrays.at(x)[1]);
            std::cout << " (" << event << ")" << std::endl;
            continue;
        }
        if(vectorOfArrays.at(x)[0].compare("info_hash") == 0)
        {
            infoHash = decode(vectorOfArrays.at(x)[1]);
            continue;
        }
        if(vectorOfArrays.at(x)[0].compare("peer_id") == 0)
        {
            peerId = vectorOfArrays.at(x)[1];
            continue;
        }
        if(vectorOfArrays.at(x)[0].compare("downloaded") == 0)
        {
            downloaded = stoll(vectorOfArrays.at(x)[1]);
            continue;
        }
        if(vectorOfArrays.at(x)[0].compare("left") == 0)
        {
            left = stoll(vectorOfArrays.at(x)[1]);
            continue;
        }
        if(vectorOfArrays.at(x)[0].compare("uploaded") == 0)
        {
            uploaded = stoll(vectorOfArrays.at(x)[1]);
            continue;
        }
        if(vectorOfArrays.at(x)[0].compare("urlKey") == 0)
        {
            torrentPass = vectorOfArrays.at(x)[1];
            continue;
        }
    }
    std::cout << torrentPass << std::endl;

    annInfo = new AnnounceInfo(ipa, infoHash, peerId, torrentPass, event, port, downloaded, left, uploaded);

    if (updateClientTorrents())
    {
        return infoHash;
    }
    else
    {
        return "";
    }
}

std::vector<int> Database::getTorrentData(std::string infoHash)
{
    std::vector<int> vec;
    try
    {
        pstmt = con->prepareStatement
        (
            "SELECT "
                    "(SELECT IFNULL(SUM(isActive), 0) FROM clientTorrents AS ct WHERE ct.torrentId = torrent.id "
                    "AND ct.left > 0) AS 'leechers', "
                    "(SELECT IFNULL(SUM(isActive), 0) FROM clientTorrents AS ct WHERE ct.torrentId = torrent.id "
                    "AND (TIMESTAMPDIFF(MINUTE, ct.lastActivity, NOW()) < 60)) AS 'seeders' "
            "FROM "
                    "torrent "
            "WHERE "
                "   infoHash = ?"
        );
        pstmt->setString(1, infoHash);
        res = pstmt->executeQuery();
        if (res->next())
        {
            vec.push_back(res->getInt("seeders"));
            vec.push_back(res->getInt("leechers"));
            return vec;
        }
        else
        {
            std::cout << "Failed to get torrent Id" << std::endl;
            return vec;
        }
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::getTorrentId ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return vec;
    }
}

Torrent Database::getTorrent(std::string infoHash)
{
    std::vector<int> torrentData = getTorrentData(infoHash);

    Torrent t(/*TrackerId*/1, /*seeders*/torrentData[0], /*leechers*/torrentData[1]);

    t.peers = getPeers(infoHash);

    return t;
}

bool Database::getUserId()
{
    try
    {
        pstmt = con->prepareStatement("SELECT id FROM user WHERE torrentPass = ?");
        pstmt->setString(1, annInfo->getTorrentPass());
        res = pstmt->executeQuery();
        if (res->next())
        {
            int userId = res->getInt("id");
            annInfo->setUserId(userId);
            std::cout << "Valid User ID" << std::endl;
            return true;
        }
        else
        {
            std::cout << "Invalid User ID" << std::endl;
            errorMessage = "User not found!";
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

bool Database::userCanLeech()
{
    try
    {
        pstmt = con->prepareStatement("SELECT canLeech FROM user WHERE id = ?");
        pstmt->setInt(1, annInfo->getUserId());
        res = pstmt->executeQuery();
        if (res->next())
        {
            std::cout << "User can leech" << std::endl;
            return (res->getInt("canLeech")) ? true : false;
        }
        else
        {
            std::cout << "User can't leech" << std::endl;
            errorMessage = "You do not have the privilege to leech!";
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

std::vector<Peer*> Database::getPeers(std::string infoHash)
{
    std::vector<Peer*> peers;
    try
    {
        pstmt = con->prepareStatement
        (
            "SELECT "
                    "peerId,"
                    "ipa, "
                    "port "
            "FROM "
                    "torrent AS t, "
                    "clientTorrents AS ct, "
                    "client AS c, "
                    "ipAddress AS ip "
            "WHERE "
                    "t.id = ct.torrentId AND "
                    "ct.isActive = 1 AND "
                    "(TIMESTAMPDIFF(MINUTE, ct.lastActivity, NOW()) < 60) AND "
                    "ct.clientId = c.id AND "
                    "c.ipaId = ip.id AND "
                    "t.infoHash = ?"
        );
        pstmt->setString(1, infoHash);
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

bool Database::torrentExists()
{
    try
    {
        pstmt = con->prepareStatement("SELECT id FROM torrent WHERE infoHash = ?");
        pstmt->setString(1, annInfo->getInfoHash());
        res = pstmt->executeQuery();
        if (res->next())
        {
            int torrentId = res->getInt("id");
            annInfo->setTorrentId(torrentId);
            std::cout << "Found existing torrent" << std::endl;
            return true;
        }
        else
        {
            std::cout << "Torrent doesn't exist. Something went wrong. info hash: "<< annInfo->getInfoHash() << std::endl;
            errorMessage = "Torrent doesn't exist!";
            return false;
        }
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::torrentExists ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool Database::ipaIsBanned()
{
    try
    {
        pstmt = con->prepareStatement("SELECT isBanned FROM ipAddress WHERE ipa = ?");
        pstmt->setString(1, annInfo->getIpa());
        res = pstmt->executeQuery();
        if(res->next())
        {
            int isBanned = res->getInt("isBanned");
            if (isBanned == 0)
            {
                std::cout << "Valid IP Address" << std::endl;
                return false;
            }
            else
            {
                std::cout << "Banned IP Address " << std::endl;
                errorMessage = "You are using a banned IP Address!";
                return true;
            }
        }
        else
        {
            std::cout << "IP Address does not exist, so user is not banned" << std::endl;
            return false;
        }
        
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::ipaIsBanned ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return true;
    }
}

bool Database::updateClientTorrents()
{
    //int userId, torrentId, clientId = -1;
    if (getUserId() && userCanLeech() && !ipaIsBanned())
    {
        if(torrentExists())
        {
            if (getClientId(true))
            {
                try
                {
                    //Get new seed minutes for later bonus point calc:
                    pstmt = con->prepareStatement
                    (
                        "SELECT "
                            "TIMESTAMPDIFF(MINUTE, lastActivity, NOW()) AS 'newSeedMinutes' "
                        "FROM "
                            "clientTorrents "
                        "WHERE "
                            "torrentId = ? "
                        "AND "
                            "clientId = ? "
                    );
                    pstmt->setInt(1, annInfo->getTorrentId());
                    pstmt->setInt(2, annInfo->getClientId());
                    int newSeedMinutes = 0;
                    res = pstmt->executeQuery();
                    if(res->next())
                    {
                        newSeedMinutes = res->getInt("newSeedMinutes");
                    }


                    pstmt = con->prepareStatement
                            (
                                "UPDATE "
                                        "clientTorrents AS ct, "
                                        "client AS c, "
                                        "ipAddress AS ip, "
                                        "user AS u "
                                "SET "
                                        "timeActive = IF(? = 2, timeActive, IF(isActive = 1, timeActive + TIMESTAMPDIFF(MINUTE, lastActivity, NOW()), timeActive)), "
                                        "isActive = IF(? < 3, 1, 0), "
                                        "announced = announced + 1, "
                                        "completed = IF(? = 1, completed + 1, completed), "
                                        "downloaded = ?, "
                                        "`left` = ?, "
                                        "uploaded = ?, "
                                        "lastEvent = ?, "
                                        "lastActivity = NOW(), "
                                        "clientId = ? "
                                "WHERE "
                                        "torrentId = ? "
                                        "AND ct.clientId = c.id "
                                        "AND c.ipaId = ip.id "
                                        "AND ip.userId = u.id "
                                        "AND u.torrentPass = ?"
                            );
                    pstmt->setInt(1, annInfo->getEvent());
                    pstmt->setInt(2, annInfo->getEvent());
                    pstmt->setInt(3, annInfo->getEvent());
                    pstmt->setUInt64(4, annInfo->getDownloaded());
                    pstmt->setUInt64(5, annInfo->getLeft());
                    pstmt->setUInt64(6, annInfo->getUploaded());
                    pstmt->setInt(7, annInfo->getEvent());
                    pstmt->setInt(8, annInfo->getClientId());
                    pstmt->setInt(9, annInfo->getTorrentId());
                    pstmt->setString(10, annInfo->getTorrentPass());
                    if (pstmt->executeUpdate() <= 0)
                    {
                        std::cout << "clientTorrent doesn't exist. Will create one. " << std::endl;
                        if (!createClientTorrent())
                        {
                            return false;
                        }
                    }

                    if(!updateUserBonusPoints(newSeedMinutes))
                        std::cout << "An error occored within updateUserBonusPoints" << std::endl;

                    if (!updateUserTorrentTotals())
                    {
                        return false;
                    }
                    return true;
                }
                catch (sql::SQLException &e)
                {
                    std::cout << "Database::updateClientTorrents ";
                    std::cout << " (MySQL error code: " << e.getErrorCode();
                    std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
                    return true;
                }
            }
        }
    }
    return false;
}

bool Database::updateUserBonusPoints(int newSeedMinutes)
{
    try
    {
        //Bonus point-calc
        pstmt = con->prepareStatement
        (
            "SELECT "
                "timeActive, " 
                "(SELECT IFNULL(SUM(isActive), 0) FROM clientTorrents AS ct WHERE ct.torrentId = torrentId "
                "AND (TIMESTAMPDIFF(MINUTE, ct.lastActivity, NOW()) < 60)) AS 'seeders', "
                "(SELECT SUM(length) FROM torrentFiles AS tf WHERE tf.torrentId = torrentId) AS 'size' "
            "FROM "
                "clientTorrents "
            "WHERE "
                "torrentId = ? "
            "AND "
                "clientId = ? "
        );
        pstmt->setInt(1, annInfo->getTorrentId());
        pstmt->setInt(2, annInfo->getClientId());
        double bonusPointIncrement = 0;
        res = pstmt->executeQuery();
        //If found, get calculations for BP
        if(res->next())
        {
            int totalTimeActive = res->getInt("timeActive");
            int seeders = res->getInt("seeders");
            uint64_t size = res->getUInt64("size");
            if(seeders != 0)
            {
                bonusPointIncrement = calcBonusPoints(size, newSeedMinutes, seeders, totalTimeActive);
            }
        }
        //Update bonus points
        pstmt = con->prepareStatement
        (
            "UPDATE user SET points = points + ? WHERE id = ?"
        );
        pstmt->setInt(1, bonusPointIncrement);
        pstmt->setInt(2, annInfo->getUserId());
        if(pstmt->executeUpdate() <= 0)
        {
            std::cout << "Added " << bonusPointIncrement << " to user: " << annInfo->getUserId() << std::endl;
            return true;
        }
        return false;
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::updateUserBonusPoints ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool Database::createClientTorrent()
{
    try
    {
        pstmt = con->prepareStatement
                (
                    "INSERT INTO clientTorrents "
                    "(torrentId, clientId, downloaded, `left`, uploaded, lastEvent, lastActivity) "
                    "VALUES (?, ?, ?, ?, ?, ?, NOW())"
                    );
        pstmt->setInt(1, annInfo->getTorrentId());
        pstmt->setInt(2, annInfo->getClientId());
        pstmt->setUInt64(3, annInfo->getDownloaded());
        pstmt->setUInt64(4, annInfo->getLeft());
        pstmt->setUInt64(5, annInfo->getUploaded());
        pstmt->setInt(6, annInfo->getEvent());
        if (pstmt->executeQuery())
        {
            std::cout << "Created new clientTorrents " << std::endl;
            return true;
        }
        else
        {
            std::cout << "Failed creating new clientTorrents " << std::endl;
            return false;
        }
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::createClientTorrent ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool Database::getIpaId(bool recursive)
{
    try
    {
        pstmt = con->prepareStatement("SELECT id FROM ipAddress WHERE ipa = ?");
        pstmt->setString(1, annInfo->getIpa());
        res = pstmt->executeQuery();
        if (res->next())
        {
            int ipaId = res->getInt("id");
            annInfo->setIpaId(ipaId);
            std::cout << "Using known IP Address " << std::endl;
            return true;
        }
        else
        {
            std::cout << "Using unknown IP Address. Will insert into database " << std::endl;
            if (recursive)
            {
                return createIpAddress();
            }
            else
            {
                return false;
            }
        }
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::getIpaId ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool Database::getClientId(bool recursive)
{
    if (getIpaId(true))
    {
        try
        {
            pstmt = con->prepareStatement("SELECT id FROM client WHERE peerId = ? AND port = ? AND ipaId = ?");
            pstmt->setString(1, annInfo->getPeerId());
            pstmt->setInt(2, annInfo->getPort());
            pstmt->setInt(3, annInfo->getIpaId());
            res = pstmt->executeQuery();
            if (res->next())
            {
                int clientId = res->getInt("id");
                annInfo->setClientId(clientId);
                std::cout << "Using a known client" << std::endl;
                return true;
            }
            else
            {
                std::cout << "Client unknown. Will try to update or create one. " << std::endl;
                if (recursive)
                {
                    return updateClient();
                }
                else
                {
                    return false;
                }
                
            }
        }
        catch (sql::SQLException &e)
        {
            std::cout << "Database::getClientId ";
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
            return false;
        }
    }
    else
    {
        std::cout << "Should never end up here.." << std::endl;
        return false;
    }
    
}

bool Database::createIpAddress()
{
    try
    {
        pstmt = con->prepareStatement("INSERT INTO ipAddress (ipa, userId) VALUES (?, ?)");
        pstmt->setString(1, annInfo->getIpa());
        pstmt->setInt(2, annInfo->getUserId());
        if (pstmt->executeQuery())
        {
            std::cout << "IP Address added to Database. Will do recursive function to get ipaId" << std::endl;
            return getIpaId(false);
        }
        else
        {
            std::cout << "Failed adding IP Address to the Database " << std::endl;
            return false;
        }
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::createIpAddress ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool Database::createClient()
{
    try
    {
        pstmt = con->prepareStatement("INSERT IGNORE INTO client (peerId, port, ipaId) VALUES (?, ?, ?)");
        pstmt->setString(1, annInfo->getPeerId());
        pstmt->setInt(2, annInfo->getPort());
        pstmt->setInt(3, annInfo->getIpaId());
        if (pstmt->executeQuery())
        {
            std::cout << "Added new client to the Database. Will do recursive function to get clientId" << std::endl;
            return getClientId(false);
        }
        else
        {
            std::cout << "Failed adding new client to the Database " << std::endl;
            return false;
        }
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::createClient ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool Database::updateUserTorrentTotals()
{
    try
    {
        pstmt = con->prepareStatement
                (
                    "SELECT downloaded,uploaded, timeActive FROM "
                    "clientTorrents "
                    "WHERE torrentId = ? "
                    "AND clientId = ?"
                    );
        pstmt->setInt(1, annInfo->getTorrentId());
        pstmt->setInt(2, annInfo->getClientId());
        res = pstmt->executeQuery();
        if(res->next())
        {
            uint64_t oldDownloaded = res->getUInt64("downloaded");
            uint64_t oldUploaded = res->getUInt64("uploaded");

            uint64_t downloadedTotalIncrement = annInfo->getDownloaded() - oldDownloaded;
            uint64_t uploadedTotalIncrement = annInfo->getUploaded() - oldUploaded;

            //Checks if torrentId,userId combo exists before update
            pstmt = con->prepareStatement
                    (
                        "SELECT 1 FROM "
                        "userTorrentTotals "
                        "WHERE torrentId = ? "
                        "AND userId = ?"
                        );
            pstmt->setInt(1, annInfo->getTorrentId());
            pstmt->setInt(2, annInfo->getUserId()); 
            res = pstmt->executeQuery();
            if(res->next())
            {
                int boolTest = res->getInt("1");
                if(boolTest==1)
                {

                    /* If user har restartet the same torrent we do not
                    want to update the total with a negative number */
                    //if(downloadedTotalIncrement <= 0)
                    if(downloadedTotalIncrement < 0)
                    {
                        downloadedTotalIncrement = annInfo->getDownloaded();
                    }
                    //if(uploadedTotalIncrement<=0)
                    if(uploadedTotalIncrement < 0)
                    {
                        uploadedTotalIncrement = annInfo->getUploaded();
                    }
                    std::string query = "UPDATE userTorrentTotals SET "
                                        "totalDownloaded = totalDownloaded + ?,"
                                        "totalUploaded = totalUploaded + ? "
                                        "WHERE torrentId = ? AND userId = ?;";
                    //std::cout << "query: \n" << query << std::endl;
                    pstmt = con->prepareStatement(query);
                    pstmt->setUInt64(1, downloadedTotalIncrement);
                    pstmt->setUInt64(2, uploadedTotalIncrement);
                    pstmt->setInt(3, annInfo->getTorrentId());
                    pstmt->setInt(4, annInfo->getUserId());
                    pstmt->executeUpdate();
                    std::cout << "Updated userTorrentTotals in the Database" << std::endl;
                    return true;
                }               
            }
        }
        std::cout << "Failed to update userTorrentTotals in the Database. Will create one " << std::endl;
        return createUserTorrentTotals();
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::updateUserTorrentTotals ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool Database::createUserTorrentTotals()
{
    try
    {
        pstmt = con->prepareStatement("INSERT INTO userTorrentTotals (torrentId, userId, totalDownloaded, totalUploaded) VALUES (?, ?, ?, ?)");
        pstmt->setInt(1, annInfo->getTorrentId());
        pstmt->setInt(2, annInfo->getUserId());
        pstmt->setUInt64(3, annInfo->getDownloaded());
        pstmt->setUInt64(4, annInfo->getUploaded());
        if (pstmt->executeQuery())
        {
            std::cout << "Added new userTorrentTotals to the Database" << std::endl;
            return true;
        }
        else
        {
            std::cout << "Failed adding new userTorrentTotals to the Database " << std::endl;
            return false;
        }
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::createUserTorrentTotals ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

double Database::calcBonusPoints(uint64_t torrentSizeBytes, int newSeedMinutes, int numberOfSeeders, int totalSeedTimeMinutes)
{
    const double bytesInGB = 1000000000.0;
    const double minutesInHour = 60.0;
    const double minutesInDay = 1400.0;
    double torrentSizeGb =  torrentSizeBytes / bytesInGB;
    double newSeedHours = newSeedMinutes / minutesInHour;
    double totalSeedTimeDays = totalSeedTimeMinutes / minutesInDay;

    return (torrentSizeGb * (0.025 + (0.6 * log(1 + totalSeedTimeDays) / (pow(numberOfSeeders, 0.6)))))*newSeedHours;
}

bool Database::updateClient()
{
    try
    {
        pstmt = con->prepareStatement
                (
                    "UPDATE client "
                    "SET "
                    "peerId = ? "
                    "WHERE "
                    "ipaId = ? AND "
                    "port = ?"
                );
        pstmt->setString(1, annInfo->getPeerId());
        pstmt->setInt(2, annInfo->getIpaId());
        pstmt->setInt(3, annInfo->getPort());
        if (pstmt->executeUpdate() > 0)
        {
            std::cout << "Updated Client. Will do recursive function to get clientId" << std::endl;
            return getClientId(false);
        }
        else
        {
            std::cout << "Failed to update Client. Will create one" << std::endl;
            return createClient();
        }
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Database::updateClient ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

std::string Database::getErrorMessage()
{
    return errorMessage;
}
