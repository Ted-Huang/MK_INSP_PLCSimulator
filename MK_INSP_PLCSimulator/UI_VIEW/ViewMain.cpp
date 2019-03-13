#include "stdafx.h"
#include "Resource.h"
#include "ViewMain.h"
#include "AoiFont.h"
#include "PLC_PACKET\AsyncSocketServer.h"
#include "MK_INSP_PLCSimulator.h"

CViewMain::CViewMain(RECT &rcTarget, CWnd *pParent, UINT ResourceId)
{
	Init();
	Create(NULL, _T("CViewMain"), WS_CHILD | WS_VISIBLE, rcTarget, pParent, ResourceId);
	InitUiRectPos();
	InitUI();
	if (m_pServer->Start()){
		m_pServer->AttachNotify(this);
		CString strMsg;
		strMsg.Format(L"Listening Port  %d ", PLC_PORT);
		AddSocketMsg(strMsg);
	}
	else{
		AddSocketMsg(L"socket server start fail");
	}
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
	QueryPerformanceFrequency(&frequency);
	memset(arInspTime, 0, sizeof(arInspTime));
	memset(arVerifyTime, 0, sizeof(arVerifyTime));
	memset(arIMGRCVTime, 0, sizeof(arIMGRCVTime));
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
			//LIST BOX
		case UI_LB_MSG:
			ptBase = { 700, 30 };
			ptSize = { 280, 150 };
			break;
			//BTN
		case UI_BTN_CLEARALL:
			ptBase = { 700, 200 };
			ptSize = { 80, 25 };
			nCaptionID = IDS_CLEARALL;
			break;
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
		case UI_lABEL_CAMDIR_RESP:
			ptBase = { 350, 30 };
			ptSize = { 150, 20 };
			break;
		case UI_LABEL_BARWIDTH:
			ptBase = { 30, 80 };
			ptSize = { 70, 50 };
			nCaptionID = IDS_BARWIDTH;
			break;
		case UI_lABEL_BARWIDTH_RESP:
			ptBase = { 350, 80 };
			ptSize = { 150, 20 };
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
			ptSize = { 550, 100 };
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
	//LIST BOX
	for (int x = UI_LB_BEGIN; x < UI_LB_END; x++){
		m_xUi[x].pLB = new CListBox;
		m_xUi[x].pLB->Create(WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | WS_HSCROLL, m_xUi[x].rcUi, this, m_xUi[x].nID);
		m_xUi[x].pLB->SetHorizontalExtent(1000);
		g_AoiFont.SetWindowFont(m_xUi[x].pCB, FontDef::typeT1);
	}
	InitListInspHeader();
	InitListInspContent();
	InitListInfoHeader();
	InitListInfoContent();

	//init value

	m_xUi[UI_CB_EVENT].pCB->AddString(LoadResourceString(IDS_INSPTRIGGER));
	m_xUi[UI_CB_EVENT].pCB->AddString(LoadResourceString(IDS_INSPVERIFY));
	m_xUi[UI_CB_EVENT].pCB->SetItemData(0, FIELD_INSP_TRIGGER);
	m_xUi[UI_CB_EVENT].pCB->SetItemData(1, FIELD_INSP_VERIFY_TRIGGER);


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
	//LIST BOX
	for (int x = UI_LB_BEGIN; x < UI_LB_END; x++){
		if (m_xUi[x].pLB){
			m_xUi[x].pLB->DestroyWindow();
			delete m_xUi[x].pLB;
			m_xUi[x].pLB = NULL;
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
	pList->InsertColumn(UI_FIELD_INSP_INSPRESULT, LoadResourceString(IDS_INSPRESULT), LVCFMT_CENTER, 80);
	pList->InsertColumn(UI_FIELD_INSP_INSPTIME, LoadResourceString(IDS_INSPTIME), LVCFMT_CENTER, 100);
	pList->InsertColumn(UI_FIELD_INSP_VERIFYRESULT, LoadResourceString(IDS_VERIFYRESULT), LVCFMT_CENTER, 80);
	pList->InsertColumn(UI_FIELD_INSP_VERIFYIME, LoadResourceString(IDS_VERIFYTIME), LVCFMT_CENTER, 100);
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
	xBody.wValue = nValue;
	//memcpy(&xBody.wValue, &nValue, sizeof(xBody.wValue));

	//xBody.wValue = SWAP(xBody.wValue); //change to Big-Endian

	if (m_pServer){
		m_pServer->SendData(CMDTYPE_OP, (BYTE*)&xBody);
	}
}

void CViewMain::SetListCtrl(int nCtrlID, PLC_CMD_FIELD_BODY* pBody)
{
	if (!m_xUi[nCtrlID].pList)
		return;

	LARGE_INTEGER tNow;
	double elapsedTime;
	CString strMsg;

	switch (pBody->cField){
	case FIELD_INSP_RESULT:
	case FIELD_INSP_ERR:
		QueryPerformanceCounter(&tNow);
		elapsedTime = ((tNow.QuadPart - arInspTime[pBody->cCh].QuadPart) * 1000.0) / frequency.QuadPart;

		if (pBody->cField == FIELD_INSP_RESULT)
			strMsg.Format(L"ok, %d", pBody->wValue > SHRT_MAX ? pBody->wValue - (USHRT_MAX + 1) : pBody->wValue); //unsigned to signed
		else
			strMsg.Format(L"not ok, %d", pBody->wValue);

		m_xUi[nCtrlID].pList->SetItemText(pBody->cCh, UI_FIELD_INSP_INSPRESULT, strMsg);
		strMsg.Format(L"%.2f", elapsedTime);
		m_xUi[nCtrlID].pList->SetItemText(pBody->cCh, UI_FIELD_INSP_INSPTIME, strMsg);
		break;
	case FIELD_INSP_VERIFY_RESULT:
		QueryPerformanceCounter(&tNow);
		elapsedTime = ((tNow.QuadPart - arVerifyTime[pBody->cCh].QuadPart) * 1000.0) / frequency.QuadPart;

		if (pBody->cField == FIELD_INSP_VERIFY_RESULT)
			strMsg.Format(L"ok, %d", pBody->wValue);
		else
			strMsg.Format(L"not ok, %d", pBody->wValue);

		m_xUi[nCtrlID].pList->SetItemText(pBody->cCh, UI_FIELD_INSP_VERIFYRESULT, strMsg);
		strMsg.Format(L"%.2f", elapsedTime);
		m_xUi[nCtrlID].pList->SetItemText(pBody->cCh, UI_FIELD_INSP_VERIFYIME, strMsg);
		break;
	case FIELD_CAM_IMG_RECVBIT:

		QueryPerformanceCounter(&tNow);
		elapsedTime = ((tNow.QuadPart - arIMGRCVTime[pBody->cCh].QuadPart) * 1000.0) / frequency.QuadPart;
		strMsg.Format(L"%.2f", elapsedTime);
		m_xUi[nCtrlID].pList->SetItemText(pBody->cCh, UI_FIELD_INSP_IMGRCVTIME, strMsg);
		break;
	case FIELD_CAM_ONLINE:
	case FIELD_INSP_VERIFY:
		{
			strMsg.Format(pBody->wValue > 0 ? L"ok" : L"not ok");

			int nCol = GetListColumn(pBody->cField), nRow = GetListRow(pBody->cCh);
			if (nCol >= 0 && nRow >= 0)
				m_xUi[nCtrlID].pList->SetItemText(nRow, nCol, strMsg);
		}
		break;
	case FIELD_INSP_MODE:
		{
			strMsg.Format(L"%d", pBody->wValue);
			int nCol = GetListColumn(pBody->cField), nRow = GetListRow(pBody->cCh);
			if (nCol >= 0 && nRow >= 0)
				m_xUi[nCtrlID].pList->SetItemText(nRow, nCol, strMsg);
		}
		break;
	default:
		{
			int nCol = GetListColumn(pBody->cField)
				, nRow = GetListRow(pBody->cCh);
			if (nCol >= 0 && nRow >= 0){
				if (nRow == CAMERA_ALL){
					m_xUi[nCtrlID].pList->SetItemText(UI_ROW_ROLL, nCol, L"ok");
					m_xUi[nCtrlID].pList->SetItemText(UI_ROW_OP, nCol, L"ok");
					m_xUi[nCtrlID].pList->SetItemText(UI_ROW_SIDE, nCol, L"ok");
				}
				else
					m_xUi[nCtrlID].pList->SetItemText(nRow, nCol, L"ok");
			}
		}
		break;
	}	

	
}

int CViewMain::GetListRow(BYTE cCh)
{
	int nRow = -1;
	switch (cCh){
	case CAMERA_ROLL:
		nRow = UI_ROW_ROLL;
		break;
	case CAMERA_OP:
		nRow = UI_ROW_OP;
		break;
	case CAMERA_SIDE:
		nRow = UI_ROW_SIDE;
		break;
	case CAMERA_ALL:
		nRow = CAMERA_ALL;
	}
	ASSERT(nRow >= 0);
	return nRow;
}

int CViewMain::GetListColumn(BYTE cField)
{
	int nCol = -1;
	switch (cField){
	case FIELD_CAM_ONLINE: //相機狀態
		nCol = UI_FIELD_INFO_CAMSTATUS;
		break;
	case FIELD_INSP_VERIFY: //2次校驗
		nCol = UI_FIELD_INFO_VERIFY;
		break;
	case FIELD_INSP_MODE: //檢測類別
		nCol = UI_FIELD_INFO_SOLL;
		break;
	case FIELD_GOLDEN_READY: //Golden Ready
		nCol = UI_FIELD_INFO_GOLDEN_READY;
		break;
	case FIELD_VERIFY_READY: //Verify Ready
		nCol = UI_FIELD_INFO_VERIFY_GOLDEN_READY;
		break;
	case FIELD_GOLDEN_RESET: //Golden reset
		nCol = UI_FIELD_INFO_GOLDEN_RESET;
		break;
	case FIELD_VERIFY_RESET: //Verify reset
		nCol = UI_FIELD_INFO_VERIFY_GOLDEN_RESET;
		break;
	case FIELD_INSP_RESULT:
		nCol = UI_FIELD_INSP_INSPRESULT;
		break;
	case FIELD_INSP_ERR:
		nCol = UI_FIELD_INSP_INSPRESULT;
		break;
	case FIELD_CAM_IMG_RECVBIT:
		nCol = UI_FIELD_INSP_IMGRCVTIME;
		break;
	default:
		break;
	}
	ASSERT(nCol >= 0);
	return nCol;
}

void CViewMain::AddSocketMsg(CString strMsg)
{
	if (m_xUi[UI_LB_MSG].pLB){
		m_xUi[UI_LB_MSG].pLB->AddString(strMsg);
		m_xUi[UI_LB_MSG].pLB->SetCurSel(m_xUi[UI_LB_MSG].pLB->GetCount() - 1);
	}
	theApp.InsertDebugLog(strMsg, LOG_PLCSOCKET);
}

void CViewMain::ClearINSPCol(int nCol)
{
	m_xUi[UI_LIST_INSP].pList->SetItemText(0, nCol, L""); //ROLL
	m_xUi[UI_LIST_INSP].pList->SetItemText(1, nCol, L""); //OP
	m_xUi[UI_LIST_INSP].pList->SetItemText(2, nCol, L""); //SIDE
}

BEGIN_MESSAGE_MAP(CViewMain, CWnd)
	ON_BN_CLICKED(UI_BTN_CAMDIR, OnSendCamDir)
	ON_BN_CLICKED(UI_BTN_BARWIDTH, OnSendBarWidth)
	ON_BN_CLICKED(UI_BTN_EVENT, OnSendEvent)
	ON_BN_CLICKED(UI_BTN_CLEARALL, OnClearAll)
END_MESSAGE_MAP()

void CViewMain::OnSendCamDir()
{
	if (!m_xUi[UI_RADIO_CAMDIR_LEFT].pBtn || !m_xUi[UI_RADIO_CAMDIR_RIGHT].pBtn || !m_xUi[UI_lABEL_CAMDIR_RESP].pLabel)
		return;

	m_xUi[UI_lABEL_CAMDIR_RESP].pLabel->SetWindowText(L"setting...");
	BOOL bRight = m_xUi[UI_RADIO_CAMDIR_RIGHT].pBtn->GetCheck();

	SendCmd(CAMERA_SIDE, OPCODE_SET, FIELD_CAM_DIR, bRight);
}

void CViewMain::OnSendBarWidth()
{
	if (!m_xUi[UI_EDIT_BARWIDTH].pEdit || !m_xUi[UI_lABEL_BARWIDTH_RESP].pLabel)
		return;

	CString strBarWidth;
	m_xUi[UI_EDIT_BARWIDTH].pEdit->GetWindowText(strBarWidth);

	if (!strBarWidth.GetLength()){
		AfxMessageBox(L"BAR_WIDTH empty!");
		return;
	}
	m_xUi[UI_lABEL_BARWIDTH_RESP].pLabel->SetWindowText(L"setting...");

	SendCmd(CAMERA_ALL, OPCODE_SET, FIELD_BAR_WIDTH, _ttoi(strBarWidth));
}

void CViewMain::OnSendEvent()
{
	if (!m_xUi[UI_CB_EVENT].pCB)
		return;

	memset(arInspTime, 0, sizeof(arInspTime));
	memset(arVerifyTime, 0, sizeof(arVerifyTime));
	memset(arIMGRCVTime, 0, sizeof(arIMGRCVTime));


	int nEventID = m_xUi[UI_CB_EVENT].pCB->GetItemData(m_xUi[UI_CB_EVENT].pCB->GetCurSel());

	if (nEventID == FIELD_INSP_TRIGGER){
		ClearINSPCol(UI_FIELD_INSP_INSPRESULT); //Clear col 
		ClearINSPCol(UI_FIELD_INSP_INSPTIME);
		ClearINSPCol(UI_FIELD_INSP_IMGRCVTIME);

		QueryPerformanceCounter(&arInspTime[UI_ROW_ROLL]); //query time
		QueryPerformanceCounter(&arInspTime[UI_ROW_OP]);
		QueryPerformanceCounter(&arInspTime[UI_ROW_SIDE]);
	}
	else if (nEventID == FIELD_INSP_VERIFY_TRIGGER){
		ClearINSPCol(UI_FIELD_INSP_VERIFYRESULT); //Clear col 
		ClearINSPCol(UI_FIELD_INSP_VERIFYIME);
		ClearINSPCol(UI_FIELD_INSP_IMGRCVTIME);

		QueryPerformanceCounter(&arVerifyTime[UI_ROW_ROLL]); //query time
		QueryPerformanceCounter(&arVerifyTime[UI_ROW_OP]);
		QueryPerformanceCounter(&arVerifyTime[UI_ROW_SIDE]);
	}

	QueryPerformanceCounter(&arIMGRCVTime[UI_ROW_ROLL]);  //query time
	QueryPerformanceCounter(&arIMGRCVTime[UI_ROW_OP]);
	QueryPerformanceCounter(&arIMGRCVTime[UI_ROW_SIDE]);

	SendCmd(CAMERA_ALL, OPCODE_SET, *(BYTE*)&nEventID, NULL);
}

void CViewMain::OnClearAll()
{
	if (!m_xUi[UI_LIST_INSP].pList || !m_xUi[UI_LIST_INFO].pList)
		return;

	for (int nCol = UI_FIELD_INSP_INSPRESULT; nCol < UI_FIELD_INSP_MAX; nCol++){
		m_xUi[UI_LIST_INSP].pList->SetItemText(0, nCol, L"");
		m_xUi[UI_LIST_INSP].pList->SetItemText(1, nCol, L"");
		m_xUi[UI_LIST_INSP].pList->SetItemText(2, nCol, L"");
	}

	for (int nCol = UI_FIELD_INFO_CAMSTATUS; nCol < UI_FIELD_INFO_MAX; nCol++){
		m_xUi[UI_LIST_INFO].pList->SetItemText(0, nCol, L"");
		m_xUi[UI_LIST_INFO].pList->SetItemText(1, nCol, L"");
		m_xUi[UI_LIST_INFO].pList->SetItemText(2, nCol, L"");
	}

	m_xUi[UI_lABEL_CAMDIR_RESP].pLabel->SetWindowText(L"");
	m_xUi[UI_lABEL_BARWIDTH_RESP].pLabel->SetWindowText(L"");
}

void CViewMain::DoSocketNotify(void *pInstance, long ErrorId)
{
	CString strMsg;
	switch (ErrorId)
	{
	case ERR_SDK_SOCKET_CONNECT:
		if (pInstance){
			CAsyncSocketSession* pSession = (CAsyncSocketSession*)pInstance;
			CString strIP;
			UINT nPort;
			pSession->GetPeerName(strIP, nPort);
			strMsg.Format(L"Client %s: %d connect", strIP, nPort);
		}
		break;
	case ERR_SDK_SOCKET_CLOSE:
		{
			if (pInstance){
				CAsyncSocketSession* pSession = (CAsyncSocketSession*)pInstance;
				CString strIP;
				UINT nPort;
				pSession->GetPeerName(strIP, nPort);
				strMsg.Format(L"Client %s: %d disconnect", strIP, nPort);
			}
		}
		break;
	default:
		strMsg.Format(L"Socket Notify not implement %d", ErrorId);
		break;
	}
	AddSocketMsg(strMsg);
	if (m_pServer){
		m_pServer->DoSocketNotify(pInstance, ErrorId);
	}
}

void CViewMain::DoSessionReceivePacket(void *pInstance, PLC_CMD_FIELD_BODY* pBody)
{ 
	BOOL bDump = TRUE;
	if (pBody){
		switch (pBody->cField){
		case FIELD_CAM_DIR:
			if (pBody->cOpCode == OPCODE_ECHO){
				m_xUi[UI_lABEL_CAMDIR_RESP].pLabel->SetWindowText(L"set CAM_DIR done");
				bDump = FALSE;
			}
			else if (pBody->cOpCode == OPCODE_QUERY){
				bDump = FALSE;
				OnSendCamDir();
			}
			break;
		case FIELD_BAR_WIDTH:
			if (pBody->cOpCode == OPCODE_ECHO){
				m_xUi[UI_lABEL_BARWIDTH_RESP].pLabel->SetWindowText(L"set BAR_WIDTH done");
				bDump = FALSE;
			}
			else if (pBody->cOpCode == OPCODE_QUERY){
				bDump = FALSE;
				CString strBarWidth;
				m_xUi[UI_EDIT_BARWIDTH].pEdit->GetWindowText(strBarWidth);
				if (strBarWidth.GetLength() == 0){
					m_xUi[UI_EDIT_BARWIDTH].pEdit->SetWindowText(L"100"); // set default barwidth to 1 mm
				}
				OnSendBarWidth();
			}
			break;
		case FIELD_CAM_ONLINE: //相機狀態
		case FIELD_INSP_VERIFY: //2次校驗
		case FIELD_INSP_MODE: //檢測類別
		case FIELD_GOLDEN_READY: //Golden Ready
		case FIELD_VERIFY_READY: //Verify Ready
		case FIELD_GOLDEN_RESET: //Golden reset
		case FIELD_VERIFY_RESET: //Verify reset
			if (pBody->cOpCode == OPCODE_SET){
				SetListCtrl(UI_LIST_INFO, pBody);
				bDump = FALSE;
			}
			break;
		case FIELD_INSP_RESULT:
		case FIELD_INSP_ERR:
		case FIELD_CAM_IMG_RECVBIT:
		case FIELD_INSP_VERIFY_RESULT:
			if (pBody->cOpCode == OPCODE_SET){
				SetListCtrl(UI_LIST_INSP, pBody);
				bDump = FALSE;
			}
			break;
		case FIELD_INSP_TOGGLEBIT: //igonre TOGGLEBIT
			bDump = FALSE;
			break;
		case FIELD_INSP_TRIGGER:
		case FIELD_INSP_VERIFY_TRIGGER:
			if (pBody->cOpCode == OPCODE_ECHO){ //igonre FIELD_INSP_TRIGGER/FIELD_INSP_VERIFY_TRIGGER echo
				bDump = FALSE;
			}
			break;
		default:
			break;
		}
		if (bDump){
			CString strMsg;
			strMsg.Format(L"未處理訊息: %d %d %d %d \n", pBody->cCh, pBody->cField, pBody->cOpCode, pBody->wValue);
			TRACE(strMsg);
			theApp.InsertDebugLog(strMsg, LOG_PLCSOCKET);
		}
		//delete pBody;
	}

}

