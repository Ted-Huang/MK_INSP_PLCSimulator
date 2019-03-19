#pragma once

#include "OpenGLControl.h"

class CViewTest : public CWnd{
public:
	CViewTest(RECT &rcTarget, CWnd *pParent, UINT ResourceId);
	~CViewTest();


protected:
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void OnPaint();
};
class CViewMain : public CWnd{
public:
	CViewMain(RECT &rcTarget, CWnd *pParent, UINT ResourceId);
	~CViewMain();
private:
	void Init();
	COpenGLControl *m_pOgl;
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void OnLButtonDown(UINT nFlags, CPoint point);
};