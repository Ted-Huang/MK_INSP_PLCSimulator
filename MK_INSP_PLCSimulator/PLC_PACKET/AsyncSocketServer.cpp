#include "stdafx.h"
#include "AsyncSocketServer.h"
#include "AsyncSocketSession.h"

CAsyncSocketServer::CAsyncSocketServer()
{
	Init();
}

CAsyncSocketServer::~CAsyncSocketServer()
{
	Finalize();
}

BOOL CAsyncSocketServer::Start()
{
	BOOL bFlag = FALSE;
	if (Create(PLC_PORT)){
		if (Listen()){
			bFlag = TRUE;
		}
	}
	return bFlag;
}

void CAsyncSocketServer::OnAccept(int nErrorCode)
{
	CAsyncSocket::OnAccept(nErrorCode);
	if (nErrorCode != NO_ERROR)
		return;

	CAsyncSocketSession* pSession = new CAsyncSocketSession;
	m_vSession.push_back(pSession);
	Accept(*pSession);

	//pSession->AttachNotify(this);
}

void CAsyncSocketServer::Init()
{
	if (!AfxSocketInit()){
		CString strLogMsg;
		strLogMsg.Format(_T("Failed to Initialize Sockets"));
		//theApp.InsertDebugLog(strLogMsg, LOG_PLCSOCKET);
		AfxMessageBox(strLogMsg, MB_OK | MB_ICONSTOP);
	}
}

void CAsyncSocketServer::Finalize()
{
}

