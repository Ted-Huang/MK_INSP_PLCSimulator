/******************************************************************************
 *  SysUtil -- Collection of System Utilities. Put your utilities here.       *
 ******************************************************************************
 * Beagle 20121019 added.
 *
 * Replacement for obsoleted Windows function:
 *	PathFileExists()		==>	IsPathExist()
 *	SHCreateDirectoryEx()	==>	CreateFullPath()
 */
#pragma once

class SysUtil
{
public:
	static void SaveUTF8_FromGB(const wchar_t *pDstFileName,const wchar_t *pSrcFileName);
	//eric chao 20150903
	static void SaveUTF8_FromBIG5(const wchar_t *pDstFileName,const wchar_t *pSrcFileName);
	static void SaveUTF8_FromKorean(const wchar_t *pDstFileName,const wchar_t *pSrcFileName);
	static void SaveUTF8_FromJIS(const wchar_t *pDstFileName,const wchar_t *pSrcFileName);
	static void SaveUTF8_FromKOI8R(const wchar_t *pDstFileName,const wchar_t *pSrcFileName);

	static void ToUTF8_FromGB(unsigned char *pOut,int nDstSize,unsigned char *pIn,int nSrcSize); //eric chao 20150917
	static void ToUTF8_FromBIG5(unsigned char *pOut,int nDstSize,unsigned char *pIn,int nSrcSize); //eric chao 20150917
	static void ToUTF8_FromJIS(unsigned char *pOut,int nDstSize,unsigned char *pIn,int nSrcSize); //eric chao 20150917
	static void ToUTF8_FromKorean(unsigned char *pOut,int nDstSize,unsigned char *pIn,int nSrcSize); //eric chao 20150917
	static void ToUTF8_FromKOI8R(unsigned char *pOut,int nDstSize,unsigned char *pIn,int nSrcSize); //eric chao 20150917
	static void ToUTF8_FromUnicode(unsigned char *pOut,int nDstSize,wchar_t *pIn,int nSrcSize); //eric chao 20151005

	static CString MakeUnicode_UTF8(const char *pData); //eric chao 20150729
	static CString MakeUnicode_BIG5(const char *pData); //eric chao 20150903
	static CString MakeUnicode_GB(const char *pData); //eric chao 20150903
	static CString MakeUnicode_Korean(const char *pData); //eric chao 20150903
	static CString MakeUnicode_JIS(const char *pData); //eric chao 20150903
	static CString MakeUnicode_KOI8R(const char *pData); //eric chao 20150903


	static int	CreateFullPath(const wchar_t *path);
	static int	IsPathExist(const wchar_t *path);
	static int	IsDirectory(const wchar_t *path);
	static CString StrToFilename(CString str);
	static void StrReplaceBlank(char *str);
	static void	ClearMessageQueue(UINT filter_min, UINT filter_max);
	static void ClearMouseMessage(void) {
		ClearMessageQueue(WM_MOUSEFIRST, WM_MOUSELAST);
	}
	static void ClearKeyboardMessage(void) {
		ClearMessageQueue(WM_KEYFIRST, WM_KEYLAST);
	}
	static void OpenWithNotepad(const wchar_t *filename);	// Beagle 20131119
	static void SaveTextFile_UTF8(const wchar_t *filename, const wchar_t *text);	// Beagle 20131119
	static void WriteFile_UTF8(HANDLE hFile, int wbufsize, const wchar_t *wbuf);
private:
	static void ConvertByCodePage(unsigned char *pOut,int nDstSize,unsigned char *pIn,int nSrcSize,int nDstCodePage,int nSrcCodePage); //eric chao 20150917
	static CString MakeUnicode_CodePage(const char *pData,int nCodePage); //eric chao 20150903
	static void SaveUTF8_FromCodePage(const wchar_t *pDstFileName,const wchar_t *pSrcFileName,int nCodePage);
};

/* End of SysUtil.h */
