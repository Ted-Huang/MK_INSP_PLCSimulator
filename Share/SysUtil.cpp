/******************************************************************************
 *  SysUtil -- Collection of System Utilities. Put your utilities here.       *
 ******************************************************************************
 * Beagle 20121019 added.
 */

#include "stdafx.h"

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <Shellapi.h>
#include "SysUtil.h"

#define __T(x) L ## x
#define _T(x) __T(x)

#ifndef MAX_PATH
#define MAX_PATH	260
#endif

//eric chao 20150903

enum AOI_CODE_PAGE_INDEX_{
	CT_CODE_GB2312			=0,
	CT_CODE_BIG5			=1,
	CT_CODE_UTF8,
	CT_CODE_KOREAN,
	CT_CODE_JIS,
	CT_CODE_KOI8R,
	CT_CODE_MAX
}AOI_CODE_PAGE_INDEX;

const int ctCodePage[CT_CODE_MAX] = {
	936,	//CT_CODE_GB2312
	950,	//CT_CODE_BIG5
	CP_UTF8,//CT_CODE_UTF8
	949,	//CT_CODE_KOREAN
	932,	//CT_CODE_JIS
	20866	//ctCODE_KOI8R
};


static BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lparam)
{
	DWORD	pid;

	GetWindowThreadProcessId(hwnd, &pid);
	if (pid == lparam) {
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
		return FALSE;
	} else {
		return TRUE;
	}
}

int SysUtil::CreateFullPath(const wchar_t *path)
{
	const wchar_t *ptr;
	wchar_t msg[512], buf[MAX_PATH];

	if (wcslen(path) >= MAX_PATH) {
		swprintf_s(msg, 512, L"Path [%.200s] is too long.", path);
		MessageBoxW(NULL, msg, _T(__FUNCTION__), 0);
		return FALSE;
	}

	if (path[0]==L'\\' && path[1]==L'\\') {
		// UNC path -- Skip ServerName and ShareName
		ptr = wcschr(path+2, L'\\');
		if (ptr == NULL) {
			swprintf_s(msg, 512, L"Path [%.200s] is not a valid path.", path);
			MessageBoxW(NULL, msg, _T(__FUNCTION__), 0);
			return FALSE;
		}
		ptr = wcschr(ptr+1, L'\\');
		if (ptr == NULL) {
			// If it exists, don't have to create anything and return TRUE.
			memset(buf, 0, MAX_PATH*sizeof(wchar_t));
			wcscpy_s(buf, MAX_PATH, path);
			wcscat_s(buf, MAX_PATH, L"\\");
		} else {
			memset(buf, 0, MAX_PATH*sizeof(wchar_t));
			wcsncpy_s(buf, MAX_PATH, path, ptr-path+1);
		}

		if (!IsPathExist(buf)) {
			swprintf_s(msg, 512, L"Path [%.200s] does not exist.", buf);
			MessageBoxW(NULL, msg, _T(__FUNCTION__), 0);
			return FALSE;
		}

		if (ptr == NULL) { return TRUE; }
		ptr++;
		if (*ptr == L'\0') { return TRUE; }
	} else if (IsCharAlphaW(path[0]) && path[1]==L':') {
		// Absolute path
		if (path[2]==L'\\') {
			ptr=path+3;
		} else {
			swprintf_s(msg, 512, L"Invalid path [%.200s].", path);
			MessageBoxW(NULL, msg, _T(__FUNCTION__), 0);
			return FALSE;
		}
	} else {
		// Relative path
		ptr = path;
	}

	while (1) {
		ptr = wcschr(ptr, L'\\');
		memset(buf, 0, MAX_PATH*sizeof(wchar_t));
		if (ptr == NULL) {	// Last part.
			wcscpy_s(buf, MAX_PATH, path);
		} else {
			wcsncpy_s(buf, MAX_PATH, path, ptr-path);
		}

		if (IsPathExist(buf)) {
			if (!IsDirectory(buf)) {
				swprintf_s(msg, 512, L"Path [%.200s] exists but is not a directory.",
					buf);
				MessageBoxW(NULL, msg, _T(__FUNCTION__), 0);
				return FALSE;
			}
		} else {
			if (!CreateDirectory(buf, NULL)) {
				swprintf_s(msg, 512, L"Create [%.200s] failed at [%.200s]: Error %d.",
					path, buf, GetLastError());
				MessageBoxW(NULL, msg, _T(__FUNCTION__), 0);
				return FALSE;
			}
		}

		if (ptr == NULL) { return TRUE; }
		ptr++;
		if (*ptr == L'\0') { return TRUE; }
	}

	return TRUE;
}

