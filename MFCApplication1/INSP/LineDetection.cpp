#include "LineDetection.h"
#include "AoiBaseOp.h"//saveimage

CLineDetection::CLineDetection()
{
	ImageInit(&m_xGolden, IMAGE_TYPE_NOTYPE, NULL, 0, 0);
	m_iWidthTH = 15;
	m_fGreyLevelCountTH = 0.5f;
	m_iGreyLevelTH = 30;
	m_iLineSelectedIdx = 0;
	m_lineType = LD_LINE_TYPE_UNKNOWN;
	memset(&m_xInspParam, 0, sizeof(m_xInspParam));
	memset(&m_xEdgeLineRef, 0, sizeof(m_xEdgeLineRef));
	memset(&m_xGoldenResult, 0, sizeof(m_xGoldenResult));
}

CLineDetection::~CLineDetection()
{
	if(m_xGolden.ptr!=NULL){
		delete [] m_xGolden.ptr;
	}
	ImageInit(&m_xGolden, IMAGE_TYPE_NOTYPE, NULL, 0, 0);
}

void CLineDetection::setGolden(IMAGE *pImg)
{
	if(m_xGolden.ptr!=NULL){
		delete [] m_xGolden.ptr;
	}
	ImageInit(&m_xGolden, IMAGE_TYPE_NOTYPE, NULL, 0, 0);

	if( verifyIMAGE(pImg)==false ){ return; }

	unsigned char *src = pImg->ptr;
	int w = pImg->data_w;
	int h = pImg->data_h;
	int color = GetImagePixelBytes(pImg);

	m_xGolden.ptr = new unsigned char[w*h*color];
	memcpy(m_xGolden.ptr, pImg->ptr, w*h*color);
	ImageInit(&m_xGolden, color==3?IMAGE_TYPE_BGR24:IMAGE_TYPE_MONO8, m_xGolden.ptr, w, h);
}

void getImageProjection(IMAGE *pImg, unsigned char *horiProjection, unsigned char *vertProjection)
{
	if( verifyIMAGE(pImg)==false ){ return; }

	unsigned char *src = pImg->ptr;
	int w = pImg->data_w;
	int h = pImg->data_h;
	int color = GetImagePixelBytes(pImg);

	if(color==3){
		if(horiProjection!=NULL){
			for(int j=0; j<h; j++){
				int sum = 0;
				for(int i=0; i<w; i++){
					sum += (src[(j*w+i)*3+0]+src[(j*w+i)*3+1]+src[(j*w+i)*3+2])/3;
				}
				horiProjection[j] = sum/w;
			}
		}

		if(vertProjection!=NULL){
			for(int i=0; i<w; i++){
				int sum = 0;
				for(int j=0; j<h; j++){
					sum += (src[(j*w+i)*3+0]+src[(j*w+i)*3+1]+src[(j*w+i)*3+2])/3;
				}
				vertProjection[i] = sum/h;
			}
		}
	}
	else{
		if(horiProjection!=NULL){
			for(int j=0; j<h; j++){
				int sum = 0;
				for(int i=0; i<w; i++){
					sum += src[j*w+i];
				}
				horiProjection[j] = sum/w;
			}
		}

		if(vertProjection!=NULL){
			for(int i=0; i<w; i++){
				int sum = 0;
				for(int j=0; j<h; j++){
					sum += src[j*w+i];
				}
				vertProjection[i] = sum/h;
			}
		}
	}
}

void findStraightLine(unsigned char *strip, int length, vector<LINE_INFO> *vLineInfo)
{
	int dynamicRange = 30;
	int observation = 10;

//	int *diff_list = new int[observation];
//	int debug_strip[10];
	int diff_list[10];//observation
	for(int i=0; i<length-observation; i++){
		for(int j=0; j<observation; j++){
			diff_list[j] = abs(strip[i]-strip[i+j+1]);
		//	debug_strip[j] = strip[i+j];
		}
		bool flag = true;
		for(int j=0; j<observation-1; j++){
			if(diff_list[j] > diff_list[j+1]){
				flag = false;
			}
		}
		if(flag && diff_list[observation-1]>=dynamicRange){
			LINE_INFO lineInfo;
			memset(&lineInfo, 0, sizeof(lineInfo));

			lineInfo.greyLevel_0 = 0;
			lineInfo.greyLevel_1 = 255;
			for(int k=0; k<observation*2; k++){
				if(i+k < length){
					lineInfo.greyLevel_0 = max(lineInfo.greyLevel_0, strip[i+k]);
					lineInfo.greyLevel_1 = min(lineInfo.greyLevel_1, strip[i+k]);
				}
			}
			lineInfo.position = (float)i+observation-1;

			vLineInfo->push_back(lineInfo);
			i+=observation*2;
		}
	}

//	delete [] diff_list;
}

