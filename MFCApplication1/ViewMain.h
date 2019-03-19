#pragma once

#include "OpenGLControl.h"

class CViewMain : public CWnd{
public:
	CViewMain(RECT &rcTarget, CWnd *pParent, UINT ResourceId);
	~CViewMain();
private:
	void Init();
	COpenGLControl *m_pOgl;
};