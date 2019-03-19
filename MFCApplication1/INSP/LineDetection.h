#pragma once

#include <windows.h>//POINT
#include <vector>
using std::vector;

#include "SharedComponent.h" //IMAGE

enum LD_INSP_MODE
{
	LD_INSP_MODE_UNKNOWN=0,
	LD_INSP_MODE_EDGE,		// edge
	LD_INSP_MODE_COLOR,		// color bar
	LD_INSP_MODE_ANCHOR,	// correlation

	LD_INSP_MODE_MAX_NUM
};

enum LD_LINE_TYPE
{
	LD_LINE_TYPE_UNKNOWN=0,
	LD_LINE_TYPE_VERTICAL,
	LD_LINE_TYPE_HORIZONTAL
};

enum LD_LIGHT_SOURCE
{
	LD_LIGHT_SOURCE_UNKNOWN=0,
	LD_LIGHT_SOURCE_DARK,
	LD_LIGHT_SOURCE_BRIGHT
};

enum LD_INSP_STATUS
{
	LD_INSP_SUCCESS=0,
	LD_INSP_EDGE_FAIL,
	LD_INSP_COLOR_FAIL,
	LD_INSP_ANCHOR_FAIL,
	LD_GOLDEN_IMG_ERROR,
	LD_SAMPLE_IMG_ERROR
};

typedef struct _LINE_INFO{
	int greyLevel_0;
	int greyLevel_1;
	float position;
}LINE_INFO;

typedef struct _LD_INSP_PARAM{
	LD_INSP_MODE inspMode;

	// LD_INSP_MODE_EDGE
	int lineSelectedIdx;
	LINE_INFO xEdgeLineRef;

	// LD_INSP_MODE_COLOR
	int colorBarWidth;
	bool bUseGreyLevel;
	int greyLevel;

	// LD_INSP_MODE_ANCHOR
	POINT ptAnchor; // position in Golden
}LD_INSP_PARAM;

typedef struct _LD_RESULT{
	LD_INSP_MODE inspMode;

	// LD_INSP_MODE_EDGE
	LINE_INFO xLineInfo;
	// LD_INSP_MODE_COLOR
	std::pair<LINE_INFO, LINE_INFO> xPairLine;
	// LD_INSP_MODE_ANCHOR
	POINT ptAnchor;

	LD_INSP_STATUS status;
	float lineShift;
}LD_RESULT;

class CLineDetection
{
private:
	IMAGE m_xGolden;//for LineType & ptAnchor
	int m_iWidthTH;
	float m_fGreyLevelCountTH;
	int m_iGreyLevelTH;
	int m_iLineSelectedIdx;
	LD_LINE_TYPE m_lineType;
	LD_INSP_PARAM m_xInspParam;
	LINE_INFO m_xEdgeLineRef;
	LD_RESULT m_xGoldenResult;

	bool checkGreyLevel(IMAGE *pImg, LD_INSP_PARAM xInspParam, int start, int end);
	float findAnchor(IMAGE *pSample, POINT &ptResult);
	void setGolden(IMAGE *pImg);
	void findLineInfo(IMAGE *pImg, vector<LINE_INFO> *vLineInfo);

	LD_INSP_STATUS InspModeEdge(IMAGE *pSample, vector<LINE_INFO> *vLineInfo, LD_RESULT *pResult);
	LD_INSP_STATUS InspModeColor(IMAGE *pSample, vector<LINE_INFO> *vLineInfo, LD_RESULT *pResult);
	LD_INSP_STATUS InspModeAnchor(IMAGE *pSample, LD_RESULT *pResult);
public:
	CLineDetection();
	~CLineDetection();

	void Calibrate();
	LD_LIGHT_SOURCE DetectLightSource(IMAGE *pImgBright, IMAGE *pImgDark);
	LD_LINE_TYPE DetectLineType(IMAGE *pImg);

	void setLineType(LD_LINE_TYPE lineType){ m_lineType = lineType; };
	
	void EdgeModePreprocess(IMAGE *pGoldenImg, vector<LINE_INFO> *vLineInfo);
	void setInspParam(IMAGE *pGoldenImg, LD_INSP_PARAM xInspParam);
	void getGoldenResult(LD_RESULT *pResult);
	LD_INSP_STATUS Insp(IMAGE *pImg, LD_RESULT *pResult);
};