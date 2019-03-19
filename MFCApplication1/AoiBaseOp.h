#pragma once
#pragma pack(push, 1)
typedef struct tagBMP_FILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BMP_FILEHEADER;
typedef struct tagBMP_INFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BMP_INFOHEADER;
//Order rgbBlue,rgbGreen,rgbRed,rgbReserved
#pragma pack(pop)
#include "AoiConst.h"

enum {READ_TOPDOWN=0,READ_BOTTOMUP,READ_DEFAULT};

static void setimageheader(BMP_FILEHEADER *pbf,BMP_INFOHEADER *pbi,int w,int h,int color)
{
	if ((color >= 1) && (color <=3)){
	//	BOOL bTopDown=FALSE;
		int iPicHeight=h;
		if (h<0){
	//		bTopDown=TRUE;
			iPicHeight=-h;
		}
		int iScanLine = ((((w * color * 8) + 31) & ~31) >> 3);
		int iBmpSize = iScanLine * iPicHeight ;
		if (pbf){
			pbf->bfType=0x4D42; //'BM'
			pbf->bfOffBits=sizeof(BMP_FILEHEADER)+sizeof(BMP_INFOHEADER);
			if (color==1)
				pbf->bfOffBits+=1024;
			pbf->bfSize=iBmpSize+pbf->bfOffBits;
		}
		if (pbi){
			pbi->biSize=sizeof(BMP_INFOHEADER);
			pbi->biWidth=w;
			pbi->biHeight=h;
			pbi->biBitCount=color*8;
			pbi->biPlanes=1;
			pbi->biSizeImage=iBmpSize;
			if (color==1){ //eric chao 20140303 fix mono,biClrUsed should be color table size
				pbi->biClrUsed = 256;
			}
		}
	}
}
static int saveimage(const wchar_t *filename,unsigned char *pdata,int w,int h,int color,BOOL bSwapRB) //for top-down ==> h<0
{
	if (color!=1 && color !=3)
		return -1;
	BMP_FILEHEADER bf;
	memset(&bf,0,sizeof(bf));
	BMP_INFOHEADER bi;
	memset(&bi,0,sizeof(bi));
	BYTE *pBmpData=NULL;
	setimageheader(&bf,&bi,w,h,color);

	if (bi.biSizeImage){
		int iScanLine = ((((w * color * 8) + 31) & ~31) >> 3);
		HANDLE	imgfile = INVALID_HANDLE_VALUE;
		DWORD count=0;
		imgfile = CreateFileW(filename, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (imgfile == INVALID_HANDLE_VALUE) {
			//_D(L"CreateFile(%s) failed: %d\n", filename, GetLastError());
			return -1;
		}
		WriteFile(imgfile, &bf, sizeof(bf), &count, NULL);	//BITMAP FILE HEADER
		WriteFile(imgfile, &bi, sizeof(bi), &count, NULL);	//BITMAP INFO HEADER
		if (color == 1)
			WriteFile(imgfile, ctMonoColorTable, sizeof(ctMonoColorTable), &count, NULL);	//COLOR TABLE
		pBmpData= new BYTE[bi.biSizeImage];
		memset(pBmpData,0,bi.biSizeImage);
		BYTE *pOutputData=NULL;
		if (bSwapRB && color==3){
			if (h<0)
				h=-h;
			BYTE *pDst=pBmpData;
			BYTE *pSrc=pdata;
			int iSrcLine=w*color;
			for (int j=0;j<h;j++){
				for (int i=0;i<w;i++){
					pDst[3*i+0]=pSrc[3*i+2];
					pDst[3*i+1]=pSrc[3*i+1];
					pDst[3*i+2]=pSrc[3*i+0];
				}
				pSrc+= iSrcLine;
				pDst+= iScanLine;
			}
			pOutputData=pBmpData;
		}
		else{
			if (iScanLine!= (w*color)){
				if (h<0)
					h=-h;
				BYTE *pDst=pBmpData;
				BYTE *pSrc=pdata;
				int iSrcLine=w*color;
				for (int j=0;j<h;j++){
					memcpy(pDst,pSrc,iSrcLine);
					pSrc+= iSrcLine;
					pDst+= iScanLine;
				}
				pOutputData=pBmpData;			
			}
			else{
				pOutputData=pdata;
			}
		}
		WriteFile(imgfile,pOutputData,bi.biSizeImage,&count,NULL);
		delete []pBmpData;
		CloseHandle(imgfile);
	}
	return 0;
};

static int saveimage(const wchar_t *filename, unsigned char *data, int w, int h,int color) //eric chao 20130220 temp use
{
	return saveimage(filename,data,w,h,color,TRUE);
};

static int loadimage(const wchar_t *filename, unsigned char **data, int *w, int *h, int *color,int nStatus,BOOL bSwapRB)
{
	BMP_FILEHEADER bf;
	memset(&bf,0,sizeof(BMP_FILEHEADER));
	BMP_INFOHEADER bi;
	memset(&bi,0,sizeof(BMP_INFOHEADER));
	unsigned char	*pbuf, *tbuf;
	HANDLE	imgfile = INVALID_HANDLE_VALUE;
	DWORD	count;
	int		width, height, size, pitch, psize;

	imgfile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (imgfile == INVALID_HANDLE_VALUE) {
		LPWSTR lpMsgBuf=NULL;
		DWORD dw = GetLastError(); 
		FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			lpMsgBuf,
			0, NULL );
		return -1;
	}

	ReadFile(imgfile, &bf, sizeof(bf), &count, NULL);	// BMP FILE header.
	ReadFile(imgfile, &bi, sizeof(bi), &count, NULL);   // BMP INFO header.
	width = bi.biWidth;
	height = bi.biHeight;
	height = (height>0 ? height : -height);
	int iColor = bi.biBitCount/8;

	pitch = ((((width * bi.biBitCount) + 31) & ~31) >> 3);
	int nDstPitch = width * (bi.biBitCount/8);

	size = nDstPitch * height;
	psize = pitch * height;
	//tbuf = (unsigned char *) malloc(size);
	tbuf = new unsigned char[size];
	pbuf = (unsigned char *) malloc(psize);

	//eric chao 20140303
	//for old save image bug,mono data biClrUsed save as 0
	//for normal mono bmp,use color table biClrUsed = 256
	if (bi.biBitCount==8 && (bi.biClrUsed==0 || bi.biClrUsed==256)){
		BYTE tColorTable[1024]; //256*4
		ReadFile(imgfile,tColorTable,sizeof(tColorTable),&count,NULL);
	}

	ReadFile(imgfile, pbuf, psize, &count, NULL);

	if ( (bi.biBitCount == 24 &&!bSwapRB) || (bi.biBitCount == 8 && (bi.biClrUsed==0 || bi.biClrUsed==256))) { //eric chao 20140109
		if ((nStatus == READ_TOPDOWN) || (nStatus == READ_DEFAULT && bi.biHeight < 0)){
			for (int y=0; y<height; y++) {
				memcpy(&tbuf[y*width*iColor],&pbuf[y*pitch],width*iColor);
			}
		} else {
			for (int y=0; y<height; y++) {
				memcpy(&tbuf[y*width*iColor],&pbuf[(height-y-1)*pitch],width*iColor);
			}
		}
	}else if (bi.biBitCount==24 && bSwapRB){
		if ((nStatus == READ_TOPDOWN) || (nStatus == READ_DEFAULT && bi.biHeight < 0)){
			unsigned char *pSrc = pbuf;
			unsigned char *pDst = tbuf;
			for (int y=0; y<height; y++) {
				for (int x=0;x<width*iColor;x+=iColor){
					pDst[x] = pSrc[x+2];
					pDst[x+1] = pSrc[x+1];
					pDst[x+2] = pSrc[x];
				}
				pSrc += pitch;
				pDst += nDstPitch;
			}
		} else {
			unsigned char *pSrc = &pbuf[(height-1)*pitch];
			unsigned char *pDst = tbuf;
			for (int y=0; y<height; y++) {
				for (int x=0;x<width*iColor;x+=iColor){
					pDst[x] = pSrc[x+2];
					pDst[x+1] = pSrc[x+1];
					pDst[x+2] = pSrc[x];
				}
				pSrc -= pitch;
				pDst += nDstPitch;
			}
		}
	}

	*data = tbuf;
	*w = width;
	*h = height;
	*color = iColor;

	free(pbuf);
	CloseHandle(imgfile);

	return 0;
};

