
// MK_INSP_PLCSimulator.h : PROJECT_NAME ���ε{�����D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'"
#endif

#include "resource.h"		// �D�n�Ÿ�
#include "AppLogProcess.h"

// CMK_INSP_PLCSimulatorApp: 
// �аѾ\��@�����O�� MK_INSP_PLCSimulator.cpp
//

class CMK_INSP_PLCSimulatorApp : public CWinApp, public AppLogProcess
{
public:
	CMK_INSP_PLCSimulatorApp();

// �мg
public:
	virtual BOOL InitInstance();

// �{���X��@

	DECLARE_MESSAGE_MAP()
};

extern CMK_INSP_PLCSimulatorApp theApp;