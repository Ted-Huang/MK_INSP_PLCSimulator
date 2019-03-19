// **************************************************************************
// ** SharedComponent.cpp -- Common utility function of shared components. **
// **************************************************************************
// Beagle 20130222 added.

#include <windows.h>
#include <math.h>
#include "SharedComponent.h"

int ImageTypeToPixelBytes(int type)
{
	switch (type) {
	case IMAGE_TYPE_RGB24:
	case IMAGE_TYPE_BGR24:
		return 3;
	case IMAGE_TYPE_MONO8:
	case IMAGE_TYPE_RGB8_332:
		return 1;
	case IMAGE_TYPE_NOTYPE:
	default:
		// This should not happen.
		return 3;
	}
}
void G_IMAGE_INFO(IMAGE &xImg,int &w,int &h,int &c,unsigned char **ppPtr) //eric chao 20150203
{
	w = xImg.data_w;
	h = xImg.data_h;
	c = ImageTypeToPixelBytes(xImg.type);
	if (ppPtr){
		*ppPtr = xImg.ptr;
	}
}
int GetImagePixelBytes(IMAGE *pImg)
{
	if (pImg){
		int pb = ImageTypeToPixelBytes(pImg->type);
		return pb;
	}
	return 3;
}

int GetImageSize(IMAGE *pImg) //eric chao 20130305
{
	if (pImg){
		int pb = ImageTypeToPixelBytes(pImg->type);
		return pImg->data_h*pImg->data_w*pb;
	}
	return 0;
}

void ImageInit(IMAGE *img, int type, unsigned char *ptr,
   int data_w, int data_h, int orig_w, int orig_h, int memsize)
{
	if (img == NULL) return;

	img->type = type;
	img->ptr = ptr;

	img->data_w = data_w;
	img->data_h = data_h;

	img->orig_w = (orig_w==0 ? data_w : orig_w);
	img->orig_h = (orig_h==0 ? data_h : orig_h);

	if (memsize > 0) {
		// Force override.
		img->memsize = memsize;
	}
	else if (ptr == NULL) { 
		//special usage for ericchao 
		img->memsize = 0;
	} else {
		// Calculate the memory size.
		img->memsize = GetImageSize(img);
	}
}

void ImageCrop(IMAGE *src, IMAGE *dst, IMAGE_CROPINFO *ci)
{
	float src_x, src_dx, src_y, src_dy, src_w, src_h, dst_ratio, cx, cy;
	int pixel_bytes;

	if (src == NULL || dst == NULL || ci == NULL)	{ return; }
	if (ci->dst_width == 0 || ci->dst_height == 0)	{ return; }

	pixel_bytes = ImageTypeToPixelBytes(src->type);
	dst->data_w = ci->dst_width;
	dst->data_h = ci->dst_height;

	cx = (ci->src_left+ci->src_right)/2.0f;
	cy = (ci->src_top+ci->src_bottom)/2.0f;

	dst_ratio = (float)ci->dst_width / (float)ci->dst_height;
	src_w = (float)(ci->src_right - ci->src_left);
	src_h = (float)(ci->src_bottom - ci->src_top);
	if (src_w/src_h > dst_ratio) {	// 寬扁型區域
		src_h = src_w / dst_ratio;
	} else {						// 瘦高型區域
		src_w = src_h * dst_ratio;
	}

	src_x = cx - src_w/2.0f;
	src_y = cy - src_h/2.0f;
	src_dx = src_w / (ci->dst_width-1);
	src_dy = src_h / (ci->dst_height-1);

	for (int y=0; y<ci->dst_height; y++) {
		for (int x=0; x<ci->dst_width; x++) {
			int ix, iy;
			float fx, fy;	// fragment

			ix = (int) floorf(src_x);
			fx = src_x - ix;
			iy = (int) floorf(src_y);
			fy = src_y - iy;

#if 1	// Nearest
			if (fx > 0.5f) { ix++; }
			if (fy > 0.5f) { iy++; }
			if (pixel_bytes == 1) {
				dst->ptr[y*dst->data_w+x] = src->ptr[iy*src->data_w+ix];
			} else {
				dst->ptr[(y*dst->data_w+x)*3]   = src->ptr[(iy*src->data_w+ix)*3];
				dst->ptr[(y*dst->data_w+x)*3+1] = src->ptr[(iy*src->data_w+ix)*3+1];
				dst->ptr[(y*dst->data_w+x)*3+2] = src->ptr[(iy*src->data_w+ix)*3+2];
			}
#endif
#if 0	// Bilinear
#endif
			src_x += src_dx;
		}
		src_y += src_dy;
	}
}

