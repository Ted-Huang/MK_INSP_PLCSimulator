#include "stdafx.h"
#include "InspThread.h"
#ifdef USE_OLG_EDGE
#include "EdgeFinder.h"
#else //USE_OLG_EDGE
#include "LineDetection.h"
#endif //USE_OLG_EDGE
#include "AnchorAlignment.h"
#include "AoiBaseOp.h"
//#include "InspChData.h" //for g_vInsp
#include "MachineInfo.h" //g_xMachine
#include "DevManager.h" //g_Dev
CMachineInfo g_xMachine;

IMPLEMENT_DYNCREATE(CInspThread, CWinThread)
CInspThread::CInspThread()
{
	m_pLinkWnd = NULL;
	m_nThreadKey = 0;
	m_pProc = NULL;
	m_pVerif = NULL;
#ifdef INSP_DUMP_IMG
	m_bSaveInsp = TRUE;
#else //INSP_DUMP_IMG
	m_bSaveInsp = FALSE;
#endif //INSP_DUMP_IMG
	memset(&m_xImgLowLv,0,sizeof(m_xImgLowLv));
	memset(&m_xImgHighLv,0,sizeof(m_xImgHighLv));
#ifdef USE_OLG_EDGE
	if (m_pProc == NULL){
		m_pProc = new CEdgeFinder();
	}
#else //USE_OLG_EDGE
	if (m_pProc == NULL){
		m_pProc = new CLineDetection();
	}
#endif //USE_OLG_EDGE
	if (m_pVerif == NULL){
		m_pVerif = new AnchorAlignment();
	}
#ifdef _UNICODE					
	wchar_t	workingDir[_MAX_PATH];	// the working path
	_wgetcwd(workingDir, _MAX_PATH);
#else
	char	workingDir[_MAX_PATH];	// the working path
	_getcwd(workingDir, _MAX_PATH);
#endif
	m_strPath = workingDir;

	m_xFreq.QuadPart = 0;
	m_xStart.QuadPart = 0;
	m_xEnd.QuadPart = 0;
	OpThreadTime(OP_TIME_FREQ);
}


CInspThread::~CInspThread()
{
	if (m_xImgLowLv.ptr){
		delete []m_xImgLowLv.ptr;
		m_xImgLowLv.ptr = NULL;
	}
	if (m_xImgHighLv.ptr){
		delete []m_xImgHighLv.ptr;
		m_xImgHighLv.ptr = NULL;
	}

	if (m_pProc){
		delete m_pProc;
		m_pProc = NULL;
	}
	if (m_pVerif){
		delete m_pVerif;
		m_pVerif = NULL;
	}
}


BOOL CInspThread::InitInstance()
{
	return TRUE;
}


