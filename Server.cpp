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

	//write(newsockfd, answer.c_str(), strlen(answer.c_str()));
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