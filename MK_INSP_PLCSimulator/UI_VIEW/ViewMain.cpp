#include "stdafx.h"
#include "Resource.h"
#include "ViewMain.h"
#include "AoiFont.h"
#include "PLC_PACKET\PLC_PACKET_const.h"

CViewMain::CViewMain(RECT &rcTarget, CWnd *pParent, UINT ResourceId)
{
	Init();
	Create(NULL, _T("CViewMain"), WS_CHILD | WS_VISIBLE, rcTarget, pParent, ResourceId);
	InitUiRectPos();
	InitUI();
	//TRACE("%d \n", sizeof(PLC_CMD_FIELD_BODY));
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
			//BTN
		case UI_BTN_CAMDIR:
			ptBase = { 280, 30 };
			ptSize = { 50, 25 };
			nCaptionID = IDS_SEND;
			break;
		case UI_BTN_BARWIDTH:
			ptBase = { 280, 80 };
			ptSize = { 50, 25 };
			nCaptionID = IDS_SEND;
			break;		
		case UI_BTN_EVENT:
			ptBase = { 280, 130 };
			ptSize = { 50, 25 };
			nCaptionID = IDS_SEND;
			break;
			//LABEL
		case UI_LABEL_CAMDIR:
			ptBase = { 30, 30 };
			ptSize = { 70, 20 };
			nCaptionID = IDS_CAMDIR;
			break;
		case UI_LABEL_BARWIDTH:
			ptBase = { 30, 80 };
			ptSize = { 70, 20 };
			nCaptionID = IDS_BARWIDTH;
			break;
		case UI_LABEL_EVENT:
			ptBase = { 30, 130 };
			ptSize = { 70, 20 };
			nCaptionID = IDS_EVENT;
			break;
		case UI_LABEL_INSPRESULT:
			ptBase = { 30, 180 };
			ptSize = { 70, 20 };
			nCaptionID = IDS_INSPRESULT;
			break;
			//EDIT
		case UI_EDIT_BARWIDTH:
			ptBase = { 130, 80 };
			ptSize = { 120, 25 };
			break;
			//RADIO
		case UI_RADIO_CAMDIR_LEFT:
			ptBase = { 130, 30 };
			ptSize = { 60, 25 };
			nCaptionID = IDS_LEFT;
			break;
		case UI_RADIO_CAMDIR_RIGHT:
			ptBase = { 200, 30 };
			ptSize = { 60, 25 };
			nCaptionID = IDS_RIGHT;
			break;
			//COMBO
		case UI_CB_EVENT:
			ptBase = { 130, 130 };
			ptSize = { 120, 25 };
			nCaptionID = IDS_RIGHT;
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
		g_AoiFont.SetWindowFont(m_xUi[x].pBtn, FontDef::typeT1);
	}	
	//LABEL
	for (int x = UI_LABEL_BEGIN; x < UI_LABEL_END; x++){
		m_xUi[x].pLabel = new CStatic;
		m_xUi[x].pLabel->Create(LoadResourceString(m_xUi[x].nCaptionID), WS_VISIBLE | WS_CHILD, m_xUi[x].rcUi, this, m_xUi[x].nID);
		g_AoiFont.SetWindowFont(m_xUi[x].pLabel, FontDef::typeT1);
	}
	//CB
	for (int x = UI_CB_BEGIN; x < UI_CB_END; x++){
		m_xUi[x].pCB = new CComboBox;
		m_xUi[x].pCB->Create(WS_VISIBLE | WS_CHILD | CBS_AUTOHSCROLL | CBS_DROPDOWNLIST, m_xUi[x].rcUi, this, m_xUi[x].nID);
		g_AoiFont.SetWindowFont(m_xUi[x].pCB, FontDef::typeT1);
	}
	//EDIT
	for (int x = UI_EDIT_BEGIN; x < UI_EDIT_END; x++){
		m_xUi[x].pEdit = new CEdit;
		m_xUi[x].pEdit->Create(WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, m_xUi[x].rcUi, this, m_xUi[x].nID);
		g_AoiFont.SetWindowFont(m_xUi[x].pEdit, FontDef::typeT1);
	}
	//RADIO
	for (int x = UI_RADIO_BEGIN; x < UI_RADIO_END; x++){
		m_xUi[x].pBtn = new CButton;

		m_xUi[x].pBtn->Create(LoadResourceString(m_xUi[x].nCaptionID), WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, m_xUi[x].rcUi, this, m_xUi[x].nID);
		g_AoiFont.SetWindowFont(m_xUi[x].pBtn, FontDef::typeT1);
	}
	//init value
	m_xUi[UI_CB_EVENT].pCB->SetItemData(0, FIELD_INSP_TRIGGER);
	m_xUi[UI_CB_EVENT].pCB->SetItemData(1, FIELD_INSP_VERIFY2);
	m_xUi[UI_CB_EVENT].pCB->AddString(LoadResourceString(IDS_INSPTRIGGER));
	m_xUi[UI_CB_EVENT].pCB->AddString(LoadResourceString(IDS_INSPVERIFY));
	m_xUi[UI_CB_EVENT].pCB->SetCurSel(0);

	m_xUi[UI_RADIO_CAMDIR_LEFT].pBtn->SetCheck(TRUE);
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
	//LABEL
	for (int x = UI_LABEL_BEGIN; x < UI_LABEL_END; x++){
		if (m_xUi[x].pLabel){
			m_xUi[x].pLabel->DestroyWindow();
			delete m_xUi[x].pLabel;
			m_xUi[x].pLabel = NULL;
		}
	}
	//CB
	for (int x = UI_CB_BEGIN; x < UI_CB_END; x++){
		if (m_xUi[x].pCB){
			m_xUi[x].pCB->DestroyWindow();
			delete m_xUi[x].pCB;
			m_xUi[x].pCB = NULL;
		}
	}	
	//EDIT
	for (int x = UI_EDIT_BEGIN; x < UI_EDIT_END; x++){
		if (m_xUi[x].pEdit){
			m_xUi[x].pEdit->DestroyWindow();
			delete m_xUi[x].pEdit;
			m_xUi[x].pEdit = NULL;
		}
	}
	//RADIO
	for (int x = UI_RADIO_BEGIN; x < UI_RADIO_END; x++){
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

	return str;
}