// Special cases:
//	\\ServerName and \\ServerName\ will always fail.
//	\\ServerName\ShareName will always fail even ShareName exists.
//	\\ServerName\ShareName\ will success if ShareName available.
//	C: will always fail even if drive C: exists.
int SysUtil::IsPathExist(const wchar_t *path)
{
	int rc;
	struct __stat64 st;

	rc = _wstat64(path, &st);
	if (rc == 0) {
		return TRUE;
	} else switch (errno) {
	case ENOENT:
		return FALSE;
	default:
#ifdef _DEBUG
		{
			wchar_t msg[256], errstr[256];
			swprintf_s(msg, 256, L"SysUtil::" _T(__FUNCTION__) L"(%s): %d\n",
				path, _wcserror_s(errstr, 256, errno));
			OutputDebugString(msg);
		}
#endif
		return FALSE;
	}

	return FALSE;
}

int SysUtil::IsDirectory(const wchar_t *path)
{
	int rc;
	struct __stat64 st;

	rc = _wstat64(path, &st);
	if (rc == 0) {
		if (st.st_mode & _S_IFDIR) {
			return TRUE;
		} else {
			return FALSE;
		}
	} else switch (errno) {
	case ENOENT:
		return FALSE;
	default:
#ifdef _DEBUG
		{
			wchar_t msg[256], errstr[256];
			swprintf_s(msg, 256, L"SysUtil::" _T(__FUNCTION__) L"(%s): %d\n",
				path, _wcserror_s(errstr, 256, errno));
			OutputDebugString(msg);
		}
#endif
		return FALSE;
	}

	return FALSE;
}

CString SysUtil::StrToFilename(CString str)
{
	CString filename = str;

	filename.Replace(L"<",	L"%3C");
	filename.Replace(L">",	L"%3E");
	filename.Replace(L":",	L"%3A");
	filename.Replace(L"/",	L"%2F");
	filename.Replace(L"\\",	L"%5C");
	filename.Replace(L"|",	L"%7C");
	filename.Replace(L"?",	L"%3F");
	filename.Replace(L"*",	L"%2A");
	filename.Replace(L" ",	L"%20");
	filename.Replace(L".",	L"%2E");

	return filename;
}

void SysUtil::StrReplaceBlank(char *str)
{
	int	c;

	for (c=0; str[c]!='\0'; c++) {
		if (!isprint(str[c]))	{ str[c] = '_'; }
		else if (str[c]==' ')	{ str[c] = '+'; }
		else if (str[c]=='\t')	{ str[c] = '+'; }
	}
}

void SysUtil::ClearMessageQueue(UINT filter_min, UINT filter_max)
{
	MSG wm;

	while (::PeekMessage(&wm, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)) {
		// Do nothing. Let PeekMessage() do the hard work.
	}
}

