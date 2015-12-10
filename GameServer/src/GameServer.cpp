#include "GameServer.h"

GameServer* GameServer::_instance = nullptr;

GameServer* GameServer::getInstance()
{
	if (_instance == nullptr)
	{
		_instance = new GameServer;
	}
	return _instance;
}

void GameServer::cleanup()
{
	delete _instance;
	_instance = nullptr;
}

GameServer::GameServer()
{
	ServerConfig svrConfig;
	svrConfig.listenPort = 10001;
	svrConfig.maxConnection = 2000;
	svrConfig.numIOCPThreads = 0;	// auto
	svrConfig.kickTime = std::chrono::minutes(2);
	svrConfig.createClient = std::bind(&GameServer::createClient, this);
	svrConfig.destroyClient = std::bind(&GameServer::destroyClient, this, std::placeholders::_1);
	logicServer = new ServerSocket(svrConfig);

	svrConfig.listenPort = 10011;
	svrConfig.maxConnection = 50;
	svrConfig.numIOCPThreads = 1;	// auto
	svrConfig.kickTime = std::chrono::minutes(30);
	svrConfig.createClient = std::bind(&GameServer::createClient, this);
	svrConfig.destroyClient = std::bind(&GameServer::destroyClient, this, std::placeholders::_1);
	consoleServer = new ServerSocket(svrConfig);

	// init db
	MYSQL_CONFIG dbConfig;
#ifdef WIN32
	dbConfig.strHost = "192.168.11.151";
	dbConfig.strPassword = "";
#elif LINUX
	dbConfig.strHost = "localhost";
	dbConfig.strPassword = "";
	dbConfig.strUnixSock = "/tmp/mysql.sock";
#endif
	dbConfig.nPort = 3306;
	dbConfig.strUser = "root";
	dbConfig.strDB = "star2015";
	dbQueue = new DBQueue;
	dbQueue->init(5, dbConfig);
}

GameServer::~GameServer()
{
	delete logicServer;
	delete consoleServer;
	delete dbQueue;
}

int GameServer::startListen()
{
	if (logicServer->startListen() == -1)
	{
		return -1;
	}
	if (consoleServer->startListen() == -1)
	{
		return -1;
	}
	return 0;
}

void GameServer::update()
{
	logicServer->update();
	consoleServer->update();
	dbQueue->update();
}

ClientSocket* GameServer::createClient()
{
	return new Client;
}

void GameServer::destroyClient(ClientSocket* cs)
{
	delete cs;
}
