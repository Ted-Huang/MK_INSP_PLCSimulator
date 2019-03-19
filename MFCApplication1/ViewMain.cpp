#include "stdafx.h"
#include "ViewMain.h"

#include "aoiconst.h"
#include "AoiBaseOp.h"

static int nn = 0;
const CRect ctGL = { 10, 10, 100, 100 };

const CRect ctBtn = { 200, 200, 250, 250 };

const CRect ctChange = { 300, 300, 330, 330 };

#define BK_COLOR_MAIN	RGB(230, 230, 230)


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

		CRect rr = ctGL;
		//CRect rr = {ctGL.left + 300, ctGL.top, ctGL.left+300+ctGL.Width(), ctGL.bottom};
		m_pOgl->oglCreate(rr, this, 1231);
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

void CViewMain::OnPaint()
{
	CWnd::OnPaint();

	CRect rc123 = { ctGL.left - 5, ctGL.top - 5, ctGL.right + 10, ctGL.bottom + 10 };
	CDC* pDC = GetDC();
	CPen pen, backgroundpen, *oldpen, redpen, bluepen;
	CBrush brush, *olbrush;

	bluepen.CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
	redpen.CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	pen.CreatePen(PS_SOLID, 2, RGB(169, 169, 169));
	backgroundpen.CreatePen(PS_SOLID, 2, RGB(230, 230, 230)); // color same as background
	brush.CreateStockObject(NULL_BRUSH);
	SetBkMode(*pDC, TRANSPARENT);

	oldpen = pDC->SelectObject(&pen);
	olbrush = pDC->SelectObject(&brush);


	pDC->SelectObject(&pen);
	//pDC->RoundRect(&rc123, { 10, 10 });
	pDC->Rectangle(rc123);

	pDC->RoundRect(&ctBtn, { 10, 10 });

	if (nn % 2){
		pDC->SelectObject(redpen);
		//pDC->RoundRect(&ctChange, { 10, 10 });
	}
	else{
		pDC->SelectObject(bluepen);
	}
	pDC->RoundRect(&ctChange, { 10, 10 });
	pDC->SelectObject(oldpen);
	pDC->SelectObject(olbrush);

	backgroundpen.DeleteObject();
	pen.DeleteObject();
	brush.DeleteObject();
	ReleaseDC(pDC);
}
BOOL CViewMain::OnEraseBkgnd(CDC* pDC)
{
	//CRect rt;
	//GetClientRect(rt);
	//pDC->FillRect(rt, &CBrush(BK_COLOR_MAIN));
	return CWnd::OnEraseBkgnd(pDC);
}
BEGIN_MESSAGE_MAP(CViewMain, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


void CViewMain::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (ctBtn.PtInRect(point)){
		nn++;
		InvalidateRect(ctChange);
	}
}