// Beagle 20131119
void SysUtil::OpenWithNotepad(const wchar_t *filename)
{
	SHELLEXECUTEINFO	sei;
	DWORD	pid;

	memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.lpVerb = L"open";
	sei.lpFile = L"C:\\WINDOWS\\NOTEPAD.EXE";
	sei.lpParameters = filename;
	sei.nShow = SW_SHOW;
	sei.fMask = SEE_MASK_WAITFORINPUTIDLE | SEE_MASK_FLAG_DDEWAIT | SEE_MASK_NOCLOSEPROCESS;
	ShellExecuteEx(&sei);

	if (sei.hProcess != NULL) {
		pid = GetProcessId(sei.hProcess);
		EnumWindows(EnumWindowsCallback, pid);
	}
}
void SysUtil::SaveUTF8_FromGB(const wchar_t *pDstFileName,const wchar_t *pSrcFileName) //eric chao 20150728
{ //Convert GB to UTF8
	SaveUTF8_FromCodePage(pDstFileName,pSrcFileName,ctCodePage[CT_CODE_GB2312]);
}
void SysUtil::SaveUTF8_FromBIG5(const wchar_t *pDstFileName,const wchar_t *pSrcFileName)
{
	SaveUTF8_FromCodePage(pDstFileName,pSrcFileName,ctCodePage[CT_CODE_BIG5]);
}
void SysUtil::SaveUTF8_FromKorean(const wchar_t *pDstFileName,const wchar_t *pSrcFileName)
{
	SaveUTF8_FromCodePage(pDstFileName,pSrcFileName,ctCodePage[CT_CODE_KOREAN]);
}
void SysUtil::SaveUTF8_FromJIS(const wchar_t *pDstFileName,const wchar_t *pSrcFileName)
{
	SaveUTF8_FromCodePage(pDstFileName,pSrcFileName,ctCodePage[CT_CODE_JIS]);
}
void SysUtil::SaveUTF8_FromKOI8R(const wchar_t *pDstFileName,const wchar_t *pSrcFileName)
{
	SaveUTF8_FromCodePage(pDstFileName,pSrcFileName,ctCodePage[CT_CODE_KOI8R]);
}
void SysUtil::SaveUTF8_FromCodePage(const wchar_t *pDstFileName,const wchar_t *pSrcFileName,int nCodePage)
{
	int nSize = sizeof(ctCodePage) /sizeof(ctCodePage[0]);
	bool bSupport = false;
	for (int i=0;i<nSize;i++){
		if (ctCodePage[i] == nCodePage){
			bSupport = true;
			break;
		}
	}
	if (bSupport){

		HANDLE hSrcFile,hDstFile;
		hSrcFile = CreateFile(pSrcFileName, GENERIC_READ, 0, NULL,
			OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hSrcFile == INVALID_HANDLE_VALUE) { return; }
		LARGE_INTEGER nFileSize;
		DWORD nReadSize = 0;
		GetFileSizeEx(hSrcFile, &nFileSize);
		char *pSrc = new char[nFileSize.QuadPart+1];
		memset(pSrc,0,nFileSize.QuadPart+1);
		if (ReadFile(hSrcFile, pSrc, (DWORD)nFileSize.QuadPart, &nReadSize, NULL)){
			CloseHandle(hSrcFile);
		}
		bool bUniCode = false;
		bool bUTF8 = false;
		if (nFileSize.QuadPart){
			if (*pSrc == (char)0xFF &&  *(pSrc+1) == (char)0xFE){ //Unicode File
				bUniCode = true;
			}
			if (*pSrc == (char)0xEF && *(pSrc+1) == (char)0xBB && *(pSrc+2) == (char)0xBF){ //UTF8 File
				bUTF8 = true;
			}
		}
		if (bUTF8){
			CopyFile(pSrcFileName,pDstFileName,FALSE);
		}else{
			hDstFile = CreateFile(pDstFileName, GENERIC_WRITE, 0, NULL,
				OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hDstFile != INVALID_HANDLE_VALUE){
				wchar_t *pDst = NULL;
				int nLen = 0;
				if (!bUniCode){
					nLen = MultiByteToWideChar(nCodePage, 0, pSrc, nFileSize.QuadPart, NULL,NULL );  //Code Page Convert
					pDst = new wchar_t[nLen+1];
					memset(pDst,0,(nLen+1));
					MultiByteToWideChar(nCodePage, 0, pSrc, nFileSize.QuadPart, pDst, nLen );  //Code Page Convert
				}else{
					pDst = (wchar_t *)(pSrc+2);
					nLen = ((int)nFileSize.QuadPart-2)/2;
				}
				int nUtf8Len = WideCharToMultiByte(CP_UTF8,0,pDst,nLen,NULL,NULL,NULL,NULL);
				char *pDstUtf8 = new char[nUtf8Len+1];
				memset(pDstUtf8,0,nUtf8Len+1);
				WideCharToMultiByte(CP_UTF8,0,pDst,nLen,pDstUtf8,nUtf8Len,NULL,NULL);

				DWORD nWriteCount = 0;
				WriteFile(hDstFile, "\xEF\xBB\xBF", 3, &nWriteCount, NULL); //Write BOM
				WriteFile(hDstFile,pDstUtf8,nUtf8Len,&nWriteCount,NULL);
				SetEndOfFile(hDstFile);
				CloseHandle(hDstFile);
				if (!bUniCode){
					delete []pDst;
				}
				delete []pDstUtf8;
			}
		}
		delete []pSrc;

	}else{
		CString strDump,strFileName;
		strFileName = pDstFileName;
		strDump.Format(_T("Save UTF8 File(%s) Fail!"),strFileName);
		AfxMessageBox(strDump);
	}
}
void SysUtil::ConvertByCodePage(unsigned char *pOut,int nDstSize,unsigned char *pIn,int nSrcSize,int nDstCodePage,int nSrcCodePage) //eric chao 20150917
{
	if (pIn && pOut && nSrcSize && nDstSize){
		wchar_t *pDst = NULL;
		int nLen = 0;
		nLen = MultiByteToWideChar(nSrcCodePage, 0, (char*)pIn, nSrcSize, NULL,NULL );  //Code Page Convert
		pDst = new wchar_t[nLen+1];
		memset(pDst,0,(nLen+1)*sizeof(wchar_t));
		MultiByteToWideChar(nSrcCodePage, 0, (char*)pIn, nSrcSize, pDst, nLen );  //Code Page Convert

		int nDstLen = WideCharToMultiByte(nDstCodePage,0,pDst,nLen,NULL,NULL,NULL,NULL);
		char *pDstStr = new char[nDstLen+1];
		memset(pDstStr,0,nDstLen+1);
		WideCharToMultiByte(nDstCodePage,0,pDst,nLen,pDstStr,nDstLen,NULL,NULL);
		int nSize = nDstLen;
		if (nDstLen > (nDstSize-1)){
			nSize = nDstSize-1;
		}
		memcpy(pOut,pDstStr,nSize);
		pOut[nSize]=0x00;
		delete []pDst;
		delete []pDstStr;
	}
}
void SysUtil::ToUTF8_FromUnicode(unsigned char *pOut,int nDstSize,wchar_t *pIn,int nSrcSize) //eric chao 20151005
{ //nSrcSize ==>Wide Char Len
	if (pOut && pIn && nDstSize!=0 && nSrcSize!=0){
		int nDstLen = WideCharToMultiByte(CP_UTF8,0,pIn,nSrcSize,NULL,NULL,NULL,NULL);
		char *pDstStr = new char[nDstLen+1];
		memset(pDstStr,0,nDstLen+1);
		WideCharToMultiByte(CP_UTF8,0,pIn,nSrcSize,pDstStr,nDstLen,NULL,NULL);
		int nSize = nDstLen;
		if (nDstLen > (nDstSize-1)){
			nSize = nDstSize-1;
		}
		memcpy(pOut,pDstStr,nSize);
		pOut[nSize]=0x00;
		delete []pDstStr;
	}
}
void SysUtil::ToUTF8_FromGB(unsigned char *pOut,int nDstSize,unsigned char *pIn,int nSrcSize) //eric chao 20150917
{
	ConvertByCodePage(pOut,nDstSize,pIn,nSrcSize,ctCodePage[CT_CODE_UTF8],ctCodePage[CT_CODE_GB2312]);
}
void SysUtil::ToUTF8_FromBIG5(unsigned char *pOut,int nDstSize,unsigned char *pIn,int nSrcSize) //eric chao 20150917
{
	ConvertByCodePage(pOut,nDstSize,pIn,nSrcSize,ctCodePage[CT_CODE_UTF8],ctCodePage[CT_CODE_BIG5]);
}
void SysUtil::ToUTF8_FromJIS(unsigned char *pOut,int nDstSize,unsigned char *pIn,int nSrcSize) //eric chao 20150917
{
	ConvertByCodePage(pOut,nDstSize,pIn,nSrcSize,ctCodePage[CT_CODE_UTF8],ctCodePage[CT_CODE_JIS]);
}
void SysUtil::ToUTF8_FromKorean(unsigned char *pOut,int nDstSize,unsigned char *pIn,int nSrcSize) //eric chao 20150917
{
	ConvertByCodePage(pOut,nDstSize,pIn,nSrcSize,ctCodePage[CT_CODE_UTF8],ctCodePage[CT_CODE_KOREAN]);
}
void SysUtil::ToUTF8_FromKOI8R(unsigned char *pOut,int nDstSize,unsigned char *pIn,int nSrcSize) //eric chao 20150917
{
	ConvertByCodePage(pOut,nDstSize,pIn,nSrcSize,ctCodePage[CT_CODE_UTF8],ctCodePage[CT_CODE_KOI8R]);
}

