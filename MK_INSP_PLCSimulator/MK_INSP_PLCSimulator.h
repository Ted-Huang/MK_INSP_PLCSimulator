
// MK_INSP_PLCSimulator.h : PROJECT_NAME 應用程式的主要標頭檔
//

#pragma once

#ifndef __AFXWIN_H__
	#error "對 PCH 包含此檔案前先包含 'stdafx.h'"
#endif

#include "resource.h"		// 主要符號
#include "AppLogProcess.h"

// CMK_INSP_PLCSimulatorApp: 
// 請參閱實作此類別的 MK_INSP_PLCSimulator.cpp
//

class CMK_INSP_PLCSimulatorApp : public CWinApp, public AppLogProcess
{
public:
	CMK_INSP_PLCSimulatorApp();

// 覆寫
public:
	virtual BOOL InitInstance();

// 程式碼實作

	DECLARE_MESSAGE_MAP()
};

extern CMK_INSP_PLCSimulatorApp theApp;