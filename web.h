#include <string>
#include <iostream>
#include <map>
#include <typeinfo>
#include <stdexcept>
#include <memory>
#include <fstream>
#include <unordered_map>
#include <signal.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

namespace web
{
	class UrlParam
	{
	private:
		std::optional<std::string> val;
		std::unordered_map<std::string, std::unique_ptr<UrlParam>> params;

	public:
		//std::string operator[](std::string_view _key) const
		//{
		//	return this->values.at(_key.data());
		//}

		UrlParam* operator[](std::string_view _key)
		{
			return this->params[_key.data()].get();
		}

		std::string_view ToString()
		{
			if(this->val.has_value())
			{
				throw std::runtime_error("not have val");
			}

			return *this->val;
		}

		void Add(std::string_view _key, std::string_view _value)
		{
			//this->values.insert(std::pair<std::string, >
		}
	};	

	class HttpAttr
	{
	private:
		std::string key;
		std::string value;

	public:
		HttpAttr(std::string _key, std::string _value):
			key(std::move(_key)),
			value(std::move(_value))
		{
		}

		const std::string& GetKey() const
		{
			return this->key;
		}

		const std::string& GetValue() const
		{
			return this->value;	
		}

	};

	class HttpRequest
	{
	private:
		//请求类型(例:get post)
		std::string type;
		std::string url;
		std::string version;
		std::unordered_map<std::string, HttpAttr> attrs;
		std::unique_ptr<char> body;

		inline std::string::size_type ReadBasic(std::string_view _header)
		{
			std::string::size_type left = 0;
			std::string::size_type right = _header.find("\r\n");

			if(right == std::string::npos)
			{
				throw std::runtime_error("http reqeust not vaild!");
			}

			std::string_view line(_header.substr(left, right));

			//读取type
			right = line.find(" ");
			if(right == std::string::npos)
			{
				throw std::runtime_error("could not read type!");
			}
			this->type = line.substr(left, right - left);
			left = right + 1;

			//读取url
			right = line.find(" ", left);
			if(right == std::string::npos)
			{
				throw std::runtime_error("could not read url!");
			}
			this->url = line.substr(left, right - left);
			left = right + 1;
			//去掉地址?开头的参数
			right = this->url.find("?", left);
			if(right != std::string::npos)
			{
				this->url = line.substr(left, right - left);
			}
	

			//读取http协议版本
			right = line.size();
			if(left == right)
			{
				throw std::runtime_error("could not read version!");
			}
			this->version = line.substr(left, right);

			return right + 2;
		}

		void ReadAttr(std::string_view _content)
		{
			std::string::size_type left(0);
			std::string::size_type right(_content.find("\r\n"));
			while(right != std::string::npos)
			{
				std::string_view temp(_content.substr(left, right - left));
				
				std::string::size_type pos = temp.find(":");
				if(pos == std::string::npos)
				{
					continue;
				}

				std::string key(temp.substr(0, pos));
				std::string value(temp.substr(pos + 2, temp.size() - pos - 1));
				
				this->attrs.insert(std::pair<std::string, HttpAttr>(std::move(key), HttpAttr(key, std::move(value))));

				left = right + 2;
				right = _content.find("\r\n", left);
			}
		}

	public:
		HttpRequest(const char* _buffers)
		{
			const char* pos = strstr(_buffers, "\r\n\r\n");		
			
			if(pos == nullptr)
			{
				throw std::runtime_error("http request not vaild!");
			}
			
			const std::string header(_buffers, pos);

			std::string::size_type left(this->ReadBasic(header));
			std::string::size_type right(header.size() - left - 1);
			this->ReadAttr(header.substr(left, right));
		}

		const std::string& GetType() const
		{
			return this->type;
		}

		const std::string& GetUrl() const
		{
			return this->url;
		}

		const std::string& GetAttrValue(std::string_view _key)
		{
			auto iter = this->attrs.find(_key.data());
			if(iter == this->attrs.end())
			{
				this->attrs.insert(std::pair<std::string, HttpAttr>(_key.data(), HttpAttr(_key.data(), "")));
				return this->attrs.at(_key.data()).GetValue();
			}
			else
			{
				return iter->second.GetValue();
			}
		}