typedef struct _GRADIENT_INFO {
	int x;
	int y;
	float gradient;
} GRADIENT_INFO;

void calculateSubPixelEdge_hori(IMAGE *img, vector<LINE_INFO> *vLineInfo)
{
	if(verifyIMAGE(img)==false){ return; }
	if(vLineInfo==NULL){ return; }

	int size = (int)vLineInfo->size();
	if(size <= 0){ return; }

	unsigned char *ptr = img->ptr;
	int w = img->data_w;
	int h = img->data_h;
	int color = GetImagePixelBytes(img);

	float M_x[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
	float M_y[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

	int observationLength = 15;
	int x_in = w/2;
	for(int c=0; c<size; c++){
		int y_start = (int)(*vLineInfo)[c].position-observationLength;
		int y_end = (int)(*vLineInfo)[c].position+observationLength;

		y_start = y_start < 0 ? 0 : y_start;
		y_start = y_start >= img->data_h ? img->data_h-1 : y_start;

		y_end = y_end < 0 ? 0 : y_end;
		y_end = y_end >= img->data_h ? img->data_h-1 : y_end;


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
					(*vLineInfo)[c].position = tao;
				}
			}
		}
	}
}

void calculateSubPixelEdge_vert(IMAGE *img, vector<LINE_INFO> *vLineInfo)
{
	if(verifyIMAGE(img)==false){ return; }
	if(vLineInfo==NULL){ return; }

	int size = (int)vLineInfo->size();
	if(size <= 0){ return; }
	
	unsigned char *ptr = img->ptr;
	int w = img->data_w;
	int h = img->data_h;
	int color = GetImagePixelBytes(img);

	float M_x[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
	float M_y[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

	int observationLength = 15;
	int y_in = h/2;
	for(int c=0; c<size; c++){
		int x_start = (int)(*vLineInfo)[c].position-observationLength;
		int x_end = (int)(*vLineInfo)[c].position+observationLength;

		x_start = x_start < 0 ? 0 : x_start;
		x_start = x_start >= img->data_w ? img->data_w-1 : x_start;

		x_end = x_end < 0 ? 0 : x_end;
		x_end = x_end >= img->data_w ? img->data_w-1 : x_end;

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
					(*vLineInfo)[c].position = tao;
				}
			}
		}

	}
}

void CLineDetection::findLineInfo(IMAGE *pImg, vector<LINE_INFO> *vLineInfo)
{
	if( verifyIMAGE(pImg)==false ){ return; }

	unsigned char *src = pImg->ptr;
	int w = pImg->data_w;
	int h = pImg->data_h;
	int color = GetImagePixelBytes(pImg);

	unsigned char *horiProjection = new unsigned char[h];
	unsigned char *vertProjection = new unsigned char[w];

	getImageProjection(pImg, horiProjection, vertProjection);

	if(m_lineType == LD_LINE_TYPE_HORIZONTAL){
		findStraightLine(horiProjection, h, vLineInfo);
		calculateSubPixelEdge_hori(pImg, vLineInfo);
	}
	else if(m_lineType == LD_LINE_TYPE_VERTICAL){
		findStraightLine(vertProjection, w, vLineInfo);
		calculateSubPixelEdge_vert(pImg, vLineInfo);
#ifdef _DEBUG
		unsigned char *debug = new unsigned char[w*h];
		for(int j=0; j<h; j++){
			for(int i=0; i<w; i++){
				debug[j*w+i] = vertProjection[i];
			}
		}
		saveimage(L"output\\vertProjection.bmp", debug, w, -h, 1, FALSE);
		delete [] debug;
#endif
	}

	delete [] horiProjection;
	delete [] vertProjection;
}

void CLineDetection::Calibrate()
{


}

