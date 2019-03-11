#pragma once
#include "afxsock.h"
#include "AsyncSocketSession.h"
#include <vector>
using namespace std;
#define PLC_PORT	9999


class CAsyncSocketServer : public CAsyncSocket, public ISESSION_NOTIFY
{
public:
	CAsyncSocketServer();
	virtual ~CAsyncSocketServer();

	BOOL Start();
	void SendData(BYTE cCmdType, BYTE* pData);
protected:
	virtual void OnAccept(int nErrorCode);

public:
	//ISESSION_NOTIFY
	virtual void DoSocketNotify(void *pInstance, long ErrorId);
private:
	void Init();
	void Finalize();

private:

	vector<CAsyncSocketSession*> m_vSession;
};