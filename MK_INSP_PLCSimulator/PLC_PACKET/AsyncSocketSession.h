#pragma once
#include "afxsock.h"

#define MAX_RECEIVE_BUFFER_SIZE 64000
#define MAX_SEND_BUFFER_SIZE 16000

class CAsyncSocketSession : public CAsyncSocket
{
public:
	CAsyncSocketSession();
	virtual ~CAsyncSocketSession();


protected:
	virtual void OnClose(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);

private:
	void Init();
	void Finalize();

private:

	int m_nReceiveSize;
	BYTE m_cReceiveBuf[MAX_RECEIVE_BUFFER_SIZE];
	int m_nSendSize;
	BYTE m_cSendBuf[MAX_SEND_BUFFER_SIZE];
};