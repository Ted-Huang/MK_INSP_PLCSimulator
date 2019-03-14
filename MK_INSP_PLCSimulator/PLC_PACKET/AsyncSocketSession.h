#pragma once
#include "afxsock.h"
#include "PLC_PACKET_const.h"

#define WM_SOCKETMSG					(WM_APP+2000)
#define IDM_SOCKETTHREADQUIT_MSG		(WM_APP+2001)
#define IDM_QUERYALIVESEND_MSG			(WM_APP+2002)
#define IDM_UPDATEUI_MSG				(WM_APP+2003)

#define MAX_RECEIVE_BUFFER_SIZE 64000
#define MAX_SEND_BUFFER_SIZE 16000
class CAsyncSocketSession;
class CQueryAliveThread :
	public CWinThread
{
	DECLARE_DYNCREATE(CQueryAliveThread)
public:
	CQueryAliveThread();
	void SetSession(CAsyncSocketSession* pSession);
	~CQueryAliveThread();
	virtual BOOL InitInstance();
	virtual int ExitInstance();
protected:
	DECLARE_MESSAGE_MAP()
	void OnSocketMessage(WPARAM wParam, LPARAM lParam);
private:
	CAsyncSocketSession* m_pSession;
};

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
	void ParseQueryAliveCmd(PLC_CMDEX_ALIVE_BODY *pData);
	void DumpFieldCmdLog(PLC_CMD_FIELD_BODY *pData);
	bool SyncPacketCheck(PLC_CMDEX_PACKET* pData);

private:
	int m_nReceiveSize;
	BYTE m_cReceiveBuf[MAX_RECEIVE_BUFFER_SIZE];
	int m_nSendSize;
	BYTE m_cSendBuf[MAX_SEND_BUFFER_SIZE];

	CWinThread* m_pQueryAliveThread;
};