//------------------------------------------------------------------------
// 計算機台上,預計的最大未處理產品數
// 規則, Trigger to Camera上的紙盒數 + Camera to Solenoid的一半的紙盒數
//-------------------------------------------------------------------------
static int CalcMaxQueueSize(double t2c, double t2s,int nBlankLength,int nBlankInterval)
{
	int maxQ;
	// blank number between camera and trigger, it should be the min queue size
	maxQ = (int)(t2c / (nBlankLength + nBlankInterval) + 0.5) + 1;

	// safe blank number from camera to solenoid
	maxQ += (int)((t2s - t2c) / ((nBlankLength + nBlankInterval)*2) + 0.5);

	return maxQ;
}

// added by amike at 20130627
static void getDefectPiece(unsigned char *dst, int dstW, int dstH, unsigned char *src, int srcW, int srcH, int color, int left, int top, float shrinkRatio)
{
	int xid, yid;
	for(int j=0; j<dstH; j++){
		yid = (int)(j*shrinkRatio+top+0.5f);
		if(yid>=0 && yid<srcH){
			for(int i=0; i<dstW; i++){
				xid = (int)(i*shrinkRatio+left+0.5f);
				if(xid>=0 && xid<srcW){
					for(int c=0; c<color; c++){
						dst[(j*dstW+i)*color+c] = src[(yid*srcW+xid)*color+c];
					}
				}
			}
		}
	}
}