bool CLineDetection::checkGreyLevel(IMAGE *pImg, LD_INSP_PARAM xInspParam, int start, int end)
{
	if( verifyIMAGE(pImg)==false ){ return false; }
	if( xInspParam.bUseGreyLevel==false ){ return false; }

	float checkTH = 0.5f;//50%
	int greyLevel = xInspParam.greyLevel;

	unsigned char *src = pImg->ptr;
	int w = pImg->data_w;
	int h = pImg->data_h;
	int color = GetImagePixelBytes(pImg);

	int count = 0;
	int total = 0;
	if(m_lineType == LD_LINE_TYPE_VERTICAL){
		if(color==3){
			for(int j=0; j<h; j++){
				for(int i=start; i<end; i++){
					int val = (src[(j*w+i)*3+0]+src[(j*w+i)*3+1]+src[(j*w+i)*3+2])/3;
					if(val>greyLevel-m_iGreyLevelTH && val<greyLevel+m_iGreyLevelTH){
						count++;
					}
					total++;
				}
			}
		}
		else{
			for(int j=0; j<h; j++){
				for(int i=start; i<end; i++){
					int val = src[j*w+i];
					if(val>greyLevel-m_iGreyLevelTH && val<greyLevel+m_iGreyLevelTH){
						count++;
					}
					total++;
				}
			}
		}
	}
	else if(m_lineType == LD_LINE_TYPE_HORIZONTAL){
		if(color==3){
			for(int j=start; j<end; j++){
				for(int i=0; i<w; i++){
					int val = (src[(j*w+i)*3+0]+src[(j*w+i)*3+1]+src[(j*w+i)*3+2])/3;
					if(val>greyLevel-m_iGreyLevelTH && val<greyLevel+m_iGreyLevelTH){
						count++;
					}
					total++;
				}
			}
		}
		else{
			for(int j=start; j<end; j++){
				for(int i=0; i<w; i++){
					int val = src[j*w+i];
					if(val>greyLevel-m_iGreyLevelTH && val<greyLevel+m_iGreyLevelTH){
						count++;
					}
					total++;
				}
			}
		}
	}

	if(total>0){
		if(count*1.0f/total > m_fGreyLevelCountTH){
			return true;
		}
	}

	return false;
}

void shrinkImage(unsigned char *src, int w, int h, int color, unsigned char *dst, int shrinkRateX, int shrinkRateY)
{
	if(color!=1){ return; }

	int shrinkW = w / shrinkRateX;
	int shrinkH = h / shrinkRateY;

	for(int j=0; j<shrinkH; j++){
		for(int i=0; i<shrinkW; i++){
			int x = i*shrinkRateX;
			int y = j*shrinkRateY;
			if(x>=0 && x<w && y>=0 && y<h){
				dst[j*shrinkW+i] = src[y*w+x];
			}
		}
	}
#ifdef _DEBUG
	saveimage(L"shrink.bmp", dst, shrinkW, -shrinkH, 1, FALSE);
#endif
}

