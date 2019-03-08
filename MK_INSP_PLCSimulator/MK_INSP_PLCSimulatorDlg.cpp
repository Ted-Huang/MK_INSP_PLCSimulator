
// MK_INSP_PLCSimulatorDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "MK_INSP_PLCSimulator.h"
#include "MK_INSP_PLCSimulatorDlg.h"
#include "afxdialogex.h"
#include "UI_VIEW\ViewMain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMK_INSP_PLCSimulatorDlg 對話方塊



CMK_INSP_PLCSimulatorDlg::CMK_INSP_PLCSimulatorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMK_INSP_PLCSimulatorDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CMK_INSP_PLCSimulatorDlg::~CMK_INSP_PLCSimulatorDlg()
{
	if (m_pMain){
		m_pMain->DestroyWindow();
		delete m_pMain;
		m_pMain = NULL;
	}
}

void CMK_INSP_PLCSimulatorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMK_INSP_PLCSimulatorDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CMK_INSP_PLCSimulatorDlg 訊息處理常式

BOOL CMK_INSP_PLCSimulatorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示

	// TODO:  在此加入額外的初始設定
	Init();
	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，
// 以便繪製圖示。對於使用文件/檢視模式的 MFC 應用程式，
// 框架會自動完成此作業。

void CMK_INSP_PLCSimulatorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 繪製的裝置內容

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 將圖示置中於用戶端矩形
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 描繪圖示
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 當使用者拖曳最小化視窗時，
// 系統呼叫這個功能取得游標顯示。
HCURSOR CMK_INSP_PLCSimulatorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMK_INSP_PLCSimulatorDlg::Init()
{
	if (m_pMain == NULL){
		RECT rcTarget;
		GetClientRect(&rcTarget);
		
		m_pMain = new CViewMain(rcTarget, this, 1);
	}
}