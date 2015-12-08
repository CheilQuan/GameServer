#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "Message.h"
#include "ServerSocket.h"

using namespace ws;

class Client : public ClientSocket
{
public:
	Client();
	virtual ~Client();
	virtual void onRecv();

protected:
	MessageHead* pHead;

};

#endif