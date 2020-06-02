#include "Database.h"

Database::Database(Logger* _logger): logger(_logger)
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
        logger->timestamp();
       // std::to_string(3.1415926)
        *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
        *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
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
        std::cout << "event = Bump (0)" << "\n";
        *logger << "event = Bump (0)" << "\n";
        return 0;
    }
}

std::string Database::decode(std::string str)
{
    std::cout << "\n" << "-------- (Stian Decoder) --------" << "\n" << "\n";
    *logger << "\n" << "-------- (Stian Decoder) --------" << "\n" << "\n";
    std::string decoded = "";
    for (int i = 0; i < str.size(); i++)
    {
        if (str[i] != '%')
        {
            std::ostringstream os;
            os << std::hex << (int)str[i];
            decoded += os.str();
            std::string s;
            s.push_back(str[i]);
            std::cout << str[i] << " ---> " << os.str() << "\n";
            *logger << s << " ---> " << os.str() << "\n";
        }
        else
        {
            decoded += std::tolower(str[i+1]);
            decoded += std::tolower(str[i+2]);
            i = i + 2;
        }
    }
    std::cout << "Size: " << decoded.size() << "\n";
    std::cout << decoded << "\n";
    std::cout << "\n" << "-------- (Stian Decoder) --------" << "\n" << "\n";
    *logger << "Size: " << std::to_string(decoded.size()) << "\n";
    *logger << decoded << "\n";
    *logger << "\n" << "-------- (Stian Decoder) --------" << "\n" << "\n";
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
    //*logger <<
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
            *logger << "event = " << vectorOfArrays.at(x)[1];
            event = parseEventString(vectorOfArrays.at(x)[1]);
            std::cout << " (" << event << ")" << "\n";
            *logger << " (" << std::to_string(event) << ")" << "\n";
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
    std::cout << torrentPass << "\n";
    *logger << torrentPass << "\n";

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
            std::cout << "Failed to get torrent Id" << "\n";
            *logger << "Failed to get torrent Id" << "\n";
            return vec;
        }
    }
    catch (sql::SQLException &e)
    {
        *logger << "Database::getTorrentId ";
        *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
        *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
        std::cout << "Database::getTorrentId ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
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
            std::cout << "Valid User ID" << "\n";
            *logger << "Valid User ID" << "\n";
            return true;
        }
        else
        {
            std::cout << "Invalid User ID" << "\n";
            *logger << "Invalid User ID" << "\n";
            errorMessage = "User not found!";
            return false;
        }
    }
    catch (sql::SQLException &e)
    {
        *logger << "Database::getUserId ";
        *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
        *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
        std::cout << "Database::getUserId ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
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
            *logger << "User can leech" << "\n";
            std::cout << "User can leech" << "\n";
            return (res->getInt("canLeech")) ? true : false;
        }
        else
        {
            *logger << "User can't leech" << "\n";
            std::cout << "User can't leech" << "\n";
            errorMessage = "You do not have the privilege to leech!";
            return false;
        }
    }
    catch (sql::SQLException &e)
    {
        *logger << "Database::userCanLeech ";
        *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
        *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
        std::cout << "Database::userCanLeech ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
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
        *logger << "Database::getPeers ";
        *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
        *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
        std::cout << "Database::getPeers ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
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
            *logger << "Found existing torrent" << "\n";
            std::cout << "Found existing torrent" << "\n";
            return true;
        }
        else
        {
            *logger << "Torrent doesn't exist. Something went wrong. info hash: "<< annInfo->getInfoHash() << "\n";
            std::cout << "Torrent doesn't exist. Something went wrong. info hash: "<< annInfo->getInfoHash() << "\n";
            errorMessage = "Torrent doesn't exist!";
            return false;
        }
    }
    catch (sql::SQLException &e)
    {
        *logger << "Database::torrentExists ";
        *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
        *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
        std::cout << "Database::torrentExists ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
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
                *logger << "Valid IP Address" << "\n";
                std::cout << "Valid IP Address" << "\n";
                return false;
            }
            else
            {
                *logger << "Banned IP Address " << "\n";
                std::cout << "Banned IP Address " << "\n";
                errorMessage = "You are using a banned IP Address!";
                return true;
            }
        }
        else
        {
            *logger << "IP Address does not exist, so user is not banned" << "\n";
            std::cout << "IP Address does not exist, so user is not banned" << "\n";
            return false;
        }
        
    }
    catch (sql::SQLException &e)
    {
        *logger << "Database::ipaIsBanned ";
        *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
        *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
        std::cout << "Database::ipaIsBanned ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
        return true;
    }
}

