#include "stdafx.h"
#include "PLC_PACKET_const.h"
#include "AsyncSocketServer.h"



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

void CAsyncSocketServer::SendData(BYTE cCmdType, BYTE* pData)
{
	if (!pData)
		return;

	PLC_CMDEX_PACKET xPacket;
	memset(&xPacket, 0, sizeof(PLC_CMDEX_PACKET));
	xPacket.cStart = CMD_START;
	xPacket.cCmdType = cCmdType;
	memcpy(xPacket.cBody, pData, sizeof(PLC_CMD_FIELD_BODY));
	xPacket.cEnd = CMD_END;

	auto it = m_vSession.begin();
	while (it != m_vSession.end()){
		if (*it){
			(*it)->Send(&xPacket, sizeof(PLC_CMDEX_PACKET));
		}
		it++;
	}
}

void CAsyncSocketServer::OnAccept(int nErrorCode)
{
	CAsyncSocket::OnAccept(nErrorCode);
	if (nErrorCode != NO_ERROR)
		return;

	CAsyncSocketSession* pSession = new CAsyncSocketSession;
	m_vSession.push_back(pSession);
	Accept(*pSession);

	pSession->AttachNotify(this);
}

void CAsyncSocketServer::OnError(void *pInstance, long ErrorId, long ErrorData)
{
	if (!pInstance)
		return;

	auto it = m_vSession.begin();
	while (it != m_vSession.end()){
		if (*it && *it == pInstance){
			delete *it;
			*it = NULL;
			m_vSession.erase(it);
			break;
		}
		it++;
	}
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
	while (m_vSession.size()){
		CAsyncSocketSession* pSession = m_vSession.back();
		if (pSession){
			pSession->Close();
			delete pSession;
			pSession = NULL;
		}
		m_vSession.pop_back();
	}
}

