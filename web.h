#include <string>
#include <iostream>
#include <map>
#include <typeinfo>
#include <stdexcept>

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
};
