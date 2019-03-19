#include "stdafx.h"
#include "AnchorAlignment.h"
#include <vector>
//#include <dlib/all/ImageRegistration.h>
#include <math.h>

#ifdef ENABLE_DECODE
#include "DecoderUtil\BarDecoder.h"
#include "DecoderUtil\QRDecoder.h"
#include "DecoderUtil\DMDecoder.h"
#include "DecoderMemPool.h"
#endif
//#define FIND_ANCHOR_AUTO

//#define DISPLAY_INSP_TIME
#ifdef DISPLAY_INSP_TIME
#include "cudaErrorHandler.h"
#endif

//#define DEBUG_SELF_TEST
#ifdef DEBUG_SELF_TEST
#include "AoiBaseOp.h"
#endif

//#include <fstream>

CCriticalSection AnchorAlignment::g_csPtInspLib;

AnchorAlignment::AnchorAlignment(void)
{
	m_pGoldenImg = NULL;
	m_pSampleImg = NULL;
	m_pAnchorGoldenImg = NULL;
	m_pAnchorSampleImg = NULL;

	m_pGlodenPlane = NULL;
	m_pSamplePlane = NULL;
	m_pAnchorPlane = NULL;

	m_sampleFFTData = NULL;
	m_anchorFFTData = NULL;
	m_convolutionFFTResult = NULL;
	m_convolutionResult = NULL;

	m_plan_sample_r2c_r = NULL;
	m_plan_sample_r2c_g = NULL;
	m_plan_sample_r2c_b = NULL;
	m_plan_anchor_r2c_r = NULL;
	m_plan_anchor_r2c_g = NULL;
	m_plan_anchor_r2c_b = NULL;
	m_plan_c2r_r = NULL;
	m_plan_c2r_g = NULL;
	m_plan_c2r_b = NULL;

	m_bInitImageData = FALSE;
	m_bInitFFTData = FALSE;

	m_iW = 0;
	m_iH = 0;
	m_iColor = 0;
	m_iAnchorSizeX = 64;
	m_iAnchorSizeY = 64;
	m_iSearchRangeX = 256;
	m_iSearchRangeY = 256;

	m_ptAnchor.x = 0;
	m_ptAnchor.y = 0;

	m_XShift = 0;
	m_YShift = 0;

	memset(&m_xInspParam, 0, sizeof(m_xInspParam));

	m_uCorrelation = 0;
	m_corrCalc = new Correlation();

	// --- decode parameters
	m_bInitDecodeData = FALSE;
	m_bVerticalBarcode = FALSE;
	m_emDecodeType = DECODETYPE_QR;
	memset(&m_rtDecodeROI, 0, sizeof(RECT));
	m_iEnhanceLevel = 0;

	m_pGoldDecodeImg = NULL;
	m_pDecodeImg = NULL;
	m_pDecodeTransImg = NULL;

}

AnchorAlignment::~AnchorAlignment()
{
	DestroyImageData();
	DestroyFFTData();
	if(m_corrCalc){
		delete m_corrCalc;
		m_corrCalc = NULL;
	}
	
	if(m_pGoldDecodeImg){
		delete [] m_pGoldDecodeImg;
		m_pGoldDecodeImg = NULL;
	}

	if(m_pDecodeImg){
		delete [] m_pDecodeImg;
		m_pDecodeImg = NULL;
	}

	if(m_pDecodeTransImg){
		delete [] m_pDecodeTransImg;
		m_pDecodeTransImg = NULL;
	}
}
BOOL AnchorAlignment::findAnchor(unsigned char *ptr, int w, int h, int anchorSize, RECT &rtBest)
{
	int stepSize = anchorSize / 4;
	int x_block_size = ((w - anchorSize) + stepSize - 1) / stepSize + 1;
	int y_block_size = ((h - anchorSize) + stepSize - 1) / stepSize + 1;

	BOOL success = FALSE;
	rtBest = { 0 };
	RECT rt = { 0 };
	float energy = 0.0f;
	for (int j = 0; j<y_block_size; j++){
		rt.top = j*stepSize;
		rt.bottom = rt.top + anchorSize;
		if (rt.bottom >= h || rt.top >= h){
			rt.bottom = h - 1;
			rt.top = rt.bottom - anchorSize;
		}
		for (int i = 0; i<x_block_size; i++){
			rt.left = i*stepSize;
			rt.right = rt.left + anchorSize;
			if (rt.left >= w || rt.right >= w){
				rt.right = w - 1;
				rt.left = rt.right - anchorSize;
			}
			float G_x = 0.0f;
			float G_y = 0.0f;
			computeEnergy(ptr, w, h, rt, G_x, G_y);

			float x_energy = abs(G_x);
			float y_energy = abs(G_y);
			float total = x_energy + y_energy;
			if ((total > energy)
				&& (total / (anchorSize*anchorSize) > 0.01f)
				&& (x_energy / total)>0.3f && (y_energy / total)>0.3f){
				energy = total;
				if(rt.left==0){
					rt.left = 1;
					rt.right = rt.left + anchorSize;
				}
				if(rt.top==0){
					rt.top = 1;
					rt.bottom = rt.top + anchorSize;
				}
				if(rt.right==w-1){
					rt.right = w-2;
					rt.left = rt.right - anchorSize;
				}
				if(rt.bottom==h-1){
					rt.bottom = h-2;
					rt.top = rt.bottom - anchorSize;
				}
				rtBest = rt;
				success = TRUE;
			}
		}
	}

	return success;
}
void AnchorAlignment::computeEnergy(unsigned char *ptr, int w, int h, RECT rt, float &G_x, float &G_y)
{
	float M_x[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
	float M_y[9] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };

	float normalize = 1.0f / 255.0f;

	G_x = 0.0f;
	G_y = 0.0f;
	for (int j = rt.top + 1; j<rt.bottom - 1; j++){
		for (int i = rt.left + 1; i<rt.right - 1; i++){
			for (int jj = 0; jj<3; jj++){
				for (int ii = 0; ii<3; ii++){
					G_x += ptr[(j + jj - 1)*w + i + ii - 1] * normalize*M_x[jj * 3 + ii];
					G_y += ptr[(j + jj - 1)*w + i + ii - 1] * normalize*M_y[jj * 3 + ii];
				}
			}
		}
	}
}

#ifdef ENABLE_DECODE
void AnchorAlignment::DestroyInstance() //20160831
{
	CDecoderMemPool::DestroyInstance(); //20160831
}
#endif

void AnchorAlignment::SetSearchRange(int iSearchRangeX, int iSearchRangeY)
{
	m_iSearchRangeX = iSearchRangeX;
	m_iSearchRangeY = iSearchRangeY;
}

void AnchorAlignment::SetInspParam(GEN_INSP_PARAM *xInspParam)
{
	m_xInspParam.rcSearchRange = xInspParam->rcSearchRange;
	m_xInspParam.uCorrelation = xInspParam->uCorrelation;

	m_iAnchorSizeX = m_xInspParam.rcSearchRange.right-m_xInspParam.rcSearchRange.left;
	m_iAnchorSizeY = m_xInspParam.rcSearchRange.bottom-m_xInspParam.rcSearchRange.top;

	if(m_iAnchorSizeX>=256 || m_iAnchorSizeY>=256){
		if(m_iAnchorSizeX>=256){
			m_iAnchorSizeX = 256;
		}
		if(m_iAnchorSizeY>=256){
			m_iAnchorSizeY = 256;
		}
		xInspParam->iSearchSize = xInspParam->iSearchSize<=0 ? 512 : xInspParam->iSearchSize;
	}
	else{
		xInspParam->iSearchSize = xInspParam->iSearchSize<=0 ? 256 : xInspParam->iSearchSize;
	}
	SetSearchRange(xInspParam->iSearchSize, xInspParam->iSearchSize);
	CreateFFTData();

#ifdef FIND_ANCHOR_AUTO//find anchor automatically
	ProcessAnchorFromGolden();
#else
	POINT ptAnchor;
	ptAnchor.x = (m_xInspParam.rcSearchRange.left+m_xInspParam.rcSearchRange.right)/2;
	ptAnchor.y = (m_xInspParam.rcSearchRange.top+m_xInspParam.rcSearchRange.bottom)/2;
	SetAnchor(ptAnchor);
#endif
}

