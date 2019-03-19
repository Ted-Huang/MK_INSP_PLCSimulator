// *************************************************************************
// **  SharedComponent.h                                                  **
// **      Common definition & declaration of shared components.          **
// **      此處放置 Share 資料夾中各組件的共用 struct 與共用 function 。  **
// *************************************************************************
// Beagle 20120920 added.

#pragma once

#ifdef _AFX
// Macro used in try-catch clause.
#define MY_CATCH_ALL(x)											\
	catch (CMemoryException* e) {								\
		CString errmsg;											\
		wchar_t szCause[256];									\
		e->GetErrorMessage(szCause, 256);						\
		errmsg.Format(L"Memory exception in %s: %S(%d) %s\n",	\
			x, __FILE__, __LINE__, szCause);					\
		AfxMessageBox(errmsg);									\
		e->Delete();											\
	} catch (CFileException* e) {								\
		CString errmsg;											\
		wchar_t szCause[256];									\
		e->GetErrorMessage(szCause, 256);						\
		errmsg.Format(L"File exception in %s: %S(%d) %s\n",		\
			x, __FILE__, __LINE__, szCause);					\
		AfxMessageBox(errmsg);									\
		e->Delete();											\
	} catch (CException* e) {									\
		CString errmsg;											\
		wchar_t szCause[256];									\
		e->GetErrorMessage(szCause, 256);						\
		errmsg.Format(L"Exception in %s: %S(%d) %s\n",			\
			x, __FILE__, __LINE__, szCause);					\
		AfxMessageBox(errmsg);									\
		e->Delete();											\
	}
#else
// Macro used in try-catch clause.
#define MY_CATCH_ALL(x)											\
	catch (char* str) {											\
		wchar_t msg[1024];										\
		swprintf_s(msg, 1024, L"Exception in %s: %S(%d): %s\n",	\
			x, __FILE__, __LINE__, str);						\
	}
#endif

enum IMAGE_TYPE
{
	IMAGE_TYPE_NOTYPE = 0,
	IMAGE_TYPE_MONO8,
	IMAGE_TYPE_RGB24,
	IMAGE_TYPE_BGR24,
	IMAGE_TYPE_RGB8_332,
};

typedef struct _IMAGE {
	unsigned char *ptr;
	int type;		//ImageTypeToPixelBytes
	int	orig_w;
	int	orig_h;
	int data_w;
	int data_h;
	int	memsize;	// 實際上 allocate 的 memory 大小。
} IMAGE;

typedef struct _IMAGE_CROPINFO {
	int src_left;
	int src_right;
	int src_top;
	int src_bottom;
	int dst_width;
	int dst_height;
} IMAGE_CROPINFO;

extern void ImageInit(IMAGE *img, int type, unsigned char *ptr,
	int data_w, int data_h, int orig_w=0, int orig_h=0, int memsize=0);
extern int ImageTypeToPixelBytes(int type);
extern void G_IMAGE_INFO(IMAGE &xImg,int &w,int &h,int &c,unsigned char **ppPtr);
extern int GetImageSize(IMAGE *pImg);
extern int GetImagePixelBytes(IMAGE *pImg);
extern void ImageShrink(IMAGE *pImg, int shrink_rate);
extern void ImageCrop(IMAGE *src, IMAGE *dst, IMAGE_CROPINFO *ci);

extern int CreateIMAGE(IMAGE **ppDst, IMAGE* pSrc, bool bInit);
extern bool CloneIMAGE(IMAGE **ppDst, IMAGE *pSrc);
extern bool DestroyIMAGE(IMAGE **ppDst);
extern bool verifyIMAGE(IMAGE *xImg);
/* End of SharedComponent.h */
