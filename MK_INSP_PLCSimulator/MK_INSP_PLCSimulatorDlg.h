
// MK_INSP_PLCSimulatorDlg.h : 標頭檔
//

#pragma once


// CMK_INSP_PLCSimulatorDlg 對話方塊
class CMK_INSP_PLCSimulatorDlg : public CDialogEx
{
// 建構
public:
	CMK_INSP_PLCSimulatorDlg(CWnd* pParent = NULL);	// 標準建構函式

// 對話方塊資料
	enum { IDD = IDD_MK_INSP_PLCSIMULATOR_DIALOG };

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
};
