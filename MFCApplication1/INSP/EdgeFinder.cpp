#include "EdgeFinder.h"
#include "utils.h"

#include <windows.h>
#include <vector>
using std::vector;

#ifndef PI
#define PI 3.141592653589793f
#endif

//#define DISPLAY_INSP_TIME
#ifdef DISPLAY_INSP_TIME
static void _D(const wchar_t *fmt, ...)
{
	wchar_t text[2048];
	va_list	ap;
	va_start(ap, fmt);
	vswprintf_s(text, 2048, fmt, ap);
	va_end(ap);
	OutputDebugStringW(text);
}
struct CpuTimer
{
	LARGE_INTEGER PerfFreq;
	LARGE_INTEGER PerfPart1;
	LARGE_INTEGER PerfPart2;

	CpuTimer(void)
	{
		QueryPerformanceFrequency(&PerfFreq);
	}
	void Start(void)
	{
		QueryPerformanceCounter(&PerfPart1);
	}
	void Stop(void)
	{
		QueryPerformanceCounter(&PerfPart2);
	}
	void Display(wchar_t *_msg)
	{
		wchar_t	msg[1024];
		swprintf_s(msg, 1024, L"%s %lld us\n", _msg, (PerfPart2.QuadPart-PerfPart1.QuadPart)*1000000/PerfFreq.QuadPart);
		_D(msg);
	}
};
#endif

typedef struct RECTCORNER_{
	POINT lefttop;
	POINT righttop;
	POINT leftbottom;
	POINT rightbottom;
}RECTCORNER;

const int minDynamicRange = 24;

CEdgeFinder::CEdgeFinder()
{
	ImageInit(&m_xGolden, IMAGE_TYPE_NOTYPE, NULL, 0, 0);
	ImageInit(&m_xMaxMap, IMAGE_TYPE_NOTYPE, NULL, 0, 0);
	ImageInit(&m_xMinMap, IMAGE_TYPE_NOTYPE, NULL, 0, 0);

	m_xDefectParam.iDefectTH = INT_MAX;
	m_xDefectParam.iDefectPnt = INT_MAX;

	m_budget = 20;
	m_dynamicRange = 24;
	m_cameraOri = GRAB_DEGREE_0;
	m_BGColor = 20;
	memset(&m_xModeInfo, 0, sizeof(MODE_INFO));

	m_fCaliWidth = 20.0f;
	m_fCaliHeight = 20.0f;
}

CEdgeFinder::~CEdgeFinder()
{
	if(m_xGolden.ptr!=NULL){
		delete [] m_xGolden.ptr;
	}
	if(m_xMaxMap.ptr!=NULL){
		delete [] m_xMaxMap.ptr;
	}
	if(m_xMinMap.ptr!=NULL){
		delete [] m_xMinMap.ptr;
	}
}

inline int capInt(int value, int min, int max){
	return value < min ? min : value > max ? max : value;
}

void findEdgeByDiff(IMAGE *img, int dynamicRange, int x_in, int y_start, int y_end, vector<int> *y_out)
{
	if(y_out==NULL){ return; }
	y_out->clear();

	if(verifyIMAGE(img)==false){ return; }

	unsigned char *ptr = img->ptr;
	int w = img->data_w;
	int h = img->data_h;
	int color = GetImagePixelBytes(img);

	int diffDistance = 20;
	int distanceTH = diffDistance*30/40;

	int blackWidth = 0;
	int whiteWidth = 0;

	for(int j=y_start; j<y_end-diffDistance-1; j++){
		if( j<0 || j>=h || j+diffDistance<0 || j+diffDistance>=h ){ continue; }
		int diff = 0;
		if(color==3){
			diff = ptr[((j+diffDistance)*w+x_in)*color+2]-ptr[(j*w+x_in)*color+2];
		}
		else{
			diff = ptr[(j+diffDistance)*w+x_in]-ptr[j*w+x_in];
		}

		if(diff>dynamicRange){
			blackWidth++;
		}
		else if(diff<-dynamicRange){
			whiteWidth++;
		}
		else{
			if(blackWidth>=distanceTH){
				y_out->push_back(j);
			}
			if(whiteWidth>=distanceTH){
				y_out->push_back(j);
			}
			blackWidth=0;
			whiteWidth=0;
		}
	}
}

void findEdgeByDiff_x(IMAGE *img, int dynamicRange, int y_in, int x_start, int x_end, vector<int> *x_out)
{
	if(x_out==NULL){ return; }
	x_out->clear();

	if(verifyIMAGE(img)==false){ return; }

	unsigned char *ptr = img->ptr;
	int w = img->data_w;
	int h = img->data_h;
	int color = GetImagePixelBytes(img);

	int diffDistance = 20;
	int distanceTH = diffDistance*30/40;

	int blackWidth = 0;
	int whiteWidth = 0;

	for(int j=x_start; j<x_end-diffDistance-1; j++){
		if( j<0 || j>=w || j+diffDistance<0 || j+diffDistance>=w ){ continue; }
		int diff = 0;
		if(color==3){
			diff = ptr[(y_in*w+(j+diffDistance))*color+2]-ptr[(y_in*w+j)*color+2];
		}
		else{
			diff = ptr[y_in*w+(j+diffDistance)]-ptr[y_in*w+j];
		}

		if(diff>dynamicRange){
			blackWidth++;
		}
		else if(diff<-dynamicRange){
			whiteWidth++;
		}
		else{
			if(blackWidth>=distanceTH){
				x_out->push_back(j);
			}
			if(whiteWidth>=distanceTH){
				x_out->push_back(j);
			}
			blackWidth=0;
			whiteWidth=0;
		}
	}
}

typedef struct _GRADIENT_INFO {
	int x;
	int y;
	float gradient;
} GRADIENT_INFO;

