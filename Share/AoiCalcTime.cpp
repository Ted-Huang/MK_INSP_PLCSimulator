#include "StdAfx.h"
#include "AoiCalcTime.h"

AoiCalcTime::AoiCalcTime(int nLine,bool bDebugDump)
{
	Init();
	PushCalcTime();
	m_nStartLine = nLine;
	m_bDebugDump = bDebugDump;
}
AoiCalcTime::AoiCalcTime(CString strDes, bool bDebugDump)
{
	Init();
	PushCalcTime(strDes);
	m_bDebugDump = bDebugDump;
}
void AoiCalcTime::Init()
{
	m_nStartLine = 0;
#ifndef USE_QUEUE
	m_nCount = 0;
#endif //USE_QUEUE
	m_Freq.QuadPart = 0;
	m_bReleaseDump = false;
	QueryPerformanceFrequency(&m_Freq);
}
void AoiCalcTime::PushCalcTime(CString sDes,int nLine)
{
	CALC_TIME_INFO xInfo;
	xInfo.sDes = sDes;
	if (nLine){
		CString tExInfo;
		int nOffLine =nLine;
		if (m_nStartLine){
			nOffLine -= m_nStartLine;
		}
		tExInfo.Format(_T("----#%d"),nOffLine);
		xInfo.sDes += tExInfo;
	}
	QueryPerformanceCounter(&xInfo.xTime);
#ifdef USE_QUEUE
	m_xCalcTime.push(xInfo);
#else //USE_QUEUE
	m_xCalcTime[m_nCount] = xInfo;
	m_nCount++;
	if (m_nCount >= MAX_INFO_SIZE){
		m_nCount = MAX_INFO_SIZE-1;
	}
#endif //USE_QUEUE
}
bool AoiCalcTime::IsEmpty()
{
#ifdef USE_QUEUE
	return m_xCalcTime.empty();	
#else //USE_QUEUE
	return m_nCount==0;
#endif //USE_QUEUE
}
int AoiCalcTime::PopCalcTime(LARGE_INTEGER *pEndTime,bool bDump,CString *pStr)
{
	int nCalcTime = 0;
#ifdef USE_QUEUE
	int nSize = (int)m_xCalcTime.size();
	if (nSize>0){
		CALC_TIME_INFO xInfo = m_xCalcTime.front();
#else //USE_QUEUE
	if (m_nCount>0){
		m_nCount--;
		CALC_TIME_INFO xInfo = m_xCalcTime[m_nCount];
#endif //USE_QUEUE
		LARGE_INTEGER tCalcTimeEnd;
		tCalcTimeEnd.QuadPart = 0;
		if (pEndTime==NULL){
			QueryPerformanceCounter(&tCalcTimeEnd);
		}else{
			tCalcTimeEnd = *pEndTime;
		}
		nCalcTime = (int)((tCalcTimeEnd.QuadPart-xInfo.xTime.QuadPart)*1000000/m_Freq.QuadPart);
		if (bDump){
			CString tDumpInfo;
			if (xInfo.sDes.IsEmpty()){
				tDumpInfo.Format(_T("\nTime:%d us"),nCalcTime);
			}else{
				tDumpInfo.Format(_T("\n%s Time:%d us"),xInfo.sDes,nCalcTime);
			}
			if (m_bReleaseDump){
				OutputDebugString(tDumpInfo);
			}else{
				if(m_bDebugDump)
					TRACE(_T("%s"),tDumpInfo);
			}
			if (pStr){
				*pStr = tDumpInfo;
			}
		}
#ifdef USE_QUEUE
		m_xCalcTime.pop();
#endif //USE_QUEUE
	}
	return nCalcTime;
}
AoiCalcTime::~AoiCalcTime(void)
{
	LARGE_INTEGER tEndTime;
	tEndTime.QuadPart = 0;
	QueryPerformanceCounter(&tEndTime);
#ifdef USE_QUEUE
	while (m_xCalcTime.size()){
#else //USE_QUEUE
	while (m_nCount){
#endif //USE_QUEUE
		PopCalcTime(&tEndTime,true);
	}
}
