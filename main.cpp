#include <iostream>
#include <assert.h>
#include <sys/socket.h>
#include <sys/epoll.h>
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

	const int max = 20;

	listen(serverSock, max);

	std::cout << "server start listen..." << std::endl;

	epoll_event ev;
	epoll_event events[max];
	const int epfd = epoll_create(max);

	if(epfd == -1)
	{
		std::cout << "create epoll faile!" << std::endl;
		return;
	}

	ev.data.fd = serverSock;
	ev.events = EPOLLIN;
	if(epoll_ctl(epfd, EPOLL_CTL_ADD, serverSock, &ev) == -1)
	{
		std::cout << "epoll control failed!" << std::endl;
		return;
	}

	while(true)
	{
		int nfds = epoll_wait(epfd, events, max, -1);

		if(nfds == -1)
		{
			std::cout << "epoll wait failed!" << std::endl;
		}

		std::cout << "nfds:" << nfds  << std::endl;

		for(int i = 0; i < nfds; i++)
		{
			if(events[i].data.fd == serverSock)
			{
				sockaddr_in clntAddr = {};
		             	socklen_t size = sizeof(clntAddr);

				const int connfd = accept(serverSock, reinterpret_cast<sockaddr*>(&clntAddr), &size);
				if(connfd == -1)
				{
					std::cout << "accept failed" << std::endl;
					return;
				}

				ev.data.fd = connfd;
				ev.events = EPOLLIN;
				epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
				std::cout << "accept client_addr" << inet_ntoa(clntAddr.sin_addr) << std::endl;

			}
			else if(events[i].events & EPOLLIN)
			{
				char buffer[256];

				memset(buffer, 0, sizeof(buffer));
				const int ret =	recv(events[i].data.fd, buffer, sizeof(buffer), 0);

				if(ret <= 0)
				{
					close(events[i].data.fd);
					epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &events[i]);
				}
				else
				{
					std::cout << "server recv:" << buffer << std::endl;
					strcpy(buffer, "server reply!");
					send(events[i].data.fd, buffer, sizeof(buffer), 0);
				}
			}
			else
			{
				std::cout << "something else happen" << std::endl;
			}
		}

//		sockaddr_in clntAddr = {};
//		socklen_t size = sizeof(clntAddr);
//
//		int clientSock = accept(serverSock, reinterpret_cast<sockaddr*>(&clntAddr), &size);
//		char content[256];
//
//		memset(content, 0, sizeof(content));
//
//		recv(clientSock, content, sizeof(content), 0);
//
//		std::cout << "server recv:" << content << std::endl;
//
//		strcpy(content, "server reply!");
//
//		send(clientSock, content, sizeof(content), 0);
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

	std::thread serverProc(ServerProc, serverAddr);

	for(int i = 0; i < 100; i++)
	{
		std::thread proc(ClientProc, serverAddr);

		proc.detach();
	}

	serverProc.join();

	std::cout << "finish" << std::endl;
	return 0;
}