CString SysUtil::MakeUnicode_CodePage(const char *pData,int nCodePage) //eric chao 20150903
{//Check Code Page is Support
	int nSize = sizeof(ctCodePage) /sizeof(ctCodePage[0]);
	bool bSupport = false;
	for (int i=0;i<nSize;i++){
		if (ctCodePage[i] == nCodePage){
			bSupport = true;
			break;
		}
	}
	CString strResult = _T("Not Support!");
	if (bSupport){
		wchar_t *pBuffer = NULL;
		int buflen =0;
		int nDataSize  = (int)strlen(pData);
		buflen = MultiByteToWideChar(nCodePage, 0,pData,nDataSize,NULL,NULL);

		pBuffer = new wchar_t[buflen+1];
		if (pBuffer){
			memset(pBuffer, 0, sizeof(wchar_t)*(buflen + 1));

			MultiByteToWideChar(nCodePage, 0, pData, nDataSize, pBuffer, buflen);

			strResult = pBuffer;
			delete[]pBuffer;
		}
	}
	return strResult;
}
CString SysUtil::MakeUnicode_GB(const char *pData)
{
	return MakeUnicode_CodePage(pData,ctCodePage[CT_CODE_GB2312]);
}
//eric chao 20150729
CString SysUtil::MakeUnicode_UTF8(const char *pData)
{ //Convert UTF8 to Unicode
	return MakeUnicode_CodePage(pData,ctCodePage[CT_CODE_UTF8]);
}
//eric chao 20150903-------------------
CString SysUtil::MakeUnicode_BIG5(const char *pData)
{
	return MakeUnicode_CodePage(pData,ctCodePage[CT_CODE_BIG5]);
}
CString SysUtil::MakeUnicode_Korean(const char *pData)
{
	return MakeUnicode_CodePage(pData,ctCodePage[CT_CODE_KOREAN]);
}
CString SysUtil::MakeUnicode_JIS(const char *pData)
{
	return MakeUnicode_CodePage(pData,ctCodePage[CT_CODE_JIS]);
}
CString SysUtil::MakeUnicode_KOI8R(const char *pData)
{
	return MakeUnicode_CodePage(pData,ctCodePage[CT_CODE_KOI8R]);
}
//--------------------------------------