float computeCorrelation_fast(unsigned char *golden, unsigned char *sample, int w, int h, int color,
	                          RECT searchRange, RECT anchorRange, POINT &ptResult)
{
	ptResult.x = 0;
	ptResult.y = 0;

	if(color != 1){ return -2.0f; }

	int searchW = searchRange.right - searchRange.left;
	int searchH = searchRange.bottom - searchRange.top;

	int anchorW = anchorRange.right - anchorRange.left;
	int anchorH = anchorRange.bottom - anchorRange.top;

	int candX = searchW - anchorW + 1;
	int candY = searchH - anchorH + 1;

	if(searchW < anchorW || searchH < anchorH){ return -2.0f; }

	unsigned char *goldenAnchor = new unsigned char[anchorW*anchorH];

	// --- golden anchor
	float xi_bar = 0.0f;
	float xi_sum = 0.0f;
	float xi_xb = 0.0f;

	for(int j=0; j<anchorH; j++){
		for(int i=0; i<anchorW; i++){
			int x = anchorRange.left + i;
			int y = anchorRange.top + j;
			unsigned char val = golden[y*w+x];
			goldenAnchor[j*anchorW+i] = val;
			xi_bar += val;
		}
	}
	xi_bar /= (anchorW*anchorH);

#ifdef _DEBUG
	saveimage(L"goldenAnchor.bmp", goldenAnchor, anchorW, -anchorH, 1, FALSE);
#endif

	for(int j=0; j<anchorH; j++){
		for(int i=0; i<anchorW; i++){
			int x = i;
			int y = j;
			float val = goldenAnchor[y*anchorW+x];
			xi_sum += val;
			xi_xb += (xi_bar - val)*(xi_bar - val);
		}
	}
	float xi_xb_sqrt = sqrt(xi_xb);

	float yi_sum0 = 0.0f;
	float yi_sum = 0.0f;
	float yi_pow2_sum0 = 0.0f;
	float yi_pow2_sum = 0.0f;

	float testAVG0 = 0.0f;
	float yi_bar = 0.0f;
	float yi_yb = 0.0f;

	float fincorr = -2.0f;
	for(int i=0; i<candX; i++){
		for(int j=0; j<candY; j++){
			// --- testAVG
			if(i==0 && j==0){
				yi_sum = 0.0f;
				for(int jj=0; jj<anchorH; jj++){
					for(int ii=0; ii<anchorW; ii++){
						int x = searchRange.left + i + ii;
						int y = searchRange.top + j + jj;
						if(x>=0 && x<w && y>=0 && y<h){
							float val = sample[y*w+x];
							yi_sum += val;
							yi_pow2_sum += val*val;
						}
					}
				}
				yi_sum0 = yi_sum;
				yi_pow2_sum0 = yi_pow2_sum;
			}
			else if(j==0){
				for(int jj=0; jj<anchorH; jj++){
					int x = searchRange.left + i - 1;
					int y = searchRange.top + j + jj;
					if(x>=0 && x<w && y>=0 && y<h){
						float val = sample[y*w+x];
						yi_sum0 -= val;
						yi_pow2_sum0 -= (val*val);
					}
				}
				for(int jj=0; jj<anchorH; jj++){
					int x = searchRange.left + i + anchorW - 1;
					int y = searchRange.top + j + jj;
					if(x>=0 && x<w && y>=0 && y<h){
						float val = sample[y*w+x];
						yi_sum0 += val;
						yi_pow2_sum0 += (val*val);
					}
				}
				yi_sum = yi_sum0;
				yi_pow2_sum = yi_pow2_sum0;
			}
			else{
				for(int ii=0; ii<anchorW; ii++){
					int x = searchRange.left + i + ii;
					int y = searchRange.top + j - 1;
					if(x>=0 && x<w && y>=0 && y<h){
						float val = sample[y*w+x];
						yi_sum -= val;
						yi_pow2_sum -= val*val;
					}
				}
				for(int ii=0; ii<anchorW; ii++){
					int x = searchRange.left + i + ii;
					int y = searchRange.top + j + anchorH - 1;
					if(x>=0 && x<w && y>=0 && y<h){
						float val = sample[y*w+x];
						yi_sum += val;
						yi_pow2_sum += val*val;
					}
				}
			}

			yi_bar = yi_sum / (anchorW*anchorH);
			yi_yb = yi_pow2_sum - 2.0f*yi_bar*yi_sum + anchorW*anchorH*yi_bar*yi_bar;
			
			// --- inner product
			float xy_sum = 0.0f;
			for(int jj=0; jj<anchorH; jj++){
				for(int ii=0; ii<anchorW; ii++){
					int x = searchRange.left + i + ii;
					int y = searchRange.top + j + jj;
					if(x>=0 && x<w && y>=0 && y<h){
						float testVal = sample[y*w+x];
						float goldVal = goldenAnchor[jj*anchorW+ii];

						xy_sum += testVal*goldVal;
					}
				}
			}


			float yi_yb_sqrt = sqrt(yi_yb);
			float corr = (xy_sum - yi_bar*xi_sum - xi_bar*yi_sum + anchorW*anchorH*xi_bar*yi_bar)/(xi_xb_sqrt*yi_yb_sqrt);
			if(corr > fincorr){
				fincorr = corr;
				ptResult.x = i;
				ptResult.y = j;
			}
		}
	}

	ptResult.x += searchRange.left;
	ptResult.y += searchRange.top;

	delete [] goldenAnchor;

	return fincorr;
}

void CLineDetection::setInspParam(IMAGE *pGoldenImg, LD_INSP_PARAM xInspParam)
{
	if(m_lineType == LD_LINE_TYPE_UNKNOWN){
		m_lineType = DetectLineType(pGoldenImg);
	}
	setGolden(pGoldenImg);
	memcpy(&m_xInspParam, &xInspParam, sizeof(m_xInspParam));

	// get golden result
	Insp(pGoldenImg, &m_xGoldenResult);
}

void CLineDetection::getGoldenResult(LD_RESULT *p_xResult)
{
	if(p_xResult){
		memcpy(p_xResult, &m_xGoldenResult, sizeof(m_xGoldenResult));
	}
}