void ImageShrink(IMAGE *pImg, int shrink_rate)
{
	int pb, stride, src_pitch, shrink_w, shrink_h;
	unsigned char *pSrc, *pDst;

	pb = ImageTypeToPixelBytes(pImg->type);
	shrink_w = pImg->data_w / shrink_rate;
	shrink_h = pImg->data_h / shrink_rate;
	stride = pb * shrink_rate;
	src_pitch = pb * shrink_rate * pImg->data_w;
	pSrc = pDst = pImg->ptr;

	for (int y=0; y<shrink_h; y++) {
		unsigned char *pSrcTmp = pSrc;
		for (int x=0; x<shrink_w; x++) {
			if (pb == 3) {
				pDst[0] = pSrcTmp[0];
				pDst[1] = pSrcTmp[1];
				pDst[2] = pSrcTmp[2];
			} else {
				pDst[0] = pSrcTmp[0];
			}
			pSrcTmp += stride;
			pDst += pb;
		}
		pSrc += src_pitch;
	}

	pImg->data_w = shrink_w;
	pImg->data_h = shrink_h;
}

int CreateIMAGE(IMAGE **ppDst, IMAGE* pSrc, bool bInit)
{
	bool bCreate = false;
	if (*ppDst == NULL){
		*ppDst = new IMAGE;
		memset(*ppDst, 0, sizeof(IMAGE));
		bCreate = true;
	}
	int nMemSize = pSrc->memsize;
	if (pSrc->ptr && (pSrc->memsize == 0)){ //IMAGE SRC maybe use others ptr,but not alloc memory
		nMemSize = GetImageSize(pSrc);
	}
	if ((*ppDst)->ptr && (*ppDst)->memsize == 0){
		(*ppDst)->memsize = GetImageSize(*ppDst);
	}
	if ((*ppDst) && ((*ppDst)->ptr == NULL)){
		(*ppDst)->memsize = 0;
	}
	if ((*ppDst) && pSrc && pSrc->ptr && nMemSize){
		int nCreateSize = nMemSize;
		if (pSrc->memsize != 0){
			nCreateSize = GetImageSize(pSrc);
		}
		if ((*ppDst)->memsize < nCreateSize){
			if ((*ppDst)->ptr){
				delete[](*ppDst)->ptr;
				(*ppDst)->ptr = NULL;
			}
			if (nCreateSize > 0){
				(*ppDst)->ptr = new unsigned char[nCreateSize];
				if (bInit){
					memset((*ppDst)->ptr, 0, nCreateSize);
				}
			}
			(*ppDst)->memsize = nCreateSize;
			(*ppDst)->data_h = pSrc->data_h;
			(*ppDst)->data_w = pSrc->data_w;
			(*ppDst)->orig_h = pSrc->orig_h;
			(*ppDst)->orig_w = pSrc->orig_w;
			(*ppDst)->type = pSrc->type;
			return nCreateSize;
		}
		else{//Use Old Memory
			(*ppDst)->data_h = pSrc->data_h;
			(*ppDst)->data_w = pSrc->data_w;
			(*ppDst)->orig_h = pSrc->orig_h;
			(*ppDst)->orig_w = pSrc->orig_w;
			(*ppDst)->type = pSrc->type;
			if (bInit && (*ppDst)->ptr){
				memset((*ppDst)->ptr, 0, nCreateSize);
			}
			return 0;
		}
	}
	if ((*ppDst) && bCreate){ //eric chao 20160114 fix bug
		delete (*ppDst);
		(*ppDst) = NULL;
	}
	return -1;
}
bool CloneIMAGE(IMAGE **ppDst, IMAGE *pSrc)
{
	int nCreateSize = CreateIMAGE(ppDst, pSrc, FALSE);
	if (nCreateSize >= 0){
		int nDataSize = GetImageSize(pSrc);
		memcpy((*ppDst)->ptr, pSrc->ptr, nDataSize);
		return TRUE;
	}
	return FALSE;
}
bool DestroyIMAGE(IMAGE **ppDst)
{
	if (ppDst && *ppDst){
		if ((*ppDst)->ptr){
			delete[](*ppDst)->ptr;
			(*ppDst)->ptr = NULL;
		}
		delete (*ppDst);
		*ppDst = NULL;
		return TRUE;
	}
	return FALSE;
}

bool verifyIMAGE(IMAGE *xImg)
{
	if (xImg == NULL){ return false; }
	if (xImg->ptr == NULL){ return false; }
	if (xImg->data_w <= 0 || xImg->data_h <= 0 || GetImagePixelBytes(xImg) <= 0){ return false; }

	return true;
}
/* End of SharedComponent.cpp */