// Beagle 20131119
// Save wchar_t string as UTF-8 file. Truncate old file if exist.
void SysUtil::SaveTextFile_UTF8(const wchar_t *filename, const wchar_t *text)
{
	HANDLE hFile;
	DWORD count;
	char *buffer;
	int buflen, textlen;

	textlen = (int) wcslen(text);
	buflen = WideCharToMultiByte(CP_UTF8, 0, text, textlen,
		NULL, 0, NULL, NULL);

	buffer = (char*) malloc(buflen);
	if (buffer == NULL) { return; }
	memset(buffer, 0, buflen);

	WideCharToMultiByte(CP_UTF8, 0, text, textlen,
		buffer, buflen, NULL, NULL);

	hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) { return; }

	WriteFile(hFile, "\xEF\xBB\xBF", 3, &count, NULL);	// UTF-8 Byte Order Mark
	WriteFile(hFile, buffer, buflen, &count, NULL);

	SetEndOfFile(hFile);
	CloseHandle(hFile);

	free(buffer);
}

void SysUtil::WriteFile_UTF8(HANDLE hFile, int wbufsize, const wchar_t *wbuf)
{
	DWORD count;
	char *buffer;
	int buflen, textlen;

	textlen = (int) wcslen(wbuf);
	buflen = WideCharToMultiByte(CP_UTF8, 0, wbuf, wbufsize,
		NULL, 0, NULL, NULL);

	buffer = (char*) malloc(buflen);
	if (buffer){
		memset(buffer, 0, buflen);
		WideCharToMultiByte(CP_UTF8, 0, wbuf, wbufsize,
			buffer, buflen, NULL, NULL);

		WriteFile(hFile, buffer, buflen, &count, NULL);

		free(buffer);
	}
}

/* End of SysUtil.cpp */
