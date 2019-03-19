#include <iostream>
#include "Correlation.h"
#include <math.h>
#include "SharedComponent.h"

Correlation::Correlation()
{
	m_sAvgR = 0.0f;
	m_sAvgG = 0.0f;
	m_sAvgB = 0.0f;

	m_sSumR = 0.0f;
	m_sSumG = 0.0f;
	m_sSumB = 0.0f;
	
	m_gAvgR = 0.0f;
	m_gAvgG = 0.0f;
	m_gAvgB = 0.0f;

	m_gSumR = 0.0f;
	m_gSumG = 0.0f;
	m_gSumB = 0.0f;

	m_innerR = 0.0f;
	m_innerG = 0.0f;
	m_innerB = 0.0f;

	m_corrR = 0.0f;
	m_corrG = 0.0f;
	m_corrB = 0.0f;

	memset(&m_xSampleImg, 0, sizeof(m_xSampleImg));
	memset(&m_xGoldenImg, 0, sizeof(m_xGoldenImg));
}

Correlation::~Correlation()
{

}

void Correlation::SetSampleImg(IMAGE *xImg)
{
	ImageInit(&m_xSampleImg, xImg->type, xImg->ptr, xImg->data_w, xImg->data_h);
	CalculateSampleMean();
	CalculateSampleSum();
}

void Correlation::SetGoldenImg(IMAGE *xImg)
{
	ImageInit(&m_xGoldenImg, xImg->type, xImg->ptr, xImg->data_w, xImg->data_h);
	CalculateGoldenMean();
	CalculateGoldenSum();
}

void Correlation::CalculateSampleMean(void)
{
	if(m_xSampleImg.ptr==NULL){ return; }

	m_sAvgR = 0.0f;
	m_sAvgG = 0.0f;
	m_sAvgB = 0.0f;
	for(int j=0; j<m_xSampleImg.data_h; j++){
		for(int i=0; i<m_xSampleImg.data_w; i++){
			int idx = j*m_xSampleImg.data_w+i;
			if(ImageTypeToPixelBytes(m_xSampleImg.type)==1){
				m_sAvgR += m_xSampleImg.ptr[idx];
			}
			else{
				m_sAvgR += m_xSampleImg.ptr[idx*3+0];
				m_sAvgG += m_xSampleImg.ptr[idx*3+1];
				m_sAvgB += m_xSampleImg.ptr[idx*3+2];
			}
		}
	}
	m_sAvgR /= (m_xSampleImg.data_w*m_xSampleImg.data_h);
	m_sAvgG /= (m_xSampleImg.data_w*m_xSampleImg.data_h);
	m_sAvgB /= (m_xSampleImg.data_w*m_xSampleImg.data_h);
}

void Correlation::CalculateSampleSum(void)
{
	if(m_xSampleImg.ptr==NULL){ return; }

	m_sSumR = 0.0f;
	m_sSumG = 0.0f;
	m_sSumB = 0.0f;
	if(ImageTypeToPixelBytes(m_xSampleImg.type)==1){
		for(int j=0; j<m_xSampleImg.data_h; j++){
			for(int i=0; i<m_xSampleImg.data_w; i++){
				int idx = j*m_xSampleImg.data_w+i;
				m_sSumR += (m_xSampleImg.ptr[idx]-m_sAvgR)*(m_xSampleImg.ptr[idx]-m_sAvgR);
			}
		}
	}
	else{
		for(int j=0; j<m_xSampleImg.data_h; j++){
			for(int i=0; i<m_xSampleImg.data_w; i++){
				int idx = j*m_xSampleImg.data_w+i;
				m_sSumR += (m_xSampleImg.ptr[idx*3+0]-m_sAvgR)*(m_xSampleImg.ptr[idx*3+0]-m_sAvgR);
				m_sSumG += (m_xSampleImg.ptr[idx*3+1]-m_sAvgG)*(m_xSampleImg.ptr[idx*3+1]-m_sAvgG);
				m_sSumB += (m_xSampleImg.ptr[idx*3+2]-m_sAvgB)*(m_xSampleImg.ptr[idx*3+2]-m_sAvgB);
			}
		}
	}
}