void CLineDetection::EdgeModePreprocess(IMAGE *pGoldenImg, vector<LINE_INFO> *vLineInfo)
{
	if(m_lineType == LD_LINE_TYPE_UNKNOWN){
		m_lineType = DetectLineType(pGoldenImg);
	}
	findLineInfo(pGoldenImg, vLineInfo);
}

float CLineDetection::findAnchor(IMAGE *pSample, POINT &ptResult)
{
	if( verifyIMAGE(&m_xGolden)==false ){ return -2.0f; }
	if( verifyIMAGE(pSample)==false ){ return -2.0f; }

	unsigned char *src = pSample->ptr;
	int w = pSample->data_w;
	int h = pSample->data_h;
	int color = GetImagePixelBytes(pSample);

	RECT searchRange;
	searchRange.left = 0;
	searchRange.top = 0;
	searchRange.right = w;
	searchRange.bottom = h;

	int anchorW = min(50, m_xGolden.data_w);
	int anchorH = min(100, m_xGolden.data_h);
	if(m_lineType == LD_LINE_TYPE_VERTICAL){
		anchorW = min(100, m_xGolden.data_w);
		anchorH = min(50, m_xGolden.data_h);
	}
	if(anchorW<=0 || anchorH<=0){ return -2.0f; }

	RECT anchorRange;
	anchorRange.left   = m_xInspParam.ptAnchor.x-anchorW/2;
	anchorRange.top    = m_xInspParam.ptAnchor.y-anchorH/2;
	anchorRange.right  = m_xInspParam.ptAnchor.x+anchorW/2;
	anchorRange.bottom = m_xInspParam.ptAnchor.y+anchorH/2;

	if(anchorRange.left<0 || anchorRange.left>w){ return -2.0f; }
	if(anchorRange.right<0 || anchorRange.right>w){ return -2.0f; }
	if(anchorRange.top<0 || anchorRange.top>h){ return -2.0f; }
	if(anchorRange.bottom<0 || anchorRange.bottom>h){ return -2.0f; }

	int shrinkRateX = 4;
	int shrinkRateY = 4;

	RECT shrinkSearchRange;
	shrinkSearchRange.left   = searchRange.left   / shrinkRateX;
	shrinkSearchRange.top    = searchRange.top    / shrinkRateY;
	shrinkSearchRange.right  = searchRange.right  / shrinkRateX;
	shrinkSearchRange.bottom = searchRange.bottom / shrinkRateY;

	RECT shrinkAnchorRange;
	shrinkAnchorRange.left   = anchorRange.left   / shrinkRateX;
	shrinkAnchorRange.top    = anchorRange.top    / shrinkRateY;
	shrinkAnchorRange.right  = anchorRange.right  / shrinkRateX;
	shrinkAnchorRange.bottom = anchorRange.bottom / shrinkRateY;

	int shrinkW = w / shrinkRateX;
	int shrinkH = h / shrinkRateY;
	unsigned char *shrinkGolden = new unsigned char[shrinkW*shrinkH];
	unsigned char *shrinkSample = new unsigned char[shrinkW*shrinkH];

	shrinkImage(m_xGolden.ptr, w, h, color, shrinkGolden, shrinkRateX, shrinkRateY);
	shrinkImage(pSample->ptr, w, h, color, shrinkSample, shrinkRateX, shrinkRateY);

	float corr = computeCorrelation_fast(shrinkGolden, shrinkSample, shrinkW, shrinkH, color, shrinkSearchRange, shrinkAnchorRange, ptResult);

	// ------------
	// --- 2nd step: original image
	RECT searchRange_2ndstep;
	searchRange_2ndstep.left   = max(ptResult.x*shrinkRateX - 5, 0);
	searchRange_2ndstep.top    = max(ptResult.y*shrinkRateY - 5, 0);
	searchRange_2ndstep.right  = min(ptResult.x*shrinkRateX + 5 + anchorW, w);
	searchRange_2ndstep.bottom = min(ptResult.y*shrinkRateY + 5 + anchorH, h);

	corr = computeCorrelation_fast(m_xGolden.ptr, pSample->ptr, w, h, color, searchRange_2ndstep, anchorRange, ptResult);

	ptResult.x += anchorW/2;
	ptResult.y += anchorH/2;

	delete [] shrinkGolden;
	delete [] shrinkSample;
	// ------------

	return corr;
}

