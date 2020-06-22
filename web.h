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

	class HttpRequest
	{
	private:


	public:
		HttpRequest(std::string_view _buffers)
		{
			
		}
	};

	class HttpServer
	{
	private:
		std::unique_ptr<Router> router;

		bool listenSignal;

		static HttpRequest GetHttpRequest(const int _epfd, epoll_event* _event)
		{
			bool finish(false);
			int contentLen(0);
			std::string content;

			std::cout << "开始接收报文" << std::endl;
			do
			{
				std::cout << "循环" << std::endl;

				char buffer[1024];

				memset(buffer, 0, sizeof(buffer));

				const int recvLen = recv(_event->data.fd, buffer, sizeof(buffer) - 1, 0);

				std::cout << "字节数:" << recvLen <<  std::endl;

				if(recvLen <= 0)
				{
					finish = true;
				}

				contentLen += recvLen;

				//请求头部接收完毕
				content += buffer;

				if(content.find("\r\n\r\n") != std::string::npos)
				{
					finish = true;
				}

			}
			while(!finish);

			std::cout << "报文结束" << std::endl;
			std::cout << content << std::endl;			

			return HttpRequest("");
		}

		static void ListenProc(bool* _listenSignal, sockaddr_in _sockAddr)
		{
			const int serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if(serverSock == -1)
			{
				std::cout << "create socket failed!" << std::endl;
				return;
			}
		
			if(bind(serverSock, reinterpret_cast<sockaddr*>(&_sockAddr), sizeof(_sockAddr)) == -1)
			{
				std::cout << "bind socket failed!" << std::endl;
				return;
			}
		
			const int max = 20;
		
			listen(serverSock, max);
		
			std::cout << "server start listen..." << " -port " << ntohs(_sockAddr.sin_port)  << std::endl;
		
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
					//该描述符为服务器则accept
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
					//是客户端则返回信息
					else if(events[i].events & EPOLLIN)
					{
						HttpServer::GetHttpRequest(epfd, &events[i]);

						close(events[i].data.fd);
						epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &events[i]);	
					}
					else
					{
						std::cout << "something else happen" << std::endl;
					}
				}

			}
		
			close(serverSock);
			std::cout << "server close" << std::endl;
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

		void Stop()
		{
			if(this->listenSignal == false)
			{
				throw std::runtime_error("server is not listening.");
			}

			this->listenSignal = false;
		}
	};
};
