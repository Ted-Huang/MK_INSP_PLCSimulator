#include "stdafx.h"
#include "AsyncSocketSession.h"
#include "MK_INSP_PLCSimulator.h"

#define DUMP_FIELD_PACKET
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CSocketThread, CWinThread)

CSocketThread::CSocketThread()
{

}
void CSocketThread::SetSession(CAsyncSocketSession* pSession)
{
	m_pSession = pSession;
}
CSocketThread::~CSocketThread()
{

}
BOOL CSocketThread::InitInstance()
{
	return TRUE;
}

int CSocketThread::ExitInstance()
{
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CSocketThread, CWinThread)
	ON_THREAD_MESSAGE(WM_SOCKETMSG, OnSocketMessage)
END_MESSAGE_MAP()

void CSocketThread::OnSocketMessage(WPARAM wParam, LPARAM lParam)
{
	switch (wParam){
	case IDM_SOCKETTHREADQUIT_MSG:
		AfxEndThread(0);
		break;
	case IDM_LOG_MSG:
		{
			PLC_CMDEX_PACKET* pData = (PLC_CMDEX_PACKET*)lParam;
			CString strLogSrc;
			strLogSrc.Format(_T("REV----%02X%02X%02X%02X%02X%02X%02X"), pData->cStart, pData->cCmdType, pData->cBody[0], pData->cBody[1], pData->cBody[2], pData->cBody[3], pData->cEnd);
			theApp.InsertDebugLog(strLogSrc, LOG_PLCSOCKET);
			DumpFieldCmdLog((PLC_CMD_FIELD_BODY*)pData->cBody);
			delete pData;
		}
		break;
	case IDM_QUERYALIVESEND_MSG:
		{	
			PLC_CMDEX_ALIVE_BODY *pSrc = (PLC_CMDEX_ALIVE_BODY*)lParam;
			if (!pSrc)
				return;

			PLC_CMDEX_PACKET xCmd;
			memset(&xCmd, 0, sizeof(xCmd));
			PLC_CMDEX_ALIVE_BODY *pBody = (PLC_CMDEX_ALIVE_BODY*)xCmd.cBody;
			xCmd.cStart = CMD_START;
			xCmd.cEnd = CMD_END;
			xCmd.cCmdType = CMDTYPE_QUERYALIVE;
			
			BOOL bSendFlag = FALSE;

			if (pSrc->cTypeR == 3){ //Receive Flag Echo
				//Record Echo Time....
				pBody->cTypeR = 1;
				pBody->cValR = pSrc->cValR;
				bSendFlag = TRUE;
			}
			else if (pSrc->cTypeR == 1){
				pBody->cTypeR = 3;
				pBody->cValR = (pSrc->cValR + 1) & 0xFF;
				bSendFlag = TRUE;
			}

			if (pSrc->cTypeS == 0){ //Send Flag Query
				pBody->cTypeS = 2;
				pBody->cValS = (pSrc->cValS + 1) & 0xFF;
				bSendFlag = TRUE;
			}
			else if (pSrc->cTypeS == 2){
				pBody->cTypeS = 0;
				pBody->cValS = pSrc->cValS;
				bSendFlag = TRUE;
			}
			if (bSendFlag){
				if (m_pSession){
					m_pSession->Send(&xCmd, sizeof(xCmd));
				}
			}
			if (pSrc)
				delete pSrc;
		}
		break;
	}
}

