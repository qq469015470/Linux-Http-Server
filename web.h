#include <string>
#include <iostream>
#include <map>
#include <typeinfo>
#include <stdexcept>
#include <memory>
#include <fstream>

namespace web
{
	struct UrlParam
	{
		std::string name;
		const std::type_info* type;
	};

	enum HttpType
	{
		GET,
		POST,	
	};

	class HttpAttr
	{
	private:
		std::string key;
		std::string values;

	public:
		HttpAttr(std::string _key, const std::vector<std::string>& _values):
			key(std::move(_key))
		{
			for(const auto& item: _values)
			{
				this->values += item + ",";
			}

			this->values.pop_back();
		}

		std::string_view GetKey() const
		{
			return this->key;
		}

		std::string_view GetValues() const
		{
			return this->values;	
		}

	};

	class HttpRequest
	{
	private:
		//请求类型(例:get post)
		std::string type;
		std::string url;

	public:
		HttpRequest(std::string_view _buffers)
		{
			std::string::size_type left = 0;
			std::string::size_type right = _buffers.find(" ");

			if(right != std::string::npos)
				this->type = _buffers.substr(left,right - left);
			else
				throw std::runtime_error("type not exists");

			left = right + 1;
			right = _buffers.find(" ", left);

			if(right != std::string::npos)
				this->url = _buffers.substr(left, right - left);
			else
				throw std::runtime_error("url not exists");
		}

		std::string_view GetType() const
		{
			return this->type;
		}

		std::string_view GetUrl() const
		{
			return this->url;
		}
	};

	class HttpResponse
	{
	private:
		std::vector<char> content;
		size_t size;

		static inline std::string GetSpec(int _code)
		{
			switch (_code)
			{
				case 200:
					return std::string("OK");
					break;
				case 404:
					return std::string("NOT FOUND");
				default:
					return std::string("NONE SPEC");
			}
		}

	public:
		HttpResponse(int _stateCode, const char* _body, unsigned long _bodyLen):
			content({'\0'}),
			size(0)
		{
			std::string header;
			
			header = "HTTP/1.1 " + std::to_string(_stateCode) + " " + HttpResponse::GetSpec(_stateCode) + "\r\n";
			header += "Content-Length: " + std::to_string(_bodyLen) + "\r\n";
			header += "\r\n";

			this->content.resize(header.size() + _bodyLen);

			std::copy(header.data(), header.data() + header.size(), this->content.data());
			std::copy(_body, _body + _bodyLen, this->content.data() + header.size());
		}

		const char* GetContent() const
		{
			return this->content.data();
		}

		size_t GetSize() const
		{
			return this->content.size();
		}
	};

	class Router
	{
	private:
		using Callback = HttpResponse();

		struct Info
		{
			HttpType type;
			std::string url;
			std::vector<UrlParam> params;
			Callback* callback;
		};


		std::map<std::string, Info> infos;

	public:
		void RegisterUrl(HttpType _type, std::string_view _url, std::vector<UrlParam> _params, Callback* _func)
		{
			if(this->infos.find(_url.data()) != this->infos.end())
			{
				throw std::logic_error("url has been register");
			}

			Info temp = {};

			temp.type = _type;
			temp.url = std::string(_url.data());
			temp.params = std::move(_params);
			temp.callback = std::move(_func);
	
			this->infos.insert(std::pair<std::string, Info>(temp.url, std::move(temp)));
		}

		const std::vector<UrlParam>& GetUrlParams(std::string_view _url) const
		{
			return this->infos.at(_url.data()).params;
		}

		const Callback* GetUrlCallback(std::string_view _url) const
		{
			return this->infos.at(_url.data()).callback;
		}
	};


	class HttpServer
	{
	private:
		std::unique_ptr<Router> router;

		bool listenSignal;

		static HttpRequest GetHttpRequest(int _sockfd)
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

				const int recvLen = recv(_sockfd, buffer, sizeof(buffer) - 1, 0);

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

			return HttpRequest(std::move(content));
		}

		static void SendHttpResponse(int _sockfd, const HttpResponse& _response)
		{
			std::cout << "响应报文:" << std::endl;
			//std::cout << _response.GetContent() << std::endl;

			const size_t contentSize = _response.GetSize();
			const char* content = _response.GetContent();

			send(_sockfd, content, contentSize, 0);
		}

		static std::vector<char> GetRootFile(std::string_view _view)
		{
			const std::string root = "wwwroot/";

			std::ifstream file(root + _view.data());
			std::vector<char> bytes;

			if(file.is_open())
			{
				file.seekg(0, std::ios::end);
				bytes.resize(file.tellg());
				file.seekg(0, std::ios::beg);
	
				file.read(bytes.data(), bytes.size());
			}
			else
			{
				throw std::runtime_error("could not find wwwroot file");
			}

			file.close();

			return bytes;
		}

		static void ListenProc(HttpServer* _httpServer, sockaddr_in _sockAddr)
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
		
			while(_httpServer->listenSignal == true)
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

						try
						{
							const HttpRequest request = HttpServer::GetHttpRequest(events[i].data.fd);
			

							std::cout << "类型:\"" << request.GetType() << "\" 地址:\"" << request.GetUrl() << "\"" << std::endl;
							try
							{
								std::cout << "1" << std::endl;
								HttpResponse response = _httpServer->router->GetUrlCallback(request.GetUrl())();
								std::cout << "2" << std::endl;	
								HttpServer::SendHttpResponse(events[i].data.fd, std::move(response));
								std::cout << "3" << std::endl;
							}
							catch (std::out_of_range _ex)
							{
								try
								{
									const std::vector<char> body = HttpServer::GetRootFile(request.GetUrl());
									HttpResponse response(200, body.data(), body.size());
	
									HttpServer::SendHttpResponse(events[i].data.fd, std::move(response));
								}
								catch(std::runtime_error _ex)
								{	
									std::cout << _ex.what() << std::endl;
								}
							}
						}
						catch(std::runtime_error _ex)
						{
							std::cout << _ex.what() << std::endl;
						}

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

			std::thread proc(HttpServer::ListenProc, this, serverAddr);

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

	HttpResponse View(std::string_view _path)
	{
		const std::string root ="view/";
		const std::string path = root + _path.data();

		std::ifstream file(path);

		std::cout << "return view \"" << path << "\"" << std::endl;

		int stateCode(200);
		std::vector<char> body;

		if(file.is_open())
		{
			file.seekg(0, std::ios::end);
			body.resize(file.tellg());
			file.seekg(0, std::ios::beg);
			
			file.read(body.data(), body.size());
		}
		else
		{
			const char* temp = "Ops! file not found!";

			body.resize(strlen(temp));

			std::copy(temp, temp + body.size(), body.data());
		}

		file.close();

		return HttpResponse(stateCode, body.data(), body.size());
	}
};
