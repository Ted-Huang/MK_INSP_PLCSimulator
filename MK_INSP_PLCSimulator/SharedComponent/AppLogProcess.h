#pragma once
#include "afxwin.h"

const unsigned int _LOG_SIZE = 1024 * 1024;

enum AOI_LOG_TYPE{
	LOG_TYPE_BEGIN = 0,
	LOG_SYSTEM = 0,
	LOG_LIGHT,
	LOG_IO,
	LOG_WEBMODE,
	LOG_DEBUG,
	LOG_CFGCHECK,
	LOG_INSP,
	LOG_SDK_DEBUG,
	LOG_UPDATEDB,
	LOG_MODEL,
	LOG_TOOLBOX,
	LOG_P2_PROCRESS,
	LOG_PROJECT_DEBUG,
	LOG_EXDEBUG,
	LOG_SDKSYS_DECODE, //For Sean SDK
	LOG_SDKRES_DECODE, //For Sean SDK
	LOG_BATCH_SDK,
	LOG_AOICRASHED,
	LOG_SOLENOID,
	LOG_CHANGE_CLIENT,
	LOG_PLCSOCKET,
	LOG_PLCQUERYALIVE,
	LOG_TYPE_MAX
};

typedef struct LOG_ITEM_INFO_
{
	AOI_LOG_TYPE xType;
	CString strFile;
	UINT nLimitSize;
}LOG_ITEM_INFO;

const LOG_ITEM_INFO ctLOG_INFO[] = {
	{ LOG_SYSTEM, _T("System.log"), _LOG_SIZE },
	{ LOG_LIGHT, _T("LightInfo.log"), _LOG_SIZE },
	{ LOG_IO, _T("IoCard.log"), _LOG_SIZE },
	{ LOG_WEBMODE, _T("WebMode.log"), _LOG_SIZE },
	{ LOG_DEBUG, _T("Debug.log"), _LOG_SIZE },
	{ LOG_CFGCHECK, _T("CfgCheck.log"), _LOG_SIZE },
	{ LOG_INSP, _T("Insp.log"), _LOG_SIZE },
	{ LOG_SDK_DEBUG, _T("SDKDbg.log"), _LOG_SIZE },
	{ LOG_UPDATEDB, _T("UpdateDB.log"), _LOG_SIZE },
	{ LOG_MODEL, _T("InspModel.log"), _LOG_SIZE },
	{ LOG_TOOLBOX, _T("ToolBox.log"), _LOG_SIZE },
	{ LOG_P2_PROCRESS, _T("P2Procress.log"), _LOG_SIZE },
	{ LOG_PROJECT_DEBUG, _T("ProjectInfo.log"), _LOG_SIZE },
	{ LOG_EXDEBUG, _T("ExDebug.log"), _LOG_SIZE },
	{ LOG_SDKSYS_DECODE, _T("SDKsys.log"), _LOG_SIZE },
	{ LOG_SDKRES_DECODE, _T("SDKRsp.log"), _LOG_SIZE },
	{ LOG_BATCH_SDK, _T("BatchSDK.log"), _LOG_SIZE },
	{ LOG_AOICRASHED, _T("AoiCrashed.log"), _LOG_SIZE },
	{ LOG_SOLENOID, _T("Solenoid.log"), _LOG_SIZE },
	{ LOG_CHANGE_CLIENT, _T("AoiChangeClient.log"), _LOG_SIZE },
	{ LOG_PLCSOCKET, _T("PLCSocket.log"), _LOG_SIZE },
	{ LOG_PLCQUERYALIVE, _T("PLCSocketQueryAlive.log"), _LOG_SIZE },
};


#define WM_LOG_PROCESS		(WM_USER + 10)
#define WM_LOG_TERMINATE	(WM_USER + 11)

class AppLogBase
{
public:
	AppLogBase();
	~AppLogBase();
public:
	int LogFileCount() { return m_nFileCount; };
protected:
	void InsertLog(CString &xMsg, AOI_LOG_TYPE xType = LOG_SYSTEM);
private:
	int m_nFileCount;
	TCHAR m_workingDir[_MAX_PATH];
};


class CAoiLogThread :
	public CWinThread
	, public AppLogBase
{
	DECLARE_DYNCREATE(CAoiLogThread)
public:
	CAoiLogThread(void) {};
	~CAoiLogThread(void) {};
	virtual BOOL InitInstance() { return TRUE; };
	virtual int ExitInstance() { return CWinThread::ExitInstance(); };

protected:
	DECLARE_MESSAGE_MAP()
	void OnLogMessage(WPARAM wParam, LPARAM lParam);
	void OnLogExit(WPARAM wParam, LPARAM lParam);
};


class AppLogProcess :
	public AppLogBase
{
public:
	AppLogProcess();
	~AppLogProcess();
	void StartLogServer() { OpLogThread(OP_THREAD_CREATE); };
	void StopLogServer() { OpLogThread(OP_THREAD_DESTROY); };
	void InsertDebugLog(CString xMsg, AOI_LOG_TYPE xType = LOG_SYSTEM);
	CString GetLogFileName(int xType);

private:
	void Init();
	void Finalize();
	enum {
		OP_THREAD_CREATE = 0,
		OP_THREAD_DESTROY
	};
	void OpLogThread(int nOpCode);

private:
	CWinThread *m_pLogThread;
};

