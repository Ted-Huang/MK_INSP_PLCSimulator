#include "stdafx.h"
#include "AsyncSocketSession.h"

CAsyncSocketSession::CAsyncSocketSession()
{
	Init();
}

CAsyncSocketSession::~CAsyncSocketSession()
{

}

void CAsyncSocketSession::Init()
{
	m_pINotify = NULL;
	m_nReceiveSize = 0;
	m_nSendSize = 0;
	memset(m_cReceiveBuf, 0, sizeof(m_cReceiveBuf));
	memset(m_cSendBuf, 0, sizeof(m_cSendBuf));
}


void CAsyncSocketSession::OnClose(int nErrorCode)
{
	if (m_pINotify){
		m_pINotify->OnError(this, ISessionNotify::ERR_SDK_SOCKET_CLOSE, NULL);
	}
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
		if (m_pINotify){
			m_pINotify->OnError(this, ISessionNotify::ERR_SDK_SOCKET_CLOSE, NULL);
		}
	}
	CAsyncSocket::OnReceive(nErrorCode);
}
void CAsyncSocketSession::CheckDataBuf()
{
	//BYTE *pStart = m_cDataBuf;
	//MovePacketToStart(&pStart, m_nDataSize);
	//AOI_SYNC_PACKET_HEADER *pHdr = (AOI_SYNC_PACKET_HEADER*)m_cDataBuf;
	//int nCmdPacketSize = sizeof(AOI_SYNC_PACKET_HEADER) + pHdr->nSize + 1; //Header+BodySize+CheckSum
	//while ((m_nDataSize >= sizeof(AOI_SYNC_PACKET_HEADER)) && (m_nDataSize >= nCmdPacketSize)){
	//	ParseCommand((BYTE*)pHdr, nCmdPacketSize);
	//	pHdr = (AOI_SYNC_PACKET_HEADER*)((BYTE*)pHdr + nCmdPacketSize);
	//	m_nDataSize -= nCmdPacketSize;
	//	nCmdPacketSize = sizeof(AOI_SYNC_PACKET_HEADER) + pHdr->nSize + 1;
	//}
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