#include <iostream>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <chrono>

void ServerProc(sockaddr_in _sockAddr)
{
	const int serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(serverSock == -1)
	{
		std::cout << "create socket failed!" << std::endl;
		return;
	}

	bind(serverSock, reinterpret_cast<sockaddr*>(&_sockAddr), sizeof(_sockAddr));

	listen(serverSock, 20);

	std::cout << "server start listen..." << std::endl;

	while(true)
	{
		sockaddr_in clntAddr = {};
		socklen_t size = sizeof(clntAddr);

		int clientSock = accept(serverSock, reinterpret_cast<sockaddr*>(&clntAddr), &size);
		char content[256];

		memset(content, 0, sizeof(content));

		recv(clientSock, content, sizeof(content), 0);

		std::cout << "server recv:" << content << std::endl;

		strcpy(content, "server reply!");

		send(clientSock, content, sizeof(content), 0);
	}

	close(serverSock);
}

void ClientProc(sockaddr_in _serverAddr)
{
	const int clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(clientSock == -1)
	{
		std::cout <<"create socket failed!" << std::endl;
	}

	while(connect(clientSock, reinterpret_cast<sockaddr*>(&_serverAddr), sizeof(_serverAddr)) == -1)
	{
		std::cout << "connect server failed!" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));	
	}

	std::cout << "connected" << std::endl;

	char content[256];

	memset(content, 0, sizeof(content));
	strcpy(content, "client send!");

	send(clientSock, content, sizeof(content), 0);

	recv(clientSock, content, sizeof(content), 0);

	std::cout << "client recv:" << content << std::endl;

	
	close(clientSock);
}

int main()
{

	sockaddr_in serverAddr = {};

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(8888);

//	ServerProc(serverAddr);

//	return 0;

	std::thread serverProc(ServerProc, serverAddr);

	for(int i = 0; i < 100; i++)
		ClientProc(serverAddr);

	serverProc.join();

	std::cout << "finish" << std::endl;
	return 0;
}
