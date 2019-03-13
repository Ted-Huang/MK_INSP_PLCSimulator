#include "stdafx.h"
#include "AppLogProcess.h"

//--------------------------------
//  AppLogBase
//--------------------------------

AppLogBase::AppLogBase()
{
	_tgetcwd(m_workingDir, _MAX_PATH);
	m_nFileCount = sizeof(ctLOG_INFO) / sizeof(ctLOG_INFO[0]);
}
AppLogBase::~AppLogBase()
{


}
void AppLogBase::InsertLog(CString &xMsg, AOI_LOG_TYPE xType)
{
	DWORD cbBuf = 0;
	CString MySysLogFileName;
	MySysLogFileName.Format(_T("%s\\%s"), m_workingDir, ctLOG_INFO[LOG_SYSTEM].strFile);
	int nSize = m_nFileCount;
	LOG_ITEM_INFO xTarget = ctLOG_INFO[LOG_SYSTEM];
	for (int i = 0; i < nSize; i++){
		if (ctLOG_INFO[i].xType == xType){
			MySysLogFileName.Format(_T("%s\\%s"), m_workingDir, ctLOG_INFO[i].strFile);
			xTarget = ctLOG_INFO[i];
			break;
		}
	}

	if (xMsg.GetLength() < (int)xTarget.nLimitSize){
		CFile fileLog;
		if (fileLog.Open(MySysLogFileName, CFile::modeReadWrite) ||
			fileLog.Open(MySysLogFileName, CFile::modeReadWrite | CFile::modeCreate)){
			CString newLog(CTime::GetCurrentTime().Format(_T("%c ")) + xMsg + _T("\r\n"));
#ifdef _UNICODE
			wchar_t *pNewLog = newLog.GetBuffer();
			UINT nLogSize = lstrlenA(CW2A(pNewLog));
#else
			UINT nLogSize = lstrlen(newLog);
#endif
			UINT szNew = (UINT)fileLog.GetLength() + nLogSize;
			cbBuf = (UINT)((szNew>xTarget.nLimitSize) ? xTarget.nLimitSize : szNew);
			char* pBuf = new char[cbBuf + 1];
			pBuf[cbBuf] = 0;
#ifdef 	_UNICODE
			lstrcpyA(pBuf, CW2A(pNewLog));
			newLog.ReleaseBuffer();
#else //_UNICODE
			lstrcpy(pBuf, newLog);
#endif //_UNICODE
			UINT cbLog = cbBuf - nLogSize;
			char* pLog = pBuf + nLogSize;
			TRY
			{
				if (cbLog)
				{
					VERIFY(fileLog.Read(pLog, cbLog) == cbLog);
					fileLog.SeekToBegin();
				}
				fileLog.Write(pBuf, cbBuf);
				fileLog.SetLength(cbBuf);
			}
				CATCH_ALL(e)
				END_CATCH_ALL
				if (pBuf) { delete[] pBuf; pBuf = NULL; }
			TRY	fileLog.Close();
			CATCH_ALL(e) END_CATCH_ALL
				newLog.ReleaseBuffer();
		}
	}
}


//--------------------------------
// CAoiLogThread
//---------------------------------

IMPLEMENT_DYNCREATE(CAoiLogThread, CWinThread)

BEGIN_MESSAGE_MAP(CAoiLogThread, CWinThread)
	ON_THREAD_MESSAGE(WM_LOG_PROCESS, OnLogMessage)
	ON_THREAD_MESSAGE(WM_LOG_TERMINATE, OnLogExit)
END_MESSAGE_MAP()

void CAoiLogThread::OnLogExit(WPARAM wParam, LPARAM lParam)
{
	AfxEndThread(0);
}

void CAoiLogThread::OnLogMessage(WPARAM wParam, LPARAM lParam)
{
	CString *pMsg = (CString*)lParam;
	InsertLog(*pMsg, (AOI_LOG_TYPE)wParam);
	if (pMsg){
		delete pMsg;
	}
}
//------------------

AppLogProcess::AppLogProcess()
{
	Init();
}

AppLogProcess::~AppLogProcess()
{
	Finalize();
}
void AppLogProcess::Init()
{
	m_pLogThread = NULL;
}
void AppLogProcess::Finalize()
{
	OpLogThread(OP_THREAD_DESTROY);
}
CString AppLogProcess::GetLogFileName(int xType)
{
	CString strName;
	int nCount = LogFileCount();
	for (int i = 0; i < nCount; i++){
		if (ctLOG_INFO[i].xType == xType){
			strName = ctLOG_INFO[i].strFile;
			break;
		}
	}
	return strName;
}
void AppLogProcess::OpLogThread(int nOpCode)
{
	if (nOpCode == OP_THREAD_CREATE){
		OpLogThread(OP_THREAD_DESTROY);
		m_pLogThread = AfxBeginThread(RUNTIME_CLASS(CAoiLogThread), THREAD_PRIORITY_TIME_CRITICAL, 0, CREATE_SUSPENDED);
		if (m_pLogThread){
			SetPriorityClass(HANDLE(m_pLogThread), REALTIME_PRIORITY_CLASS);
			m_pLogThread->m_bAutoDelete = TRUE;
			m_pLogThread->ResumeThread();
		}
	}
	else if (nOpCode == OP_THREAD_DESTROY){
		if (m_pLogThread){
			m_pLogThread->PostThreadMessage(WM_LOG_TERMINATE, NULL, NULL);
			m_pLogThread = NULL;
		}
	}
}
void AppLogProcess::InsertDebugLog(CString xMsg, AOI_LOG_TYPE xType)
{
	if (m_pLogThread){ //eric chao 20160603
		CString *pNewStr = new CString;
		*pNewStr = xMsg;
		m_pLogThread->PostThreadMessage(WM_LOG_PROCESS, xType, (LPARAM)pNewStr);
	}
	else{
		InsertLog(xMsg, xType);
	}
}