BOOL AnchorAlignment::SetAnchor(POINT pt)
{
	m_ptAnchor.x = pt.x;
	m_ptAnchor.y = pt.y;

	return SetAnchorData();
}

BOOL AnchorAlignment::SetGoldenImg(IMAGE *img)//CreateFFTData(iColor)
{

	m_iW = img->data_w;
	m_iH = img->data_h;	
	m_iColor = GetImagePixelBytes(img);
	CreateImageData();//m_pGoldenImg & m_pSampleImg
	CreateFFTData();
	if(m_iW>0 && m_iH>0 && m_iColor>0){
		memcpy(m_pGoldenImg, img->ptr, m_iW*m_iH*m_iColor);
	}
//	saveimage(L"golden.bmp", img->ptr, img->data_w, -img->data_h, m_iColor, FALSE);
#ifdef FIND_ANCHOR_AUTO
	return ProcessAnchorFromGolden();
#else
	return SetAnchorData();
#endif
}

static void cropImage(unsigned char *src, int w, int h, int color, RECT roi, unsigned char *dstImg)
{
	int roiW = roi.right-roi.left;
	int roiH = roi.bottom-roi.top;
	memset(dstImg, 0, roiW*roiH*color);

	for(int j=0; j<roiH; j++){
		int y = j+roi.top;
		if(y>=0 && y<h){
			for(int i=0; i<roiW; i++){
				int x = i+roi.left;
				if(x>=0 && x<w){
					dstImg[(j*roiW+i)*color] = src[(y*w+x)*color];
					if(color==3){
						dstImg[(j*roiW+i)*color+1] = src[(y*w+x)*color+1];
						dstImg[(j*roiW+i)*color+2] = src[(y*w+x)*color+2];
					}
				}
			}
		}
	}
}

#ifdef FIND_ANCHOR_AUTO//find anchor automatically
BOOL AnchorAlignment::ProcessAnchorFromImage(IMAGE *img)
{
	BOOL bFound = FALSE;
	if(img->ptr==NULL || img->data_w<=0 || img->data_h<=0){ return bFound; }

	int roiW = m_xInspParam.rcSearchRange.right-m_xInspParam.rcSearchRange.left;
	int roiH = m_xInspParam.rcSearchRange.bottom-m_xInspParam.rcSearchRange.top;
	unsigned char *shrinkImg = new unsigned char[roiW*roiH*m_iColor];

	cropImage(img->ptr, img->data_w, img->data_h, GetImagePixelBytes(img), m_xInspParam.rcSearchRange, shrinkImg);

//	saveimage(L"debug.bmp", shrinkImg, roiW, -roiH, m_iColor, FALSE);

	std::vector<POINT> surfPos;
	dlib_surf(shrinkImg, roiW, roiH, m_iColor, &surfPos);
	if(surfPos.size()>0){
		surfPos[0].x += m_xInspParam.rcSearchRange.left;
		surfPos[0].y += m_xInspParam.rcSearchRange.top;
		SetAnchor(surfPos[0]);//(422,285)
		bFound = TRUE;
	}

	delete [] shrinkImg;

	return bFound;
}

BOOL AnchorAlignment::ProcessAnchorFromGolden(void)
{
	IMAGE img;
	ImageInit(&img, m_iColor==3?IMAGE_TYPE_RGB24:IMAGE_TYPE_MONO8, m_pGoldenImg, m_iW, m_iH);
	return ProcessAnchorFromImage(&img);
}
#endif

BOOL AnchorAlignment::SetAnchorData(void)
{
	if(!m_bInitImageData || !m_bInitFFTData){ return FALSE; }

	int x = m_ptAnchor.x-m_iAnchorSizeX/2;
	int y = m_ptAnchor.y-m_iAnchorSizeY/2;

	// crop AnchorGolden
	RECT roi;
	roi.left = x;
	roi.top = y;
	roi.right = roi.left+m_iAnchorSizeX;
	roi.bottom = roi.top+m_iAnchorSizeY;

	memset(m_pAnchorGoldenImg, 0, m_iAnchorSizeX*m_iAnchorSizeY*m_iColor);
	cropImage(m_pGoldenImg, m_iW, m_iH, m_iColor, roi, m_pAnchorGoldenImg);

//	saveimage(L"D:\\AOI\\DEBUG\\AnchorGoldenImg.bmp", m_pAnchorGoldenImg, m_iAnchorSizeX, -m_iAnchorSizeY, m_iColor, FALSE);

	IMAGE img;
	ImageInit(&img, m_iColor==3?IMAGE_TYPE_RGB24:IMAGE_TYPE_MONO8, m_pAnchorGoldenImg, m_iAnchorSizeX, m_iAnchorSizeY);
	m_corrCalc->SetGoldenImg(&img);

	int imgOffset = m_iSearchRangeX*m_iSearchRangeY;
	memset(m_pAnchorPlane, 0, imgOffset*m_iColor*sizeof(float));
	for(int j=0; j<m_iAnchorSizeY; j++){
		for(int i=0; i<m_iAnchorSizeX; i++){
			m_pAnchorPlane[j*m_iSearchRangeX+i+imgOffset*0] = m_pAnchorGoldenImg[(j*m_iAnchorSizeX+i)*m_iColor+0]/255.0f;
			if(m_iColor==3){
				m_pAnchorPlane[j*m_iSearchRangeX+i+imgOffset*1] = m_pAnchorGoldenImg[(j*m_iAnchorSizeX+i)*m_iColor+1]/255.0f;
				m_pAnchorPlane[j*m_iSearchRangeX+i+imgOffset*2] = m_pAnchorGoldenImg[(j*m_iAnchorSizeX+i)*m_iColor+2]/255.0f;
			}
		}
	}

	return TRUE;
}

void AnchorAlignment::SetSampleData(IMAGE *img)
{
	if(img==NULL){ return; }
	if(img->ptr==NULL){ return; }
	if(!m_bInitImageData || !m_bInitFFTData){ return; }
	if(m_iW!=img->data_w || m_iH!=img->data_h || m_iColor!=GetImagePixelBytes(img)){ return; }

//	saveimage(L"D:\\AOI\\DEBUG\\sample.bmp", img->ptr, img->data_w, -img->data_h, m_iColor, FALSE);
	memcpy(m_pSampleImg, img->ptr, m_iW*m_iH*m_iColor);

	int x = m_ptAnchor.x-m_iSearchRangeX/2;//m_xInspParam.rcSearchRange.left;
	int y = m_ptAnchor.y-m_iSearchRangeY/2;//m_xInspParam.rcSearchRange.top;

//	unsigned char *debug = new unsigned char[m_iSearchRangeX*m_iSearchRangeY];

	int imgOffset = m_iSearchRangeX*m_iSearchRangeY;
	memset(m_pSamplePlane, 0, imgOffset*m_iColor*sizeof(float));
	for(int j=0; j<m_iSearchRangeY; j++){
		for(int i=0; i<m_iSearchRangeX; i++){
			if(i+x>=0 && i+x<m_iW && j+y>=0 && j+y<m_iH){
				m_pSamplePlane[j*m_iSearchRangeX+i+imgOffset*0] = m_pSampleImg[((j+y)*m_iW+i+x)*m_iColor+0]/255.0f;
			//	debug[j*m_iSearchRangeX+i] = m_pSampleImg[((j+y)*m_iW+i+x)*m_iColor+0];
				if(m_iColor==3){
					m_pSamplePlane[j*m_iSearchRangeX+i+imgOffset*1] = m_pSampleImg[((j+y)*m_iW+i+x)*m_iColor+1]/255.0f;
					m_pSamplePlane[j*m_iSearchRangeX+i+imgOffset*2] = m_pSampleImg[((j+y)*m_iW+i+x)*m_iColor+2]/255.0f;
				}
			}
		}
	}

//	saveimage(L"D:\\AOI\\DEBUG\\sample.bmp", debug, m_iSearchRangeX, -m_iSearchRangeY, 1, FALSE);
//	delete [] debug;
}