		char* GetBody()
		{
			return this->body.get();
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
			std::string type;
			std::string url;
			Callback* callback;
		};


		//第一层用url映射，第二层用http GET SET等类型映射
		std::map<std::string, std::map<std::string, Info>> infos;

		const Info* const GetInfo(std::string_view _type, std::string_view _url) const
		{
			auto urlIter = this->infos.find(_url.data());
			if(urlIter == this->infos.end())
				return nullptr; 
			                                                           
			auto typeIter = urlIter->second.find(_type.data());
			if(typeIter == urlIter->second.end())
				return nullptr;

			return &typeIter->second;
		}

	public:
		void RegisterUrl(std::string_view _type, std::string_view _url, Callback* _func)
		{
			if(this->infos.find(_url.data()) != this->infos.end())
			{
				throw std::logic_error("url has been register");
			}

			Info temp = {};

			temp.type = _type;
			temp.url = std::string(_url.data());
			temp.callback = std::move(_func);
	
			this->infos[_url.data()].insert(std::pair<std::string, Info>(_type, std::move(temp)));
		}

		const Callback* GetUrlCallback(std::string_view _type, std::string_view _url) const
		{
			const Info* const info = this->GetInfo(_type, _url);
			
		       	if(info == nullptr)
		 		return nullptr;
			else
				return info->callback;			
		}
	};


	class HttpServer
	{
	private:
		using SSL_Ptr = std::unique_ptr<SSL, decltype(&SSL_free)>;
		using SSL_CTX_Ptr = std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)>;

		std::unique_ptr<Router> router;

		bool listenSignal;

		static HttpRequest GetHttpRequest(SSL* _ssl)
		{
			bool finish(false);
			int contentLen(0);
			std::vector<char> content;

			std::cout << "开始接收报文" << std::endl;
			do
			{
				std::cout << "循环" << std::endl;

				char buffer[1024];

				memset(buffer, 0, sizeof(buffer));

				const int recvLen =  SSL_read(_ssl, buffer, sizeof(buffer));

				//const int recvLen = recv(_sockfd, buffer, sizeof(buffer)  1, 0);
				
				std::cout << "字节数:" << recvLen <<  std::endl;

				if(recvLen <= 0)
				{
					//recv超时
					if(recvLen == -1)
					{
						SSL_shutdown(_ssl);
					}

					std::string temp("recv but return ");

					temp += std::to_string(recvLen);

					throw std::runtime_error(temp.c_str());
					//finish = true;
					//ERR_print_errors_fp(stderr);
				}

				contentLen += recvLen;

				//请求头部接收完毕
				content.insert(content.end(), buffer, buffer + recvLen);

				if(strstr(content.data(), "\r\n\r\n") != nullptr)
				{
					finish = true;
				}

			}
			while(!finish);

			std::cout << "报文结束" << std::endl;
			//std::cout << content.data() << std::endl;			

			return HttpRequest(content.data());
		}

		static void SendHttpResponse(SSL* _ssl, const HttpResponse& _response)
		{
			//std::cout << "响应报文:" << std::endl;
			//std::cout << _response.GetContent() << std::endl;

			const size_t contentSize = _response.GetSize();
			const char* content = _response.GetContent();

			SSL_write(_ssl, content, contentSize);
			//send(_sockfd, content, contentSize, 0);
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

		static inline SSL_Ptr HandleAccept(int _sockfd, int _epfd, SSL_CTX* _ctx)
		{
			//设置超时时间
			struct timeval timeout={3,0};//3s
    			if(setsockopt(_sockfd, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout)) == -1)
				std::cout << "setsoockopt failed!" << std::endl;
    			if(setsockopt(_sockfd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout)) == -1)
				std::cout << "setsoockopt failed!" << std::endl;
			