int CInspThread::ExitInstance()
{
	return CWinThread::ExitInstance();
}
int CInspThread::OpThreadTime(int nOpCode)
{
	int nTime = 0;
	switch (nOpCode){
	case OP_TIME_START:
		m_xStart.QuadPart = 0;
		QueryPerformanceCounter(&m_xStart);
		break;
	case OP_TIME_FREQ:
		m_xFreq.QuadPart = 0;
		QueryPerformanceFrequency(&m_xFreq);
		break;
	case OP_TIME_STOP:
		m_xEnd.QuadPart = 0;
		QueryPerformanceCounter(&m_xEnd);
		break;
	case OP_TIME_DUMP:
		nTime = (m_xEnd.QuadPart - m_xStart.QuadPart) * 1000 / m_xFreq.QuadPart;
		break;
	}
	return nTime;
}
BOOL CInspThread::VerifiInitImg(IMAGE *pImg)
{
	if (m_pVerif){
		if (pImg->type == IMAGE_TYPE_MONO8){
			int anchorSize = 128;
			g_xMachine.GetMachineGoldenAnchorSize(0, anchorSize);
			RECT rtBest = { 0 };
			BOOL bSuccess = m_pVerif->findAnchor(pImg->ptr, pImg->data_w, pImg->data_h, anchorSize, rtBest);
			if (m_pLinkWnd){
				if (bSuccess){
					GEN_INSP_PARAM xParam;
					memset(&xParam, 0, sizeof(xParam));
					xParam.rcSearchRange = rtBest;
					xParam.iSearchSize = 256;
					g_xMachine.GetMachineGoldenSearchSize(0, xParam.iSearchSize);
					m_pVerif->SetGoldenImg(pImg);
					m_pVerif->SetInspParam(&xParam);
#if 0 //NO NEED
					INSP_THREAD_PARAM *pGoldParam = new INSP_THREAD_PARAM;
					pGoldParam->nKey = m_nThreadKey;
					pGoldParam->nThreadId = m_nThreadID;
					pGoldParam->nInspTime = 0;
					IMAGE *pGold = NULL;
					CloneIMAGE(&pGold, pImg);
					pGoldParam->pLinkData = (void *)pGold;
					m_pLinkWnd->PostMessage(WM_INSP_BACK_MSG, IDM_IMG_VER_GOLDEN, (LPARAM)pGoldParam);
#endif //0
				}
				INSP_THREAD_PARAM *pParam = new INSP_THREAD_PARAM;
				pParam->nKey = m_nThreadKey;
				pParam->nThreadId = m_nThreadID;
				pParam->nInspTime = 0;
				pParam->pLinkData = new RECT;
				*(RECT*)(pParam->pLinkData) = rtBest;
				m_pLinkWnd->PostMessage(WM_INSP_BACK_MSG, IDM_IMG_VER_INIT, (LPARAM)pParam);
				return TRUE;
			}
			return TRUE;
		}
	}
	return FALSE;
}
BOOL CInspThread::VerificateImg(IMAGE *pImg)
{
//for State 2
	if (m_pVerif){
		// inspect
		m_pVerif->SetSampleData(pImg);
		m_pVerif->DoAnchorAlignMent();
		// result
		int xshift = 0, yshift = 0;
		unsigned int uCorrelation = 0;
		POINT ptGold;
		ptGold = m_pVerif->GetAnchor();
		m_pVerif->GetAnchorShift(xshift, yshift);
		m_pVerif->GetAnchorCorr(uCorrelation);
		if (uCorrelation >= 0){ //anchor OK
			if (m_pLinkWnd){
				INSP_THREAD_PARAM *pParam = new INSP_THREAD_PARAM;
				pParam->nKey = m_nThreadKey;
				pParam->nThreadId = m_nThreadID;
				pParam->pLinkData = new POINT;
				pParam->nInspTime = 0;
				POINT *ptResult = (POINT*)pParam->pLinkData;
				ptResult->x = ptGold.x + xshift;
				ptResult->y = ptGold.y + yshift;
				SaveImg(pImg, 1); //20190318
				m_pLinkWnd->PostMessage(WM_INSP_BACK_MSG, IDM_IMG_VER_RESULT, (LPARAM)pParam);
			}
			return TRUE;
		}
	}
	return FALSE;
}
BOOL CInspThread::InspImg(IMAGE *pImg)
{
//for State 1
#ifdef USE_OLG_EDGE
	RESULT_INFO xResult;
	memset(&xResult, 0, sizeof(xResult));
#else //USE_OLG_EDGE
	LD_RESULT xResult;
	memset(&xResult.ptAnchor, 0, sizeof(xResult.ptAnchor));
	memset(&xResult.xLineInfo, 0, sizeof(xResult.xLineInfo));
	xResult.lineShift = 0.0;
#endif //USE_OLG_EDGE
	if (m_pProc){
		OpThreadTime(OP_TIME_START);
#ifdef USE_OLG_EDGE
		m_pProc->GetMode(xResult.xModeInfo);
		m_pProc->Insp(pImg, &xResult.xAlignmentInfo);
#else //USE_OLG_EDGE
		LD_INSP_STATUS xStatus = m_pProc->Insp(pImg, &xResult);
#endif //USE_OLG_EDGE
		OpThreadTime(OP_TIME_STOP);
		if (m_pLinkWnd){
			INSP_THREAD_PARAM *pParam = new INSP_THREAD_PARAM;
			pParam->nKey = m_nThreadKey;
			pParam->nThreadId = m_nThreadID;
			pParam->nInspTime = OpThreadTime(OP_TIME_DUMP);
#ifdef USE_OLG_EDGE
			pParam->pLinkData = new RESULT_INFO;
			*(RESULT_INFO*)(pParam->pLinkData) = xResult;
#else //USE_OLG_EDGE
			pParam->pLinkData = new LD_RESULT;
			pParam->pGoldData = new LD_RESULT;
			*(LD_RESULT*)(pParam->pLinkData) = xResult;
			m_pProc->getGoldenResult((LD_RESULT*)(pParam->pGoldData));
			SaveImg(pImg, 0); //20190318
#endif //USE_OLG_EDGE
			m_pLinkWnd->PostMessage(WM_INSP_BACK_MSG, IDM_RESULT_STATE1_DONE, (LPARAM)pParam);
		}
		return TRUE;
	}
	return FALSE;
}
void CInspThread::SaveImg(IMAGE *pImg, int nType)
{
	if (pImg && pImg->ptr && m_bSaveInsp){
		CString strBmpFile;
		if (nType == 0){ //for Normal Insp
			strBmpFile.Format(_T("%s\\Dump\\CH%d.bmp"), m_strPath, (int)m_nThreadKey + 1);
		}
		else if (nType == 1){ //for Verify Insp
			strBmpFile.Format(_T("%s\\Dump\\CH%d_VER.bmp"), m_strPath, (int)m_nThreadKey + 1);
		}
		int nColor = GetImagePixelBytes(pImg);
		saveimage(strBmpFile, pImg->ptr, pImg->data_w, -pImg->data_h, nColor, FALSE);
	}

}
void CInspThread::SaveGolden(IMAGE *pImg)
{
	if (pImg && pImg->ptr){
		CString strGoldFile;
		strGoldFile.Format(_T("%s\\GOLDEN\\GOLD%d.bmp"), m_strPath, (int)m_nThreadKey + 1);
		int nColor = GetImagePixelBytes(pImg);
		saveimage(strGoldFile, pImg->ptr, pImg->data_w, -pImg->data_h, nColor, FALSE);
	}
}
void CInspThread::SaveVerifyImg(IMAGE *pImg)
{
	if (pImg && pImg->ptr){
		CString strGoldFile;
		strGoldFile.Format(_T("%s\\GOLDEN\\GOLD_VER%d.bmp"), m_strPath, (int)m_nThreadKey + 1);
		int nColor = GetImagePixelBytes(pImg);
		saveimage(strGoldFile, pImg->ptr, pImg->data_w, -pImg->data_h, nColor, FALSE);
	}
}
BOOL CInspThread::GoldenInitImg(IMAGE *pImg)
{
#ifdef USE_OLG_EDGE
	RESULT_INFO xResult;
	if (m_pProc){
		m_pProc->CheckMode(pImg, &xResult);
	}
#endif //USE_OLG_EDGE
	if (m_pLinkWnd){
		INSP_THREAD_PARAM *pParam = new INSP_THREAD_PARAM;
		pParam->nKey = m_nThreadKey;
		pParam->nThreadId = m_nThreadID;
#ifdef USE_OLG_EDGE
		pParam->pLinkData = new RESULT_INFO;
		*(RESULT_INFO*)(pParam->pLinkData) = xResult;
#else //USE_OLG_EDGE
		int nMethod = 0;
		g_Dev.GetInspMethod((int)m_nThreadKey, nMethod);
		LD_INSP_PARAM xInspParam;
		memset(&xInspParam, 0, sizeof(xInspParam));
		BOOL bErrEdge = FALSE;
		if (nMethod ==0){ //Edge Mode
			vector<LINE_INFO> vLine;
			m_pProc->EdgeModePreprocess(pImg, &vLine);
			xInspParam.inspMode = LD_INSP_MODE_EDGE;
			if (vLine.size()){
				int nDstNo = 0;
				g_Dev.GetInspLineNo((int)m_nThreadKey, nDstNo);
				int nLineIdx = 0;
				for (auto xLine : vLine){
					g_xMachine.SetMachineGoldenLinePixel((int)m_nThreadKey, nLineIdx, xLine.position);
					nLineIdx++;
				}
				if (nDstNo < vLine.size()){
					xInspParam.lineSelectedIdx = nDstNo;
					xInspParam.xEdgeLineRef = vLine[nDstNo];
				}
				else{
					xInspParam.lineSelectedIdx = 0;
					xInspParam.xEdgeLineRef = vLine[0];
					bErrEdge = TRUE;
				}
			}
		}
		else if (nMethod == 1){ //Color Bar Mode
			xInspParam.inspMode = LD_INSP_MODE_COLOR;
			xInspParam.bUseGreyLevel = false;
			xInspParam.greyLevel = 0;
			g_Dev.GetInspBarWidth((int)m_nThreadKey, xInspParam.colorBarWidth);
			int nEnable = 0;
			g_Dev.GetInspBarEnable((int)m_nThreadKey, nEnable);
			if (nEnable){
				int nColor = 0;
				g_Dev.GetInspBarColor((int)m_nThreadKey, nColor);
				xInspParam.bUseGreyLevel = true;
				xInspParam.greyLevel = (nColor&0xFF);
			}
		}
		else if (nMethod == 2){ //Anchor Mode
			xInspParam.inspMode = LD_INSP_MODE_ANCHOR;
			xInspParam.ptAnchor = {  pImg->data_w/2,pImg->data_h/2};
		}
		else {
			xInspParam.inspMode = LD_INSP_MODE_EDGE;
		}
		m_pProc->setInspParam(pImg, xInspParam);
		pParam->pLinkData = new LD_INSP_PARAM;
		pParam->pGoldData = new LD_RESULT;
		*(LD_INSP_PARAM*)pParam->pLinkData = xInspParam;
		m_pProc->getGoldenResult((LD_RESULT*)pParam->pGoldData);
		if (bErrEdge){
			((LD_RESULT*)pParam->pGoldData)->status = LD_INSP_EDGE_FAIL;
		}
#endif //USE_OLG_EDGE
		m_pLinkWnd->PostMessage(WM_INSP_BACK_MSG, IDM_IMG_GOLDEN_INIT, (LPARAM)pParam);
		return TRUE;
	}
	return FALSE;
}
void CInspThread::InspDone(IMAGE *pImg, BOOL bVerify)
{
	DestroyIMAGE(&pImg);
}
void CInspThread::InspOriInit(int nOri)
{
	if (m_pProc){
		GRAB_DEGREE_ xOri;
		xOri = GRAB_DEGREE_0;
		if (nOri == 1){
			xOri = GRAB_DEGREE_90;
		}
		else if (nOri == 2){
			xOri = GRAB_DEGREE_180;
		}
		else if (nOri == 3){
			xOri = GRAB_DEGREE_270;
		}
#ifdef USE_OLG_EDGE
		m_pProc->SetCameraOri(xOri);
#else //USE_OLG_EDGE
		m_pProc->setLineType(LD_LINE_TYPE_VERTICAL);
#endif //USE_OLG_EDGE
	}
}
#ifdef USE_OLG_EDGE
void CInspThread::InspBGColorInit(int nBGColor)
{
	if (m_pProc){
		m_pProc->SetBGColor(nBGColor);
	}
}
void CInspThread::InspInit(void *pData)
{
	if (m_pProc){
		MODE_INFO *pMode = (MODE_INFO *)pData;
		m_pProc->SetMode(*pMode);
	}
}
#endif //USE_OLG_EDGE
void CInspThread::InspVerificateSetGold(IMAGE *pImg)
{
	if (m_pVerif){
		int anchorSize = 128;
		g_xMachine.GetMachineGoldenAnchorSize(0, anchorSize);
		RECT rtBest = { 0 };
		BOOL bSuccess = m_pVerif->findAnchor(pImg->ptr, pImg->data_w, pImg->data_h, anchorSize, rtBest);
		if (bSuccess){
			GEN_INSP_PARAM xParam;
			memset(&xParam, 0, sizeof(xParam));
			xParam.rcSearchRange = rtBest;
			xParam.iSearchSize = 256;
			g_xMachine.GetMachineGoldenSearchSize(0, xParam.iSearchSize);
			m_pVerif->SetGoldenImg(pImg);
			m_pVerif->SetInspParam(&xParam);
		}
	}
}
BEGIN_MESSAGE_MAP(CInspThread, CWinThread)
	ON_THREAD_MESSAGE(WM_INSPMSG, OnInspMessage)