void AnchorAlignment::CreateImageData(void)
{
	//m_iColor
	//m_iW
	//m_iH

	DestroyImageData();
	if(m_iColor<=0 || m_iW<=0 || m_iH<=0){ return; }

	m_pGoldenImg = new unsigned char[m_iW*m_iH*m_iColor];
	memset(m_pGoldenImg, 0, m_iW*m_iH*m_iColor);
	m_pSampleImg = new unsigned char[m_iW*m_iH*m_iColor];
	memset(m_pSampleImg, 0, m_iW*m_iH*m_iColor);

	m_bInitImageData = TRUE;
}

void AnchorAlignment::DestroyImageData(void)
{
	if(m_pGoldenImg){
		delete [] m_pGoldenImg;
		m_pGoldenImg = NULL;
	}
	
	if(m_pSampleImg){
		delete [] m_pSampleImg;
		m_pSampleImg = NULL;
	}

	m_bInitImageData = FALSE;
}

void AnchorAlignment::CreateFFTData(void)
{
	//m_iAnchorSizeX
	//m_iAnchorSizeY
	//m_iColor
	//m_iSearchRangeX
	//m_iSearchRangeY

	DestroyFFTData();
	if(m_iColor<=0 || m_iSearchRangeX<=0 || m_iSearchRangeY<=0 || m_iAnchorSizeX<=0 || m_iAnchorSizeY<=0){ return; }

//	fftwf_init_threads();
//	fftwf_plan_with_nthreads(2);
	
	int anchorSize = m_iAnchorSizeX*m_iAnchorSizeY;
	m_pAnchorGoldenImg = new unsigned char[anchorSize*m_iColor];
	memset(m_pAnchorGoldenImg, 0, anchorSize*m_iColor);
	m_pAnchorSampleImg = new unsigned char[anchorSize*m_iColor];
	memset(m_pAnchorSampleImg, 0, anchorSize*m_iColor);

	m_pSamplePlane = new float[m_iSearchRangeX*m_iSearchRangeY*m_iColor];
	memset(m_pSamplePlane, 0, m_iSearchRangeX*m_iSearchRangeY*m_iColor);
	m_pAnchorPlane = new float[m_iSearchRangeX*m_iSearchRangeY*m_iColor];
	memset(m_pAnchorPlane, 0, m_iSearchRangeX*m_iSearchRangeY*m_iColor);

	m_sampleFFTData = (fftwf_complex*)fftwf_malloc(m_iSearchRangeX*(m_iSearchRangeY/2+1)*m_iColor*sizeof(fftwf_complex));
	m_anchorFFTData = (fftwf_complex*)fftwf_malloc(m_iSearchRangeX*(m_iSearchRangeY/2+1)*m_iColor*sizeof(fftwf_complex));

	m_convolutionFFTResult = (fftwf_complex*)fftwf_malloc(m_iSearchRangeX*(m_iSearchRangeY/2+1)*m_iColor*sizeof(fftwf_complex));
	m_convolutionResult = new float[m_iSearchRangeX*m_iSearchRangeY*m_iColor];

	int imgOffset = m_iSearchRangeX*m_iSearchRangeY;
	int fftOffset = m_iSearchRangeX*(m_iSearchRangeY/2+1);
	

	g_csPtInspLib.Lock();

	m_plan_sample_r2c_r = fftwf_plan_dft_r2c_2d(m_iSearchRangeX, m_iSearchRangeY, m_pSamplePlane+imgOffset*0, m_sampleFFTData+fftOffset*0, FFTW_ESTIMATE);
	if(m_iColor==3){
		m_plan_sample_r2c_g = fftwf_plan_dft_r2c_2d(m_iSearchRangeX, m_iSearchRangeY, m_pSamplePlane+imgOffset*1, m_sampleFFTData+fftOffset*1, FFTW_ESTIMATE);
		m_plan_sample_r2c_b = fftwf_plan_dft_r2c_2d(m_iSearchRangeX, m_iSearchRangeY, m_pSamplePlane+imgOffset*2, m_sampleFFTData+fftOffset*2, FFTW_ESTIMATE);
	}

	m_plan_anchor_r2c_r = fftwf_plan_dft_r2c_2d(m_iSearchRangeX, m_iSearchRangeY, m_pAnchorPlane+imgOffset*0, m_anchorFFTData+fftOffset*0, FFTW_ESTIMATE);
	if(m_iColor==3){
		m_plan_anchor_r2c_g = fftwf_plan_dft_r2c_2d(m_iSearchRangeX, m_iSearchRangeY, m_pAnchorPlane+imgOffset*1, m_anchorFFTData+fftOffset*1, FFTW_ESTIMATE);
		m_plan_anchor_r2c_b = fftwf_plan_dft_r2c_2d(m_iSearchRangeX, m_iSearchRangeY, m_pAnchorPlane+imgOffset*2, m_anchorFFTData+fftOffset*2, FFTW_ESTIMATE);
	}

	m_plan_c2r_r = fftwf_plan_dft_c2r_2d(m_iSearchRangeX, m_iSearchRangeY, m_convolutionFFTResult+fftOffset*0, m_convolutionResult+imgOffset*0, FFTW_ESTIMATE);
	if(m_iColor==3){
		m_plan_c2r_g = fftwf_plan_dft_c2r_2d(m_iSearchRangeX, m_iSearchRangeY, m_convolutionFFTResult+fftOffset*1, m_convolutionResult+imgOffset*1, FFTW_ESTIMATE);
		m_plan_c2r_b = fftwf_plan_dft_c2r_2d(m_iSearchRangeX, m_iSearchRangeY, m_convolutionFFTResult+fftOffset*2, m_convolutionResult+imgOffset*2, FFTW_ESTIMATE);
	}

	g_csPtInspLib.Unlock();

	m_bInitFFTData = TRUE;
}