void CSocketThread::DumpFieldCmdLog(PLC_CMD_FIELD_BODY *pData)
{
	PLC_CMD_FIELD_BODY *pBody = (PLC_CMD_FIELD_BODY*)pData;
	//if (pBody->cOpCode == 1){
	CString strLog;
	CString strTemp;
	switch (pBody->cCh){
	case 0:
		strTemp = _T("CAM(ROLL)");
		break;
	case 1:
		strTemp = _T("CAM(OP)");
		break;
	case 2:
		strTemp = _T("CAM(SIDE)");
		break;
	case 15:
		strTemp = _T("CAM(ALL)");
		break;
	}
	strLog += strTemp;
	switch (pBody->cField){
	case FIELD_INSP_RESULT:			//對應原本的 COM.VALUE
		strTemp = _T("--FIELD_INSP_RESULT--");
		break;
	case FIELD_INSP_ERR:			//對應原本的 COM.ERROR
		strTemp = _T("--FIELD_INSP_ERR--");
		break;
	case FIELD_INSP_TOGGLEBIT:	//對應原本的 COM.TOGGLEBIT
		strTemp = _T("--FIELD_INSP_TOGGLEBIT--");
		break;
	case FIELD_INSP_MODE:		//對應原本的 MODE.SOLL
		strTemp = _T("--FIELD_INSP_MODE--");
		break;
	case FIELD_INSP_VERIFY:		//對應原本的 COM.VERIFY
		strTemp = _T("--FIELD_INSP_VERIFY--");
		break;
	case FIELD_CAM_ONLINE:		//對應原本的 COM.ONLINE
		strTemp = _T("--FIELD_CAM_ONLINE--");
		break;
	case FIELD_CAM_DIR:			//對應原本的 COM.DIRECTION
		strTemp = _T("--FIELD_CAM_DIR--");
		break;
	case FIELD_CAM_IMG_RECVBIT:	//對應原本的 COM.IMGRECEIVEBIT
		strTemp = _T("--FIELD_CAM_IMG_RECVBIT--");
		break;
	case FIELD_BAR_WIDTH:		//對應	  MARKWIDTH
		strTemp = _T("--FIELD_BAR_WIDTH--");
		break;
	case FIELD_GOLDEN_RESET:		//通知系統 Golden Reset
		strTemp = _T("--FIELD_GOLDEN_RESET--");
		break;
	case FIELD_VERIFY_RESET:		//通知系統 Verify Golden Image Reset
		strTemp = _T("--FIELD_VERIFY_RESET--");
		break;
	case FIELD_GOLDEN_READY:		//通知PLC端 Golden Ready
		strTemp = _T("--FIELD_GOLDEN_READY--");
		break;
	case FIELD_VERIFY_READY:		//通知PLC端 Verify Golden Ready
		strTemp = _T("--FIELD_VERIFY_READY--");
		break;
	case FIELD_INSP_TRIGGER:		//通知系統 進行檢測
		strTemp = _T("--FIELD_INSP_TRIGGER--");
		break;
	case FIELD_INSP_VERIFY_TRIGGER://通知系統 進行2次校驗
		strTemp = _T("--FIELD_INSP_VERIFY_TRIGGER--");
		break;
	case FIELD_INSP_VERIFY_RESULT:
		strTemp = _T("--FIELD_INSP_VERIFY_RESULT--");
		break;
	}
	strLog += strTemp;
	strTemp.Format(_T("Value(%d)"), (int)pBody->wValue);
	strLog += strTemp;
	theApp.InsertDebugLog(strLog, LOG_PLCSOCKET);
	//}

}
CAsyncSocketSession::CAsyncSocketSession()
{
	Init();
}

CAsyncSocketSession::~CAsyncSocketSession()
{
	if (m_pSocketThread){
		m_pSocketThread->PostThreadMessage(WM_SOCKETMSG, IDM_SOCKETTHREADQUIT_MSG, NULL);
		m_pSocketThread = NULL;
	}
}

