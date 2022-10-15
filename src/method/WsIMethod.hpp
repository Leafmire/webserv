#ifndef WsIMethod_hpp
#define WsIMethod_hpp

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "../parser/WsConfigInfo.hpp"

#define RESET	"\033[0m"
#define RED		"\033[31m"
#define BLUE    "\033[34m"

class WsIMethod
{
	protected:

		std::string		m_method;
		std::string		m_uri;
		std::string		m_httpVersion;
		std::map<std::string, std::vector<std::string> >
						m_requestSet;
		int				m_statusCode;
		WsConfigInfo	m_conf;

		std::vector<std::string>	splitReadLine(const std::string& readLine,
				const std::string& str = " ");
		int							checkStartLine(std::vector<std::string>& splittedLine);
		void						loadBody(const std::string& readLine);

	public:
		WsIMethod(const std::string& readLine, const WsConfigInfo& conf);
		virtual ~WsIMethod();

		virtual void		loadRequest(const std::string& readLine) = 0;
		virtual void		printBody(void) const = 0;

		void				printInfo(void) const;
		const std::string&	getUri(void) const;
		const std::string&	getHttpVersion(void) const;
		const std::map<std::string, std::vector<std::string> >&
							getRequestSet(void) const;

		friend std::ostream& operator<<(std::ostream& os, const WsIMethod& method);

};
#endif //WsIMethod_hpp