LD_INSP_STATUS CLineDetection::InspModeEdge(IMAGE *pSample, vector<LINE_INFO> *vLineInfo, LD_RESULT *pResult)
{
	LD_INSP_STATUS status = LD_INSP_EDGE_FAIL;
	if(vLineInfo->size() > m_xInspParam.lineSelectedIdx){
		int idx = m_xInspParam.lineSelectedIdx;
		LINE_INFO curLineInfo = (*vLineInfo)[idx];
		// do not check greyLevel
		if(m_xInspParam.xEdgeLineRef.greyLevel_0 == 0 && m_xInspParam.xEdgeLineRef.greyLevel_1 == 0){
			status = LD_INSP_SUCCESS;
		}
		// check greyLevel
		else if(curLineInfo.greyLevel_0 > m_xInspParam.xEdgeLineRef.greyLevel_0 - m_iGreyLevelTH
		&& curLineInfo.greyLevel_0 < m_xInspParam.xEdgeLineRef.greyLevel_0 + m_iGreyLevelTH
		&& curLineInfo.greyLevel_1 > m_xInspParam.xEdgeLineRef.greyLevel_1 - m_iGreyLevelTH
		&& curLineInfo.greyLevel_1 < m_xInspParam.xEdgeLineRef.greyLevel_1 + m_iGreyLevelTH){
			status = LD_INSP_SUCCESS;
		}

		if(status == LD_INSP_SUCCESS){
			pResult->xLineInfo = curLineInfo;
			pResult->lineShift = curLineInfo.position - m_xGoldenResult.xLineInfo.position;
		}
	}

	return status;
}

LD_INSP_STATUS CLineDetection::InspModeColor(IMAGE *pSample, vector<LINE_INFO> *vLineInfo, LD_RESULT *pResult)
{
	LD_INSP_STATUS status = LD_INSP_COLOR_FAIL;
	int size = (int)vLineInfo->size();
	for(int i=0; i<size-1; i++){
		int lineWidth = (int)abs((*vLineInfo)[i].position-(*vLineInfo)[i+1].position);
		bool bPair = false;
		if(lineWidth>m_xInspParam.colorBarWidth-m_iWidthTH && lineWidth<m_xInspParam.colorBarWidth+m_iWidthTH){
			if(m_xInspParam.bUseGreyLevel){
				if(checkGreyLevel(pSample, m_xInspParam, (int)(*vLineInfo)[i].position, (int)(*vLineInfo)[i+1].position)){
					bPair = true;
				}
			}
			else{
				bPair = true;
			}
		}
		if(bPair){
			pResult->xPairLine = std::make_pair((*vLineInfo)[i], (*vLineInfo)[i+1]);
			pResult->lineShift = pResult->xPairLine.first.position - m_xGoldenResult.xPairLine.first.position;
			status = LD_INSP_SUCCESS;
			break;
		}
	}

	return status;
}

LD_INSP_STATUS CLineDetection::InspModeAnchor(IMAGE *pSample, LD_RESULT *pResult)
{
	LD_INSP_STATUS status = LD_INSP_ANCHOR_FAIL;
	float corr = findAnchor(pSample, pResult->ptAnchor);
	if(corr > 0.7f){
		if(m_lineType == LD_LINE_TYPE_VERTICAL){
			pResult->lineShift = (float)pResult->ptAnchor.x - m_xGoldenResult.ptAnchor.x;
		}
		else{
			pResult->lineShift = (float)pResult->ptAnchor.y - m_xGoldenResult.ptAnchor.y;
		}
		status = LD_INSP_SUCCESS;
	}

	return status;
}

LD_INSP_STATUS CLineDetection::Insp(IMAGE *pSample, LD_RESULT *pResult)
{
	LD_INSP_STATUS status = LD_INSP_SUCCESS;
	if( verifyIMAGE(pSample)==false ){
		pResult->status = LD_SAMPLE_IMG_ERROR;
		return LD_SAMPLE_IMG_ERROR;
	}

	pResult->inspMode = m_xInspParam.inspMode;

	if(m_lineType == LD_LINE_TYPE_UNKNOWN){
		if( verifyIMAGE(&m_xGolden)==true ){
			m_lineType = DetectLineType(&m_xGolden);
		}
		else{
			m_lineType = DetectLineType(pSample);
		}
	}

	// --- mode 1. find simple edge
	vector<LINE_INFO> vLineInfo;
	if(m_xInspParam.inspMode != LD_INSP_MODE_ANCHOR){
		findLineInfo(pSample, &vLineInfo);
		if(m_xInspParam.inspMode == LD_INSP_MODE_UNKNOWN || m_xInspParam.inspMode == LD_INSP_MODE_EDGE){
			status = InspModeEdge(pSample, &vLineInfo, pResult);
		}
	}

	// --- mode 2. find specific length pair
	if(m_xInspParam.inspMode == LD_INSP_MODE_COLOR){
		status = InspModeColor(pSample, &vLineInfo, pResult);
	}

	// --- mode 3. find specific anchor position
	if(m_xInspParam.inspMode == LD_INSP_MODE_ANCHOR){
		status = InspModeAnchor(pSample, pResult);
	}

	pResult->status = status;
	return status;
}