void AnchorAlignment::DestroyFFTData(void)
{
	if(m_pAnchorGoldenImg){
		delete [] m_pAnchorGoldenImg;
		m_pAnchorGoldenImg = NULL;
	}

	if(m_pAnchorSampleImg){
		delete [] m_pAnchorSampleImg;
		m_pAnchorSampleImg = NULL;
	}

	if(m_pGlodenPlane){
		delete [] m_pGlodenPlane;
		m_pGlodenPlane = NULL;
	}

	if(m_pSamplePlane){
		delete [] m_pSamplePlane;
		m_pSamplePlane = NULL;
	}

	if(m_pAnchorPlane){
		delete [] m_pAnchorPlane;
		m_pAnchorPlane = NULL;
	}

	if(m_sampleFFTData){
		fftwf_free(m_sampleFFTData);
		m_sampleFFTData = NULL;
	}

	if(m_anchorFFTData){
		fftwf_free(m_anchorFFTData);
		m_anchorFFTData = NULL;
	}

	if(m_convolutionFFTResult){
		fftwf_free(m_convolutionFFTResult);
		m_convolutionFFTResult = NULL;
	}

	if(m_convolutionResult){
		delete [] m_convolutionResult;
		m_convolutionResult = NULL;
	}

	if(m_plan_sample_r2c_r){
		fftwf_destroy_plan(m_plan_sample_r2c_r);
		m_plan_sample_r2c_r = NULL;
	}

	if(m_plan_sample_r2c_g){
		fftwf_destroy_plan(m_plan_sample_r2c_g);
		m_plan_sample_r2c_g = NULL;
	}

	if(m_plan_sample_r2c_b){
		fftwf_destroy_plan(m_plan_sample_r2c_b);
		m_plan_sample_r2c_b = NULL;
	}

	if(m_plan_anchor_r2c_r){
		fftwf_destroy_plan(m_plan_anchor_r2c_r);
		m_plan_anchor_r2c_r = NULL;
	}

	if(m_plan_anchor_r2c_g){
		fftwf_destroy_plan(m_plan_anchor_r2c_g);
		m_plan_anchor_r2c_g = NULL;
	}

	if(m_plan_anchor_r2c_b){
		fftwf_destroy_plan(m_plan_anchor_r2c_b);
		m_plan_anchor_r2c_b = NULL;
	}

	if(m_plan_c2r_r){
		fftwf_destroy_plan(m_plan_c2r_r);
		m_plan_c2r_r = NULL;
	}

	if(m_plan_c2r_g){
		fftwf_destroy_plan(m_plan_c2r_g);
		m_plan_c2r_g = NULL;
	}

	if(m_plan_c2r_b){
		fftwf_destroy_plan(m_plan_c2r_b);
		m_plan_c2r_b = NULL;
	}

//	fftwf_cleanup_threads();
	m_bInitFFTData = FALSE;
}

void Convolution(fftwf_complex *sampleFFTData, fftwf_complex *anchorFFTData, fftwf_complex *convolutionResult, int w, int h, int color)
{
	fftwf_complex *pSample = sampleFFTData;
	fftwf_complex *pAnchor = anchorFFTData;
	fftwf_complex *pResult = convolutionResult;

	int size = w*(h/2+1);
	float tol = 1e-06f;
	float res1, res2, tmp;
	for(int c=0; c<color; c++){
		for(int i=0; i<size; i++){
			res1 = pSample[i][0]*pAnchor[i][0] + pSample[i][1]*pAnchor[i][1];
			res2 = pSample[i][1]*pAnchor[i][0] - pSample[i][0]*pAnchor[i][1];
			tmp = sqrtf(res1*res1+res2*res2);
			if(tmp>tol){
				pResult[i][0] = res1/tmp;
				pResult[i][1] = res2/tmp;
			}
			else{
				pResult[i][0] = 0.0f;
				pResult[i][1] = 0.0f;
			}
		}
		pSample+=size;
		pAnchor+=size;
		pResult+=size;
	}
}

void AnchorAlignment::FindXYShift(void)
{
	if(m_convolutionResult==NULL || m_pSampleImg==NULL){ return; }

	int left = (m_ptAnchor.x-m_iSearchRangeX/2);
	int top = (m_ptAnchor.y-m_iSearchRangeY/2);
	int right = (m_ptAnchor.x+m_iSearchRangeX/2-1)-m_iW;
	int bottom = (m_ptAnchor.y+m_iSearchRangeY/2-1)-m_iH;

	RECT validROI;
	validROI.left = left < 0 ? abs(left) : 0;
	validROI.top = top < 0 ? abs(top) : 0;
	validROI.right = right >= 0 ? m_iSearchRangeX-right : m_iSearchRangeX;
	validROI.bottom = bottom >= 0 ? m_iSearchRangeY-bottom : m_iSearchRangeY;

	int imgOffset = m_iSearchRangeX*m_iSearchRangeY;
	float peakval = 0.0f;
	int x = 0;
	int y = 0;
	for(int j=validROI.top+1; j<validROI.bottom-1; j++){
		for(int i=validROI.left+1; i<validROI.right-1; i++){
			float val = m_convolutionResult[j*m_iSearchRangeX+i];
			if(m_iColor==3){
				val += m_convolutionResult[j*m_iSearchRangeX+i+imgOffset*1];
				val += m_convolutionResult[j*m_iSearchRangeX+i+imgOffset*2];
			}
			if(val>peakval){
				peakval = val;
				x = i;
				y = j;
			}
		}
	}

	m_XShift = (x + m_iAnchorSizeX/2) - m_iSearchRangeX/2;
	m_YShift = (y + m_iAnchorSizeY/2) - m_iSearchRangeY/2;

	if(m_XShift+m_ptAnchor.x<0){
		m_XShift -= (m_XShift+m_ptAnchor.x);
	}
	if(m_XShift+m_ptAnchor.x>m_iW){
		m_XShift -= (m_XShift+m_ptAnchor.x-m_iW);
	}

	if(m_YShift+m_ptAnchor.y<0){
		m_YShift -= (m_YShift+m_ptAnchor.y);
	}
	if(m_YShift+m_ptAnchor.y>m_iH){
		m_YShift -= (m_YShift+m_ptAnchor.y-m_iH);
	}

	// crop AnchorSample
	RECT roi;
	roi.left = m_ptAnchor.x+m_XShift-m_iAnchorSizeX/2;
	roi.top = m_ptAnchor.y+m_YShift-m_iAnchorSizeY/2;
	roi.right = roi.left+m_iAnchorSizeX;
	roi.bottom = roi.top+m_iAnchorSizeY;

	memset(m_pAnchorSampleImg, 0, m_iAnchorSizeX*m_iAnchorSizeY*m_iColor);
	cropImage(m_pSampleImg, m_iW, m_iH, m_iColor, roi, m_pAnchorSampleImg);

//	saveimage(L"D:\\AOI\\DEBUG\\AnchorSampleImg.bmp", m_pAnchorSampleImg, m_iAnchorSizeX, -m_iAnchorSizeY, m_iColor, FALSE);

	IMAGE img;
	ImageInit(&img, m_iColor==3?IMAGE_TYPE_RGB24:IMAGE_TYPE_MONO8, m_pAnchorSampleImg, m_iAnchorSizeX, m_iAnchorSizeY);
	m_corrCalc->SetSampleImg(&img);
}

BOOL AnchorAlignment::DoAnchorAlignMent(void)
{
	if(!m_bInitImageData || !m_bInitFFTData){ return FALSE; }

	// FFT forward
	fftwf_execute(m_plan_sample_r2c_r);
	if(m_iColor==3){
		fftwf_execute(m_plan_sample_r2c_g);
		fftwf_execute(m_plan_sample_r2c_b);
	}

	fftwf_execute(m_plan_anchor_r2c_r);
	if(m_iColor==3){
		fftwf_execute(m_plan_anchor_r2c_g);
		fftwf_execute(m_plan_anchor_r2c_b);
	}

	// Convolution
	Convolution(m_sampleFFTData, m_anchorFFTData, m_convolutionFFTResult, m_iSearchRangeX, m_iSearchRangeY, m_iColor);

	// FFT backward
	fftwf_execute(m_plan_c2r_r);
	if(m_iColor==3){
		fftwf_execute(m_plan_c2r_g);
		fftwf_execute(m_plan_c2r_b);
	}

	// find peak position
	FindXYShift();

	// calculate correlation
	float corr[3];
	m_corrCalc->CalculateCorrelation();
	m_corrCalc->GetCorrelation(corr);
	if(m_iColor==3){
		m_uCorrelation = (unsigned int)(((corr[0]+corr[1]+corr[2])/3.0f)*100.0f);
	}
	else{
		m_uCorrelation = (unsigned int)(corr[0]*100.0f);
	}

	return TRUE;
}

