#pragma once
#include "afxwin.h"
#include "SharedComponent.h" //For IMAGE
#define WM_INSPMSG			(WM_APP+2000)

#define IDM_SETMAINWIN_MSG	(WM_APP+2003)
#define IDM_INSPQUIT_MSG	(WM_APP+2004)
#define IDM_SETKEY_MSG		(WM_APP+2005)
#define IDM_IMG_IN			(WM_APP+2006)
#define IDM_IMG_GOLDEN_INIT (WM_APP+2007)
#ifdef USE_OLG_EDGE
#define IDM_IMG_GOLDEN_SET	(WM_APP+2008)
#endif //USE_OLG_EDGE
#define IDM_CAM_ORI_SET		(WM_APP+2009)
#define IDM_IMG_VER_IN		(WM_APP+2010)  //For Image VERIFICATION
#define IDM_IMG_VER_INIT	(WM_APP+2011)
#define IDM_IMG_VER_GOLDEN	(WM_APP+2012)
#define IDM_IMG_VER_RESULT  (WM_APP+2013)
#ifdef USE_OLG_EDGE
#define IDM_BG_COLOR_SET	(WM_APP+2014)
#endif //USE_OLG_EDGE

#define IDM_IMG_LOW_LEVEL	(WM_APP+2015)
#define IDM_IMG_HIGH_LEVEL	(WM_APP+2016)
#define IDM_IMG_MARK_LHT_LEVEL (WM_APP+2017)

//Back to Main Wnd
#define WM_INSP_BACK_MSG	(WM_APP+1000)

#define IDM_IMG_DONE			(WM_APP+1001)
#define IDM_RESULT_STATE1_DONE	(WM_APP+1002)
#define IDM_RESULT_STATE2_DONE	(WM_APP+1003)
#define IDM_IMG_VERIFY_DONE		(WM_APP+1004)
#define IDM_IMG_LEVEL_CHECK		(WM_APP+1005)


typedef struct INSP_THREAD_PARAM_
{
	UINT_PTR nKey;
	UINT_PTR nThreadId;
	int nInspTime;
	void *pLinkData;
#ifndef USE_OLG_EDGE
	void *pGoldData;
#endif //USE_OLG_EDGE
}INSP_THREAD_PARAM;

class AnchorAlignment;
#ifdef USE_OLG_EDGE
class CEdgeFinder;
#else //USE_OLG_EDGE
class CLineDetection;
#endif //USE_OLG_EDGE

class CInspThread :
	public CWinThread
{
	DECLARE_DYNCREATE(CInspThread)
public:
	CInspThread();
	~CInspThread();
	virtual BOOL InitInstance();
	virtual int ExitInstance();
private:
	BOOL VerifiInitImg(IMAGE *pImg);
	BOOL VerificateImg(IMAGE *pImg);
	BOOL GoldenInitImg(IMAGE *pImg);
	void InspOriInit(int nOri);
#ifdef USE_OLG_EDGE
	void InspBGColorInit(int nBGColor);
	void InspInit(void *pData);
#endif //USE_OLG_EDGE
	void InspVerificateSetGold(IMAGE *pImg);

	BOOL InspImg(IMAGE *pImg);
	void InspDone(IMAGE *pImg,BOOL bVerify=FALSE);
	void SaveGolden(IMAGE *pImg);
	void SaveVerifyImg(IMAGE *pImg);
	void SaveImg(IMAGE *pImg, int nType);
private:
	enum {
		OP_TIME_START = 0,
		OP_TIME_STOP,
		OP_TIME_FREQ,
		OP_TIME_DUMP,
	};
	int OpThreadTime(int nOpCode);
private:
	LARGE_INTEGER m_xFreq;
	LARGE_INTEGER m_xStart;
	LARGE_INTEGER m_xEnd;
	IMAGE m_xImgLowLv;
	IMAGE m_xImgHighLv;
	CWnd *m_pLinkWnd;
	UINT_PTR  m_nThreadKey;
	AnchorAlignment *m_pVerif;
#ifdef USE_OLG_EDGE
	CEdgeFinder *m_pProc;
#else //USE_OLG_EDGE
	CLineDetection *m_pProc;
#endif //USE_OLG_EDGE
	CString m_strPath;
	BOOL m_bSaveInsp;
protected:
	DECLARE_MESSAGE_MAP()
	void OnInspMessage(WPARAM wParam, LPARAM lParam);
};

