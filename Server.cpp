#include "Server.h"

using namespace std;

Server::Server(int _port): port(_port)
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero((char*)&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	bind(sockfd, (struct sockaddr *) &serv_addr,
		sizeof(serv_addr));

	listen(sockfd, 5);

	clilen = sizeof(cli_addr);

}

void Server::central()
{
	int newsockfd;
	while(true)
	{
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		//struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&cli_addr;
		//struct in_addr ipAddr = pV4Addr->sin_addr;
		printf("IP address is: %s\n", inet_ntoa(cli_addr.sin_addr));
		printf("port is: %d\n", (int) ntohs(cli_addr.sin_port));
		//std::cout << cli_addr.sin_addr << "\n";
		//char str[INET_ADDRSTRLEN];
		//inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );
		
		//std::cout << "IP: " << str << "\n";

		pid = fork();
		if(pid == 0)
		{
			close(sockfd);
			handle_client(newsockfd);
			exit(0);
		}
		else
		{
			close(newsockfd);
			signal(SIGCHLD,SIG_IGN); 
		}
	}
}

void Server::handle_client(int newsockfd)
{
	bzero(buffer, 4096);
	read(newsockfd, buffer, 4096);

	//Http h(array_to_string(buffer, 4096));
	std::cout << array_to_string(buffer, 4096);	
	std::vector<std::string*> vectorOfArrays;
	std::string keyString = "";
	std::string valueString = "";
	std::string url = "";
	bool key = false;
	bool value = false;
	int urlFound = 0;
	for(int i=0;i<buffersize;i++)
	{
		char c = buffer[i];
		if(!value && c==' ')
		{
			urlFound++;
			continue;
		}
		if(c=='?')
		{
			urlFound++;
			key = true;
			continue;
		}
		if(urlFound==1)
		{
			url += c;
		}
		if(c=='=')
		{
			key = false;
			value = true;
			continue;
		}
		if(value && (c=='&' || c==' '))
		{
			//cout << "key: " << keyString << "   value: " << valueString << endl;
			key = true;
			value = false;
			string *stringArray = new string[2];
			stringArray[0] = keyString;
			stringArray[1] = valueString;
			vectorOfArrays.push_back(stringArray);
			keyString = "";
			valueString = "";
			continue;
		}
		if(key)
		{
			keyString += c;
		}
		if(value)
		{
			valueString += c;
		}
		//temp += buffer[i];
	}
	string *urlArray = new string[2];
	urlArray[0] = "url";
	urlArray[1] = url;
	string *urlKeyArray = new string[2];
	urlKeyArray[0] = "urlKey";
	urlKeyArray[1] = getKeyFromURL(url);
	string *ipKeyArray = new string[2];
	ipKeyArray[0] = "ip";
	ipKeyArray[1] = inet_ntoa(cli_addr.sin_addr);
	vectorOfArrays.push_back(urlArray);
	vectorOfArrays.push_back(urlKeyArray);
	vectorOfArrays.push_back(ipKeyArray);

	for(size_t x=0;x<vectorOfArrays.size();x++)
	{
		cout << vectorOfArrays.at(x)[0] << "=" << vectorOfArrays.at(x)[1] << endl;
	}
	
	std::ostringstream stream;
	bencode::encode(stream, bencode::dict{
		{"tracker_id", 1},
		{"peers", bencode::list{
			bencode::dict{
				{"peer_id", 1},
				{"ip", 1},
				{"port", 1}
			},
			bencode::dict{
				{"peer_id", 1},
				{"ip", 1},
				{"port", 1}
			},
			bencode::dict{
				{"peer_id", 1},
				{"ip", 1},
				{"port", 1}
			},
			bencode::dict{
				{"peer_id", 1},
				{"ip", 1},
				{"port", 1}
			},
			bencode::dict{
				{"peer_id", 1},
				{"ip", 1},
				{"port", 1}
			}
		}
	},
	{"interval", "3"},
	{"complete", "3"},
	{"incomplete", "3"}
	});
	std::string streamString =  stream.str();
	std::string answer = "";
	answer += "HTTP/1.1 200 OK\r\n";
	answer += "Content-length: ";
	answer += to_string(strlen(streamString.c_str()));
	answer += "\r\n";
	answer += "Content-Type: text/plain\r\n";
	answer += "Connection: close\r\n";
	answer += "\r\n";
	answer += streamString;
	//std::cout << answer << "\n";
	write(newsockfd, answer.c_str(), strlen(answer.c_str()));

	for(int i = 0; i < vectorOfArrays.size(); i++)
	{
		delete[] vectorOfArrays[i];
	}
}

std::string Server::buildDictionary()
{
	//std::string test = "{}";
	//std::string encodedString = boost::get<bencode::string>(test);
}

std::string Server::getKeyFromURL(std::string url)
{
	std::string key = "";
	int slashCount = 0;
	for(int i=0;i<url.length();i++)
	{
		if(url[i]=='/')
		{
			slashCount++;
			continue;
		}
		if(slashCount == 1)
		{
			key+=url[i];
		}
	}
	return key;
}

std::string Server::array_to_string(char* arr, int size)
{
	int i;
	std::string s = "";
	for (i = 0; i < size; i++) {
		s = s + arr[i];
	}
	return s;
}