LD_LINE_TYPE CLineDetection::DetectLineType(IMAGE *pImg)
{
	LD_LINE_TYPE lineType = LD_LINE_TYPE_UNKNOWN;
	if( verifyIMAGE(pImg)==false ){ return lineType; }

	unsigned char *src = pImg->ptr;
	int w = pImg->data_w;
	int h = pImg->data_h;
	int color = GetImagePixelBytes(pImg);

	unsigned char *horiProjection = new unsigned char[h];
	unsigned char *vertProjection = new unsigned char[w];

	getImageProjection(pImg, horiProjection, vertProjection);

	// --- mode 1. find simple edge
	vector<LINE_INFO> vHoriLineInfo;
	findStraightLine(horiProjection, h, &vHoriLineInfo);

	vector<LINE_INFO> vVertLineInfo;
	findStraightLine(vertProjection, w, &vVertLineInfo);

	if(vHoriLineInfo.size() > vVertLineInfo.size()){
		lineType = LD_LINE_TYPE_HORIZONTAL;
	}
	else if(vHoriLineInfo.size() < vVertLineInfo.size()){
		lineType = LD_LINE_TYPE_VERTICAL;
	}

#ifdef _DEBUG
	// --- debug
	unsigned char *output = new unsigned char[w*h*3];
	for(int j=0; j<h; j++){
		for(int i=0; i<w; i++){
			output[(j*w+i)*3+0] = src[j*w+i];
			output[(j*w+i)*3+1] = src[j*w+i];
			output[(j*w+i)*3+2] = src[j*w+i];
		}
	}
	for(int i=0; i<(int)vHoriLineInfo.size(); i++){
		int y = (int)(vHoriLineInfo[i].position+0.5f);
		if(vHoriLineInfo[i].greyLevel_0>vHoriLineInfo[i].greyLevel_1){
			for(int j=0; j<w; j++){
				output[(y*w+j)*3+0] = 0;
				output[(y*w+j)*3+1] = 255;
				output[(y*w+j)*3+2] = 0;
			}

			y = (int)(vHoriLineInfo[i].position-1+0.5f);
			for(int j=0; j<w; j++){
				output[(y*w+j)*3+0] = 0;
				output[(y*w+j)*3+1] = 255;
				output[(y*w+j)*3+2] = 0;
			}

			y = (int)(vHoriLineInfo[i].position+1+0.5f);
			for(int j=0; j<w; j++){
				output[(y*w+j)*3+0] = 0;
				output[(y*w+j)*3+1] = 255;
				output[(y*w+j)*3+2] = 0;
			}
		}
		else if(vHoriLineInfo[i].greyLevel_0<vHoriLineInfo[i].greyLevel_1){
			for(int j=0; j<w; j++){
				output[(y*w+j)*3+0] = 0;
				output[(y*w+j)*3+1] = 0;
				output[(y*w+j)*3+2] = 255;
			}

			y = (int)(vHoriLineInfo[i].position-1+0.5f);
			for(int j=0; j<w; j++){
				output[(y*w+j)*3+0] = 0;
				output[(y*w+j)*3+1] = 0;
				output[(y*w+j)*3+2] = 255;
			}

			y = (int)(vHoriLineInfo[i].position+1+0.5f);
			for(int j=0; j<w; j++){
				output[(y*w+j)*3+0] = 0;
				output[(y*w+j)*3+1] = 0;
				output[(y*w+j)*3+2] = 255;
			}
		}
	}
	for(int i=0; i<(int)vVertLineInfo.size(); i++){
		int x = (int)(vVertLineInfo[i].position+0.5f);
		if(vVertLineInfo[i].greyLevel_0>vVertLineInfo[i].greyLevel_1){
			for(int j=0; j<h; j++){
				output[(j*w+x)*3+0] = 0;
				output[(j*w+x)*3+1] = 255;
				output[(j*w+x)*3+2] = 0;
			}

			x = (int)(vVertLineInfo[i].position-1+0.5f);
			for(int j=0; j<h; j++){
				output[(j*w+x)*3+0] = 0;
				output[(j*w+x)*3+1] = 255;
				output[(j*w+x)*3+2] = 0;
			}

			x = (int)(vVertLineInfo[i].position+1+0.5f);
			for(int j=0; j<h; j++){
				output[(j*w+x)*3+0] = 0;
				output[(j*w+x)*3+1] = 255;
				output[(j*w+x)*3+2] = 0;
			}
		}
		else if(vVertLineInfo[i].greyLevel_0<vVertLineInfo[i].greyLevel_1){
			for(int j=0; j<h; j++){
				output[(j*w+x)*3+0] = 0;
				output[(j*w+x)*3+1] = 0;
				output[(j*w+x)*3+2] = 255;
			}

			x = (int)(vVertLineInfo[i].position-1+0.5f);
			for(int j=0; j<h; j++){
				output[(j*w+x)*3+0] = 0;
				output[(j*w+x)*3+1] = 0;
				output[(j*w+x)*3+2] = 255;
			}

			x = (int)(vVertLineInfo[i].position+1+0.5f);
			for(int j=0; j<h; j++){
				output[(j*w+x)*3+0] = 0;
				output[(j*w+x)*3+1] = 0;
				output[(j*w+x)*3+2] = 255;
			}
		}
	}
	saveimage(L"output\\result1.bmp", output, w, -h, 3, FALSE);
	// ----------------------------------------------------------------------
#endif

	delete [] horiProjection;
	delete [] vertProjection;

	return lineType;
}

