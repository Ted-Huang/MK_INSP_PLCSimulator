#pragma once
#include "PLC_PACKET\PLC_PACKET_const.h"
#include "PLC_PACKET\AsyncSocketSession.h"
typedef struct UI_OBJ_{  
	RECT rcUi;
	UINT nID;
	UINT nCaptionID;
	CEdit* pEdit;
	CStatic* pLabel;
	CButton* pBtn;
	CComboBox* pCB;
	CListCtrl* pList;
	CListBox* pLB;
	UI_OBJ_(){
		memset(&rcUi, 0, sizeof(rcUi));
		nID = 0;
		nCaptionID = 0;
		pEdit = NULL;
		pLabel = NULL;
		pBtn = NULL;
		pCB = NULL;
		pList = NULL;
		pLB = NULL;
	}
	~UI_OBJ_(){
		if (pEdit){
			delete pEdit;
			pEdit = NULL;
		};
		if (pLabel){
			delete pLabel;
			pLabel = NULL;
		};
		if (pBtn){
			delete pBtn;
			pBtn = NULL;
		};
		if (pCB){
			delete pCB;
			pCB = NULL;
		};
		if (pList){
			delete pList;
			pList = NULL;
		};
		if (pLB){
			delete pLB;
			pLB = NULL;
		};
	}
}UI_OBJ;

class CAsyncSocketServer;
class CViewMain : public CWnd, public ISESSION_NOTIFY{
public:
	CViewMain(RECT &rcTarget, CWnd *pParent, UINT ResourceId);
	~CViewMain();
private:
	void Init();
	void InitUiRectPos();
	void InitUI();
	void DestroyUI();

	CString LoadResourceString(UINT nID);

	void InitListInspHeader();
	void InitListInspContent();
	void InitListInfoHeader();
	void InitListInfoContent();
	WORD SWAP(WORD tData);
	void SendCmd(BYTE cCh, BYTE cOpCode, BYTE cField, int nValue);
	void SetListCtrl(int nCtrlID, PLC_CMD_FIELD_BODY* pBody, BOOL bOK);
	int GetListRow(BYTE cCh);
	int GetListColumn(BYTE cField);
	void AddSocketMsg(CString strMsg);
protected:
	afx_msg void OnSendCamDir();
	afx_msg void OnSendBarWidth();
	afx_msg void OnSendEvent();
	afx_msg void OnClearAll();
	DECLARE_MESSAGE_MAP()
protected:
	virtual void DoSocketNotify(void *pInstance, long ErrorId);
	virtual void DoSessionReceivePacket(void *pInstance, PLC_CMD_FIELD_BODY* pBody);
private:
	enum{
		UI_ITEM_BEGIN,
		//BTN
		UI_BTN_BEGIN,
		UI_BTN_CAMDIR = UI_BTN_BEGIN, 
		UI_BTN_BARWIDTH,
		UI_BTN_EVENT,
		UI_BTN_CLEARALL,
		UI_BTN_END,
		//LABEL
		UI_LABEL_BEGIN,
		UI_LABEL_CAMDIR = UI_LABEL_BEGIN,
		UI_lABEL_CAMDIR_RESP,
		UI_LABEL_BARWIDTH,
		UI_lABEL_BARWIDTH_RESP,
		UI_LABEL_INSPRESULT,
		UI_LABEL_OTHERINFORMATION,
		UI_LABEL_EVENT,
		UI_LABEL_END,
		//COMBO
		UI_CB_BEGIN,
		UI_CB_EVENT = UI_CB_BEGIN,
		UI_CB_END,
		//EDIT
		UI_EDIT_BEGIN,
		UI_EDIT_BARWIDTH = UI_EDIT_BEGIN ,
		UI_EDIT_END,
		//RADIO
		UI_RADIO_BEGIN,
		UI_RADIO_CAMDIR_LEFT = UI_RADIO_BEGIN,
		UI_RADIO_CAMDIR_RIGHT,
		UI_RADIO_END,
		//ListCtrl
		UI_LIST_BEGIN,
		UI_LIST_INSP = UI_LIST_BEGIN,
		UI_LIST_INFO,
		UI_LIST_END,
		//LIST BOX
		UI_LB_BEGIN,
		UI_LB_MSG = UI_LB_BEGIN,
		UI_LB_END,
		UI_ITEM_END,
	};

	enum{
		UI_FIELD_INSP_CAMERA,
		UI_FIELD_INSP_RESULT,
		UI_FIELD_INSP_INSPTIME,
		UI_FIELD_VERIFY_INSPTIME,
		UI_FIELD_INSP_IMGRCVTIME,
		UI_FIELD_INSP_MAX
	};

	enum{
		UI_FIELD_INFO_CAMERA,
		UI_FIELD_INFO_CAMSTATUS,
		UI_FIELD_INFO_VERIFY,
		UI_FIELD_INFO_SOLL,
		UI_FIELD_INFO_GOLDEN_READY,
		UI_FIELD_INFO_VERIFY_GOLDEN_READY,
		UI_FIELD_INFO_GOLDEN_RESET,
		UI_FIELD_INFO_VERIFY_GOLDEN_RESET,	
		UI_FIELD_INFO_MAX
	};

	enum{
		UI_ROW_ROLL,
		UI_ROW_OP,
		UI_ROW_SIDE,
		UI_ROW_MAX
	};
	UI_OBJ m_xUi[UI_ITEM_END];
	CAsyncSocketServer* m_pServer;
};