			//绑定ssl
			SSL_Ptr ssl(SSL_new(_ctx), SSL_free);
			SSL_set_fd(ssl.get(), _sockfd);
			//ssl握手
			if(SSL_accept(ssl.get()) == -1)
			{
				close(_sockfd);
				throw std::runtime_error("SSL accept failed!");
			}	

			epoll_event ev;

			ev.data.fd = _sockfd;
			ev.events = EPOLLIN;
			epoll_ctl(_epfd, EPOLL_CTL_ADD, _sockfd, &ev);
		
			return ssl;
		}

		static inline void CloseSocket(int _epfd, SSL* _ssl, epoll_event* _ev)
		{
			std::cout << "close socket" << std::endl;

			close(_ev->data.fd);
			epoll_ctl(_epfd, EPOLL_CTL_DEL, _ev->data.fd, _ev);
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
				perror("bind");
				return;
			}
		
			const int max = 20;
		
			if(listen(serverSock, max))
			{
				std::cout << "listen failed!" << std::endl;
				return;
			}	
			
			epoll_event events[max];
			const int epfd = epoll_create(max);
		
			if(epfd == -1)
			{
				std::cout << "create epoll faile!" << std::endl;
				return;
			}

			epoll_event ev;	
			ev.data.fd = serverSock;
			ev.events = EPOLLIN;
			if(epoll_ctl(epfd, EPOLL_CTL_ADD, serverSock, &ev) == -1)
			{
				std::cout << "epoll control failed!" << std::endl;
				return;
			}
			
			//openssl 资料
			//https://blog.csdn.net/ck784101777/article/details/103833822
			//https://www.csdn.net/gather_29/NtDagg3sNTAtYmxvZwO0O0OO0O0O.html
			
			///支持ssl绑定证书
			SSL_CTX_Ptr ctx(SSL_CTX_new(SSLv23_server_method()), SSL_CTX_free);
			//SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
			if(ctx == nullptr)
			{
				std::cout << "ctx is null" << std::endl;
				return;
			}
			
			SSL_CTX_set_options(ctx.get(), SSL_OP_SINGLE_DH_USE | SSL_OP_SINGLE_ECDH_USE | SSL_OP_NO_SSLv2);
			SSL_CTX_set_verify(ctx.get(), SSL_VERIFY_NONE, NULL);
			EC_KEY* ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
			if(ecdh == nullptr)
			{
				std::cout << "EC_KEY_new_by_curve_name failed!" << std::endl;
				return;
			}

			if(SSL_CTX_set_tmp_ecdh(ctx.get(), ecdh) != 1)
			{
				std::cout << "SSL_CTX_set_tmp_ecdh failed!" << std::endl;
				return;
			}
			//

			const char* publicKey = "cert.pem";
			const char* privateKey = "cert.key";

			//配置证书公匙
			if(SSL_CTX_use_certificate_file(ctx.get(), publicKey, SSL_FILETYPE_PEM) != 1)
			{
				std::cout << "SSL_CTX_use_cretificate_file failed!" << std::endl;
				return;
			}
			//配置证书私钥
			if(SSL_CTX_use_PrivateKey_file(ctx.get(), privateKey, SSL_FILETYPE_PEM) != 1)
			{
				std::cout << "SSL_CTX_use_PrivateKey_file failed!" << std::endl;
				return;
			}

			//验证证书
			if(SSL_CTX_check_private_key(ctx.get()) != 1)
			{
				std::cout << "SSL_CTX_check_private_key failed" << std::endl;
				return;
			}
			
			std::unordered_map<int, SSL_Ptr> sslMap;

			while(_httpServer->listenSignal == true)
			{
				int nfds = epoll_wait(epfd, events, max, -1);
		
				if(nfds == -1)
				{
					std::cout << "epoll wait failed!" << std::endl;
				}
		
				std::cout << "nfds:" << nfds  << std::endl;
				std::cout << "sslMap size:" << sslMap.size() << std::endl;
		
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
							std::cout << "accept failed!" << std::endl;
							continue;
						}

						std::cout << "accept client_addr" << inet_ntoa(clntAddr.sin_addr) << std::endl;

						try
						{
							SSL_Ptr temp(HttpServer::HandleAccept(connfd, epfd, ctx.get()));
							sslMap.insert(std::pair<int, SSL_Ptr>(connfd, std::move(temp)));
						}
						catch(std::runtime_error _ex)
						{
							std::cout << _ex.what() << std::endl;
						}
					}
					//是客户端则返回信息
					else if(events[i].events & EPOLLIN)
					{

						try
						{
							HttpRequest request = HttpServer::GetHttpRequest(sslMap.at(events[i].data.fd).get());	

							std::cout << "类型:\"" << request.GetType() << "\" 地址:\"" << request.GetUrl() << "\"" << std::endl;
							std::cout << "1" << std::endl;

							auto callback = _httpServer->router->GetUrlCallback(request.GetType(), request.GetUrl());

							std::cout << "2" << std::endl;
							
							if(callback != nullptr)
							{
								HttpResponse response = callback();
								std::cout << "3" << std::endl;
								HttpServer::SendHttpResponse(sslMap.at(events[i].data.fd).get(), std::move(response));
								std::cout << "4" << std::endl;
							}
							else
							{
								std::cout << "5" << std::endl;
								const std::vector<char> body = HttpServer::GetRootFile(request.GetUrl());
								std::cout << "6" << std::endl;
								HttpResponse response(200, body.data(), body.size());
								                             
      								std::cout << "7" << std::endl;								
								HttpServer::SendHttpResponse(sslMap.at(events[i].data.fd).get(), std::move(response));
								std::cout << "8" << std::endl;
							}
							
							std::cout << "connect:" << request.GetAttrValue("Connection").c_str() << std::endl;
							if(request.GetAttrValue("Connection") != "keep-alive")
							{
								HttpServer::CloseSocket(epfd, sslMap.at(events[i].data.fd).get(), &events[i]);
								sslMap.erase(events[i].data.fd);
							}

						}
						catch(std::runtime_error _ex)
						{
							std::cout << _ex.what() << std::endl;
							HttpServer::CloseSocket(epfd, sslMap.at(events[i].data.fd).get(), &events[i]);
							sslMap.erase(events[i].data.fd);
						}
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

		void InitSSL()
		{
			SSL_library_init();//SSL初始化
			SSL_load_error_strings();//为ssl加载更友好的错误提示
			OpenSSL_add_all_algorithms();
		}

		static void sigpipe_handler(int _val)
		{
			std::cout<< "sigpipe_handler:" << _val << std::endl;
		}

	public:
		HttpServer(std::unique_ptr<Router>&& _router):
			router(std::move(_router))
		{
			this->InitSSL();

			struct sigaction sh;
   			struct sigaction osh;

			//当SSL_shutdown、SSL_write 等对远端进行操作但对方已经断开连接时会触发异常信号、导致程序崩溃、
			//以下代码似乎可以忽略这个信号
			//https://stackoverflow.com/questions/32040760/c-openssl-sigpipe-when-writing-in-closed-pipe?r=SearchResults
   			sh.sa_handler = &HttpServer::sigpipe_handler; //Can set to SIG_IGN
   			// Restart interrupted system calls
   			sh.sa_flags = SA_RESTART;

   			// Block every signal during the handler
   			sigemptyset(&sh.sa_mask);

   			if (sigaction(SIGPIPE, &sh, &osh) < 0)
   			{
				std::cout << "sigcation failed!" << std::endl;
   			}
			//
		}

		void Listen(std::string_view _address, int _port)
		{
			this->listenSignal = true;
			sockaddr_in serverAddr = {};

			serverAddr.sin_family = AF_INET;
			serverAddr.sin_addr.s_addr = inet_addr(_address.data());
			serverAddr.sin_port = htons(_port);

			std::thread proc(HttpServer::ListenProc, this, serverAddr);

			std::cout << "server listening... ip:" << _address << " port:" << ntohs(serverAddr.sin_port) << std::endl;

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

	HttpResponse Json(std::string_view _str)
	{
		return HttpResponse(200, _str.data(), _str.size());
	}
};
