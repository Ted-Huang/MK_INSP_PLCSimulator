
// MK_INSP_PLCSimulatorDlg.h : ���Y��
//

#pragma once


// CMK_INSP_PLCSimulatorDlg ��ܤ��
class CMK_INSP_PLCSimulatorDlg : public CDialogEx
{
// �غc
public:
	CMK_INSP_PLCSimulatorDlg(CWnd* pParent = NULL);	// �зǫغc�禡

// ��ܤ�����
	enum { IDD = IDD_MK_INSP_PLCSIMULATOR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �䴩


// �{���X��@
protected:
	HICON m_hIcon;

	// ���ͪ��T�������禡
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