void CAsyncSocketSession::Init()
{
	//QueryPerformanceFrequency(&freq);

	m_nReceiveSize = 0;
	m_nSendSize = 0;
	memset(m_cReceiveBuf, 0, sizeof(m_cReceiveBuf));
	memset(m_cSendBuf, 0, sizeof(m_cSendBuf));
	m_pSocketThread = AfxBeginThread(RUNTIME_CLASS(CSocketThread), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	((CSocketThread*)m_pSocketThread)->SetSession(this);
	m_pSocketThread->ResumeThread();
}


void CAsyncSocketSession::OnClose(int nErrorCode)
{
	OnSocketNotify(this, ERR_SDK_SOCKET_CLOSE);
	
	CAsyncSocket::OnClose(nErrorCode);
}

void CAsyncSocketSession::OnReceive(int nErrorCode)
{
	int nRead = 0;
	
	//QueryPerformanceCounter(&t1);
	nRead = Receive(m_cReceiveBuf + m_nReceiveSize, sizeof(m_cReceiveBuf) - m_nReceiveSize);
	bool bSuccess = true;
	switch (nRead){
	case 0: //already disconnect
		Close();
		bSuccess = false;
		break;
	case SOCKET_ERROR:
		if (GetLastError() != WSAEWOULDBLOCK){
			AfxMessageBox(_T("Error occurred"));
			Close();
			bSuccess = false;
		}
		break;
	default:
		m_nReceiveSize += nRead;
		break;
	}
	if (bSuccess){
		CheckDataBuf();
		//QueryPerformanceCounter(&t2);
		//double elapsedTime = ((t2.QuadPart - t1.QuadPart) * 1000.0) / freq.QuadPart;
		//CString strMsg;
		//strMsg.Format(L"\n packet handle time %.2f\n ", elapsedTime);
		//theApp.InsertDebugLog(strMsg, LOG_PLCSOCKET);
	}
	else{
		OnSocketNotify(this, ERR_SDK_SOCKET_CLOSE);
	}
	CAsyncSocket::OnReceive(nErrorCode);
}

void CAsyncSocketSession::CheckDataBuf()
{
	BYTE *pStart = m_cReceiveBuf;
	MovePacketToStart(&pStart, m_nReceiveSize);
	PLC_CMDEX_PACKET *pPacket = (PLC_CMDEX_PACKET*)m_cReceiveBuf;
	int nCmdPacketSize = sizeof(PLC_CMDEX_PACKET);
	while ((m_nReceiveSize >= sizeof(PLC_CMDEX_PACKET)) && (m_nReceiveSize >= nCmdPacketSize)){
		ParseCommand(pPacket);
		pPacket = (PLC_CMDEX_PACKET*)((BYTE*)pPacket + nCmdPacketSize);
		m_nReceiveSize -= nCmdPacketSize;
	}
}

void CAsyncSocketSession::MovePacketToStart(BYTE **ppCurrent, int &DataSize)
{
	if (ppCurrent && (*ppCurrent) && DataSize){
		BYTE *pPtr = *ppCurrent;
		long nNewSize = DataSize;
		while ((*(BYTE*)(pPtr)) != CMD_START){
			if (nNewSize <= 0){
				break;
			}
			pPtr++;
			nNewSize--;
		}
		long nOffset = DataSize - nNewSize;
		if (nOffset >0){
			//Move Memory & Clear Memory
			memmove(*ppCurrent, pPtr, nNewSize);
			memset(*ppCurrent + nNewSize, 0, nOffset);
			DataSize = nNewSize;
		}
	}
}

bool CAsyncSocketSession::ParseCommand(PLC_CMDEX_PACKET *pData)
{
	bool bFlag = false;
	if (SyncPacketCheck(pData)){

		switch (pData->cCmdType){
		case CMDTYPE_QUERYALIVE:
			{
				ParseQueryAliveCmd((PLC_CMDEX_ALIVE_BODY*)pData->cBody);
			}
			break;
		case CMDTYPE_OP:
			{	
				OnSessionReceivePacket(this, (PLC_CMD_FIELD_BODY*)pData->cBody);
#ifdef DUMP_FIELD_PACKET
				if (m_pSocketThread){
					PLC_CMDEX_PACKET* newPacket = new PLC_CMDEX_PACKET;
					memcpy(newPacket, pData, sizeof(PLC_CMDEX_PACKET));
					m_pSocketThread->PostThreadMessage(WM_SOCKETMSG, IDM_LOG_MSG, (LPARAM)newPacket);
				}
#endif
			}
			break;
		default:
			{
				CString strMsg;
				strMsg.Format(L"Unknown Command Type!");
				TRACE(strMsg + L"\n");

				theApp.InsertDebugLog(strMsg, LOG_PLCSOCKET);
			}
			break;
		}
	}
	return bFlag;
}

void CAsyncSocketSession::ParseQueryAliveCmd(PLC_CMDEX_ALIVE_BODY *pData)
{
	if (pData){
		if (m_pSocketThread){
			PLC_CMDEX_ALIVE_BODY* pBody = new PLC_CMDEX_ALIVE_BODY;
			memcpy(pBody, pData, sizeof(PLC_CMDEX_ALIVE_BODY));
			m_pSocketThread->PostThreadMessage(WM_SOCKETMSG, IDM_QUERYALIVESEND_MSG, (LPARAM)pBody);
		}
		
	}
}

bool CAsyncSocketSession::SyncPacketCheck(PLC_CMDEX_PACKET* pData)
{
	return pData->cStart == CMD_START && pData->cEnd == CMD_END;
}

void CAsyncSocketSession::OnSend(int nErrorCode)
{
	while (m_nSendSize >0){
		int dwBytes;
		if ((dwBytes = Send((LPCTSTR)m_cSendBuf, m_nSendSize)) == SOCKET_ERROR){
			if (GetLastError() == WSAEWOULDBLOCK) {
			}
			else{
				Close();
			}
			break;
		}
		else{
			int nDataSize = m_nSendSize - dwBytes;
			if (nDataSize){
				memmove(m_cSendBuf, m_cSendBuf + dwBytes, nDataSize);
				memset(m_cSendBuf + dwBytes, 0, nDataSize);
			}
			m_nSendSize -= dwBytes;
		}
	}
	CAsyncSocket::OnSend(nErrorCode);
}