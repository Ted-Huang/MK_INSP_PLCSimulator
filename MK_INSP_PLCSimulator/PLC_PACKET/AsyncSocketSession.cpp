#include "stdafx.h"
#include "AsyncSocketSession.h"
#include "MK_INSP_PLCSimulator.h"

CAsyncSocketSession::CAsyncSocketSession()
{
	Init();
}

CAsyncSocketSession::~CAsyncSocketSession()
{

}

void CAsyncSocketSession::Init()
{
	m_nReceiveSize = 0;
	m_nSendSize = 0;
	memset(m_cReceiveBuf, 0, sizeof(m_cReceiveBuf));
	memset(m_cSendBuf, 0, sizeof(m_cSendBuf));
}


void CAsyncSocketSession::OnClose(int nErrorCode)
{
	OnSocketNotify(this, ERR_SDK_SOCKET_CLOSE);
	
	CAsyncSocket::OnClose(nErrorCode);
}

void CAsyncSocketSession::OnReceive(int nErrorCode)
{
	int nRead = 0;
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
		CString strLogSrc;
		strLogSrc.Format(_T("REV----%02X%02X%02X%02X%02X%02X%02X"), pData->cStart, pData->cCmdType, pData->cBody[0], pData->cBody[1], pData->cBody[2], pData->cBody[3], pData->cEnd);
		theApp.InsertDebugLog(strLogSrc, LOG_PLCSOCKET);
		switch (pData->cCmdType){
		case CMDTYPE_QUERYALIVE:
			{
				CString strMsg;
				strMsg.Format(L"igonre query alive cmd");
				TRACE(strMsg + L"\n");

				theApp.InsertDebugLog(strMsg, LOG_PLCSOCKET);
			}
			break;
		case CMDTYPE_OP:
			{
				PLC_CMD_FIELD_BODY* pBody = new PLC_CMD_FIELD_BODY;
				memcpy(pBody, pData->cBody, sizeof(PLC_CMD_FIELD_BODY));
				OnSessionReceivePacket(this, pBody);
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