void calculateSubPixelEdge(IMAGE *img, int x_in, int y_start, int y_end, float &y_out, int &y_int_out)
{
	if(verifyIMAGE(img)==false){ return; }

	y_start = y_start < 0 ? 0 : y_start;
	y_start = y_start >= img->data_h ? img->data_h-1 : y_start;

	y_end = y_end < 0 ? 0 : y_end;
	y_end = y_end >= img->data_h ? img->data_h-1 : y_end;

	unsigned char *ptr = img->ptr;
	int w = img->data_w;
	int h = img->data_h;
	int color = GetImagePixelBytes(img);

	float M_x[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
	float M_y[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

	int length = (y_end-1) - (y_start+1);
	GRADIENT_INFO xInfo;
	std::vector<GRADIENT_INFO> vGradientInfo;
	vGradientInfo.reserve(length);

	if(color==3){
		for(int j=y_start+1; j<y_end-1; j++){
			float G_x = 0.0f;
			float G_y = 0.0f;
			for(int jj=0; jj<3; jj++){
				for(int ii=0; ii<3; ii++){
					G_x += ptr[((j+jj-1)*w+x_in+ii-1)*color+1]*M_x[jj*3+ii];
					G_y += ptr[((j+jj-1)*w+x_in+ii-1)*color+1]*M_y[jj*3+ii];
				}
			}

			xInfo.x = x_in;
			xInfo.y = j;
			xInfo.gradient = sqrtf(G_x*G_x + G_y*G_y);
			vGradientInfo.push_back(xInfo);
		}
	}
	else{
		for(int j=y_start+1; j<y_end-1; j++){
			float G_x = 0.0f;
			float G_y = 0.0f;
			for(int jj=0; jj<3; jj++){
				for(int ii=0; ii<3; ii++){
					G_x += ptr[(j+jj-1)*w+x_in+ii-1]*M_x[jj*3+ii];
					G_y += ptr[(j+jj-1)*w+x_in+ii-1]*M_y[jj*3+ii];
				}
			}

			xInfo.x = x_in;
			xInfo.y = j;
			xInfo.gradient = sqrtf(G_x*G_x + G_y*G_y);
			vGradientInfo.push_back(xInfo);
		}
	}

	int size = (int)vGradientInfo.size();
	if(size>=3){
		float maxGradient = 0.0f;
		for(int i=1; i<size-1; i++){
			float g_a = vGradientInfo[i-1].gradient;
			float g_b = vGradientInfo[i  ].gradient;
			float g_c = vGradientInfo[i+1].gradient;

			float tao = 0.5f*(g_a-g_c)/(g_a+g_c-g_b-g_b) + vGradientInfo[i].y;
			if(g_b>maxGradient){
				maxGradient = g_b;
				y_out = tao;
#ifdef ENABLE_PIXEL_BASE
				y_int_out = vGradientInfo[i].y;
#endif
			}
		}
	}
}

void calculateSubPixelEdge_x(IMAGE *img, int y_in, int x_start, int x_end, float &x_out, int &x_int_out)
{
	if(verifyIMAGE(img)==false){ return; }

	x_start = x_start < 0 ? 0 : x_start;
	x_start = x_start >= img->data_w ? img->data_w-1 : x_start;

	x_end = x_end < 0 ? 0 : x_end;
	x_end = x_end >= img->data_w ? img->data_w-1 : x_end;

	unsigned char *ptr = img->ptr;
	int w = img->data_w;
	int h = img->data_h;
	int color = GetImagePixelBytes(img);

	float M_x[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
	float M_y[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

	int length = (x_end-1) - (x_start+1);
	GRADIENT_INFO xInfo;
	std::vector<GRADIENT_INFO> vGradientInfo;
	vGradientInfo.reserve(length);
	
	if(color==3){
		for(int j=x_start+1; j<x_end-1; j++){
			float G_x = 0.0f;
			float G_y = 0.0f;
			for(int jj=0; jj<3; jj++){
				for(int ii=0; ii<3; ii++){
					G_x += ptr[((y_in+jj-1)*w+j+ii-1)*color+1]*M_x[jj*3+ii];
					G_y += ptr[((y_in+jj-1)*w+j+ii-1)*color+1]*M_y[jj*3+ii];
				}
			}

			xInfo.x = j;
			xInfo.y = y_in;
			xInfo.gradient = sqrtf(G_x*G_x + G_y*G_y);
			vGradientInfo.push_back(xInfo);
		}
	}
	else{
		for(int j=x_start+1; j<x_end-1; j++){
			float G_x = 0.0f;
			float G_y = 0.0f;
			for(int jj=0; jj<3; jj++){
				for(int ii=0; ii<3; ii++){
					G_x += ptr[(y_in+jj-1)*w+j+ii-1]*M_x[jj*3+ii];
					G_y += ptr[(y_in+jj-1)*w+j+ii-1]*M_y[jj*3+ii];
				}
			}

			xInfo.x = j;
			xInfo.y = y_in;
			xInfo.gradient = sqrtf(G_x*G_x + G_y*G_y);
			vGradientInfo.push_back(xInfo);
		}
	}

	int size = (int)vGradientInfo.size();
	if(size>=3){
		float maxGradient = 0.0f;
		for(int i=1; i<size-1; i++){
			float g_a = vGradientInfo[i-1].gradient;
			float g_b = vGradientInfo[i  ].gradient;
			float g_c = vGradientInfo[i+1].gradient;

			float tao = 0.5f*(g_a-g_c)/(g_a+g_c-g_b-g_b) + vGradientInfo[i].x;
			if(g_b>maxGradient){
				maxGradient = g_b;
				x_out = tao;
#ifdef ENABLE_PIXEL_BASE
				x_int_out = vGradientInfo[i].x;
#endif
			}
		}
	}
}

void outputResultByMode(vector<vector<float>> *y_sub, vector<vector<int>> *y_int, MODE_INFO *xModeInfo, ALIGNMENT_INFO *xAlignmentInfo, INSP_MODE cur_mode)
{
	INSP_MODE mode = cur_mode;//xModeInfo->mode;
	size_t budget = y_sub->size();

	int count = 0;
	for(size_t i=0; i<budget; i++){
		size_t size = (*y_sub)[i].size();
		if(size==(int)mode){
			count++;
			for(int j=0; j<size; j++){
				xAlignmentInfo->line[j] += (*y_sub)[i][j];
#ifdef ENABLE_PIXEL_BASE
				xAlignmentInfo->line_int[j] += (*y_int)[i][j];
#endif
			}
		}
	}
	if(count>0){
		for(int j=0; j<(int)mode; j++){
			xAlignmentInfo->line[j] /= count;
#ifdef ENABLE_PIXEL_BASE
			xAlignmentInfo->line_int[j] /= count;
#endif
		}
	}
}

int countTop(IMAGE *xImg, int budget, int countLength, int blackTH)
{
	if(xImg->data_h<countLength){ return 0; }

	unsigned char *ptr = xImg->ptr;
	int w = xImg->data_w;
	int h = xImg->data_h;
	int color = GetImagePixelBytes(xImg);

	int offset = w/(budget+1);
	int count = 0;
	if(color==3){
		for(int j=0; j<countLength; j++){
			for(int i=0; i<budget; i++){
				int val = ptr[(j*w+i*offset)*color+2];
				if(val < blackTH+minDynamicRange){
					count++;
				}
			}
		}
	}
	else if(color==1){
		for(int j=0; j<countLength; j++){
			for(int i=0; i<budget; i++){
				int val = ptr[j*w+i*offset];
				if(val < blackTH+minDynamicRange){
					count++;
				}
			}
		}
	}

	return count;
}

int countBottom(IMAGE *xImg, int budget, int countLength, int blackTH)
{
	if(xImg->data_h<countLength){ return 0; }

	unsigned char *ptr = xImg->ptr;
	int w = xImg->data_w;
	int h = xImg->data_h;
	int color = GetImagePixelBytes(xImg);

	int offset = w/(budget+1);
	int count = 0;
	if(color==3){
		for(int j=0; j<countLength; j++){
			for(int i=0; i<budget; i++){
				int val = ptr[((j+h-countLength)*w+i*offset)*color+2];
				if(val < blackTH+minDynamicRange){
					count++;
				}
			}
		}
	}
	else if(color==1){
		for(int j=0; j<countLength; j++){
			for(int i=0; i<budget; i++){
				int val = ptr[(j+h-countLength)*w+i*offset];
				if(val < blackTH+minDynamicRange){
					count++;
				}
			}
		}
	}

	return count;
}

int countLeft(IMAGE *xImg, int budget, int countLength, int blackTH)
{
	if(xImg->data_w<countLength){ return 0; }

	unsigned char *ptr = xImg->ptr;
	int w = xImg->data_w;
	int h = xImg->data_h;
	int color = GetImagePixelBytes(xImg);

	int offset = h/(budget+1);
	int count = 0;
	if(color==3){
		for(int i=0; i<budget; i++){
			for(int j=0; j<countLength; j++){
				int val = ptr[((i*offset)*w+j)*color+2];
				if(val < blackTH+minDynamicRange){
					count++;
				}
			}
		}
	}
	else if(color==1){
		for(int i=0; i<budget; i++){
			for(int j=0; j<countLength; j++){
				int val = ptr[(i*offset)*w+j];
				if(val < blackTH+minDynamicRange){
					count++;
				}
			}
		}
	}

	return count;
}

int countRight(IMAGE *xImg, int budget, int countLength, int blackTH)
{
	if(xImg->data_w<countLength){ return 0; }

	unsigned char *ptr = xImg->ptr;
	int w = xImg->data_w;
	int h = xImg->data_h;
	int color = GetImagePixelBytes(xImg);

	int offset = h/(budget+1);
	int count = 0;
	if(color==3){
		for(int i=0; i<budget; i++){
			for(int j=0; j<countLength; j++){
				int val = ptr[((i*offset)*w+j+w-countLength)*color+2];
				if(val < blackTH+minDynamicRange){
					count++;
				}
			}
		}
	}
	else if(color==1){
		for(int i=0; i<budget; i++){
			for(int j=0; j<countLength; j++){
				int val = ptr[(i*offset)*w+j+w-countLength];
				if(val < blackTH+minDynamicRange){
					count++;
				}
			}
		}
	}

	return count;
}

int getBGDis(IMAGE *xImg, int backgroundColor, ORIENTATION type)
{
	const int BGDisCountW = 5;
	const int BGDisCountH = 5;

	unsigned char *ptr = xImg->ptr;
	int w = xImg->data_w;
	int h = xImg->data_h;
	int color = GetImagePixelBytes(xImg);

	int left = 0;
	int top = 0;
	if(type==ORIENTATION::TOP){
		left = (w-BGDisCountW)/2;
	}
	else if(type==ORIENTATION::BOTTOM){
		left = (w-BGDisCountW)/2;
		top = h-BGDisCountH;
	}
	else if(type==ORIENTATION::LEFT){
		top = (h-BGDisCountH)/2;
	}
	else if(type==ORIENTATION::RIGHT){
		left = w-BGDisCountW;
		top = (h-BGDisCountH)/2;
	}

	int count = 0;
	int BGDis = 0;
	for(int j=0; j<BGDisCountH; j++){
		for(int i=0; i<BGDisCountW; i++){
			int x = left + i;
			int y = top + j;
			if(x>=0 && x<w && y>=0 && y<h){
				BGDis += abs(ptr[(y*w+x)*color+0]-backgroundColor);
				if(color==3){
					BGDis += ( abs(ptr[(y*w+x)*color+1]-backgroundColor)+abs(ptr[(y*w+x)*color+2]-backgroundColor) );
				}
				count++;
			}
		}
	}
	if(count>0){
		return BGDis/count;
	}
	return INT_MAX;
}

ORIENTATION CEdgeFinder::checkBGO(IMAGE *xImg)
{
	if(verifyIMAGE(xImg)==false){ return ORIENTATION::UNKNOWN; }

	int blackCenter = 0;
	int whiteCenter = 0;
	getHistogramGT(xImg->ptr, xImg->data_w, xImg->data_h, GetImagePixelBytes(xImg), &blackCenter, &whiteCenter);

	int budget = 20;
	int countLength = 20;

	int count = 0;
	int c1 = countTop(xImg, budget, countLength, blackCenter);
	int c2 = countBottom(xImg, budget, countLength, blackCenter);
	int c3 = countLeft(xImg, budget, countLength, blackCenter);
	int c4 = countRight(xImg, budget, countLength, blackCenter);

	int dis = INT_MAX;
	int backgroundColor = m_BGColor;
	int c1_dis = getBGDis(xImg, backgroundColor, ORIENTATION::TOP);
	int c2_dis = getBGDis(xImg, backgroundColor, ORIENTATION::BOTTOM);
	int c3_dis = getBGDis(xImg, backgroundColor, ORIENTATION::LEFT);
	int c4_dis = getBGDis(xImg, backgroundColor, ORIENTATION::RIGHT);

	if(c1==c2){
		if(c1_dis<c2_dis){
			c2 = 0;
		}
		else{
			c1 = 0;
		}
	}

	if(c3==c4){
		if(c3_dis<c4_dis){
			c4 = 0;
		}
		else{
			c3 = 0;
		}
	}
	
	ORIENTATION bgo = ORIENTATION::TOP;
	if(c1>count){//countTop
		switch (m_cameraOri)
		{
		case GRAB_DEGREE_0:
			bgo = ORIENTATION::TOP;
			break;
		case GRAB_DEGREE_90:
			bgo = ORIENTATION::RIGHT;
			break;
		case GRAB_DEGREE_180:
			bgo = ORIENTATION::BOTTOM;
			break;
		case GRAB_DEGREE_270:
			bgo = ORIENTATION::LEFT;
			break;
		default:
			bgo = ORIENTATION::TOP;
			break;
		}
		count = c1;
	}

	if(c2>count){//countBottom
		switch (m_cameraOri)
		{
		case GRAB_DEGREE_0:
			bgo = ORIENTATION::BOTTOM;
			break;
		case GRAB_DEGREE_90:
			bgo = ORIENTATION::LEFT;
			break;
		case GRAB_DEGREE_180:
			bgo = ORIENTATION::TOP;
			break;
		case GRAB_DEGREE_270:
			bgo = ORIENTATION::RIGHT;
			break;
		default:
			bgo = ORIENTATION::BOTTOM;
			break;
		}
		count = c2;
	}

	if(c3>count){//countLeft
		switch (m_cameraOri)
		{
		case GRAB_DEGREE_0:
			bgo = ORIENTATION::LEFT;
			break;
		case GRAB_DEGREE_90:
			bgo = ORIENTATION::TOP;
			break;
		case GRAB_DEGREE_180:
			bgo = ORIENTATION::RIGHT;
			break;
		case GRAB_DEGREE_270:
			bgo = ORIENTATION::BOTTOM;
			break;
		default:
			bgo = ORIENTATION::LEFT;
			break;
		}
		count = c3;
	}

	if(c4>count){//countRight
		switch (m_cameraOri)
		{
		case GRAB_DEGREE_0:
			bgo = ORIENTATION::RIGHT;
			break;
		case GRAB_DEGREE_90:
			bgo = ORIENTATION::BOTTOM;
			break;
		case GRAB_DEGREE_180:
			bgo = ORIENTATION::LEFT;
			break;
		case GRAB_DEGREE_270:
			bgo = ORIENTATION::TOP;
			break;
		default:
			bgo = ORIENTATION::RIGHT;
			break;
		}
		count = c4;
	}

	return bgo;
}

void CEdgeFinder::findHoriLine(IMAGE *xImg, ALIGNMENT_INFO *xAlignmentInfo)
{
	int mode[INSP_MODE::INSP_MAX_MODE_NUM] = {0};
	int roi = 10;

	vector<int> y_out;
	vector<vector<float>> y_sub;
	y_sub.clear();
	y_sub.resize(m_budget);

	vector<vector<int>> y_int;
#ifdef ENABLE_PIXEL_BASE
	y_int.clear();
	y_int.resize(m_budget);
#endif
	
	int offset = xImg->data_w/(m_budget+1);
	int budgetIdx = 0;
	for(int j=2; j<xImg->data_w-2; j+=offset){
		findEdgeByDiff(xImg, m_dynamicRange, j, 0, xImg->data_h, &y_out);
		int size = (int)y_out.size();
		if(size<4){
			mode[size]++;
			for(int i=0; i<size; i++){
				float y_subpixel = 0.0f;
				int y_int_out = 0;
				calculateSubPixelEdge(xImg, j, y_out[i]-roi, y_out[i]+roi, y_subpixel, y_int_out);
				y_sub[budgetIdx].push_back(y_subpixel);
#ifdef ENABLE_PIXEL_BASE
				y_int[budgetIdx].push_back(y_int_out);
#endif
			}
			budgetIdx++;
			if(budgetIdx>=m_budget){ break; }
		}
	}

	// find max mode number
	INSP_MODE cur_mode = INSP_MODE_UNKNOWN;
	int count = 0;
	for(int i=0; i<INSP_MODE::INSP_MAX_MODE_NUM; i++){
		if(mode[i]>count){
			count = mode[i];
			cur_mode = (INSP_MODE)i;
		}
	}
	if(m_xModeInfo.mode==INSP_MODE_UNKNOWN){
		m_xModeInfo.mode = cur_mode;
	}

	outputResultByMode(&y_sub, &y_int, &m_xModeInfo, xAlignmentInfo, cur_mode);
}

void CEdgeFinder::findVertLine(IMAGE *xImg, ALIGNMENT_INFO *xAlignmentInfo)
{
#ifdef DISPLAY_INSP_TIME
	CpuTimer t;
	t.Start();
#endif

	int mode[INSP_MODE::INSP_MAX_MODE_NUM] = {0};
	int roi = 10;

	vector<int> x_out;
	vector<vector<float>> x_sub;
	x_sub.clear();
	x_sub.resize(m_budget);

#ifdef DISPLAY_INSP_TIME
	t.Stop();
	t.Display(L"[Init]");
	t.Start();
#endif

	vector<vector<int>> x_int;
#ifdef ENABLE_PIXEL_BASE
	x_int.clear();
	x_int.resize(m_budget);
#endif
	
	int offset = xImg->data_h/(m_budget+1);
	int budgetIdx = 0;
	for(int j=2; j<xImg->data_h-2; j+=offset){
		findEdgeByDiff_x(xImg, m_dynamicRange, j, 0, xImg->data_w, &x_out);
		int size = (int)x_out.size();
		if(size<4){
			mode[size]++;
			for(int i=0; i<size; i++){
				float x_subpixel = 0.0f;
				int x_int_out = 0;
				calculateSubPixelEdge_x(xImg, j, x_out[i]-roi, x_out[i]+roi, x_subpixel, x_int_out);
				x_sub[budgetIdx].push_back(x_subpixel);
#ifdef ENABLE_PIXEL_BASE
				x_int[budgetIdx].push_back(x_int_out);
#endif
			}
			budgetIdx++;
			if(budgetIdx>=m_budget){ break; }
		}
	}

#ifdef DISPLAY_INSP_TIME
	t.Stop();
	t.Display(L"[calculateSubPixelEdge_x]");
	t.Start();
#endif

	// find max mode number
	INSP_MODE cur_mode = INSP_MODE_UNKNOWN;
	int count = 0;
	for(int i=0; i<INSP_MODE::INSP_MAX_MODE_NUM; i++){
		if(mode[i]>count){
			count = mode[i];
			cur_mode = (INSP_MODE)i;
		}
	}
	if(m_xModeInfo.mode==INSP_MODE_UNKNOWN){
		m_xModeInfo.mode = cur_mode;
	}

	outputResultByMode(&x_sub, &x_int, &m_xModeInfo, xAlignmentInfo, cur_mode);

#ifdef DISPLAY_INSP_TIME
	t.Stop();
	t.Display(L"[outputResultByMode]");
#endif
}

void CEdgeFinder::GetLineNo(ALIGNMENT_INFO *alignmentInfo)
{
	if(m_xModeInfo.lineSelected == LINE_SELECTED::LINE_UNKNOWN){
		alignmentInfo->lineShift = 0.0f;
		m_xModeInfo.lineSelected = LINE_SELECTED::LINE_FIRST;
		m_xModeInfo.baseLine = alignmentInfo->line[0];

		if(m_cameraOri==GRAB_DEGREE_0){
			if(m_xModeInfo.mode == INSP_MODE_DOUBLE){
				if(m_xModeInfo.bgo==ORIENTATION::TOP){
					m_xModeInfo.lineSelected = LINE_SELECTED::LINE_SECOND;
					
				}
				else if(m_xModeInfo.bgo==ORIENTATION::BOTTOM){
					m_xModeInfo.lineSelected = LINE_SELECTED::LINE_FIRST;
				}
				else if(m_xModeInfo.bgo==ORIENTATION::LEFT){
					m_xModeInfo.lineSelected = LINE_SELECTED::LINE_SECOND;
				}
				else if(m_xModeInfo.bgo==ORIENTATION::RIGHT){
					m_xModeInfo.lineSelected = LINE_SELECTED::LINE_FIRST;
				}
			}
		}
		else if(m_cameraOri==GRAB_DEGREE_180){
			if(m_xModeInfo.mode == INSP_MODE_DOUBLE){
				if(m_xModeInfo.bgo==ORIENTATION::TOP){
					m_xModeInfo.lineSelected = LINE_SELECTED::LINE_FIRST;
				}
				else if(m_xModeInfo.bgo==ORIENTATION::BOTTOM){
					m_xModeInfo.lineSelected = LINE_SELECTED::LINE_SECOND;
				}
				else if(m_xModeInfo.bgo==ORIENTATION::LEFT){
					m_xModeInfo.lineSelected = LINE_SELECTED::LINE_FIRST;
				}
				else if(m_xModeInfo.bgo==ORIENTATION::RIGHT){
					m_xModeInfo.lineSelected = LINE_SELECTED::LINE_SECOND;
				}
			}
		}
		else if(m_cameraOri==GRAB_DEGREE_90){
			if(m_xModeInfo.mode == INSP_MODE_DOUBLE){
				if(m_xModeInfo.bgo==ORIENTATION::RIGHT){
					m_xModeInfo.lineSelected = LINE_SELECTED::LINE_SECOND;
				}
				else if(m_xModeInfo.bgo==ORIENTATION::LEFT){
					m_xModeInfo.lineSelected = LINE_SELECTED::LINE_FIRST;
				}
				if(m_xModeInfo.bgo==ORIENTATION::TOP){
					m_xModeInfo.lineSelected = LINE_SELECTED::LINE_SECOND;
				}
				else if(m_xModeInfo.bgo==ORIENTATION::BOTTOM){
					m_xModeInfo.lineSelected = LINE_SELECTED::LINE_FIRST;
				}
			}
		}
		else if(m_cameraOri==GRAB_DEGREE_270){
			if(m_xModeInfo.mode == INSP_MODE_DOUBLE){
				if(m_xModeInfo.bgo==ORIENTATION::LEFT){
					m_xModeInfo.lineSelected = LINE_SELECTED::LINE_SECOND;
				}
				else if(m_xModeInfo.bgo==ORIENTATION::RIGHT){
					m_xModeInfo.lineSelected = LINE_SELECTED::LINE_FIRST;
				}
				if(m_xModeInfo.bgo==ORIENTATION::TOP){
					m_xModeInfo.lineSelected = LINE_SELECTED::LINE_FIRST;
				}
				else if(m_xModeInfo.bgo==ORIENTATION::BOTTOM){
					m_xModeInfo.lineSelected = LINE_SELECTED::LINE_SECOND;
				}
			}
		}

		if(m_xModeInfo.mode == INSP_MODE_RECT){
			m_xModeInfo.lineSelected = LINE_SELECTED::LINE_SECOND;
		}
		m_xModeInfo.baseLine = alignmentInfo->line[m_xModeInfo.lineSelected-1];
	}
	else if(m_xModeInfo.lineSelected >= LINE_SELECTED::LINE_FIRST && m_xModeInfo.lineSelected <= LINE_SELECTED::LINE_THIRD){
		alignmentInfo->lineShift = alignmentInfo->line[m_xModeInfo.lineSelected-1] - m_xModeInfo.baseLine;
	}
}

void CEdgeFinder::Insp(IMAGE *xImg, ALIGNMENT_INFO *alignmentInfo)
{
	if(alignmentInfo == NULL){ return; }
	memset(alignmentInfo, 0, sizeof(ALIGNMENT_INFO));

	if(verifyIMAGE(xImg)==false){ return; }

	if(m_xModeInfo.bgo == ORIENTATION::UNKNOWN){
		m_xModeInfo.bgo = checkBGO(xImg);
		m_xModeInfo.mode = INSP_MODE_UNKNOWN;
	}

	if( ((m_cameraOri==GRAB_DEGREE_0 || m_cameraOri==GRAB_DEGREE_180) && (m_xModeInfo.bgo==ORIENTATION::TOP || m_xModeInfo.bgo==ORIENTATION::BOTTOM))
	 || ((m_cameraOri==GRAB_DEGREE_270 || m_cameraOri==GRAB_DEGREE_90) && (m_xModeInfo.bgo==ORIENTATION::LEFT || m_xModeInfo.bgo==ORIENTATION::RIGHT))
	){
		findHoriLine(xImg, alignmentInfo);
	}
	else if( ((m_cameraOri==GRAB_DEGREE_0 || m_cameraOri==GRAB_DEGREE_180) && (m_xModeInfo.bgo==ORIENTATION::LEFT || m_xModeInfo.bgo==ORIENTATION::RIGHT))
	      || ((m_cameraOri==GRAB_DEGREE_270 || m_cameraOri==GRAB_DEGREE_90) && (m_xModeInfo.bgo==ORIENTATION::TOP || m_xModeInfo.bgo==ORIENTATION::BOTTOM))
	){
		findVertLine(xImg, alignmentInfo);
	}

	GetLineNo(alignmentInfo);
}

void CEdgeFinder::CheckMode(IMAGE *xImg, RESULT_INFO *resultInfo)
{
	if(resultInfo == NULL){ return; }
	memset(resultInfo, 0, sizeof(RESULT_INFO));

	if(verifyIMAGE(xImg)==false){ return; }

	memset(&m_xModeInfo, 0, sizeof(MODE_INFO));
	m_xModeInfo.bgo = resultInfo->xModeInfo.bgo = checkBGO(xImg);
	m_xModeInfo.mode = resultInfo->xModeInfo.mode = INSP_MODE_UNKNOWN;

	Insp(xImg, &resultInfo->xAlignmentInfo);

	resultInfo->xModeInfo.lineSelected = m_xModeInfo.lineSelected;
	resultInfo->xModeInfo.baseLine = m_xModeInfo.baseLine;
	resultInfo->xModeInfo.mode = m_xModeInfo.mode;
}

int find_caribration_line_area(unsigned char *srcImg, int w, int h, int *positionX, vector<int> *positionY)
{
	int num = 2;
	int t1 = 255;
	int t2 = 0;
	int t = 0, count = 0;

	*positionX = 0;
	for(int i=(w/40); i<w; i+=(w/40)){
		count = 0;
		positionY->clear();
		for(int j=1; j<h; j++){
			if(srcImg[(j-1)*w+i]==t1 && srcImg[j*w+i]==t2){
				count++;
				t = t1;
				t1 = t2;
				t2 = t;
				positionY->push_back(j);
			}
		}
		if(count==num){
			*positionX = i;
			break;
		}
	}

	return 0;
}

const int cornerTerminateLen = 2;
const int cornerDetectPoint = 30;
const float cornerSearchCriterion = 0.8f;

void traceCornerLeft(unsigned char *binaryImg, int w, int h, bool bTop, POINT curPt, unsigned char t1, unsigned char t2, POINT &pt)
{
	int sign = bTop ? 1 : (-1);

	pt.x = 0;
	pt.y = curPt.y;
	if(curPt.x!=0){
		for(int j=curPt.x; j>cornerTerminateLen+1; j--){
			for(int i=0; i<cornerDetectPoint; i++){
				int curY = pt.y + sign*(i-cornerDetectPoint/3);
				if(curY>=0 && curY<h){
					if(binaryImg[curY*w+j]==t1){
						pt.y = curY; 
						break;
					}
				}
			}

			int count = 0;
			int totalCount = 0;
			for(int c=1; c<=cornerTerminateLen; c++){
				for(int i=0; i<cornerDetectPoint; i++){
					int curY = pt.y + sign*(i-cornerDetectPoint/3);
					if(curY>=0 && curY<h){
						totalCount++;
						if(binaryImg[curY*w+j-c]==t2){
							count++;
						}
					}
				}
			}

			if(count>=(totalCount*cornerSearchCriterion)){
				pt.x = j;
				break;
			}
		}
	}

}

void traceCornerRight(unsigned char *binaryImg, int w, int h, bool bTop, POINT curPt, unsigned char t1, unsigned char t2, POINT &pt)
{
	int sign = bTop ? 1 : (-1);

	pt.x = w-1;
	pt.y = curPt.y;
	if(curPt.x!=0){
		for(int j=curPt.x; j<w-cornerTerminateLen-1; j++){
			for(int i=0; i<cornerDetectPoint; i++){
				int curY = pt.y + sign*(i-cornerDetectPoint/3);
				if(curY>=0 && curY<h){
					if(binaryImg[curY*w+j]==t1){
						pt.y = curY; 
						break;
					}
				}
			}

			int count = 0;
			int totalCount = 0;
			for(int c=1; c<=cornerTerminateLen; c++){
				for(int i=0; i<cornerDetectPoint; i++){
					int curY = pt.y + sign*(i-cornerDetectPoint/3);
					if(curY>=0 && curY<h){
						totalCount++;
						if(binaryImg[curY*w+j+c]==t2){
							count++;
						}
					}
				}
			}

			if(count>=(totalCount*cornerSearchCriterion)){
				pt.x = j;
				break;
			}
		}
	}

}

bool findCaliPaperCornerArea(unsigned char *src, int w, int h, int color, RECTCORNER *black_rtcorner)
{
	memset(black_rtcorner, 0, sizeof(RECTCORNER));

	unsigned char *binaryImg = new unsigned char [w*h];

	// --- binarize
	memset(binaryImg, 0, w*h);
	int blackCenter = 0;
	int whiteCenter = 0;
	int GT = 100;
	if(-1!=getHistogramGT(src, w, h, color, &blackCenter, &whiteCenter)){
		GT = (blackCenter+whiteCenter)/2;
	}
	fixedGTBinary(src, w, h, color, binaryImg, GT);

	int position_x = 0;
	vector<int> position_y;
	find_caribration_line_area(binaryImg, w, h, &position_x, &position_y);

	bool flag = false;
	int point = (int)position_y.size();
	if( point == 2 ){
		flag = true;
		POINT curPt;

		// --- black rtcorner
		curPt.x = position_x;
		curPt.y = position_y[0];
		// top-left
		traceCornerLeft(binaryImg, w, h, true, curPt, 0, 255, black_rtcorner->lefttop);
		// top-right
		traceCornerRight(binaryImg, w, h, true, curPt, 0, 255, black_rtcorner->righttop);

		curPt.x = position_x;
		curPt.y = position_y[1];
		// bottom-left
		traceCornerLeft(binaryImg, w, h, true, curPt, 255, 255, black_rtcorner->leftbottom);
		// bottom-right
		traceCornerRight(binaryImg, w, h, true, curPt, 255, 255, black_rtcorner->rightbottom);
	}

	delete [] binaryImg;

	return flag;
}

static float lineIntersection(float line1_x1, float line1_y1, float line1_x2, float line1_y2, float line2_x1, float line2_y1, float line2_x2, float line2_y2)
{
	float m1 = 0.0f;
	float m2 = 0.0f;
	float tol = 1e-06f;
	float a1 = 0.0f;
	float a2 = 0.0f;
	float angle = 0.0f;

	if( abs(line1_x1 - line1_x2) > tol ){
		m1 = (line1_y1 - line1_y2) / (line1_x1 - line1_x2);
		if( abs(line2_x1 - line2_x2) > tol ){
			m2 = (line2_y1 - line2_y2) / (line2_x1 - line2_x2);
			if( abs(m1 - m2) > tol ){
				a1 = atan(m1) * 180 / PI;
				a2 = atan(m2) * 180 / PI;
				if(m1>=0.0f){
					angle = a1 - a2;
				}
				else{
					angle = a2 - a1;
				}
			}
		}
	}
	else{
		// vertical line
		if( abs(line2_x1 - line2_x2) > tol ){
			m2 = (line2_y1 - line2_y2) / (line2_x1 - line2_x2);
			a2 = atan(m2) * 180 / PI;
			angle = 90.0f - a2;
		}
	}

	return angle;
}

bool CEdgeFinder::CalculateFOV(IMAGE *xImg, CALIBRATION &xCali)
{
	xCali.x_fov = 1.0f;
	xCali.y_fov = 1.0f;
	xCali.area_fov = 1.0f;

	RECTCORNER black_rtcorner;
	memset(&black_rtcorner, 0, sizeof(RECTCORNER));
	bool flag = findCaliPaperCornerArea(xImg->ptr, xImg->data_w, xImg->data_h, GetImagePixelBytes(xImg), &black_rtcorner);

	if(!flag){ return false; }

	float tol = 1e-06f;

	float diffx = (float)(black_rtcorner.lefttop.x-black_rtcorner.righttop.x);
	float diffy = (float)(black_rtcorner.lefttop.y-black_rtcorner.righttop.y);
	float dis_x1 = sqrt(diffx*diffx+diffy*diffy);

	diffx = (float)(black_rtcorner.leftbottom.x-black_rtcorner.rightbottom.x);
	diffy = (float)(black_rtcorner.leftbottom.y-black_rtcorner.rightbottom.y);
	float dis_x2 = sqrt(diffx*diffx+diffy*diffy);

	diffx = (float)(black_rtcorner.lefttop.x-black_rtcorner.leftbottom.x);
	diffy = (float)(black_rtcorner.lefttop.y-black_rtcorner.leftbottom.y);
	float dis_y1 = sqrt(diffx*diffx+diffy*diffy);

	diffx = (float)(black_rtcorner.righttop.x-black_rtcorner.rightbottom.x);
	diffy = (float)(black_rtcorner.righttop.y-black_rtcorner.rightbottom.y);
	float dis_y2 = sqrt(diffx*diffx+diffy*diffy);
	
	if(dis_x1>tol && dis_y1>tol){
		xCali.x_fov = m_fCaliWidth/dis_x1;
		xCali.y_fov = m_fCaliHeight/dis_y1;
		xCali.area_fov = xCali.x_fov*xCali.y_fov;

		xCali.fXSizeTop = dis_x1*xCali.x_fov;
		xCali.fXSizeBottom = dis_x2*xCali.x_fov;
		xCali.fYSizeLeft = dis_y1*xCali.y_fov;
		xCali.fYSizeRight = dis_y2*xCali.y_fov;
		xCali.fRotationAngle = lineIntersection(
				(float)black_rtcorner.lefttop.x, (float)black_rtcorner.lefttop.y,
				(float)black_rtcorner.righttop.x, (float)black_rtcorner.righttop.y,
				100.0f, 100.0f,
				xImg->data_w-100.0f, 100.0f);

		return true;
	}

	return false;
}

void CEdgeFinder::CreateMinMax(IMAGE *xImg)
{
	if(verifyIMAGE(xImg)==false){ return; }
	
	int color = GetImagePixelBytes(xImg);
	if(m_xMaxMap.ptr!=NULL){
		delete [] m_xMaxMap.ptr;
	}
	if(m_xMinMap.ptr!=NULL){
		delete [] m_xMinMap.ptr;
	}
	m_xMaxMap.ptr = new unsigned char[xImg->data_w*xImg->data_h*color];
	m_xMinMap.ptr = new unsigned char[xImg->data_w*xImg->data_h*color];
	memcpy(m_xMaxMap.ptr, xImg->ptr, xImg->data_w*xImg->data_h*color);
	memcpy(m_xMinMap.ptr, xImg->ptr, xImg->data_w*xImg->data_h*color);

	ImageInit(&m_xMaxMap, xImg->type, m_xMaxMap.ptr, xImg->data_w, xImg->data_h);
	ImageInit(&m_xMinMap, xImg->type, m_xMinMap.ptr, xImg->data_w, xImg->data_h);

	int window = 5;
	int halfWindow = window/2;
	int defaultTH = 9;
	
	if(color==3){
		for(int j=0; j<xImg->data_h; j++){
			for(int i=0; i<xImg->data_w; i++){
				for(int jj=-halfWindow; jj<=halfWindow; jj++){
					if(j+jj>=0 && j+jj<xImg->data_h){
						for(int ii=-halfWindow; ii<=halfWindow; ii++){
							if(i+ii>=0 && i+ii<xImg->data_w){
								int val = xImg->ptr[(j*xImg->data_w+i)*color];
								int maxval = m_xMaxMap.ptr[((j+jj)*xImg->data_w+i+ii)*color];
								int minval = m_xMinMap.ptr[((j+jj)*xImg->data_w+i+ii)*color];
								maxval = max(maxval, val+defaultTH);
								minval = max(minval, val-defaultTH);
								m_xMaxMap.ptr[((j+jj)*xImg->data_w+i+ii)*color] = capInt(maxval, 0, 255);
								m_xMinMap.ptr[((j+jj)*xImg->data_w+i+ii)*color] = capInt(minval, 0, 255);

								val = xImg->ptr[(j*xImg->data_w+i)*color+1];
								maxval = m_xMaxMap.ptr[((j+jj)*xImg->data_w+i+ii)*color+1];
								minval = m_xMinMap.ptr[((j+jj)*xImg->data_w+i+ii)*color+1];
								maxval = max(maxval, val+defaultTH);
								minval = max(minval, val-defaultTH);
								m_xMaxMap.ptr[((j+jj)*xImg->data_w+i+ii)*color+1] = capInt(maxval, 0, 255);
								m_xMinMap.ptr[((j+jj)*xImg->data_w+i+ii)*color+1] = capInt(minval, 0, 255);

								val = xImg->ptr[(j*xImg->data_w+i)*color+2];
								maxval = m_xMaxMap.ptr[((j+jj)*xImg->data_w+i+ii)*color+2];
								minval = m_xMinMap.ptr[((j+jj)*xImg->data_w+i+ii)*color+2];
								maxval = max(maxval, val+defaultTH);
								minval = max(minval, val-defaultTH);
								m_xMaxMap.ptr[((j+jj)*xImg->data_w+i+ii)*color+2] = capInt(maxval, 0, 255);
								m_xMinMap.ptr[((j+jj)*xImg->data_w+i+ii)*color+2] = capInt(minval, 0, 255);
							}
						}
					}
				}
			}
		}
	}
	else if(color==1){
		for(int j=0; j<xImg->data_h; j++){
			for(int i=0; i<xImg->data_w; i++){
				for(int jj=-halfWindow; jj<=halfWindow; jj++){
					if(j+jj>=0 && j+jj<xImg->data_h){
						for(int ii=-halfWindow; ii<=halfWindow; ii++){
							if(i+ii>=0 && i+ii<xImg->data_w){
								int val = xImg->ptr[j*xImg->data_w+i];
								int maxval = m_xMaxMap.ptr[(j+jj)*xImg->data_w+i+ii];
								int minval = m_xMinMap.ptr[(j+jj)*xImg->data_w+i+ii];
								maxval = max(maxval, val+defaultTH);
								minval = max(minval, val-defaultTH);
								m_xMaxMap.ptr[(j+jj)*xImg->data_w+i+ii] = capInt(maxval, 0, 255);
								m_xMinMap.ptr[(j+jj)*xImg->data_w+i+ii] = capInt(minval, 0, 255);
							}
						}
					}
				}
			}
		}
	}
}

void CEdgeFinder::SetGolden(IMAGE *xImg)
{
	if(verifyIMAGE(xImg)==false){ return; }

	int color = GetImagePixelBytes(xImg);
	if(m_xGolden.ptr!=NULL){
		delete [] m_xGolden.ptr;
	}
	m_xGolden.ptr = new unsigned char[xImg->data_w*xImg->data_h*color];
	memcpy(m_xGolden.ptr, xImg->ptr, xImg->data_w*xImg->data_h*color);

	ImageInit(&m_xGolden, xImg->type, m_xGolden.ptr, xImg->data_w, xImg->data_h);

	CreateMinMax(xImg);
}

bool CEdgeFinder::CheckDefect(IMAGE *xSample, int shiftX, int shiftY)
{
	if(verifyIMAGE(xSample)==false){ return true; }

	if(xSample->data_w!=m_xGolden.data_w){ return true; }
	if(xSample->data_h!=m_xGolden.data_h){ return true; }
	if(xSample->type!=m_xGolden.type){ return true; }

	if(m_xDefectParam.iDefectTH==INT_MAX || m_xDefectParam.iDefectPnt==INT_MAX){ return true; }

	int color = GetImagePixelBytes(&m_xGolden);

	int defectCount = 0;
	if(color==3){
		for(int j=0; j<m_xGolden.data_h; j++){
			if(j-shiftY>=0 && j-shiftY<m_xGolden.data_h){
				for(int i=0; i<m_xGolden.data_w; i++){
					if(i-shiftX>=0 && i-shiftX<m_xGolden.data_w){
						bool bDefect = false;
						int goldval = m_xGolden.ptr[((j-shiftY)*m_xGolden.data_w+i-shiftX)*3+0];
						int maxval = m_xMaxMap.ptr[((j-shiftY)*m_xGolden.data_w+i-shiftX)*3+0];
						int minval = m_xMinMap.ptr[((j-shiftY)*m_xGolden.data_w+i-shiftX)*3+0];
						int testval = xSample->ptr[(j*m_xGolden.data_w+i)*3+0];
						if( (abs(testval-goldval) > m_xDefectParam.iDefectTH)
						 && (testval>maxval || testval>minval)
						){
							bDefect = true;
						}

						goldval = m_xGolden.ptr[((j-shiftY)*m_xGolden.data_w+i-shiftX)*3+1];
						maxval = m_xMaxMap.ptr[((j-shiftY)*m_xGolden.data_w+i-shiftX)*3+1];
						minval = m_xMinMap.ptr[((j-shiftY)*m_xGolden.data_w+i-shiftX)*3+1];
						testval = xSample->ptr[(j*m_xGolden.data_w+i)*3+1];
						if( (abs(testval-goldval) > m_xDefectParam.iDefectTH)
						 && (testval>maxval || testval>minval)
						){
							bDefect = true;
						}

						goldval = m_xGolden.ptr[((j-shiftY)*m_xGolden.data_w+i-shiftX)*3+2];
						maxval = m_xMaxMap.ptr[((j-shiftY)*m_xGolden.data_w+i-shiftX)*3+2];
						minval = m_xMinMap.ptr[((j-shiftY)*m_xGolden.data_w+i-shiftX)*3+2];
						testval = xSample->ptr[(j*m_xGolden.data_w+i)*3+2];
						if( (abs(testval-goldval) > m_xDefectParam.iDefectTH)
						 && (testval>maxval || testval>minval)
						){
							bDefect = true;
						}

						if(bDefect){
							defectCount++;
						}
					}
				}
			}
		}
	}
	else if(color==1){
		for(int j=0; j<m_xGolden.data_h; j++){
			if(j-shiftY>=0 && j-shiftY<m_xGolden.data_h){
				for(int i=0; i<m_xGolden.data_w; i++){
					if(i-shiftX>=0 && i-shiftX<m_xGolden.data_w){	
						int goldval = m_xGolden.ptr[(j-shiftY)*m_xGolden.data_w+i-shiftX];
						int maxval = m_xMaxMap.ptr[(j-shiftY)*m_xGolden.data_w+i-shiftX];
						int minval = m_xMinMap.ptr[(j-shiftY)*m_xGolden.data_w+i-shiftX];
						int testval = xSample->ptr[j*m_xGolden.data_w+i];
						if( (abs(testval-goldval) > m_xDefectParam.iDefectTH)
						 && (testval>maxval || testval>minval)
						){
							defectCount++;
						}
					}
				}
			}
		}
	}

	if(defectCount >= m_xDefectParam.iDefectPnt){
		return false;
	}

	return true;
}