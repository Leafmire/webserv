#include "parser.hpp"

parser::parser(const char* path)
{
	m_fileReader.initFileReader(path);
	configInfo::setTable();
}

parser::~parser()
{}

void
parser::makeToken(void)
{
	t_token		token;
	tokenizer	tokenizer;

	while (1)
	{
		if (m_fileReader.isEof())
			break;
		token = m_fileReader.readFile();
		m_tokenizer.pushBackToken(token);
	}
	m_configBuffer = m_fileReader.getAllBuffer();
}

// void
// parser::parse(void)
// {
//     try
//     {
//         m_tokenizer.parseToken(m_config);
//     }
//     catch (WsException& e)
//     {
//         e.printConfigErr(m_configBuffer);
//         throw WsException("invalid config file");
//     }
// }
//
// void
// parser::configParse(void)
// {
//     makeToken();
//     parse();
//     for (size_t i = 0; i < m_config.size(); i++)
//     {
//         m_config[i].checkConfig();
//         // std::cout << m_config[i] << std::endl;
//     }
// }

const std::vector<configInfo>&
parser::getConfigInfo(void)
{
	return (m_config);
}


// refactor
void
parser::parse(std::vector<configInfo>& configInfo,
			  std::fstream& logFile)
{
	try
	{
		m_tokenizer.parseToken(configInfo);
	}
	catch (WsException& e)
	{
		e.printConfigErr(m_configBuffer, logFile);
		throw WsException("invalid config file");
	}
}

void
parser::configParse(std::vector<configInfo>& configInfo,
					std::fstream& logFile)
{
	makeToken();
	parse(configInfo, logFile);
	for (size_t i = 0; i < configInfo.size(); i++)
	{
		configInfo[i].checkConfig();
		logFile << configInfo[i] << std::endl;
	}
}

