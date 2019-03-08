#include "StdAfx.h"
#include <Strsafe.h>
#include "AoiFont.h"

CAoiFontManager::CAoiFontManager(void)
{
	Init();
}
CAoiFontManager::~CAoiFontManager(void)
{
	Finalize();
}
void CAoiFontManager::DestroyFont()
{
	for (int i=0;i<ctMaxFont;i++){
		m_fFont[i].DeleteObject();
	}
}
void CAoiFontManager::CreateFont()
{
	if (!m_bInit){
		CWnd *pWnd = AfxGetMainWnd();
		if (pWnd){
			CLogFont lf;
			CFont *pFont = pWnd->GetFont();
			pFont->GetLogFont(&lf);

			for (int i=0;i<ctMaxFont;i++){
				m_fFont[i].SetFontColor(ctFont[i].nFontColor);
				if (ctFont[i].nFontSubType == typeBold){
					lf.lfWeight = FW_BOLD;
				}else{
					lf.lfWeight = FW_NORMAL;
				}
				lf.lfItalic = FALSE;
				lf.lfUnderline = FALSE;
				lf.lfHeight = -ctFont[i].nFontSize;
				lf.lfEscapement = ctFont[i].nAngle * 10;
				switch(ctFont[i].nFontId){
				case typeSegoeUI: //SegoeUi.ttf
					StringCchCopy(lf.lfFaceName, LF_FACESIZE,_T("Segoe UI"));
					break;
				case typeMicroSoftBlod: //msjhbd.ttf,msjh.ttf
					StringCchCopy(lf.lfFaceName, LF_FACESIZE,_T("·L³n¥¿¶ÂÅé"));
					break;
				case typeYuGothic: //Not Yet
					StringCchCopy(lf.lfFaceName, LF_FACESIZE, _T("Segoe UI"));
					break;
				default:
					break;
				}
				BOOL bFlag = m_fFont[i].CreateFontIndirect(&lf);
			}
			m_bInit = true;
		}
	}
}
CFont* CAoiFontManager::GetFont(FontDef xId)
{
	return GetFontType(xId);
}
COLORREF CAoiFontManager::GetFontColor(FontDef xId)
{
	CAoiFont *pFont = GetFontType(xId);
	if (pFont){
		return pFont->GetFontColor();
	}
	return ctFontColor[0];
}
void CAoiFontManager::SetWindowFont(CWnd *pWnd,FontDef xId,BOOL bRedraw)
{
	if (pWnd){
		CAoiFont *pFont = GetFontType(xId);
		if (pFont){
			pWnd->SetFont(pFont,bRedraw);
		}
	}
}
void CAoiFontManager::Init()
{
	m_bInit = false;
	CreateFont();
}
void CAoiFontManager::Finalize()
{
	DestroyFont();
}
CAoiFont *CAoiFontManager::GetFontType(FontDef xIdx)
{
	CreateFont();
	CAoiFont *pFont = NULL;
	for (int i=0;i<ctMaxFont;i++){
		if (i == xIdx){
			pFont = &m_fFont[i];
		}
	}
	return pFont;
}