void extractImageChannel(unsigned char *src, int w, int h, int color, RECT roi, unsigned char *dstImg)
{
	int roiW = roi.right-roi.left;
	int roiH = roi.bottom-roi.top;
	memset(dstImg, 0, roiW*roiH);

	if(color==3){
		for(int j=0; j<roiH; j++){
			int y = j+roi.top;
			if(y>=0 && y<h){
				for(int i=0; i<roiW; i++){
					int x = i+roi.left;
					if(x>=0 && x<w){
						//get G Channel
						dstImg[j*roiW+i] = src[(y*w+x)*3+1];
					}
				}
			}
		}
	}
	else{
		for(int j=0; j<roiH; j++){
			int y = j+roi.top;
			if(y>=0 && y<h){
				for(int i=0; i<roiW; i++){
					int x = i+roi.left;
					if(x>=0 && x<w){
						dstImg[j*roiW+i] = src[y*w+x];
					}
				}
			}
		}
	}
}

void extractImage(unsigned char *src, int w, int h, int color, RECT roi, unsigned char *dstImg)
{
	int roiW = roi.right-roi.left;
	int roiH = roi.bottom-roi.top;
	memset(dstImg, 0, roiW*roiH*color);

	if(color==3){
		for(int j=0; j<roiH; j++){
			int y = j+roi.top;
			if(y>=0 && y<h){
				for(int i=0; i<roiW; i++){
					int x = i+roi.left;
					if(x>=0 && x<w){
						dstImg[(j*roiW+i)*3+0] = src[(y*w+x)*3+0];
						dstImg[(j*roiW+i)*3+1] = src[(y*w+x)*3+1];
						dstImg[(j*roiW+i)*3+2] = src[(y*w+x)*3+2];
					}
				}
			}
		}
	}
	else{
		for(int j=0; j<roiH; j++){
			int y = j+roi.top;
			if(y>=0 && y<h){
				for(int i=0; i<roiW; i++){
					int x = i+roi.left;
					if(x>=0 && x<w){
						dstImg[j*roiW+i] = src[y*w+x];
					}
				}
			}
		}
	}
}

void AnchorAlignment::SetDecodeROI(RECT roi)
{
//	roi.left = 154;
//	roi.top = 162;
//	roi.right = 154+698;
//	roi.bottom = 162+1010;
	m_rtDecodeROI = roi;
}

void AnchorAlignment::SetDecodeParam(GEN_INSP_PARAM *param)
{
	m_iEnhanceLevel = param->iEnhanceLevel;
	m_rtDecodeROI = param->rcDecodeRange;
}

void AnchorAlignment::SetGoldenDecodeData(IMAGE *img, EM_DECODETYPE emDecodeType)
{
	m_emDecodeType = emDecodeType;
	m_bVerticalBarcode = FALSE;
	if(!m_bInitDecodeData || (m_iW!=img->data_w || m_iH!=img->data_h) || m_iColor!=GetImagePixelBytes(img)){
		m_bInitDecodeData = TRUE;
		m_iW = img->data_w;
		m_iH = img->data_h;
		m_iColor = GetImagePixelBytes(img);
		
		if(m_pGoldDecodeImg){
			delete [] m_pGoldDecodeImg;
			m_pGoldDecodeImg = NULL;
		}
		m_pGoldDecodeImg = new unsigned char[m_iW*m_iH*m_iColor];

		if(m_pDecodeImg){
			delete [] m_pDecodeImg;
			m_pDecodeImg = NULL;
		}
		m_pDecodeImg = new unsigned char[m_iW*m_iH];

		if(m_pDecodeTransImg){
			delete [] m_pDecodeTransImg;
			m_pDecodeTransImg = NULL;
		}
		m_pDecodeTransImg = new unsigned char[m_iW*m_iH];
	}

	int roiW = m_rtDecodeROI.right-m_rtDecodeROI.left;
	int roiH = m_rtDecodeROI.bottom-m_rtDecodeROI.top;
	if(roiW<=0 || roiH<=0 || roiW>img->data_w || roiH>img->data_h){
		m_rtDecodeROI.left = 0;
		m_rtDecodeROI.right = img->data_w;
		m_rtDecodeROI.top = 0;
		m_rtDecodeROI.bottom = img->data_h;
	}
	
	memcpy(m_pGoldDecodeImg, img->ptr, m_iW*m_iH*m_iColor);
	EQEnhanceImage(m_pGoldDecodeImg, m_iW, m_iH, m_iColor, m_iEnhanceLevel, m_rtDecodeROI);
	// --- extract image channel
	extractImageChannel(img->ptr, m_iW, m_iH, GetImagePixelBytes(img), m_rtDecodeROI, m_pDecodeImg);
}

void AnchorAlignment::SetSampleDecodeData(IMAGE *img)
{
	if(!m_bInitDecodeData){ return; };

	//LARGE_INTEGER PerfFreq;
	//LARGE_INTEGER PerfPart1;
	//LARGE_INTEGER PerfPart2;
	//LARGE_INTEGER PerfPart3;

	//QueryPerformanceFrequency(&PerfFreq);
	//QueryPerformanceCounter(&PerfPart1);

	EQEnhanceImage(img->ptr, img->data_w, img->data_h, GetImagePixelBytes(img), m_iEnhanceLevel, m_rtDecodeROI);

//	QueryPerformanceCounter(&PerfPart2);
	// --- extract image channel
	extractImageChannel(img->ptr, img->data_w, img->data_h, GetImagePixelBytes(img), m_rtDecodeROI, m_pDecodeImg);

//	QueryPerformanceCounter(&PerfPart3);

//	saveimage(L"D:\\AOI\\DEBUG\\m_pEnhanceImg1.bmp", m_pEnhanceImg, m_rtDecodeROI.right-m_rtDecodeROI.left, -(m_rtDecodeROI.bottom-m_rtDecodeROI.top), m_iColor, FALSE);

	//fstream fp;
	//fp.open("PerformanceTest.txt", ios::out|ios::app);//¶}±ÒÀÉ®×
	//if(fp){
	//	char	str[1024];

	//	sprintf(str, "EQEnhanceImage:");
	//	fp<<str<<endl;//¼g¤J¦r¦ê
	//	sprintf(str, " %lld us\n", (PerfPart2.QuadPart-PerfPart1.QuadPart)*1000000/PerfFreq.QuadPart);
	//	fp<<str<<endl;//¼g¤J¦r¦ê

	//	sprintf(str, "extractImageChannel:");
	//	fp<<str<<endl;//¼g¤J¦r¦ê
	//	sprintf(str, " %lld us\n", (PerfPart3.QuadPart-PerfPart2.QuadPart)*1000000/PerfFreq.QuadPart);
	//	fp<<str<<endl;//¼g¤J¦r¦ê
	//}
	//fp.close();//Ãö³¬ÀÉ®×
}

