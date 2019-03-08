
// MK_INSP_PLCSimulatorDlg.cpp : ��@��
//

#include "stdafx.h"
#include "MK_INSP_PLCSimulator.h"
#include "MK_INSP_PLCSimulatorDlg.h"
#include "afxdialogex.h"
#include "UI_VIEW\ViewMain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMK_INSP_PLCSimulatorDlg ��ܤ��



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


// CMK_INSP_PLCSimulatorDlg �T���B�z�`��

BOOL CMK_INSP_PLCSimulatorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �]�w����ܤ�����ϥܡC�����ε{�����D�������O��ܤ���ɡA
	// �ج[�|�۰ʱq�Ʀ��@�~
	SetIcon(m_hIcon, TRUE);			// �]�w�j�ϥ�
	SetIcon(m_hIcon, FALSE);		// �]�w�p�ϥ�

	// TODO:  �b���[�J�B�~����l�]�w
	Init();
	return TRUE;  // �Ǧ^ TRUE�A���D�z�ﱱ��]�w�J�I
}

// �p�G�N�̤p�ƫ��s�[�J�z����ܤ���A�z�ݭn�U�C���{���X�A
// �H�Kø�s�ϥܡC���ϥΤ��/�˵��Ҧ��� MFC ���ε{���A
// �ج[�|�۰ʧ������@�~�C

void CMK_INSP_PLCSimulatorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ø�s���˸m���e

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// �N�ϥܸm����Τ�ݯx��
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �yø�ϥ�
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ��ϥΪ̩즲�̤p�Ƶ����ɡA
// �t�ΩI�s�o�ӥ\����o�����ܡC
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