LD_LIGHT_SOURCE CLineDetection::DetectLightSource(IMAGE *pImgBright, IMAGE *pImgDark)
{
	LD_LIGHT_SOURCE lightSource = LD_LIGHT_SOURCE_UNKNOWN;
	if( verifyIMAGE(pImgBright)==false ){ return lightSource; }
	if( verifyIMAGE(pImgDark)==false ){ return lightSource; }

	unsigned char *src_1 = pImgBright->ptr;
	int w_1 = pImgBright->data_w;
	int h_1 = pImgBright->data_h;
	int color_1 = GetImagePixelBytes(pImgBright);

	unsigned char *src_2 = pImgDark->ptr;
	int w_2 = pImgDark->data_w;
	int h_2 = pImgDark->data_h;
	int color_2 = GetImagePixelBytes(pImgDark);

	unsigned char *horiProjection_1 = new unsigned char[h_1];
	unsigned char *vertProjection_1 = new unsigned char[w_1];
	unsigned char *horiProjection_2 = new unsigned char[h_2];
	unsigned char *vertProjection_2 = new unsigned char[w_2];

	getImageProjection(pImgBright, horiProjection_1, vertProjection_1);
	getImageProjection(pImgDark, horiProjection_2, vertProjection_2);

	vector<LINE_INFO> vHoriLineInfo_1;
	vector<LINE_INFO> vVertLineInfo_1;
	vector<LINE_INFO> vHoriLineInfo_2;
	vector<LINE_INFO> vVertLineInfo_2;

	findStraightLine(horiProjection_1, h_1, &vHoriLineInfo_1);
	findStraightLine(vertProjection_1, w_1, &vVertLineInfo_1);
	findStraightLine(horiProjection_2, h_2, &vHoriLineInfo_2);
	findStraightLine(vertProjection_2, w_2, &vVertLineInfo_2);

	int lineCount = 0;
	if(vHoriLineInfo_1.size() > vVertLineInfo_1.size()){
		lightSource = LD_LIGHT_SOURCE_BRIGHT;
		lineCount = (int)vHoriLineInfo_1.size();
	}
	else if(vHoriLineInfo_1.size() < vVertLineInfo_1.size()){
		lightSource = LD_LIGHT_SOURCE_BRIGHT;
		lineCount = (int)vVertLineInfo_1.size();
	}

	if(vHoriLineInfo_2.size() > lineCount){
		lightSource = LD_LIGHT_SOURCE_DARK;
	}
	else if(vVertLineInfo_2.size() > lineCount){
		lightSource = LD_LIGHT_SOURCE_DARK;
	}


	delete [] horiProjection_1;
	delete [] vertProjection_1;
	delete [] horiProjection_2;
	delete [] vertProjection_2;

	return lightSource;
}