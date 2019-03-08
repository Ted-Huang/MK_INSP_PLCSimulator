#pragma once
#include "afxsock.h"
#include "AsyncSocketSession.h"
#include <vector>
using namespace std;
#define PLC_PORT	9999


class CAsyncSocketServer : public CAsyncSocket, public ISessionNotify
{
public:
	CAsyncSocketServer();
	virtual ~CAsyncSocketServer();

	BOOL Start();
	void SendData(BYTE cCmdType, BYTE* pData);
protected:
	virtual void OnAccept(int nErrorCode);

	//ISessionNotify
	virtual void OnError(void *pInstance, long ErrorId, long ErrorData);
private:
	void Init();
	void Finalize();

private:

	vector<CAsyncSocketSession*> m_vSession;
};