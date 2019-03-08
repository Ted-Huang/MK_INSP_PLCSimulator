#pragma once
#include "afxsock.h"

#define MAX_RECEIVE_BUFFER_SIZE 64000
#define MAX_SEND_BUFFER_SIZE 16000

class ISessionNotify
{
public:
	enum {
		ERR_SDK_SOCKET_CLOSE = 0,
		ERR_SDK_SOCKET_CONNECT = 1,
		ERR_SDK_SOCKET_SEND = 2,
		ERR_SDK_SOCKET_NOECHO = 3,
	};
	virtual void OnError(void *pInstance, long ErrorId, long ErrorData) = 0;
	//virtual void OnDefectIndex(int nType, int nIndex) = 0;
};

class CAsyncSocketSession : public CAsyncSocket
{
public:
	CAsyncSocketSession();
	virtual ~CAsyncSocketSession();

	void AttachNotify(ISessionNotify *pLink) { m_pINotify = pLink; };
protected:
	virtual void OnClose(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);

private:
	void Init();
	void Finalize();

private:
	ISessionNotify* m_pINotify;
	int m_nReceiveSize;
	BYTE m_cReceiveBuf[MAX_RECEIVE_BUFFER_SIZE];
	int m_nSendSize;
	BYTE m_cSendBuf[MAX_SEND_BUFFER_SIZE];
};