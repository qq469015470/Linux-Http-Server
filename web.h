#include <string>
#include <iostream>
#include <map>
#include <typeinfo>
#include <stdexcept>
#include <memory>

namespace web
{
	struct UrlParam
	{
		std::string name;
		const std::type_info* type;
	};

	class Router
	{
	private:
		std::map<std::string, std::vector<UrlParam>> urls;

	public:
		void RegisterUrl(std::string_view _url, std::vector<UrlParam> _params)
		{
			if(this->urls.find(_url.data()) != this->urls.end())
			{
				throw std::logic_error("url has been register");
			}

			this->urls.insert(std::pair<std::string, std::vector<UrlParam>>(std::move(_url), std::move(_params)));
		}

		const std::vector<UrlParam>& GetUrlParams(std::string_view _url)
		{
			return this->urls.at(_url.data());
		}
	};

	class HttpServer
	{
	private:
		std::unique_ptr<Router> router;

		bool listenSignal;

		static void ListenProc(bool* _listenSignal, sockaddr_in _sockAddr)
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
		
			while(*_listenSignal == true)
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

			}
		
			close(serverSock);

		}

	public:
		HttpServer(std::unique_ptr<Router>&& _router):
			router(std::move(_router))
		{
			
		}

		void Listen(int _port)
		{
			this->listenSignal = true;
			sockaddr_in serverAddr = {};

			serverAddr.sin_family = AF_INET;
			serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
			serverAddr.sin_port = htons(_port);

			std::thread proc(HttpServer::ListenProc, &this->listenSignal, serverAddr);

			proc.detach();
		}
	};
};
