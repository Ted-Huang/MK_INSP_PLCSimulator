#include "stdafx.h"
#include "ViewMain.h"

#include "aoiconst.h"
#include "AoiBaseOp.h"

const CRect ctGL = { 10, 10, 100, 100 };



CViewMain::CViewMain(RECT &rcTarget, CWnd *pParent, UINT ResourceId)
{
	m_pOgl = NULL;
	Create(NULL, _T("CViewMain"), WS_CHILD | WS_VISIBLE, rcTarget, pParent, ResourceId);
	Init();

}

CViewMain::~CViewMain()
{
}

void CViewMain::Init()
{
	if (m_pOgl == NULL){
		m_pOgl = new COpenGLControl();

		m_pOgl->oglCreate(ctGL, this, 1231);
		m_pOgl->oglSetBackgroundColor(::GetSysColor(COLOR_APPWORKSPACE));
		m_pOgl->oglShowAnchorPoints(TRUE);
	}
	IMAGE *pCur = new IMAGE;
	CString strGoldFile;
	strGoldFile.Format(_T("D:\\AOI_TOOL\\MK_UI\\MFCApplication1\\GOLDEN\\GOLD1.bmp"));
	unsigned char *pGoldBuf = NULL;
	int nW = 0, nH = 0, nC = 1;
	loadimage(strGoldFile, &pGoldBuf, &nW, &nH, &nC, READ_DEFAULT, FALSE);
	ImageInit(pCur, IMAGE_TYPE_MONO8, pGoldBuf, nW, nH);
	if (m_pOgl){
		m_pOgl->oglSetTexture(pCur);
		m_pOgl->oglDrawScene();
	}
}