#ifndef _ANCHOR_ALIGNMENT_H_
#define _ANCHOR_ALIGNMENT_H_

#include <windows.h>
#include <string>
#include "InspParamDef.h"
#include "Correlation.h"
#include "fftw3\fftw3.h"
#include "SharedComponent.h"

//#define ENABLE_DECODE

enum DECODE_STATUS{
	DECODE_SUCCESS=0,
	DECODE_FAILED,
	DECODE_NOT_FOUND
};

class AnchorAlignment
{
private:
	unsigned char *m_pGoldenImg;
	unsigned char *m_pSampleImg;
	unsigned char *m_pAnchorGoldenImg;
	unsigned char *m_pAnchorSampleImg;

	float *m_pGlodenPlane;
	float *m_pSamplePlane;
	float *m_pAnchorPlane;

	fftwf_complex *m_sampleFFTData;
	fftwf_complex *m_anchorFFTData;
	fftwf_complex *m_convolutionFFTResult;
	float *m_convolutionResult;

	fftwf_plan m_plan_sample_r2c_r;
	fftwf_plan m_plan_sample_r2c_g;
	fftwf_plan m_plan_sample_r2c_b;
	fftwf_plan m_plan_anchor_r2c_r;
	fftwf_plan m_plan_anchor_r2c_g;
	fftwf_plan m_plan_anchor_r2c_b;
	fftwf_plan m_plan_c2r_r;
	fftwf_plan m_plan_c2r_g;
	fftwf_plan m_plan_c2r_b;

	BOOL m_bInitImageData;
	BOOL m_bInitFFTData;

	int m_iW;
	int m_iH;
	int m_iColor;
	int m_iAnchorSizeX;
	int m_iAnchorSizeY;
	int m_iSearchRangeX;
	int m_iSearchRangeY;
	POINT m_ptAnchor;
	GEN_INSP_PARAM m_xInspParam;
	int m_XShift;
	int m_YShift;

	unsigned int m_uCorrelation;
	Correlation *m_corrCalc;


	BOOL SetAnchor(POINT pt);
	BOOL ProcessAnchorFromImage(IMAGE *img);
	BOOL ProcessAnchorFromGolden(void);
	BOOL SetAnchorData(void);
	void CreateImageData(void);
	void DestroyImageData(void);
	void CreateFFTData(void);
	void DestroyFFTData(void);
	void FindXYShift(void);

	// --- decode parameters
	BOOL m_bInitDecodeData;
	BOOL m_bVerticalBarcode;
	EM_DECODETYPE m_emDecodeType;
	RECT m_rtDecodeROI;
	int m_iEnhanceLevel;

	unsigned char *m_pGoldDecodeImg;
	unsigned char *m_pDecodeImg;
	unsigned char *m_pDecodeTransImg;
	int m_iDecodeW;
	int m_iDecodeH;
	void DataTranspose(void);

	bool Decode_QR(std::string &str, POINT &ptLeftTop, POINT &ptRightTop);
	bool Decode_DataMatrix(std::string &str, POINT &ptLeftTop, POINT &ptRightTop);
	bool Decode_Code128(std::string &str, POINT &ptLeftTop, POINT &ptRightTop);
	bool Decode_EAN13(std::string &str, POINT &ptLeftTop, POINT &ptRightTop);

//for FindAnchor
	void computeEnergy(unsigned char *ptr, int w, int h, RECT rt, float &G_x, float &G_y);
public:
	AnchorAlignment(void);
	~AnchorAlignment();
	BOOL findAnchor(unsigned char *ptr, int w, int h, int anchorSize, RECT &rtBest);
	BOOL IsInit() { return m_bInitImageData; };

	static void DestroyInstance(); //20160831

	void SetSearchRange(int iSearchRangeX, int iSearchRangeY);
	void SetInspParam(GEN_INSP_PARAM *xInspParam);
	BOOL SetGoldenImg(IMAGE *img);

	void SetSampleData(IMAGE *img);
	BOOL DoAnchorAlignMent(void);

	void SetDecodeROI(RECT roi);
	void SetDecodeParam(GEN_INSP_PARAM *param);
	void SetGoldenDecodeData(IMAGE *img, EM_DECODETYPE emDecodeType);
	int DoGoldenDecode(std::string &str, POINT &ptGoldLeftTop, POINT &ptGoldRightTop, BOOL &bVerticalBarcode);
	void SetSampleDecodeData(IMAGE *img);
	int DoDecode(std::string &str, POINT &ptLeftTop, POINT &ptRightTop, BOOL &bVerticalBarcode);

	POINT GetAnchor(void){ return m_ptAnchor; }
	void GetAnchorShift(int &XShift, int &YShift){ XShift=m_XShift; YShift=m_YShift; }
	void GetAnchorCorr(unsigned int &uCorrelation){ uCorrelation=m_uCorrelation; }

protected:
	static CCriticalSection g_csPtInspLib;
};

// sample code
void testAnchorAlignment(IMAGE *golden_img, IMAGE *sample_img, GEN_INSP_PARAM *genInspParam);
void runTest(void);

void ComputeShiftRotation(int refShiftX, int refShiftY,//GetAnchorShift
						  POINT ptGoldLeftTop, POINT ptGoldRightTop, BOOL bVerticalBarcode,//Input:DoDecode(GoldImg)
						  POINT ptTestLeftTop, POINT ptTestRightTop,//Input:DoDecode(TestImg)
						  int &fndShiftX, int &fndShiftY, float &angle);//output

//void getEQScale(unsigned char *src, const int width, const int height, const int channels, int enhanceLevel, unsigned char *dst);
void EQEnhanceImage(unsigned char *src, const int width, const int height, const int channels, int enhanceLevel, RECT roi);

inline int capInt(int value, int min, int max){
	return value < min ? min : value > max ? max : value;
}

#endif