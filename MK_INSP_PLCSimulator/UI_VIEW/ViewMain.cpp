#include "stdafx.h"
#include "Resource.h"
#include "ViewMain.h"
#include "AoiFont.h"
#include "PLC_PACKET\PLC_PACKET_const.h"
#include "PLC_PACKET\AsyncSocketServer.h"

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
	if (m_pServer){
		delete m_pServer;
		m_pServer = NULL;
	}
}

void CViewMain::Init()
{
	memset(m_xUi, 0, sizeof(m_xUi));
	m_pServer = new CAsyncSocketServer;
	m_pServer->Start();
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
		case UI_LABEL_OTHERINFORMATION:
			ptBase = { 30, 300 };
			ptSize = { 70, 20 };
			nCaptionID = IDS_OTHERINFORMATION;
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
			//LIST
		case UI_LIST_INSP:
			ptBase = { 130, 180 };
			ptSize = { 300, 100 };
			break;
		case UI_LIST_INFO:
			ptBase = { 130, 300 };
			ptSize = { 820, 100 };
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
	//LIST
	for (int x = UI_LIST_BEGIN; x < UI_LIST_END; x++){
		m_xUi[x].pList = new CListCtrl();
		m_xUi[x].pList->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT , m_xUi[x].rcUi, this, m_xUi[x].nID);
		m_xUi[x].pList->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		g_AoiFont.SetWindowFont(m_xUi[x].pBtn, FontDef::typeT1);
	}
	InitListInspHeader();
	InitListInspContent();
	InitListInfoHeader();
	InitListInfoContent();

	//init value

	m_xUi[UI_CB_EVENT].pCB->AddString(LoadResourceString(IDS_INSPTRIGGER));
	m_xUi[UI_CB_EVENT].pCB->AddString(LoadResourceString(IDS_INSPVERIFY));
	m_xUi[UI_CB_EVENT].pCB->SetItemData(0, FIELD_INSP_TRIGGER);
	m_xUi[UI_CB_EVENT].pCB->SetItemData(1, FIELD_INSP_VERIFY2);


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
	//LIST
	for (int x = UI_LIST_BEGIN; x < UI_LIST_END; x++){
		if (m_xUi[x].pList){
			m_xUi[x].pList->DestroyWindow();
			delete m_xUi[x].pList;
			m_xUi[x].pList = NULL;
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

void CViewMain::InitListInspHeader()
{
	if (!m_xUi[UI_LIST_INSP].pList)
		return;
	CListCtrl* pList = m_xUi[UI_LIST_INSP].pList;
	CHeaderCtrl* pHdrCtrl = pList->GetHeaderCtrl();
	pList->InsertColumn(UI_FIELD_INSP_CAMERA, LoadResourceString(IDS_CAMERA), LVCFMT_CENTER, 50);
	pList->InsertColumn(UI_FIELD_INSP_RESULT, LoadResourceString(IDS_RESULT), LVCFMT_CENTER, 50);
	pList->InsertColumn(UI_FIELD_INSP_INSPTIME, LoadResourceString(IDS_INSPTIME), LVCFMT_CENTER, 100);
	pList->InsertColumn(UI_FIELD_INSP_IMGRCVTIME, LoadResourceString(IDS_IMGRCVTIME), LVCFMT_CENTER, 100);

}

void CViewMain::InitListInspContent()
{
	if (!m_xUi[UI_LIST_INSP].pList)
		return;

	CListCtrl* pList = m_xUi[UI_LIST_INSP].pList;
	int nRowStrId;

	for (int x = 0; x < UI_ROW_MAX; x++){
		switch (x)
		{
		case UI_ROW_ROLL:
		default:
			nRowStrId = IDS_ROLL;
			break;
		case UI_ROW_OP:
			nRowStrId = IDS_OP;
			break;
		case UI_ROW_SIDE:
			nRowStrId = IDS_SIDE;
			break;
		}
		pList->InsertItem(x, LoadResourceString(nRowStrId));
	/*	pList->SetItemText(x, UI_FIELD_INSP_RESULT, L"");
		pList->SetItemText(x, UI_FIELD_INSP_INSPTIME, L"");
		pList->SetItemText(x, UI_FIELD_INSP_IMGRCVTIME, L"");*/
	}

}

void CViewMain::InitListInfoHeader()
{
	if (!m_xUi[UI_LIST_INFO].pList)
		return;

	CListCtrl* pList = m_xUi[UI_LIST_INFO].pList;
	pList->InsertColumn(UI_FIELD_INFO_CAMERA, LoadResourceString(IDS_CAMERA), LVCFMT_CENTER, 50);
	pList->InsertColumn(UI_FIELD_INFO_CAMSTATUS, LoadResourceString(IDS_CAMSTATUS), LVCFMT_CENTER, 80);
	pList->InsertColumn(UI_FIELD_INFO_VERIFY, LoadResourceString(IDS_VERIFY), LVCFMT_CENTER, 100);
	pList->InsertColumn(UI_FIELD_INFO_SOLL, LoadResourceString(IDS_SOLL), LVCFMT_CENTER, 80);
	pList->InsertColumn(UI_FIELD_INFO_GOLDEN_READY, LoadResourceString(IDS_GOLDEN_READY), LVCFMT_CENTER, 100);
	pList->InsertColumn(UI_FIELD_INFO_VERIFY_GOLDEN_READY, LoadResourceString(IDS_VERIFY_GOLDEN_READY), LVCFMT_CENTER, 150);
	pList->InsertColumn(UI_FIELD_INFO_GOLDEN_RESET, LoadResourceString(IDS_GOLDEN_RESET), LVCFMT_CENTER, 100);
	pList->InsertColumn(UI_FIELD_INFO_VERIFY_GOLDEN_RESET, LoadResourceString(IDS_VERIFY_GOLDEN_RESET), LVCFMT_CENTER, 150);
}

void CViewMain::InitListInfoContent()
{
	if (!m_xUi[UI_LIST_INFO].pList)
		return;

	CListCtrl* pList = m_xUi[UI_LIST_INFO].pList;
	int nRowStrId;

	for (int x = 0; x < UI_ROW_MAX; x++){
		switch (x)
		{
		case UI_ROW_ROLL:
		default:
			nRowStrId = IDS_ROLL;
			break;
		case UI_ROW_OP:
			nRowStrId = IDS_OP;
			break;
		case UI_ROW_SIDE:
			nRowStrId = IDS_SIDE;
			break;
		}
		pList->InsertItem(x, LoadResourceString(nRowStrId));
	}
}

WORD CViewMain::SWAP(WORD tData)
{
	WORD tRet = ((tData & 0xFF00) >> 8) | ((tData & 0xFF) << 8);
	return tRet;
};

void CViewMain::SendCmd(BYTE cCh, BYTE cOpCode, BYTE cField, int nValue)
{
	PLC_CMD_FIELD_BODY xBody;
	memset(&xBody, 0, sizeof(PLC_CMD_FIELD_BODY));
	xBody.cCh = cCh; 
	xBody.cOpCode = cOpCode;
	xBody.cField = cField;

	memcpy(&xBody.wValue, &nValue, sizeof(xBody.wValue));

	xBody.wValue = SWAP(xBody.wValue); //change to Big-Endian

	if (m_pServer){
		m_pServer->SendData(CMDTYPE_OP, (BYTE*)&xBody);
	}
}

BEGIN_MESSAGE_MAP(CViewMain, CWnd)
	ON_BN_CLICKED(UI_BTN_CAMDIR, OnSendCamDir)
	ON_BN_CLICKED(UI_BTN_BARWIDTH, OnSendBarWidth)
	ON_BN_CLICKED(UI_BTN_EVENT, OnSendEvent)
END_MESSAGE_MAP()

void CViewMain::OnSendCamDir()
{
	if (!m_xUi[UI_RADIO_CAMDIR_LEFT].pBtn || !m_xUi[UI_RADIO_CAMDIR_RIGHT].pBtn)
		return;

	BOOL bRight = m_xUi[UI_RADIO_CAMDIR_RIGHT].pBtn->GetCheck();

	SendCmd(CAMERA_SIDE, OPCODE_SET, FIELD_CAM_DIR, bRight);
}

void CViewMain::OnSendBarWidth()
{
	if (!m_xUi[UI_EDIT_BARWIDTH].pEdit)
		return;

	CString strBarWidth;
	m_xUi[UI_EDIT_BARWIDTH].pEdit->GetWindowText(strBarWidth);
	if (!strBarWidth.GetLength())
		return;

	SendCmd(CAMERA_ALL, OPCODE_SET, FIELD_BAR_WIDTH, _ttoi(strBarWidth));
}

void CViewMain::OnSendEvent()
{
	if (!m_xUi[UI_CB_EVENT].pCB)
		return;

	int nEventID = m_xUi[UI_CB_EVENT].pCB->GetItemData(m_xUi[UI_CB_EVENT].pCB->GetCurSel());

	SendCmd(CAMERA_ALL, OPCODE_SET, *(BYTE*)&nEventID, NULL);
}

