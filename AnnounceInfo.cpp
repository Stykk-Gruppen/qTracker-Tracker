#include "AnnounceInfo.h"

AnnounceInfo::AnnounceInfo(std::string ipa_, std::string infoHash_, std::string peerId_, std::string torrentPass_,
	int event_, int port_, uint64_t downloaded_, uint64_t left_, uint64_t uploaded_) 
	:ipa(ipa_), infoHash(infoHash_), peerId(peerId_), torrentPass(torrentPass_), event(event_), port(port_),
	downloaded(downloaded_), left(left_), uploaded(uploaded_)
{

}

AnnounceInfo::~AnnounceInfo()
{
	
}

void AnnounceInfo::setTorrentId(int torrentId_)
{
	torrentId = torrentId_;
}

void AnnounceInfo::setClientId(int clientId_)
{
	clientId = clientId_;
}

void AnnounceInfo::setUserId(int userId_)
{
	userId = userId_;
}

void AnnounceInfo::setIpaId(int ipaId_)
{
	ipaId = ipaId_;
}

void AnnounceInfo::setOldUpload(uint64_t oldUpload_)
{
	oldUpload = oldUpload_;
}

void AnnounceInfo::setOldDownload(uint64_t oldDownload_)
{
	oldDownload = oldDownload_;
}

std::string AnnounceInfo::getTorrentPass()
{
	return torrentPass;
}

std::string AnnounceInfo::getIpa()
{
	return ipa;
}

std::string AnnounceInfo::getInfoHash()
{
	return infoHash;
}

std::string AnnounceInfo::getPeerId()
{
	return peerId;
}

int AnnounceInfo::getUserId()
{
	return userId;
}

int AnnounceInfo::getPort()
{
	return port;
}

int AnnounceInfo::getTorrentId()
{
	return torrentId;
}

int AnnounceInfo::getEvent()
{
	return event;
}

int AnnounceInfo::getClientId()
{
	return clientId;
}

int AnnounceInfo::getIpaId()
{
	return ipaId;
}

uint64_t AnnounceInfo::getDownloaded()
{
	return downloaded;
}

uint64_t AnnounceInfo::getUploaded()
{
	return uploaded;
}

uint64_t AnnounceInfo::getLeft()
{
	return left;
}

uint64_t AnnounceInfo::getOldUpload()
{
	return oldUpload;
}

uint64_t AnnounceInfo::getOldDownload()
{
	return oldDownload;
}