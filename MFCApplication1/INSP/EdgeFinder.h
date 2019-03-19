#pragma once

#include "SharedComponent.h" //IMAGE
//#define ENABLE_PIXEL_BASE

enum INSP_MODE
{
	INSP_MODE_UNKNOWN = 0,	// detect insp mode
	INSP_MODE_SINGLE,	// single line
	INSP_MODE_DOUBLE,	// double line
	INSP_MODE_RECT,		// single line & single rect

	INSP_MAX_MODE_NUM
};

enum LINE_SELECTED
{
	LINE_UNKNOWN=0,
	LINE_FIRST=1,
	LINE_SECOND=2,
	LINE_THIRD=3
};

enum GRAB_DEGREE_{
	GRAB_DEGREE_0,
	GRAB_DEGREE_90,
	GRAB_DEGREE_180,
	GRAB_DEGREE_270,
};

enum ORIENTATION
{
	UNKNOWN = 0,
	TOP,	//0
	RIGHT,	//90
	BOTTOM,	//180
	LEFT	//270
};

typedef struct _ALIGNMENT_INFO{
	float line[3];
#ifdef ENABLE_PIXEL_BASE
	float line_int[3];
#endif
	float lineShift;
}ALIGNMENT_INFO;

typedef struct _MODE_INFO{
	INSP_MODE mode;
	ORIENTATION bgo;
	
	LINE_SELECTED lineSelected;
	float baseLine;
}MODE_INFO;

typedef struct _RESULT_INFO{
	ALIGNMENT_INFO xAlignmentInfo;
	MODE_INFO xModeInfo;
}RESULT_INFO;

typedef struct _CALIBRATION{
	float fXSizeTop;
	float fXSizeBottom;
	float fYSizeLeft;
	float fYSizeRight;
	float fRotationAngle;//0 or 180

	float x_fov;
	float y_fov;
	float area_fov;
}CALIBRATION;

typedef struct _DEFECT_PARAM{
	int iDefectTH;
	int iDefectPnt;
}DEFECT_PARAM;

class CEdgeFinder
{
private:
	IMAGE m_xGolden;
	IMAGE m_xMaxMap;
	IMAGE m_xMinMap;
	DEFECT_PARAM m_xDefectParam;

	int m_budget;
	int m_dynamicRange;
	GRAB_DEGREE_ m_cameraOri;
	MODE_INFO m_xModeInfo;
	int m_BGColor;

	float m_fCaliWidth;
	float m_fCaliHeight;

	void GetLineNo(ALIGNMENT_INFO *alignmentInfo);
	void findHoriLine(IMAGE *xImg, ALIGNMENT_INFO *xAlignmentInfo);
	void findVertLine(IMAGE *xImg, ALIGNMENT_INFO *xAlignmentInfo);
	ORIENTATION checkBGO(IMAGE *xImg);
	void CreateMinMax(IMAGE *xImg);
public:
	CEdgeFinder();
	~CEdgeFinder();

	void Insp(IMAGE *xImg,				// Input
		ALIGNMENT_INFO *alignmentInfo);	// Output

	void CheckMode(IMAGE *xImg,		// Input
		RESULT_INFO *resultInfo);	// Output
	void GetMode(MODE_INFO &xModeInfo){ xModeInfo = m_xModeInfo; };
	void SetBudget(int budget){ m_budget = budget; };
	void SetMode(MODE_INFO xModeInfo){ m_xModeInfo = xModeInfo; };
	void SetDynamicRange(int dynamicRange){ m_dynamicRange = dynamicRange; };//24,36,48
	void SetCameraOri(GRAB_DEGREE_ cameraOri){ m_cameraOri = cameraOri; };
	void SetBGColor(int nBGColor){ m_BGColor = nBGColor; };

	void SetCaliSize(float width, float height){ m_fCaliWidth = width; m_fCaliHeight = height; };
	bool CalculateFOV(IMAGE *xImg, CALIBRATION &xCali);

	void SetGolden(IMAGE *xImg);
	void SetDefectParam(DEFECT_PARAM defectParam){ m_xDefectParam = defectParam; };
	bool CheckDefect(IMAGE *xSample, int shiftX, int shiftY);
};