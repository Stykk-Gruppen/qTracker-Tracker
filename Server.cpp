#include "Server.h"

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
	std::string answer;
	try
	{
		//h.handle_message();
	}
	catch (...)
	{
		//answer = h.build_answer(false);
	}
	//answer = h.build_answer(true);
	answer = "HTTP/1.1 200 OK\nd8:intervali1800e5:peersld2:id20:-lt0D60-pE0A21E5FB68680FDDCBEA6:ip20:::ffff:5.79.98.209:porti45417eeee";
	std::cout << "Dette svarer jeg med: " << answer << "\n";
	write(newsockfd, answer.c_str(), strlen(answer.c_str()));
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
