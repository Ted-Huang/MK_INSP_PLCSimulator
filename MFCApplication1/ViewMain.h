#pragma once

#include "OpenGLControl.h"

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