void AnchorAlignment::DataTranspose(void)
{
	if(!m_bInitDecodeData){ return; };

	int w = m_rtDecodeROI.right-m_rtDecodeROI.left;
	int h = m_rtDecodeROI.bottom-m_rtDecodeROI.top;

	for(int j=0; j<w; j++){
		for(int i=0; i<h; i++){
			m_pDecodeTransImg[j*h+h-i-1] = m_pDecodeImg[i*w+j];
		}
	}
	memcpy(m_pDecodeImg, m_pDecodeTransImg, w*h);
}

#ifdef ENABLE_DECODE
bool AnchorAlignment::Decode_QR(std::string &str, POINT &ptLeftTop, POINT &ptRightTop)
{
	int decodeStatus = DECODE_FAILED;
	CDecoderMemPool* pInst = CDecoderMemPool::GetInstance();
	CBase2DDecoder* pDecoder = pInst->GetDecoder(0, false, 0, 0);
	bool bStatus = pDecoder->SetBinarizeImg(m_pDecodeImg, m_iDecodeW, m_iDecodeH, USE_HYBRID_BINARIZE);
	if(bStatus){
		bool bTryHarder = false;
		bStatus = pDecoder->decode(bTryHarder);

		if(bStatus){
			str = pDecoder->getResultText();
		}
		else{
			str = pDecoder->getErrorText();
		}
	}

	if(bStatus){
		float *finder = pDecoder->getFinder();
		ptLeftTop.x = (int)(finder[0]+0.5f);
		ptLeftTop.y = (int)(finder[1]+0.5f);
		ptRightTop.x = (int)(finder[2]+0.5f);
		ptRightTop.y = (int)(finder[3]+0.5f);
	}

	pInst->ReturnDecoder(pDecoder);

	return bStatus;
}

bool AnchorAlignment::Decode_DataMatrix(std::string &str, POINT &ptLeftTop, POINT &ptRightTop)
{
	int decodeStatus = DECODE_FAILED;
	CDecoderMemPool* pInst = CDecoderMemPool::GetInstance();
	CBase2DDecoder* pDecoder = pInst->GetDecoder(1, false, 0, 0);
	bool bStatus = pDecoder->SetBinarizeImg(m_pDecodeImg, m_iDecodeW, m_iDecodeH, USE_HYBRID_BINARIZE);
	if(bStatus){
		bool bTryHarder = false;
		bStatus = pDecoder->decode(bTryHarder);

		if(bStatus){
			str = pDecoder->getResultText();
		}
		else{
			str = pDecoder->getErrorText();
		}
	}
	
	if(bStatus){
		float *finder = pDecoder->getFinder();
		ptLeftTop.x = (int)(finder[0]+0.5f);
		ptLeftTop.y = (int)(finder[1]+0.5f);
		ptRightTop.x = (int)(finder[2]+0.5f);
		ptRightTop.y = (int)(finder[3]+0.5f);
	}

	pInst->ReturnDecoder(pDecoder);

	return bStatus;
}

int findBarcodeCorner(unsigned char *pBinaryImg, int w, int h, float *pFinder, POINT &ptLeftTop, POINT &ptRightTop)
{
#if 1
	ptLeftTop.x = -1;
	ptLeftTop.y = -1;

	ptRightTop.x = -1;
	ptRightTop.y = -1;

	int CORNER_SEARCH_TERMINATE_LEN = 2;
	int pointTH = 20;

	// --- top-left
	int positionX = (int)(pFinder[0]+0.5f);
	int positionY = (int)(pFinder[1]+0.5f);

	for(int j=positionY; j>CORNER_SEARCH_TERMINATE_LEN-1; j--){

		for(int i=0; i<pointTH; i++){
			int curX = positionX+i-pointTH/2;
			if(curX>=0 && curX<w){
				if(pBinaryImg[j*w+curX]==0){
					ptLeftTop.x = curX; 
					break;
				}
			}
		}

		int count = 0;
		int totalCount = 0;
		for(int c=1; c<=CORNER_SEARCH_TERMINATE_LEN; c++){
			for(int i=0; i<pointTH; i++){
				int curX = ptLeftTop.x+i-pointTH/2;
				if(curX>=0 && curX<w){
					totalCount++;
					if(pBinaryImg[(j-c)*w+curX]==255){
						count++;
					}
				}
			}
		}

		if(count >= (totalCount*4/5)){
			ptLeftTop.y = j;
			break;
		}
	}

	// --- top-right
	positionX = (int)(pFinder[2]+0.5f);
	positionY = (int)(pFinder[3]+0.5f);

	for(int j=positionY; j>CORNER_SEARCH_TERMINATE_LEN-1; j--){

		for(int i=0; i<pointTH; i++){
			int curX = positionX-i+pointTH/2;
			if(curX>=0 && curX<w){
				if(pBinaryImg[j*w+curX]==0){
					ptRightTop.x = curX; 
					break;
				}
			}
		}

		int count = 0;
		int totalCount = 0;
		for(int c=1; c<=CORNER_SEARCH_TERMINATE_LEN; c++){
			for(int i=0; i<pointTH; i++){
				int curX = ptRightTop.x-i+pointTH/2;
				if(curX>=0 && curX<w){
					totalCount++;
					if(pBinaryImg[(j-c)*w+curX]==255){
						count++;
					}
				}
			}
		}

		if(count >= (totalCount*4/5)){
			ptRightTop.y = j;
			break;
		}
	}
#endif
	return 0;
}

bool AnchorAlignment::Decode_Code128(std::string &str, POINT &ptLeftTop, POINT &ptRightTop)
{
	int decodeStatus = DECODE_FAILED;
	CBarDecoder decoder;
	bool bStatus = decoder.SetBinarizeImg(m_pDecodeImg, m_iDecodeW, m_iDecodeH, USE_HYBRID_BINARIZE);
	if(bStatus){
		bStatus = false;
		bool bTryHarder = false;
		decoder.decode(bTryHarder, DecodeLib::BarcodeFormat_CODE_128);
		if(decoder.getStartPatternStatus() && decoder.getStopPatternStatus() && decoder.getCheckSumStatus()){
			bStatus = true;
		}

		if(bStatus){
			str = decoder.getResultText();
		}
		else{
			str = decoder.getErrorText();
		}
	}

	// --- debug
//	unsigned char *debug = new unsigned char[m_iDecodeW*m_iDecodeH];
//	decoder.GetBinarizeImgData(debug);
//	saveimage(L"D:\\AOI\\DEBUG\\decodeBinary.bmp", debug, m_iDecodeW, -m_iDecodeH, 1, FALSE);
//	delete [] debug;
	// ---------

	if(bStatus){
		float *finder = decoder.getFinder();
		if(decoder.getReverseStatus()){
			float temp = finder[0];
			finder[0] = m_iDecodeW - finder[2];
			finder[2] = m_iDecodeW - temp;
		}
		unsigned char *pBinaryData = new unsigned char[m_iDecodeW*m_iDecodeH];

		decoder.GetBinarizeImgData(pBinaryData);
		findBarcodeCorner(pBinaryData, m_iDecodeW, m_iDecodeH, finder, ptLeftTop, ptRightTop);

		delete [] pBinaryData;
	}

	return bStatus;
}