bool Database::updateClientTorrents()
{
    if (getUserId() && userCanLeech() && !ipaIsBanned() && torrentExists() && getClientId(true) && setOldUploadAndDownload())
    {
        try
        {
            //Get new seed minutes for later bonus point calc:
            pstmt = con->prepareStatement
            (
                "SELECT "
                        "isActive, "
                        "TIMESTAMPDIFF(MINUTE, lastActivity, NOW()) AS 'newSeedMinutes' "
                "FROM "
                        "clientTorrents "
                "WHERE "
                        "torrentId = ? "
                        "AND clientId = ?"
            );
            pstmt->setInt(1, annInfo->getTorrentId());
            pstmt->setInt(2, annInfo->getClientId());
            int newSeedMinutes = 0;
            res = pstmt->executeQuery();

            if(res->next())
            {
                newSeedMinutes = res->getInt("newSeedMinutes");
                if(res->getInt("isActive") == 0)
                {
                    newSeedMinutes = 0;
                }
            }

            pstmt = con->prepareStatement
                    (
                        "UPDATE "
                                "clientTorrents AS ct, "
                                "client AS c, "
                                "ipAddress AS ip, "
                                "user AS u "
                        "SET "
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
            pstmt->setUInt64(3, annInfo->getDownloaded());
            pstmt->setUInt64(4, annInfo->getLeft());
            pstmt->setUInt64(5, annInfo->getUploaded());
            pstmt->setInt(6, annInfo->getEvent());
            pstmt->setInt(7, annInfo->getClientId());
            pstmt->setInt(8, annInfo->getTorrentId());
            pstmt->setString(9, annInfo->getTorrentPass());

            if (pstmt->executeUpdate() <= 0)
            {
                std::cout << "clientTorrent doesn't exist. Will create one. " << "\n";
                *logger << "clientTorrent doesn't exist. Will create one. " << "\n";
                if (!createClientTorrent())
                {
                    return false;
                }
            }

            if(!updateUserBonusPoints(newSeedMinutes))
            {
                *logger << "An error occurred within updateUserBonusPoints" << "\n";
                std::cout << "An error occurred within updateUserBonusPoints" << "\n";
            }

            if (!updateUserTorrentTotals(newSeedMinutes))
            {
                return false;
            }
            return true;
        }
        catch (sql::SQLException &e)
        {
            *logger << "Database::updateClientTorrents ";
            *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
            *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
            std::cout << "Database::updateClientTorrents ";
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
            return true;
        }
    }
    //If any of the statements fails, the program will not respond with any peers
    return false;
}

bool Database::updateUserBonusPoints(int newSeedMinutes)
{
    try
    {
        std::cout << "Inne i updatebonus\n";
        pstmt = con->prepareStatement
        (
            "SELECT "
                "timeActive "
            "FROM "
                "userTorrentTotals "
            "WHERE "
                "torrentId = ? "
                "AND userId = ?"
        );

        pstmt->setInt(1, annInfo->getTorrentId());
        pstmt->setInt(2, annInfo->getUserId());

        std::cout << "Skal kjøre query\n";

        res = pstmt->executeQuery();
        int totalTimeActive = 0;
        if(res->next())
        {
            totalTimeActive = res->getInt("timeActive");
            std::cout << "Fant totalTimeActive = " << totalTimeActive << "\n";
        }
        std::cout << "totalTimeActive = " << totalTimeActive << "\n";

        //Bonus point-calc
        pstmt = con->prepareStatement
        (
            "SELECT "
                    "(SELECT IFNULL(SUM(isActive), 0) FROM clientTorrents AS ct WHERE ct.torrentId = torrentId "
                    "AND (TIMESTAMPDIFF(MINUTE, ct.lastActivity, NOW()) < 60)) AS 'seeders', "
                    "(SELECT SUM(length) FROM torrentFiles AS tf WHERE tf.torrentId = torrentId) AS 'size' "
            "FROM "
                    "clientTorrents "
            "WHERE "
                    "torrentId = ? "
                    "AND clientId = ?"
        );
        pstmt->setInt(1, annInfo->getTorrentId());
        pstmt->setInt(2, annInfo->getClientId());
        double bonusPointIncrement = 0;
        res = pstmt->executeQuery();

        std::cout << "Skal finne bonuspointCalc\n";

        //If found, get calculations for BP
        if(res->next())
        {
            std::cout << "Inne i res-next\n";
            int seeders = res->getInt("seeders");
            uint64_t size = res->getUInt64("size");
            if(seeders != 0)
            {
                std::cout << "Kaller bonusPointIncrement\n";

                bonusPointIncrement = calcBonusPoints(size, newSeedMinutes, seeders, totalTimeActive);
            }
        }
        //Update bonus points
        std::cout << "Skal oppdatere bonus points\n Increment = " << bonusPointIncrement << "\n";
        if(bonusPointIncrement > 0)
        {
                pstmt = con->prepareStatement
            (
                "UPDATE user SET points = points + ? WHERE id = ?"
            );
            pstmt->setDouble(1, bonusPointIncrement);
            pstmt->setInt(2, annInfo->getUserId());

            std::cout << "oppdaterer poeng paa bruker " << annInfo->getUserId() << "\n"; 

            if(pstmt->executeUpdate() > 0)
            {
                std::cout << "Added " << bonusPointIncrement << " to user: " << annInfo->getUserId() << "\n";
                *logger << "Added " << std::to_string(bonusPointIncrement) << " to user: " << std::to_string(annInfo->getUserId()) << "\n";
                return true;
            }
            std::cout << "update returnerte 0 eller mindre\n";
            return false;
        }
        return true;
        
    }
    catch (sql::SQLException &e)
    {
        *logger << "Database::updateUserBonusPoints ";
        *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
        *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
        std::cout << "Database::updateUserBonusPoints ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
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
            *logger << "Created new clientTorrents " << "\n";
            std::cout << "Created new clientTorrents " << "\n";
            return true;
        }
        else
        {
            *logger << "Failed creating new clientTorrents " << "\n";
            std::cout << "Failed creating new clientTorrents " << "\n";
            return false;
        }
    }
    catch (sql::SQLException &e)
    {
        *logger << "Database::createClientTorrent ";
        *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
        *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
        std::cout << "Database::createClientTorrent ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
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
            *logger << "Using known IP Address " << "\n";
            std::cout << "Using known IP Address " << "\n";
            return true;
        }
        else
        {
            *logger << "Using unknown IP Address. Will insert into database " << "\n";
            std::cout << "Using unknown IP Address. Will insert into database " << "\n";
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
        *logger << "Database::getIpaId ";
        *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
        *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
        std::cout << "Database::getIpaId ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
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
                std::cout << "Using a known client" << "\n";
                *logger << "Using a known client" << "\n";
                return true;
            }
            else
            {
                *logger << "Client unknown. Will try to update or create one. " << "\n";
                std::cout << "Client unknown. Will try to update or create one. " << "\n";
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
            *logger << "Database::getClientId ";
            *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
            *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
            std::cout << "Database::getClientId ";
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
            return false;
        }
    }
    else
    {
        *logger << "Should never end up here.." << "\n";
        std::cout << "Should never end up here.." << "\n";
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
            *logger << "IP Address added to Database. Will do recursive function to get ipaId" << "\n";
            std::cout << "IP Address added to Database. Will do recursive function to get ipaId" << "\n";
            return getIpaId(false);
        }
        else
        {
            *logger << "Failed adding IP Address to the Database " << "\n";
            std::cout << "Failed adding IP Address to the Database " << "\n";
            return false;
        }
    }
    catch (sql::SQLException &e)
    {
        *logger << "Database::createIpAddress ";
        *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
        *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
        std::cout << "Database::createIpAddress ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
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
            *logger << "Added new client to the Database. Will do recursive function to get clientId" << "\n";
            std::cout << "Added new client to the Database. Will do recursive function to get clientId" << "\n";
            return getClientId(false);
        }
        else
        {
            *logger << "Failed adding new client to the Database " << "\n";
            std::cout << "Failed adding new client to the Database " << "\n";
            return false;
        }
    }
    catch (sql::SQLException &e)
    {
        *logger << "Database::createClient ";
        *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
        *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
        std::cout << "Database::createClient ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
        return false;
    }
}

