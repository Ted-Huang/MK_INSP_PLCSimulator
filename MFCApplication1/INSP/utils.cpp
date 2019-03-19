#include "utils.h"
#include <string.h>

const int LUMINANCE_BITS = 5;
const int LUMINANCE_SHIFT = 8 - LUMINANCE_BITS;
const int LUMINANCE_BUCKETS = 1 << LUMINANCE_BITS;

void computeHistogram(unsigned char *row, int w, int color, int *histogram, int histSize)
{
	memset(histogram, 0, histSize*sizeof(int));
	int offset = 0;
	if(color==3){
		offset = 1;
	}
	for(int c=0; c<w; c++)
		histogram[row[c*color+offset]>>LUMINANCE_SHIFT]++;
}

int histogramBinarizer(int *histogram, int histSize, int *blackCenter, int *whiteCenter)
{
	// Find 1st-tallest peak in histogram
	int maxBucketCount = 0;

	// Find tallest peak in histogram
	int firstPeak = 0;
	int firstPeakSize = 0;
	for (int i = 0; i < histSize; i++) {
		if (histogram[i] > firstPeakSize) {
			firstPeak = i;
			firstPeakSize = histogram[i];
		}
	}
	maxBucketCount = firstPeakSize;

	// Find 2nd-tallest peak
	int secondPeak = 0;
	int secondPeakScore = 0;
	for (int i = 0; i < histSize; i++) {
		int distanceToBiggest = i - firstPeak;
		// Encourage more distant second peaks by multiplying by square of distance
		int score = histogram[i] * distanceToBiggest * distanceToBiggest;
		if (score > secondPeakScore) {
			secondPeak = i;
			secondPeakScore = score;
		}
	}

	// Put firstPeak first
	if (firstPeak > secondPeak) {
		int temp = firstPeak;
		firstPeak = secondPeak;
		secondPeak = temp;
	}

	*blackCenter = firstPeak<<LUMINANCE_SHIFT;
	*whiteCenter = secondPeak<<LUMINANCE_SHIFT;

	if (secondPeak - firstPeak <= histSize >> 4) {
		return -1;
	}

	// Find a valley between them that is low and closer to the white peak
	int bestValley = secondPeak - 1;
	int bestValleyScore = -1;
	for (int i = secondPeak - 1; i > firstPeak; i--) {
		int fromFirst = i - firstPeak;
		// Favor a "valley" that is not too close to either peak -- especially not
		// the black peak -- and that has a low value of course
		int score = fromFirst * fromFirst * (secondPeak - i) *
			(maxBucketCount - histogram[i]);
		if (score > bestValleyScore) {
			bestValley = i;
			bestValleyScore = score;
		}
	}

	return bestValley << LUMINANCE_SHIFT;
}

int getHistogramGT(unsigned char *luminances, int w, int h, int color, int *blackCenter, int *whiteCenter)
{
	int histogram[LUMINANCE_BUCKETS];
	computeHistogram(luminances, w*h, color, histogram, LUMINANCE_BUCKETS);
	int bestValley = histogramBinarizer(histogram, LUMINANCE_BUCKETS, blackCenter, whiteCenter);

	return bestValley;
}

int fixedGTBinary(unsigned char *src, int width, int height, int nColor, unsigned char *dst, unsigned char GT)
{
	if(src==NULL || dst==NULL || width<=0 || height<=0 || nColor<=0){ return -1; }
	int size = width*height;
	unsigned char *ptr1, *ptr2;
	ptr1 = src;
	ptr2 = dst;
	if(nColor==3){
		for(int c=0; c<size; c++){
			(*ptr2) = *(ptr1+1)>GT?255:0;
			ptr1+=nColor;
			ptr2++;
		}
	}
	else{
		for(int c=0; c<size; c++){
			(*ptr2) = (*ptr1)>GT?255:0;
			ptr1+=nColor;
			ptr2++;
		}
	}

	return 0;
}