bool AnchorAlignment::Decode_EAN13(std::string &str, POINT &ptLeftTop, POINT &ptRightTop)
{
	int decodeStatus = DECODE_FAILED;
	CBarDecoder decoder;
	bool bStatus = decoder.SetBinarizeImg(m_pDecodeImg, m_iDecodeW, m_iDecodeH, USE_HYBRID_BINARIZE);
	if(bStatus){
		bool bTryHarder = false;
	//	bTryHarder = true;
		bStatus = decoder.decode(bTryHarder, DecodeLib::BarcodeFormat_EAN_13);

		if(bStatus){
			str = decoder.getResultText();
		}
		else{
			str = decoder.getErrorText();
		}
	}

	if(bStatus){
		float *finder = decoder.getFinder();
		if(decoder.getReverseStatus()){
			float temp = finder[0];
			finder[0] = m_iDecodeW - finder[2];
			finder[2] = m_iDecodeW - temp;
		}
		unsigned char *pBinaryData = new unsigned char[m_iDecodeW*m_iDecodeH];

		decoder.GetBinarizeImgData(pBinaryData);
		findBarcodeCorner(pBinaryData, m_iDecodeW, m_iDecodeH, finder, ptLeftTop, ptRightTop);

		delete [] pBinaryData;
	}

	return bStatus;
}

int AnchorAlignment::DoDecode(std::string &str, POINT &ptLeftTop, POINT &ptRightTop, BOOL &bVerticalBarcode)
{
	if(!m_bInitDecodeData){ return DECODE_FAILED; };

#ifdef DISPLAY_INSP_TIME
	CpuTimer t;
	t.Start();
#endif

	int w = m_rtDecodeROI.right-m_rtDecodeROI.left;
	int h = m_rtDecodeROI.bottom-m_rtDecodeROI.top;

	bVerticalBarcode = m_bVerticalBarcode;
	if(m_bVerticalBarcode){
		int tmp = w;
		w = h;
		h = tmp;
		DataTranspose();
	}
	m_iDecodeW = w;
	m_iDecodeH = h;

//	saveimage(L"D:\\AOI\\DEBUG\\m_pEnhanceImg2.bmp", m_pEnhanceImg, w, -h, m_iColor, FALSE);

	int decodeStatus = DECODE_FAILED;
	bool bStatus = true;
	switch(m_emDecodeType){
		case DECODETYPE_QR:
			bStatus = Decode_QR(str, ptLeftTop, ptRightTop);
			break;
		case DECODETYPE_DATAMATRIX:
			bStatus = Decode_DataMatrix(str, ptLeftTop, ptRightTop);
			break;
		case DECODETYPE_BARCODE:
			bStatus = Decode_Code128(str, ptLeftTop, ptRightTop);
			break;
		case DECODETYPE_EAN13:
			bStatus = Decode_EAN13(str, ptLeftTop, ptRightTop);
			break;
	}

	if(bStatus){
		decodeStatus = DECODE_SUCCESS;
	}


#ifdef DISPLAY_INSP_TIME
	t.Stop();
	t.Display(L"[DoDecode]");
#endif

#if 0
	CDecoderMemPool* pInst = CDecoderMemPool::GetInstance();
	CBase2DDecoder* pDecoder = pInst->GetDecoder(0, true, NONE, 0);

	bool bStatus = pDecoder->SetBinarizeImg(m_pSampleImg, m_iW, m_iH, USE_HYBRID_BINARIZE);
#if 0
	unsigned char *debug = new unsigned char[m_iW*m_iH];
	pDecoder->GetBinarizeImgData(debug);
	saveimage(L"D:\\AOI\\DEBUG\\debug.bmp", debug, m_iW, -m_iH, 1, FALSE);
	delete [] debug;
#endif
	if(bStatus){
		bool bTryHarder = false;
		bStatus = pDecoder->decode(bTryHarder);
	}

	if(bStatus){
		string &str = pDecoder->getResultText();
	}

	pInst->ReturnDecoder(pDecoder);
#endif

	return decodeStatus;
}

int AnchorAlignment::DoGoldenDecode(std::string &str, POINT &ptGoldLeftTop, POINT &ptGoldRightTop, BOOL &bVerticalBarcode)
{
	m_bVerticalBarcode = FALSE;
	int status = DoDecode(str, ptGoldLeftTop, ptGoldRightTop, bVerticalBarcode);
	if(status!=DECODE_SUCCESS && (m_emDecodeType==DECODETYPE_BARCODE || m_emDecodeType==DECODETYPE_EAN13)){
		m_bVerticalBarcode = TRUE;
		status = DoDecode(str, ptGoldLeftTop, ptGoldRightTop, bVerticalBarcode);
		if(status!=DECODE_SUCCESS){
			m_bVerticalBarcode = FALSE;
		}
	}
	bVerticalBarcode = m_bVerticalBarcode;

	return status;
}
#endif

void ComputeShiftRotation(int refShiftX, int refShiftY,//GetAnchorShift
						  POINT ptGoldLeftTop, POINT ptGoldRightTop, BOOL bVerticalBarcode,//Input:DoDecode(GoldImg)
						  POINT ptTestLeftTop, POINT ptTestRightTop,//Input:DoDecode(TestImg)
						  int &fndShiftX, int &fndShiftY, float &angle)//output
{
	int nPt = 2;
	if(nPt==1){
		fndShiftX = (ptTestLeftTop.x-refShiftX) - ptGoldLeftTop.x;
		fndShiftY = (ptTestLeftTop.y-refShiftY) - ptGoldLeftTop.y;
		angle = 0.0f;
	}
	else if(nPt==2){
		float a1, a2, b1, b2, gold, test, angle;
		float tol = 1e-08f;
		float sum1, sum2;
		float sx, sy, cx, cy;

		a1 = (float)(ptGoldRightTop.x-ptGoldLeftTop.x);
		a2 = (float)(ptGoldRightTop.y-ptGoldLeftTop.y);
		b1 = (float)( (ptTestRightTop.x-refShiftX) - (ptTestLeftTop.x-refShiftX) );
		b2 = (float)( (ptTestRightTop.y-refShiftY) - (ptTestLeftTop.y-refShiftY) );

		sum1 = a1*a1+a2*a2;
		sum2 = b1*b1+b2*b2;

		if(sqrtf(sum1)<tol || sqrtf(sum2)<tol){
			angle = 0.0f;
			sx = 0.0f;
			sy = 0.0f;
		}
		else{
			gold = sqrtf(sum1);
			test = sqrtf(sum2);
			angle = asin((a1*b2-a2*b1)/(gold*test));
			cx = ( (ptTestLeftTop.x-refShiftX) + (ptTestRightTop.x-refShiftX) )/2.0f;
			cy = ( (ptTestLeftTop.y-refShiftY) + (ptTestRightTop.y-refShiftY) )/2.0f;
			sx = cx - (ptGoldLeftTop.x + ptGoldRightTop.x)/2.0f;
			sy = cy - (ptGoldLeftTop.y + ptGoldRightTop.y)/2.0f;
		}
		fndShiftX = (int)(sx+0.5f);
		fndShiftY = (int)(sy+0.5f);
	}
	if(bVerticalBarcode){
		int tmp = fndShiftX;
		fndShiftX = fndShiftY;
		fndShiftY = -tmp;
	}
}

void imhist(unsigned char *img, int w, int h, int *histogram)
{
    // initialize all intensity values to 0
	for(int i = 0; i < 256; i++){
		histogram[i] = 0;
	}
 
    // calculate the no of pixels for each intensity values
	for(int j=0; j<h; j++){
		for(int i=0; i<w; i++){
			histogram[(int)img[j*w+i]]++;
		}
	}
}

void cumhist(int *histogram, int *cumhistogram)
{
	cumhistogram[0] = histogram[0];
	for(int i = 1; i < 256; i++){
		cumhistogram[i] = histogram[i] + cumhistogram[i-1];
	}
}

