#pragma once
#include <WinBase.h>
#ifdef USE_QUEUE
#include <queue>
using namespace std;
#endif //USE_QUEUE
/**********************************************************
// AOI Class For Dump Process Time
//                                       By Eric Chao....
// if using in Release Mode,need set m_bReleaseDump = true
***********************************************************/
#define MAX_INFO_SIZE 40
typedef struct CALC_TIME_INFO_
{
	CString sDes;
	LARGE_INTEGER xTime;
}CALC_TIME_INFO;
class AoiCalcTime
{
public:
	AoiCalcTime(int nLine=0,bool bDebugDump=true);
	AoiCalcTime(CString strDes, bool bDebugDump = true);
	~AoiCalcTime(void);
	void PushCalcTime(CString sDes=_T(""),int nLine=0);
	int PopCalcTime(LARGE_INTEGER *pEndTime=NULL,bool bDump=false,CString *pStr=NULL);
	void EnableReleaseDump(bool bEnable) { m_bReleaseDump = bEnable;}; //eric chao 20160608
	bool IsEmpty();
private:
	bool m_bReleaseDump;
	bool m_bDebugDump;
	int m_nStartLine;
	LARGE_INTEGER m_Freq;
	void Init();
#ifdef USE_QUEUE
	queue<CALC_TIME_INFO> m_xCalcTime;
#else //USE_QUEUE
	CALC_TIME_INFO m_xCalcTime[MAX_INFO_SIZE];
	int m_nCount;
#endif //USE_QUEUE
};
