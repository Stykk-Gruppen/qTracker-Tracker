#ifndef ANNOUNCEINFO_H
#define ANNOUNCEINFO_H

#include <string>

class AnnounceInfo
{
public:
	AnnounceInfo(std::string ipa_, std::string infoHash_, std::string peerId_, std::string torrentPass_,
	int event_, int port_, uint64_t downloaded_, uint64_t left_, uint64_t uploaded_);
	~AnnounceInfo();
	void setTorrentId(int torrentId_);
	void setClientId(int clientId_);
	void setUserId(int userId_);
	void setIpaId(int ipaId_);
	void setOldUpload(uint64_t oldUpload_);
	void setOldDownload(uint64_t oldDownload_);
	std::string getTorrentPass();
	std::string getIpa();
	std::string getInfoHash();
	std::string getPeerId();
	int getUserId();
	int getPort();
	int getTorrentId();
	int getEvent();
	int getClientId();
	int getIpaId();
	uint64_t getDownloaded();
	uint64_t getUploaded();
	uint64_t getLeft();
	uint64_t getOldUpload();
	uint64_t getOldDownload();
private:
	std::string ipa;
	std::string infoHash;
	std::string peerId;
	std::string torrentPass;
	int port;
	int event;
	uint64_t downloaded;
	uint64_t left;
	uint64_t uploaded;

	//Information from the database
	int clientId;
	int torrentId;
	int userId;
	int ipaId;
	uint64_t oldUpload = 0;
	uint64_t oldDownload = 0;
};

#endif