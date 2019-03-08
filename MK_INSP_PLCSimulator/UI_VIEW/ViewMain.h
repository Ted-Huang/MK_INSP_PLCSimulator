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
		UI_BTN_TEST = UI_BTN_BEGIN,
		UI_BTN_END,
		UI_ITEM_END,
	};
	UI_OBJ m_xUi[UI_ITEM_END];
};