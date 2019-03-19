
// MFCApplication1Dlg.h : 標頭檔
//

#pragma once
#include "ViewMain.h"
// CMFCApplication1Dlg 對話方塊
class CMFCApplication1Dlg : public CDialogEx
{
// 建構
public:
	CMFCApplication1Dlg(CWnd* pParent = NULL);	// 標準建構函式

// 對話方塊資料
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支援


// 程式碼實作
protected:
	HICON m_hIcon;

	// 產生的訊息對應函式
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	void Init();
};