void Correlation::CalculateGoldenMean(void)
{
	if(m_xGoldenImg.ptr==NULL){ return; }

	m_gAvgR = 0.0f;
	m_gAvgG = 0.0f;
	m_gAvgB = 0.0f;
	if(ImageTypeToPixelBytes(m_xGoldenImg.type)==1){
		for(int j=0; j<m_xGoldenImg.data_h; j++){
			for(int i=0; i<m_xGoldenImg.data_w; i++){
				int idx = j*m_xGoldenImg.data_w+i;
				m_gAvgR += m_xGoldenImg.ptr[idx];
			}
		}
	}
	else{
		for(int j=0; j<m_xGoldenImg.data_h; j++){
			for(int i=0; i<m_xGoldenImg.data_w; i++){
				int idx = j*m_xGoldenImg.data_w+i;
				m_gAvgR += m_xGoldenImg.ptr[idx*3+0];
				m_gAvgG += m_xGoldenImg.ptr[idx*3+1];
				m_gAvgB += m_xGoldenImg.ptr[idx*3+2];
			}
		}
	}
	m_gAvgR /= (m_xGoldenImg.data_w*m_xGoldenImg.data_h);
	m_gAvgG /= (m_xGoldenImg.data_w*m_xGoldenImg.data_h);
	m_gAvgB /= (m_xGoldenImg.data_w*m_xGoldenImg.data_h);
}

void Correlation::CalculateGoldenSum(void)
{
	if(m_xGoldenImg.ptr==NULL){ return; }

	m_gSumR = 0.0f;
	m_gSumG = 0.0f;
	m_gSumB = 0.0f;
	if(ImageTypeToPixelBytes(m_xGoldenImg.type)==1){
		for(int j=0; j<m_xGoldenImg.data_h; j++){
			for(int i=0; i<m_xGoldenImg.data_w; i++){
				int idx = j*m_xGoldenImg.data_w+i;
				m_gSumR += (m_xGoldenImg.ptr[idx]-m_gAvgR)*(m_xGoldenImg.ptr[idx]-m_gAvgR);
			}
		}
	}
	else{
		for(int j=0; j<m_xGoldenImg.data_h; j++){
			for(int i=0; i<m_xGoldenImg.data_w; i++){
				int idx = j*m_xGoldenImg.data_w+i;
				m_gSumR += (m_xGoldenImg.ptr[idx*3+0]-m_gAvgR)*(m_xGoldenImg.ptr[idx*3+0]-m_gAvgR);
				m_gSumG += (m_xGoldenImg.ptr[idx*3+1]-m_gAvgG)*(m_xGoldenImg.ptr[idx*3+1]-m_gAvgG);
				m_gSumB += (m_xGoldenImg.ptr[idx*3+2]-m_gAvgB)*(m_xGoldenImg.ptr[idx*3+2]-m_gAvgB);
			}
		}
	}
}

void Correlation::CalculateCorrelation(void)
{
	if(m_xGoldenImg.ptr==NULL || m_xSampleImg.ptr==NULL){ return; }

	m_innerR = 0.0f;
	m_innerG = 0.0f;
	m_innerB = 0.0f;
	if(ImageTypeToPixelBytes(m_xSampleImg.type)==1){
		for(int j=0; j<m_xSampleImg.data_h; j++){
			for(int i=0; i<m_xSampleImg.data_w; i++){
				int tIdx = j*m_xSampleImg.data_w+i;
				int gIdx = j*m_xGoldenImg.data_w+i;
				m_innerR += (m_xSampleImg.ptr[tIdx]-m_sAvgR)*(m_xGoldenImg.ptr[gIdx]-m_gAvgR);
			}
		}
	}
	else{
		for(int j=0; j<m_xSampleImg.data_h; j++){
			for(int i=0; i<m_xSampleImg.data_w; i++){
				int tIdx = j*m_xSampleImg.data_w+i;
				int gIdx = j*m_xGoldenImg.data_w+i;
				m_innerR += (m_xSampleImg.ptr[tIdx*3+0]-m_sAvgR)*(m_xGoldenImg.ptr[gIdx*3+0]-m_gAvgR);
				m_innerG += (m_xSampleImg.ptr[tIdx*3+1]-m_sAvgG)*(m_xGoldenImg.ptr[gIdx*3+1]-m_gAvgG);
				m_innerB += (m_xSampleImg.ptr[tIdx*3+2]-m_sAvgB)*(m_xGoldenImg.ptr[gIdx*3+2]-m_gAvgB);
			}
		}
	}

	m_corrR = -1.0f;
	m_corrG = -1.0f;
	m_corrB = -1.0f;

	m_corrR = m_innerR/( sqrt(m_sSumR)*sqrt(m_gSumR) );
	if(ImageTypeToPixelBytes(m_xSampleImg.type)==3){
		m_corrG = m_innerG/( sqrt(m_sSumG)*sqrt(m_gSumG) );
		m_corrB = m_innerB/( sqrt(m_sSumB)*sqrt(m_gSumB) );
	}
}

void Correlation::GetCorrelation(float *correlation)
{
	correlation[0] = max(m_corrR, 0.0f);
	correlation[1] = max(m_corrG, 0.0f);
	correlation[2] = max(m_corrB, 0.0f);
}