bool Database::updateUserTorrentTotals(int newSeedMinutes)
{
    try
    {
        uint64_t downloadedTotalIncrement = annInfo->getDownloaded() - annInfo->getOldDownload();
        uint64_t uploadedTotalIncrement = annInfo->getUploaded() - annInfo->getOldUpload();

        *logger << "oldDownloaded: " << std::to_string(annInfo->getOldDownload()) << "\n";
        *logger << "oldUploaded: " << std::to_string(annInfo->getOldUpload()) << "\n";
        *logger << "Downloaded INC: " << std::to_string(downloadedTotalIncrement) << "\n";
        *logger << "Uploaded INC: " << std::to_string(uploadedTotalIncrement) << "\n";
        std::cout << "oldDownloaded: " << annInfo->getOldDownload() << "\n";
        std::cout << "oldUploaded: " << annInfo->getOldUpload() << "\n";
        std::cout << "Downloaded INC: " << downloadedTotalIncrement << "\n";
        std::cout << "Uploaded INC: " << uploadedTotalIncrement << "\n";
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
            *logger << "About to do booltest" << "\n";
            std::cout << "About to do booltest" << "\n";
            int boolTest = res->getInt("1");
            if(boolTest==1)
            {
                *logger << "Booltest PASSED!\n";
                std::cout << "Booltest PASSED!\n";

                /* If user har restartet the same torrent we do not
                want to update the total with a negative number */
                //if(downloadedTotalIncrement <= 0)
                if(downloadedTotalIncrement < 0)
                {
                    *logger << "Down increment is less than zero\n";
                    std::cout << "Down increment is less than zero\n";
                    downloadedTotalIncrement = annInfo->getDownloaded();
                }
                //if(uploadedTotalIncrement<=0)
                if(uploadedTotalIncrement < 0)
                {
                    *logger << "UP increment is less than zero";
                    std::cout << "UP increment is less than zero";
                    uploadedTotalIncrement = annInfo->getUploaded();
                }
                std::string query = "UPDATE userTorrentTotals SET "
                                    "totalDownloaded = totalDownloaded + ?,"
                                    "totalUploaded = totalUploaded + ?, "
                                    "timeActive = timeActive + ? "
                                    "WHERE torrentId = ? AND userId = ?;";
                //std::cout << "query: \n" << query << "\n";
                pstmt = con->prepareStatement(query);
                pstmt->setUInt64(1, downloadedTotalIncrement);
                pstmt->setUInt64(2, uploadedTotalIncrement);
                pstmt->setInt(3, newSeedMinutes);
                pstmt->setInt(4, annInfo->getTorrentId());
                pstmt->setInt(5, annInfo->getUserId());
                pstmt->executeUpdate();
                *logger << "Updated userTorrentTotals in the Database" << "\n";
                std::cout << "Updated userTorrentTotals in the Database" << "\n";
                return true;
            }
            else
            {
                *logger << "Failed booltest\n";
                std::cout << "Failed booltest\n";
            }               
        }
        *logger << "Failed to update userTorrentTotals in the Database. Will create one " << "\n";
        std::cout << "Failed to update userTorrentTotals in the Database. Will create one " << "\n";
        return createUserTorrentTotals();
    }
    catch (sql::SQLException &e)
    {
        *logger << "Database::updateUserTorrentTotals ";
        *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
        *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
        std::cout << "Database::updateUserTorrentTotals ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
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
            *logger << "Added new userTorrentTotals to the Database" << "\n";
            std::cout << "Added new userTorrentTotals to the Database" << "\n";
            return true;
        }
        else
        {
            *logger << "Failed adding new userTorrentTotals to the Database " << "\n";
            std::cout << "Failed adding new userTorrentTotals to the Database " << "\n";
            return false;
        }
    }
    catch (sql::SQLException &e)
    {
        *logger  << "Database::createUserTorrentTotals ";
        *logger  << " (MySQL error code: " << std::to_string(e.getErrorCode());
        *logger  << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
        std::cout << "Database::createUserTorrentTotals ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
        return false;
    }
}