END_MESSAGE_MAP()

void CInspThread::OnInspMessage(WPARAM wParam, LPARAM lParam)
{
	switch (wParam){
	case IDM_INSPQUIT_MSG:
		AfxEndThread(0);
		break;
	case IDM_SETMAINWIN_MSG:
		m_pLinkWnd = (CWnd*)lParam;
		break;
	case IDM_SETKEY_MSG:
		m_nThreadKey = (UINT_PTR)lParam;
		break;
	case IDM_CAM_ORI_SET:
		InspOriInit((int)lParam);
		break;
#ifdef USE_OLG_EDGE
	case IDM_BG_COLOR_SET:
		InspBGColorInit((int)lParam);
		break;
	case IDM_IMG_GOLDEN_SET:
		InspInit((void *)lParam);
		delete (MODE_INFO *)lParam;
		break;
#endif //USE_OLG_EDGE
	case IDM_IMG_VER_GOLDEN:
		{
			IMAGE *pGold = (IMAGE*)lParam;
			InspVerificateSetGold(pGold);
			DestroyIMAGE(&pGold);
		}
		break;
	case IDM_IMG_VER_INIT:
		{
			IMAGE *pInspImg = (IMAGE*)lParam;
			VerifiInitImg(pInspImg);
			SaveVerifyImg(pInspImg);
			InspDone(pInspImg,TRUE);
		}
		break;
	case IDM_IMG_VER_IN:
		{
			IMAGE *pInspImg = (IMAGE*)lParam;
			if (m_pVerif){
				if (m_pVerif->IsInit()){
					VerificateImg(pInspImg);
				}
				else{
					VerifiInitImg(pInspImg);
					SaveVerifyImg(pInspImg);
				}
			}
			InspDone(pInspImg,TRUE);
		}
		break;
	case IDM_IMG_IN:
		{
			IMAGE *pInspImg = (IMAGE*)lParam;
			InspImg(pInspImg);
			InspDone(pInspImg,FALSE);
		}
		break;
	case IDM_IMG_GOLDEN_INIT:
		{
			IMAGE *pInspImg = (IMAGE*)lParam;
			GoldenInitImg(pInspImg);
			SaveGolden(pInspImg);
			InspDone(pInspImg,FALSE);
		}
		break;
	case IDM_IMG_LOW_LEVEL:
		{
			IMAGE *pImg = (IMAGE *)lParam;
			IMAGE *pDst = &m_xImgLowLv;
			CloneIMAGE(&pDst, pImg);
			DestroyIMAGE(&pImg);
		}
		break;
	case IDM_IMG_HIGH_LEVEL:
		{
			IMAGE *pImg = (IMAGE *)lParam;
			IMAGE *pDst = &m_xImgHighLv;
			CloneIMAGE(&pDst, pImg);
			DestroyIMAGE(&pImg);
		}
		break;
	case IDM_IMG_MARK_LHT_LEVEL:
		if (m_pProc && m_xImgLowLv.ptr && m_xImgHighLv.ptr){
			LD_LIGHT_SOURCE xResult = m_pProc->DetectLightSource(&m_xImgHighLv, &m_xImgLowLv);
			switch (xResult){
			case LD_LIGHT_SOURCE_DARK:
				g_Dev.SetInspLightType((int)m_nThreadKey, (int)LD_LIGHT_SOURCE_DARK);
				break;
			case LD_LIGHT_SOURCE_BRIGHT:
				g_Dev.SetInspLightType((int)m_nThreadKey, (int)LD_LIGHT_SOURCE_BRIGHT);
				break;
			default:
				g_Dev.SetInspLightType((int)m_nThreadKey, (int)LD_LIGHT_SOURCE_UNKNOWN);
				TRACE("\nDETECT LIGHT STATUS ERROR!");
				break;
			}
			if (m_pLinkWnd){
				INSP_THREAD_PARAM *pParam = new INSP_THREAD_PARAM;
				memset(pParam, 0, sizeof(INSP_THREAD_PARAM));
				pParam->nKey = m_nThreadKey;
				m_pLinkWnd->PostMessage(WM_INSP_BACK_MSG, IDM_IMG_LEVEL_CHECK, (LPARAM)pParam);
			}
		}
		else{
			TRACE("\nIDM_IMG_MARK_LHT_LEVEL ERROR!");
		}
		break;
	}
}