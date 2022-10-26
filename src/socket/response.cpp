#include "response.hpp"
#include <strings.h>

std::map<int, std::string> response::s_statusCode;

response::response(const configInfo& conf)
{
	m_conf = conf;
	m_isCgi = 0;
	m_method = NULL;
	m_responseBuf.clear();
	m_filePath.clear();
}

response::~response()
{
	delete m_method;
}

void
response::makeStatusLine(void)
{

	int statusCode;

	m_filePath = m_method->getFilePath();
	statusCode = m_method->getStatusCode();

	std::cout << "Path: "<< m_filePath << std::endl;
	m_file.open(m_filePath);
	m_responseBuf = "HTTP/1.1 ";
	m_responseBuf += getStatusCodeStr(statusCode);
}

void
response::makeBody(void)
{
	std::string	readLine;
	std::string	allReadLine;
	size_t		allReadLineSize;

	if (m_isCgi == 0)
	{
		while (!m_file.eof())
		{
			std::getline(m_file, readLine);
			if (readLine == "")
				continue;
			allReadLine += readLine + "\n";
		}
		allReadLine.pop_back();
	}
	else
	{
		allReadLine = m_newBody;
	}
	allReadLineSize = allReadLine.size();
	m_responseBuf += "Content-Length: ";
	m_responseBuf += std::to_string(allReadLineSize) + "\n\n";
	m_responseBuf += allReadLine;
}

void
response::extractExt()
{
	std::string filepath = m_filePath;

	while (filepath.find("/") != std::string::npos)
	{
		filepath.erase(0, filepath.find("/") + 1);
	}
	m_fileExt = filepath.substr(filepath.find("."));
}

void
response::parseBody()
{
	if (m_newBody.find("Content-type:") != std::string::npos)
	{
		m_newBody = m_newBody.substr(m_newBody.find("Content-type:"));
		m_type = m_newBody.substr(0, m_newBody.find("\n"));
		m_newBody = m_newBody.substr(m_newBody.find("\n"));
	}
	if (m_fileExt == ".png")
	{
		m_type = "Content-type: image/png";
	}
	else
	{
		m_type = "Content-type: text/css, text/html; charset=UTF-8";
	}
}

int
response::checkIsCgi()
{
	// if (m_fileExt == ".htm" || m_fileExt == ".html" || m_fileExt == ".php")
	if (m_fileExt == ".php")
		return (1);
	else
	 	return (0);
}

void
response::makeResponse(const AMethod* method)
{
	cgi defaultCgi;

	m_method = method;

	makeStatusLine();
	extractExt();
	makeResponseHeader();
	makeGeneralHeader();
	m_isCgi = checkIsCgi();
	if (m_isCgi == 1)
	{
		defaultCgi.initCgi(m_method);
		m_newBody = defaultCgi.execCgi(m_method);
		std::cout << m_newBody << std::endl;
	}
	parseBody();
	makeEntityHeader();
	makeBody();
}

const std::string&
response::operator()(void)
{
	return (m_responseBuf);
}

size_t
response::getBufSize(void) const
{
	return (m_responseBuf.size());
}

