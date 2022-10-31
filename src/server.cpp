#include "server.hpp"

server::server(const std::vector<configInfo> &conf, std::fstream& logFile)
	:m_conf(conf), m_logFile(logFile)
{
	m_serverSize = m_conf.size();
	m_kq = kqueue();
	if (m_kq < 0)
		throw (WsException("kqueue fail"));
}

server::~server()
{}

void
server::createServerSock(void)
{
	for (size_t serverSockIdx = 0; serverSockIdx < m_serverSize; serverSockIdx++)
	{
		serverSocket serverSock(m_conf[serverSockIdx]);

		serverSock.createSock();
		serverSock.initAddr();
		serverSock.bindSock();
		serverSock.listenSock();
		m_serverSock.insert(std::make_pair(serverSock.getSocketFd(), serverSock));
		addEvents(serverSock.getSocketFd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	}
	m_logFile << "all server socket created" << std::endl;
}

void
server::run(void)
{
	int newEvents;
	response::setStatusCode();

	while (1)
	{
		newEvents = waitEvent();
		communicateSock(newEvents);
	}
}

void
server::addEvents(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata)
{
	struct kevent tempEvent;

	EV_SET(&tempEvent, ident, filter, flags, fflags, data, udata);
	m_changeList.push_back(tempEvent);
}

int
server::waitEvent(void)
{
	int newEvents;

	m_logFile << "\n---waiting event---" << std::endl;
	newEvents = kevent(m_kq, &m_changeList[0], m_changeList.size(),
			m_eventList, EVENT_SIZE, NULL);
	m_logFile << "new events count : " << newEvents << std::endl;
	if (newEvents == -1)
		throw (WsException("kevent fail"));
	m_changeList.clear();
	return (newEvents);
}

void
server::communicateSock(int newEvents)
{
	for (int i = 0; i < newEvents; i++)
	{
		struct kevent* curEvent;

		curEvent = &m_eventList[i];
		m_logFile << "new event fd : " << curEvent->ident << std::endl;
		if (curEvent->filter == EVFILT_READ)
			m_logFile << "[read] event occured" << std::endl;
		else if (curEvent->filter == -2)
			m_logFile << "[write] event occured" << std::endl;
		if (curEvent->flags & EV_ERROR)
		{
			if (isServerSocket(curEvent->ident))
				throw (WsException("server socket fail"));
			if (isClientSocket(curEvent->ident))
				throw (WsException("client socket fail"));
		}
		else if (curEvent->filter == EVFILT_READ)
		{
			if (readEvent(curEvent))
					return ;
		}
		else if (curEvent->filter == EVFILT_WRITE)
			writeEvent(curEvent);
	}
}

bool
server::isServerSocket(int fd)
{
	if (m_serverSock.find(fd) != m_serverSock.end())
		return (true);
	return (false);
}

bool
server::isClientSocket(int fd)
{
	if (m_clientSock.find(fd) != m_clientSock.end())
		return (true);
	return (false);
}

int
server::readEvent(struct kevent* curEvent)
{
	if (isServerSocket(curEvent->ident))
	{
		clientSocket clientSock(m_serverSock.at(curEvent->ident));

		clientSock.createSock();
		m_clientSock.insert(std::make_pair(clientSock.getSocketFd(), clientSock));
		addEvents(clientSock.getSocketFd(), EVFILT_READ,
				EV_ADD | EV_ENABLE, 0, 0, NULL);
		m_logFile << "client socket[" << clientSock.getSocketFd() << "] created, server read finish ";
		m_logFile << "now client socket size : " << m_clientSock.size() << std::endl;
		return (1);
	}
	else if(isClientSocket(curEvent->ident))
	{
		int readRet;

		std::map<int, clientSocket>::iterator clientIt =
			m_clientSock.find(curEvent->ident);
		readRet = (*clientIt).second.readSock(m_logFile);
		if (readRet <= 0)
		{
			if (readRet < 0)
				m_logFile << "client read error" << std::endl;
			disconnectClient(curEvent->ident);
			return (1);
		}
		if ((*clientIt).second.getReadStatus())
			addEvents((*clientIt).first, EVFILT_WRITE,
				EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL);
		m_logFile << "client read finish" << std::endl;
	}
	return (0);
}

void
server::writeEvent(struct kevent* curEvent)
{
	if (isClientSocket(curEvent->ident))
	{
		int sendRet;

		std::map<int, clientSocket>::iterator clientIt =
			m_clientSock.find(curEvent->ident);
		sendRet = (*clientIt).second.sendSock(m_logFile);
		if (sendRet < 0)
		{
			m_logFile << "client send error" << std::endl;
			disconnectClient(curEvent->ident);
		}
		else
			m_logFile << "client send finish" << std::endl;
	}
}

void
server::disconnectClient(int fd)
{
	m_logFile << "client disconnected : " << fd << std::endl;
	m_clientSock.at(fd).closeSock();
	m_clientSock.erase(fd);
}
