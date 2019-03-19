#ifndef _CORRELATION_H_
#define _CORRELATION_H_

#include "SharedComponent.h"
#include <windows.h>

#ifndef NULL
#define NULL 0
#endif

class Correlation
{
private:
	// sample
	float m_sAvgR;
	float m_sAvgG;
	float m_sAvgB;

	float m_sSumR;
	float m_sSumG;
	float m_sSumB;
	
	// golden
	float m_gAvgR;
	float m_gAvgG;
	float m_gAvgB;

	float m_gSumR;
	float m_gSumG;
	float m_gSumB;

	float m_innerR;
	float m_innerG;
	float m_innerB;

	float m_corrR;
	float m_corrG;
	float m_corrB;

	IMAGE m_xSampleImg;
	IMAGE m_xGoldenImg;

	void CalculateSampleMean(void);
	void CalculateSampleSum(void);
	void CalculateGoldenMean(void);
	void CalculateGoldenSum(void);
public:
	Correlation();
	~Correlation();

	void SetSampleImg(IMAGE *xImg);
	void SetGoldenImg(IMAGE *xImg);

	void CalculateCorrelation(void);
	void GetCorrelation(float *correlation);
};

#endif /* _CORRELATION_H_ */