// 특정 유형의 http 요청이나 특정 http 헤더를 수신 했을때, 이에 응답한다.
void
response::makeResponseHeader(void)
{
	// Server
	// 서버 소프트웨어 정보
	//
	// Accept-Range
	//
	// Set-Cookie
	//
	// 서버측에서 클라이언트에게 세션 쿠키 정보를 설정 (RFC 2965에서 규정)
	// (쿠키/캐시 관련)
	//
	// Expires (쿠키/캐시 관련)
	// 리소스가 지정된 일시까지 캐시로써 유효함을 나타낸다. 즉, 응답 컨텐츠가 언제 만료되는지를 나타낸다.
	// Ex) Expires: Thu, 26 Jul 2018 07:28:00 GMT
	// Cache-Control과 별개로 응답에 Expires라는 헤더를 줄 수 있다.
	// 단, Cache-Control의 max-age가 있는 경우 이 헤더는 무시
	//
	// Age (쿠키/캐시 관련)
	// 캐시 응답. max-age 시간 내에서 얼마나 흘렀는지 초 단위로 알려준다.
	//
	// ETag (쿠키/캐시 관련)
	// HTTP 컨텐츠가 바뀌었는지를 검사할 수 있는 태그
	//
	// Proxy-authenticate
	//
	// Allow
	// 해당 엔티티에 대해 서버 측에서 지원 가능한 HTTP 메소드의 리스트를 나타낸다.
	// 때론, HTTP 요청 메세지의 HTTP 메소드 OPTIONS에 대한 응답용 항목으로 사용된다.
	// OPTIONS: 웹서버측 제공 HTTP 메소드에 대한 질의
	// Ex) Allow: GET,HEAD
	// 405 Method Not Allowed 에러와 함께
	// 웹 서버에서 제공 가능한 HTTP 메서드는 GET, HEAD 뿐임을 알린다.
	//
	// Access-Control-Allow-Origin
	// 요청을 보내는 프론트 주소와 받는 백엔드 주소가 다르면 CORS 에러가 발생
	// 서버에서 이 헤더에 프론트 주소를 적어주어야 에러가 나지 않는다.
	// Ex) Access-Control-Allow-Origin: www.zerocho.com
	// 프로토콜, 서브도메인, 도메인, 포트 중 하나만 달라도 CORS 에러가 난다.
	// Ex) Access-Control-Allow-Origin: *
	// 만약 주소를 일일이 지정하기 싫다면 *으로 모든 주소에 CORS 요청을 허용되지만 그만큼 보안이 취약해진다.
	// 유사한 헤더로 Access-Control-Request-Method, Access-Control-Request-Headers, Access-Control-Allow-Methods, Access-Control-Allow-Headers 등이 있다.
	// Request와 Allow에서 Method 단수 복수 주의
	//
	m_responseBuf += "Server: " + m_conf.getServerName()[0] + "\n";
}

	// 요청 및 응답 메시지 모두에서 사용가능한 Entity(콘텐츠, 본문, 리소스등)에 대한 설명 헤더 항목
	//
	//	http 메시지 내 포함된 선택적인 개체에 대한 구체적인 미디어 타입등의 설명
	//	http 메서지는 이미지, 오디오, 비디오, html 문서, 전자메일 등의 개체를 운반할 수 있다.
void response::makeEntityHeader(void)
{
	// Content-Type (공통 헤더)
	// 해당 개체에 포함되는 미디어 타입 정보
	// 컨텐츠의 타입(MIME 미디어타입) 및 문자 인코딩(EUC-KR, UTF-8 등)을 지정한다.
	// 타입 및 서브타입으로 지정된다.
	// 	타입 : 10개 정도 표준으로 정의됨(application, audio, font, image, multiport등)
	// 	서브 타입 : 각 타입별로 수십에서 수백개 정도
	// ex) Content-Type: text/html; charset-latin-14
	// 	해당 개체가 html 텍스트 문서이고, iso-latin-1 문자 인코딩 방식으로 표현되는 것을 의미
	//
	// Content-Language (공통 헤더)
	// 해당 개체와 가장 잘 어울리는 사용자 언어(자연언어)
	//
	// Content-Encoding (공통 헤더)
	// 해당 개체 데이터의 압축 방식
	// Ex) Content-Encoding: gzip, deflate
	// 	만일 압축이 시행되었다면 Content-Encoding 및 Content-Length 2개 항목을 토대로 압축 해제가 가능하다.
	//
	// Content-Length (공통 헤더)
	// 전달되는 해당 개체의 바이트 길이 또는 크기(10진수)
	// 응답 메시지 Body의 길이를 지정하거나 특정 지정된 개체의 길이를 지정한다.
	//
	// Content-Location (공통 헤더)
	// 해당 개체의 실제 위치를 알려준다.
	//
	// Content-Disposition (응답 헤더)
	// 응답 Body를 브라우저가 어떻게 표시해야할지 알려준다.
	// inline인 경우 웹페이지 화면에 표시되고, attachment인 경우 다운로드한다.
	// Ex) Content-Disposition: inline
	// Ex) Content-Disposition: attachment; filename='filename.csv'
	// 다운로드되길 원하는 파일은 attachment로 값을 설정하고, filename 옵션으로 파일명까지 지정해줄 수 있다.
	// 파일용 서버인 경우 이 태그를 자주 사용한다.
	//
	// Content-Security-Policy (응답 헤더)
	// 다른 외부 파일들을 불러오는 경우, 차단할 소스와 불러올 소스를 명시한다.
	// XSS 공격에 대한 방어 가능 (허용한 외부 소스만 지정 가능)
	// Ex) Content-Security-Policy: default-src https:
	// https를 통해서만 파일을 가져온다.
	// Ex) Content-Security-Policy: default-src 'self'
	// 자신의 도메인의 파일들만 가져온다.
	// Ex) Content-Security-Policy: default-src 'none'
	// 파일을 가져올 수 없다.
	//
	// Location (응답 헤더)
	// 리소스가 리다이렉트(redirect)된 때에 이동된 주소, 또는 새로 생성된 리소스 주소를 명시한다.
	// 300번대 응답이나 201 Created 응답일 때 어느 페이지로 이동할지를 알려준다.
	// 새로 생성된 리소스의 경우
	// HTTP 상태 코드 201 Created가 반환된다.
	// 300번대 응답의 경우
	// HTTP/1.1 302 Found Location: /
	// 이런 응답이 왔다면 브라우저는 / 주소로 redirect한다.
	//
	// Last-Modified (응답 헤더)
	// 리소스를 마지막으로 갱신한 일시
	//
	// Transfer-Encoding
	//
	// chuncked (응답헤더)
	// 동적으로 생성되어 Body의 길이를 모르는 경우에 조금씩 전송이 가능하다.
	// 각 chunk 마다 그 시작에 16진수 길이를 삽입하여 chunk 길이를 알려준다.
	// m_responseBuf += "Content-Type: ";
	m_responseBuf += m_type + "\n";
	// m_responseBuf += "Content-Disposition: attachment; filename=soum.html";
	// m_responseBuf += "Content-Disposition: inline\n";
}

