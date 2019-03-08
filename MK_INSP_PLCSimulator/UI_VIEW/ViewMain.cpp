#include "stdafx.h"
#include "Resource.h"
#include "ViewMain.h"


CViewMain::CViewMain(RECT &rcTarget, CWnd *pParent, UINT ResourceId)
{
	Init();
	Create(NULL, _T("CViewMain"), WS_CHILD | WS_VISIBLE, rcTarget, pParent, ResourceId);
	InitUiRectPos();
	InitUI();
}

CViewMain::~CViewMain()
{
	DestroyUI();
}

void CViewMain::Init()
{
	memset(m_xUi, 0, sizeof(m_xUi));
}

void CViewMain::InitUiRectPos()
{
	for (int x = UI_ITEM_BEGIN; x < UI_ITEM_END; x++)
	{
		UINT nID = 0, nCaptionID = 0;
		POINT ptBase = { 0, 0 };
		POINT ptSize = { 0, 0 };

		switch (x)
		{
		case UI_BTN_TEST:
			ptBase = { 320, 30 };
			ptSize = { 50, 30 };
			nCaptionID = IDS_TEST;
			break;
		}
		m_xUi[x].rcUi = { ptBase.x, ptBase.y, ptBase.x + ptSize.x, ptBase.y + ptSize.y };
		m_xUi[x].nID = x;
		m_xUi[x].nCaptionID = nCaptionID;
	}
}

void CViewMain::InitUI()
{
	//BTN
	for (int x = UI_BTN_BEGIN; x < UI_BTN_END; x++){
		m_xUi[x].pBtn = new CButton;
		m_xUi[x].pBtn->Create(LoadResourceString(m_xUi[x].nCaptionID), WS_VISIBLE | WS_CHILD, m_xUi[x].rcUi, this, m_xUi[x].nID);
		//g_AoiFont.SetWindowFont(m_xUi[x].pBtn, FontDef::typeT1);
	}
}

void CViewMain::DestroyUI()
{
	//BTN
	for (int x = UI_BTN_BEGIN; x < UI_BTN_END; x++){
		if (m_xUi[x].pBtn){
			m_xUi[x].pBtn->DestroyWindow();
			delete m_xUi[x].pBtn;
			m_xUi[x].pBtn = NULL;
		}
	}
}

CString CViewMain::LoadResourceString(UINT nID)
{
	wchar_t str[MAX_PATH];
	LoadString(AfxGetInstanceHandle(), nID, str, MAX_PATH);

	return CString(str);
}