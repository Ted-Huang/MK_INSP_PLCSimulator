#pragma once

typedef struct UI_OBJ_{  
	RECT rcUi;
	UINT nID;
	UINT nCaptionID;
	CEdit* pEdit;
	CStatic* pLabel;
	CButton* pBtn;
	CComboBox* pCB;
	CListBox* pLB;
	UI_OBJ_(){
		memset(&rcUi, 0, sizeof(rcUi));
		nID = 0;
		nCaptionID = 0;
		pEdit = NULL;
		pLabel = NULL;
		pBtn = NULL;
		pCB = NULL;
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
		if (pLB){
			delete pLB;
			pLB = NULL;
		};
	}
}UI_OBJ;

class CViewMain : public CWnd{
public:
	CViewMain(RECT &rcTarget, CWnd *pParent, UINT ResourceId);
	~CViewMain();
private:
	void Init();
	void InitUiRectPos();
	void InitUI();
	void DestroyUI();

	CString LoadResourceString(UINT nID);
private:
	enum{
		UI_ITEM_BEGIN,
		//BTN
		UI_BTN_BEGIN,
		UI_BTN_CAMDIR = UI_BTN_BEGIN, 
		UI_BTN_BARWIDTH,
		UI_BTN_EVENT,
		UI_BTN_END,
		//LABEL
		UI_LABEL_BEGIN,
		UI_LABEL_CAMDIR = UI_LABEL_BEGIN,
		UI_LABEL_BARWIDTH,
		UI_LABEL_INSPRESULT,
		UI_LABEL_OTHERINFORMATION,
		UI_LABEL_INSPTIME,
		UI_LABEL_IMGRCVTIME,
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
		UI_ITEM_END,
	};
	UI_OBJ m_xUi[UI_ITEM_END];
};