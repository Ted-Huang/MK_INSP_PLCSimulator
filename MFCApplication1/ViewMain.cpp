#include "stdafx.h"
#include "ViewMain.h"

#include "aoiconst.h"
#include "AoiBaseOp.h"

static int nn = 0;
const CRect ctGL = { 10, 10, 110, 110 };

const CRect ctBtn = { 200, 200, 250, 250 };

const CRect ctChange = { 300, 300, 330, 330 };

#define BK_COLOR_MAIN	RGB(230, 230, 230)

CViewTest::CViewTest(RECT &rcTarget, CWnd *pParent, UINT ResourceId)
{
	Create(NULL, _T("CViewTest"), WS_CHILD | WS_VISIBLE, rcTarget, pParent, ResourceId);
}

CViewTest::~CViewTest()
{

}

BEGIN_MESSAGE_MAP(CViewTest, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CViewTest::OnPaint()
{
	CWnd::OnPaint();

	CDC* pDC = GetDC();
	CRect rt;
	GetClientRect(rt);
	pDC->FillRect(rt, &CBrush(RGB(0,0,0)));

}

BOOL CViewTest::OnEraseBkgnd(CDC* pDC)
{
	//CRect rt;
	//GetClientRect(rt);
	//pDC->FillRect(rt, &CBrush(RGB(0,0,0)));
	return CWnd::OnEraseBkgnd(pDC);
}

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
		//CRect rr = {ctGL.left + 45, ctGL.top, ctGL.left+45+ctGL.Width(), ctGL.bottom};
		//new CViewTest(rr, this, 119);
		m_pOgl->oglCreate(rr, this, 1231);
		m_pOgl->oglSetBackgroundColor(::GetSysColor(COLOR_APPWORKSPACE));
		m_pOgl->oglShowAnchorPoints(TRUE);
	}

	if (m_pOgl){
		IMAGE *pCur = new IMAGE;
		CString strGoldFile;
		strGoldFile.Format(_T("D:\\AOI_TOOL\\MK_UI\\MFCApplication1\\GOLDEN\\GOLD1.bmp"));
		unsigned char *pGoldBuf = NULL;
		int nW = 0, nH = 0, nC = 1;
		loadimage(strGoldFile, &pGoldBuf, &nW, &nH, &nC, READ_DEFAULT, FALSE);
		ImageInit(pCur, IMAGE_TYPE_MONO8, pGoldBuf, nW, nH);
		m_pOgl->oglSetTexture(pCur);
		m_pOgl->oglDrawScene();
	}
}

void CViewMain::OnPaint()
{
	CWnd::OnPaint();

	CRect rc123 = { ctGL.left - 10, ctGL.top - 10, ctGL.right + 10, ctGL.bottom + 10 };
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

	pDC->SelectObject(redpen);
	
	pDC->SetArcDirection(AD_COUNTERCLOCKWISE);
	int nRadius = 20;
	CRect rectClient = rc123;// { 10, 10, 110, 110 };
	CPoint p1, p2;

	CRect rcRightTop = { rectClient.right - nRadius, rectClient.top, rectClient.right, nRadius };
	CRect rcRightBottom = { rectClient.right - nRadius, rectClient.bottom - nRadius, rectClient.right, rectClient.bottom };
	CRect rcLeftBottom = { rectClient.left, rectClient.bottom - nRadius, nRadius, rectClient.bottom };
	CRect rcLeftTop = { rectClient.left, rectClient.top, nRadius, nRadius };

	//left top 
	p1 = { rcLeftTop.CenterPoint().x, rcLeftTop.top };
	p2 = { rcLeftTop.left, rcLeftTop.CenterPoint().y };
	pDC->MoveTo(p1.x, p1.y); //first 
	pDC->ArcTo(rcLeftTop, p1, p2);


	//left bottom
	p1 = { rcLeftBottom.left, rcLeftBottom.CenterPoint().y };
	p2 = { rcLeftBottom.CenterPoint().x, rcLeftBottom.bottom };
	pDC->LineTo(p1.x, p1.y);
	pDC->ArcTo(rcLeftBottom, p1, p2);
	
	//right bottom
	p1 = { rcRightBottom.CenterPoint().x, rcRightBottom.right };
	p2 = { rcRightBottom.right, rcRightBottom.CenterPoint().y };
	pDC->LineTo(p1.x, p1.y);
	pDC->ArcTo(rcRightBottom, p1, p2);

	//right top
	p1 = { rcRightTop.right, rcRightTop.CenterPoint().y };
	p2 = { rcRightTop.CenterPoint().x, rcRightTop.top };
	pDC->LineTo(p1.x, p1.y);
	pDC->ArcTo(rcRightTop, p1, p2);

	p1 = { rcLeftTop.CenterPoint().x, rcLeftTop.top };
	pDC->LineTo(p1.x, p1.y);






	pDC->SelectObject(&pen);

	//pDC->RoundRect(&rc123, { 10, 10 });
	//pDC->Rectangle(rc123);

	//pDC->MoveTo(rc123.left, rc123.top);
	//pDC->LineTo(rc123.right, rc123.top);
	//pDC->LineTo(rc123.right, rc123.bottom);
	//pDC->LineTo(rc123.left, rc123.bottom);
	//pDC->LineTo(rc123.left, rc123.top);
	pDC->RoundRect(&ctBtn, { 10, 10 });

	if (nn % 2){
		pDC->SelectObject(redpen);
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