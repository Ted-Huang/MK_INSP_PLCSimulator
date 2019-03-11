#pragma once
#include "afxsock.h"
#include "PLC_PACKET_const.h"

#define MAX_RECEIVE_BUFFER_SIZE 64000
#define MAX_SEND_BUFFER_SIZE 16000

class ISESSION_NOTIFY
{
public:
	ISESSION_NOTIFY()
	{
		m_pNotoify = NULL;
	}
public:


	void AttachNotify(ISESSION_NOTIFY *pLink) { m_pNotoify = pLink; };

	void OnSocketNotify(void *pInstance, long ErrorId)
	{
		if (m_pNotoify){
			m_pNotoify->OnSocketNotify(pInstance, ErrorId);
		}
		else{
			DoSocketNotify(pInstance, ErrorId);
		}
	}
	void OnSessionReceivePacket(void *pInstance, PLC_CMD_FIELD_BODY* pBody)
	{
		if (m_pNotoify){
			m_pNotoify->OnSessionReceivePacket(pInstance, pBody);
		}
		else{
			DoSessionReceivePacket(pInstance, pBody);
		}
	}
protected:
	virtual void DoSocketNotify(void *pInstance, long ErrorId) {};
	virtual void DoSessionReceivePacket(void *pInstance, PLC_CMD_FIELD_BODY* pBody){};
protected:
	enum {
		ERR_SDK_SOCKET_CLOSE = 0,
		ERR_SDK_SOCKET_CONNECT = 1,
		ERR_SDK_SOCKET_SEND = 2,
		ERR_SDK_SOCKET_NOECHO = 3,
	};
private:
	ISESSION_NOTIFY *m_pNotoify;
};

class CAsyncSocketSession : public CAsyncSocket, public ISESSION_NOTIFY
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
	void CheckDataBuf();
	void MovePacketToStart(BYTE **ppCurrent, int &DataSize);
	bool ParseCommand(PLC_CMDEX_PACKET *pData);
	bool SyncPacketCheck(PLC_CMDEX_PACKET* pData);

private:
	int m_nReceiveSize;
	BYTE m_cReceiveBuf[MAX_RECEIVE_BUFFER_SIZE];
	int m_nSendSize;
	BYTE m_cSendBuf[MAX_SEND_BUFFER_SIZE];
};