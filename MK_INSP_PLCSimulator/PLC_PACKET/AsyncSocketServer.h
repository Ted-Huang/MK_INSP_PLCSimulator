#pragma once
#include "afxsock.h"
#include <vector>
using namespace std;

#define PLC_PORT	9999
class CAsyncSocketSession;
class CAsyncSocketServer : public CAsyncSocket
{
public:
	CAsyncSocketServer();
	virtual ~CAsyncSocketServer();

	BOOL Start();
	//void Send
protected:
	virtual void OnAccept(int nErrorCode);

private:
	void Init();
	void Finalize();

private:

	vector<CAsyncSocketSession*> m_vSession;
};