double Database::calcBonusPoints(uint64_t torrentSizeBytes, int newSeedMinutes, int numberOfSeeders, int totalSeedTimeMinutes)
{
    if(newSeedMinutes == 0) return 0;

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
            *logger << "Updated Client. Will do recursive function to get clientId" << "\n";
            std::cout << "Updated Client. Will do recursive function to get clientId" << "\n";
            return getClientId(false);
        }
        else
        {
            std::cout << "Failed to update Client. Will create one" << "\n";
            *logger << "Failed to update Client. Will create one" << "\n";
            return createClient();
        }
    }
    catch (sql::SQLException &e)
    {
        *logger << "Database::updateClient ";
        *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
        *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
        std::cout << "Database::updateClient ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
        return false;
    }
}

std::string Database::getErrorMessage()
{
    return errorMessage;
}

bool Database::setOldUploadAndDownload()
{
    try
    {
        pstmt = con->prepareStatement
        (
            "SELECT "
                    "uploaded, downloaded "
            "FROM "
                    "clientTorrents "
            "WHERE "
                    "torrentId = ? "
                    "AND clientId = ?"
        );
        pstmt->setInt(1, annInfo->getTorrentId());
        pstmt->setInt(2, annInfo->getClientId());
        res = pstmt->executeQuery();
        if (res->next())
        {
            annInfo->setOldUpload(res->getUInt64("uploaded"));
            annInfo->setOldDownload(res->getUInt64("downloaded"));
        }
        else
        {
            *logger << "Could not retrieve old upload and download values. Perhaps this is their first announce?" << "\n";
            std::cout << "Could not retrieve old upload and download values. Perhaps this is their first announce?" << "\n";
            //The default old upload and download is set to 0, so this should not be a problem.
        }
        //Will go forward with the code whether or not an old record exists.
        return true;
    }
    catch (sql::SQLException &e)
    {
        *logger << "Database::setOldUploadAndDownload ";
        *logger << " (MySQL error code: " << std::to_string(e.getErrorCode());
        *logger << ", SQLState: " << std::string(e.getSQLState()) << " )" << "\n";
        std::cout << "Database::setOldUploadAndDownload ";
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << "\n";
        //It is important to return false, as if a record actually exists and the old values are set to 0, the gain in 
        //total download/upload could be enormous. 
        return false;
    }
}