#if 0
void getEQScale(unsigned char *src, const int width, const int height, const int channels, int enhanceLevel, unsigned char *dst)
{
	if(enhanceLevel<-10 || enhanceLevel>5 || enhanceLevel==0){
		if(dst!=NULL){
			memcpy(dst, src, width*height*channels);
		}
		return;
	}

//	LARGE_INTEGER PerfFreq;
//	LARGE_INTEGER PerfPart1;
//	LARGE_INTEGER PerfPart2;
//	LARGE_INTEGER PerfPart3;

//	QueryPerformanceFrequency(&PerfFreq);
//	QueryPerformanceCounter(&PerfPart1);

	const int pix_type = 0;
	const int n_pixels = (width*height);
	int imgMinVal[3];
	int imgMaxVal[3];

	imgMinVal[0] = imgMinVal[1] = imgMinVal[2] = 255;
	imgMaxVal[0] = imgMaxVal[1] = imgMaxVal[2] = 0;
	for(int p=0; p<n_pixels; p++){
		for(int c=0; c<channels; c++){
			imgMinVal[c] = min(src[p*channels+c],imgMinVal[c]);
			imgMaxVal[c] = max(src[p*channels+c],imgMaxVal[c]);
		}
	}

	if(enhanceLevel>=0){
		for(int c=0; c<channels; c++){
			int step = ((imgMaxVal[c]-imgMinVal[c])/10)*enhanceLevel;
			imgMinVal[c] += step;
			imgMaxVal[c] -= step;
			if(imgMaxVal[c]<=imgMinVal[c]){
				imgMaxVal[c] = imgMinVal[c]+1;
			}
		}
	}
	else{
		for(int c=0; c<channels; c++){
			imgMinVal[c] = max((imgMinVal[c]+imgMaxVal[c])/2+enhanceLevel*5, 20);
			imgMaxVal[c] = imgMinVal[c]+1;
		}
	}

//	QueryPerformanceCounter(&PerfPart2);

	if(dst!=NULL){
		float scale[3];
		for(int c=0; c<channels; c++){
			scale[c] = 1.0f/(imgMaxVal[c]-imgMinVal[c]);
		}
		for(int i=0; i<n_pixels; i++){
			for(int c=0; c<channels; c++){
				int val = (int)(((src[i*channels+c]-imgMinVal[c])*scale[c])*255.0f+0.5f);
				val = max(val, 0);
				val = min(val, 255);
				dst[i*channels+c] = val;
			}
		}
	}

//	QueryPerformanceCounter(&PerfPart3);

//	fstream fp;
//	fp.open("PerformanceTest.txt", ios::out|ios::app);//¶}±ÒÀÉ®×
//	if(fp){
//		char	str[1024];
//		sprintf(str, "Step 1:");
//		fp<<str<<endl;//¼g¤J¦r¦ê
//		sprintf(str, " %lld us\n", (PerfPart2.QuadPart-PerfPart1.QuadPart)*1000000/PerfFreq.QuadPart);
//		fp<<str<<endl;//¼g¤J¦r¦ê
//		sprintf(str, "Step 2:");
//		fp<<str<<endl;//¼g¤J¦r¦ê
//		sprintf(str, " %lld us\n", (PerfPart3.QuadPart-PerfPart2.QuadPart)*1000000/PerfFreq.QuadPart);
//		fp<<str<<endl;//¼g¤J¦r¦ê
//	}
 
//	fp.close();//Ãö³¬ÀÉ®×
}
#endif

void EQEnhanceImage(unsigned char *src, const int width, const int height, const int channels, int enhanceLevel, RECT roi)
{
	if(enhanceLevel<-10 || enhanceLevel>5 || enhanceLevel==0){
		return;
	}

	// --- check bounds
	roi.left = max(roi.left, 0);
	roi.left = min(roi.left, width-1);
	
	roi.right = max(roi.right, 0);
	roi.right = min(roi.right, width);

	roi.top = max(roi.top, 0);
	roi.top = min(roi.top, height-1);
	
	roi.bottom = max(roi.bottom, 0);
	roi.bottom = min(roi.bottom, height);


	// --- 
	const int pix_type = 0;
	const int n_pixels = width*height;
	int imgMinVal = 255;
	int imgMaxVal = 0;
	unsigned char *ptr1 = src+(roi.top*width+roi.left)*channels;
	unsigned char *ptr2;
	int offset = 0;
	if(channels==3){
		offset = 1;
	}
	for(int j=roi.top; j<roi.bottom; j++){
		ptr2 = ptr1;
		for(int i=roi.left; i<roi.right; i++){
			imgMinVal = min(*(ptr2+offset),imgMinVal);
			imgMaxVal = max(*(ptr2+offset),imgMaxVal);
			ptr2+=channels;
		}
		ptr1+=width*channels;
	}

	if(enhanceLevel>=0){
		int step = ((imgMaxVal-imgMinVal)/10)*enhanceLevel;
		imgMinVal += step;
		imgMaxVal -= step;
		if(imgMaxVal<=imgMinVal){
			imgMaxVal = imgMinVal+1;
		}
	}
	else{
		imgMinVal = max((imgMinVal+imgMaxVal)/2+enhanceLevel*5, 20);
		imgMaxVal = imgMinVal+1;
	}


	// --- apply to src image
	float scale = 255.0f/(imgMaxVal-imgMinVal);

	ptr1 = src+(roi.top*width+roi.left)*channels;
	for(int j=roi.top; j<roi.bottom; j++){
		ptr2 = ptr1;
		for(int i=roi.left; i<roi.right; i++){
			int val = (int)((*(ptr2+offset)-imgMinVal)*scale+0.5f);
			val = max(val, 0);
			val = min(val, 255);
			for(int c=0; c<channels; c++){
				*ptr2 = val;
				ptr2++;
			}
		}
		ptr1+=width*channels;
	}

}


#ifdef FIND_ANCHOR_AUTO//find anchor automatically
void testAnchorAlignment(IMAGE *golden_img, IMAGE *sample_img, GEN_INSP_PARAM *genInspParam)
{
	AnchorAlignment anchorAlign;
	// init
	anchorAlign.SetGoldenImg(golden_img);
	anchorAlign.SetInspParam(genInspParam);
	BOOL bFound = anchorAlign.ProcessAnchorFromGolden();// or SetAnchor();
	// inspect
	anchorAlign.SetSampleData(sample_img);
	anchorAlign.DoAnchorAlignMent();
	// result
	int xshift, yshift;
	unsigned int uCorrelation;
	anchorAlign.GetAnchorShift(xshift, yshift);
	anchorAlign.GetAnchorCorr(uCorrelation);
}
#endif

#ifdef DEBUG_SELF_TEST
void runTest(void)
{
	IMAGE golden_img;
	IMAGE sample_img;

	unsigned char *golden = NULL;
	unsigned char *sample = NULL;
	int w=0, h=0, color=0;

	loadimage(L"golden.bmp", &golden, &w, &h, &color, READ_DEFAULT, FALSE);
	ImageInit(&golden_img, color==3?IMAGE_TYPE_RGB24:IMAGE_TYPE_MONO8, golden, w, h);

	loadimage(L"sample.bmp", &sample, &w, &h, &color, READ_DEFAULT, FALSE);
	ImageInit(&sample_img, color==3?IMAGE_TYPE_RGB24:IMAGE_TYPE_MONO8, sample, w, h);

	GEN_INSP_PARAM genInspParam;
	genInspParam.rcSearchRange.left = 180;
	genInspParam.rcSearchRange.top = 100;
	genInspParam.rcSearchRange.right = 692;
	genInspParam.rcSearchRange.bottom = 612;
	genInspParam.uCorrelation = 70;
	testAnchorAlignment(&golden_img, &sample_img, &genInspParam);
}
#endif