void response::makeGeneralHeader(void)
{
	m_responseBuf += "Date: " + getDate() + "\n";
	// m_responseBuf += "Connection: close\n";
	// Connection
	// 클라이언트와 서버간 연결에 대한 옵션 설정 (다소 모호한 복잡성 있음)
	// 	ex) Connection: close
	// 			현재 http 메시지 직후에 tcp 접속을 끊는다는 것을 알림
	// 		Connection: Keep-Alive
	// 			현재 tcp 커넥션을 유지한다.
	//
	// Cache-Control
	// 쿠키/캐시 관련
	//
	// Pragma
	//
	// Trailer
}

const std::string
response::getDate(void)
{
	time_t		curTime;
	struct tm*	curTimeInfo;
	char		timeBuf[1024];

	time(&curTime);
	curTimeInfo = localtime(&curTime);
	strftime(timeBuf, 1024, "%a, %d %b %Y %X %Z", curTimeInfo);
	return (timeBuf);
}

void
response::setStatusCode(void)
{
	s_statusCode[100] = "Continue";
	s_statusCode[101] = "Switching protocols";
	s_statusCode[200] = "OK";
	s_statusCode[201] = "Created";
	s_statusCode[202] = "Accepted";
	s_statusCode[203] = "Non-authoritative information";
	s_statusCode[204] = "No content";
	s_statusCode[301] = "Moved permanently";
	s_statusCode[302] = "Not temporarily";
	s_statusCode[304] = "Not modified";
	s_statusCode[400] = "Bad Request";
	s_statusCode[401] = "Unauthorized";
	s_statusCode[402] = "Payment required";
	s_statusCode[403] = "Forbidden";
	s_statusCode[404] = "Not found";
	s_statusCode[405] = "Method not allowed";
	s_statusCode[407] = "Proxy authentication required";
	s_statusCode[408] = "Request timeout";
	s_statusCode[410] = "Gone";
	s_statusCode[412] = "Precondition failed";
	s_statusCode[414] = "Request-URI too long";
	s_statusCode[500] = "Internal server error";
	s_statusCode[501] = "Not implemented";
	s_statusCode[503] = "Service unnailable";
	s_statusCode[504] = "Gateway timeout";
	s_statusCode[505] = "HTTP version not supported";
}

const std::string
response::getStatusCodeStr(int statusCode)
{
	std::string statusCodeStr;

	std::map<int, std::string>::iterator statusIt;
	statusIt = s_statusCode.find(statusCode);
	statusCodeStr = std::to_string(statusIt->first) + " ";
	statusCodeStr += statusIt->second + "\n";
	return (statusCodeStr);
}
