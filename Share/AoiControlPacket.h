#pragma once


#include <vector>
#include "hexparser.h"
/******************************************************************
 *  Communication Header Definition Update 
 *****************************************************************/
// Command Characters
#define END_CHARACTER			255
#define START_CHARACTER 		254

#define SETUP_COMMAND			253
#define ACTION_COMMAND			252
#define ECHO_COMMAND			248
#define MASS_DATA_COMMAND		247
#define MULTI_PURPOSE_COMMAND	245
#define DLE_COMMAND				242

#define	ALIGN_RADIX			256
#define CHECKSUM_RADIX		240


// -------------------------------------------------------------------------------------
// Command Communication functions
/**************************************************************************************
Message Body
────────────────────────
︱Start Character                               ︱
────────────────────────
︱Setup、Action、Network、Firmware、Echo Command︱
────────────────────────
︱isEchoing                                     ︱
────────────────────────
︱Command Sequence                              ︱
────────────────────────
︱Command Body                                  ︱
────────────────────────
︱Check Sum                                     ︱
────────────────────────
︱End Character                                 ︱
────────────────────────
**************************************************************************************/

//#define SUPPORT_SERIAL
#define SUPPORT_SOCKET
#ifdef SUPPORT_SOCKET
#endif //SUPPORT_SOCKET
#ifdef SUPPORT_SERIAL
#include <SerialComm\SerialMFC.h>
#endif //SUPPORT_SERIAL

#pragma pack(push,1)

struct AOI_CTRL_HEADER
{
BYTE STARTCODE; //0x254
BYTE CommandType;
BYTE IsEchoing;
BYTE CommandSequence; //%240
};
//Command Body
struct AOI_CTRL_TAIL
{
BYTE CheckSum;
BYTE ENDCODE; //0x255
};
struct SETUP_COMMAND_PKT
{
BYTE SlaveDeviceId;
BYTE SlaveDeviceParam;
WORD SlaveDeviceValue;
};
struct ACTION_COMMAND_PKT
{
BYTE SlaveDeviceId;
BYTE ActionParam;
};
struct ECHO_COMMAND_PKT
{
//NULL
};
struct MASS_DATA_COMMAND_HEADER
{
BYTE DataType;
BYTE DataSize;
//BYTE* pData;
//Data[]
};
struct MULTI_PURPOSE_COMMAND_HEADER
{
BYTE MPCMD;
DWORD MPDataSize;
//BYTE* pMPData;
//MPData[]
};

struct AOI_CTRL_PACKET
{
AOI_CTRL_HEADER AoiHeader;
union
{
	SETUP_COMMAND_PKT Setup;
	ACTION_COMMAND_PKT Action;
	MASS_DATA_COMMAND_HEADER MassData;
	MULTI_PURPOSE_COMMAND_HEADER MPData;
}CommandBody;
};

#pragma pack(pop)
typedef struct WAITITEM_ {
	BYTE	commandSequence;
	BYTE	*outData;
	DWORD	outLength;

	BOOL	needEcho;
	CString errorMsg;
	ULONGLONG	outTimeTick;
	int		timeoutCount;
} WaitItem;
typedef struct TIMEOUTERR_ {
	BYTE	commandSequence;
	CString	errorMsg;
} TimeoutErr;





#define REQUESTTIMEOUT		500
#define MAXTIMEOUTCOUNT		10


#define LM_USB_PACKET_RESEND_TIMER 300
#define LM_COM_PACKET_RESEND_TIMER 100
#define LM_LAN_PACKET_RESEND_TIMER 100
#define DEFAULT_PACKET_RESEND_TIMER 100  //ms
#define DEFAULT_USB_NORMAL_CHECK 300
#define DEFAULT_USB_HIGHTSPEED_CHECK 100 //for update use
#define DEFAULT_CHECK_DEVICE_TIMER 10000
#define DEFAULT_QUERYALIVE_TIMER 10000
#define IO_PACKET_RESEND_TIMER 50

#define MAX_RECEIVE_BUFFER_SIZE 64000

class CAOIPacketWnd;


enum {CONNECT_USB=1,
	  CONNECT_COM=2,
	  CONNECT_LAN=3};

class CAoiConnectSocket :public CAsyncSocket
{
	DECLARE_DYNAMIC(CAoiConnectSocket);
public:
	bool IsConnected() {return m_bConnect;};
	CAoiConnectSocket(HWND pDestWnd);
	virtual ~CAoiConnectSocket();
	virtual void OnReceive(int nErrorCode);
	virtual void OnClose(int nErrorCode);
	virtual void OnConnect(int nErrorCode);
	virtual void OnAccept(int nErrorCode);

	CAoiConnectSocket* GetActiveSocket() {return m_pProcessSocket;};
private:
	CAoiConnectSocket* m_pProcessSocket;
	HWND m_hDestWnd;
	bool m_bConnect;
};

//------------------------------------------------------
// for upgrade device,need HexParser.dll to make packet
//------------------------------------------------------

class IBaseNotify
{
public:
	virtual void OnActionNotify(BYTE ActionParam)=0;
	virtual void OnSetupNotify(BYTE DeviceParam,WORD tValue)=0;
	virtual void OnMassDataNotify(MASS_DATA_COMMAND_HEADER* pMassData,int iSize)=0;
	virtual void OnMpNotify(MULTI_PURPOSE_COMMAND_HEADER *pMPDataHdr,int iSize)=0;
	virtual void OnEchoNotify(AOI_CTRL_HEADER *pHdr)=0;
	virtual void OnUpdateFirmwareNotify(int NotifyID)=0;
};

class IDevice //COM,USB,Socket....IO Board/LD
{
public:
	virtual void OnError(void* pInstance,long ErrorMsg,long ErrorId) =0;
	virtual bool IsOpenDevice(void* pInstance) =0;
	virtual long ReceiveData(void* pInstance,unsigned char *pReadBuffer,int ReadDataSize,DWORD &BytesRead) =0;
	virtual long SendData(void* pInstance,unsigned char *pSendBuffer,int SendDataSize,int &BytesSend) =0;
	virtual bool ReOpen(void* pInstance) =0;
};

enum {DEVICE_IO=1,DEVICE_LM=2,DEVICE_BASIC=3,DEVICE_RS2CAN=4};
class AoiControlPacket
{
protected:
	enum {
		ERR_CMD_CHECKSUM=0,
		ERR_BUF_FULL=1,
		ERR_BUF_DUMP
	};
public:
	virtual void SendQueryAlive() {}; //eric chao 20150320
	void DumpCurrentBuf(); //eric chao 20150316
	AoiControlPacket(void);
	~AoiControlPacket(void);
	bool ParseCommand(BYTE * pData, int DataSize);
	void ParseBLCommand(BYTE *pData,int DataSize);
	virtual void DumpErrData(int ErrCode,BYTE *pData,int DataSize) {}; //eric chao 20150316
	int AddDle_InCommandBody(BYTE *pCommandBody,int BodyLength,CByteArray *rst);
	void RemoveDLE_InCommandBody(BYTE *pData,int &DataSize);
	void RemoveWaitingItem(BYTE);
	void addWaitingItem(BYTE commandSequence, BYTE *outData, DWORD outLength, ULONGLONG timeTick, CString errMsg);
	void updateWaiting(void);
	void ClearNoUseWaiting();
	BOOL ifWaiting(void);
	

	
	int  sendActionRequest(BYTE Device,BYTE Param, BOOL isWaitResponse,CString errmsg, int comSequence=-1);
	int  sendSetupRequest(BYTE Device,BYTE Param,WORD value, BOOL isWaitResponse,CString errmsg, int comSequence=-1);
	int  sendMassDataRequest(BYTE DataType,BYTE DataSize,BYTE *pMassData,BOOL isWaitResponse,CString errmsg, int comSequence=-1);
	int  sendMPRequest(BYTE MPCmd,long MPSize,BYTE *pMPData,BOOL isWaitResponse,CString errmsg, int comSequence=-1);

	BOOL sendSetupCommand(BYTE Device,BYTE Param,WORD value, BOOL isWaitResponse,CString errmsg, int comSequence=-1);
	BOOL sendActionCommand(BYTE Device,BYTE Param, BOOL isWaitResponse,CString errmsg, int comSequence=-1);
	BOOL sendMassDataCommand(BYTE DataType,BYTE DataSize,BYTE *pMassData,BOOL isWaitResponse,CString errmsg, int comSequence=-1); //pMassData ==> MASS_DATA_COMMAND_HEADER *ptr 
	BOOL sendMPCommand(BYTE MPCmd,long MPSize,BYTE *pMPData,BOOL isWaitResponse,CString errmsg, int comSequence=-1); //pMPData ==> MULTI_PURPOSE_COMMAND_HEADER *ptr
	BOOL sendEchoCommand(BYTE comSeq,CString errmsg = _T("Send ECHO command fail..."),bool isWaitResponse=false);	

	bool IsOpenDevice() {if (m_pDevice) return m_pDevice->IsOpenDevice(this); return false;};
	BOOL IsDeviceExist() {return m_pDevice!=NULL;};
	void Attach(IDevice *pDevice) ;
	CWnd* GetAoiPacketWnd() {return m_pAOIPacketWnd;};
	void ReceiveAllData();
#ifdef SUPPORT_SOCKET
	void ReceiveSocketData(BYTE *pBuffer,int dwSize);
#endif //SUPPORT_SOCKET

	virtual int sendInternalModelTypeRequest(CString errmsg=_T("Device internal model type failed...")){return -1;};
	virtual void QueryVerionInfo() { sendEchoCommand(0,_T("Detect Aoi device fail..."),true);};

	BOOL sendIsBootLoadRequest(void);
	BOOL sendRestartRequest(void);
	BOOL sendFWPackage(BYTE *rawData, int dataLength, BOOL resend=FALSE, CString errmsg = _T("Setup firmware package fail..."));
	BOOL IsSupportInternalMode() {return m_bIsSupportInternalMode;};
	BOOL IsAbnormal() {return m_bIsabnormal;}; //For Check Command Timeout status
	TimeoutErr GetError() {return m_xFirstTimeoutError;};
	TimeoutErr GetLastError() {return m_xLastTimeoutError;};

	int GetDeviceType() {return m_iDeviceType;};
	int GetConnectType() {return m_iConnectType;};
	BYTE GetCheckSum() {return m_CheckSum;};
	BYTE GetCmdSeq() {return m_commandSequence;};
	BYTE GetResponseCmdSeq() {return m_ResponseCmdSeq;}; //eric chao 20130819
	void AttachNotify(IBaseNotify *pNotify) {m_pNotify=pNotify;};
	void SetDebugMode(bool bDebug);
	bool IsDebugMode() {return m_bDebugMode;};

	BOOL	IsBootLoader() {if (m_isBootLoader == MODE_FIRMWARE) return FALSE; else return TRUE;};
	//void	SetIsBootLoader(int bl) {m_isBootLoader = bl;};
	WORD	GetInternalModelType() {return m_wModelType;};
	enum {MODE_FIRMWARE=-1,MODE_BOOTLOADER=1};
	enum { UPDATE_BLECHO = 0, UPDATE_MSG = 1, UPDATE_ENTERBL = 2, UPDATE_TRANSCARD_MSG = 3, UPDATE_ENCODER_CARD_MSG = 4};
	void SetConnectType(int ConnectType); //USB Type need Enable Timer to Receive data
	void CheckDeviceMode();
	void SetServerMode(bool bServer) {m_bServer=bServer;};
	int GetDeviceId() {return m_iDeviceId;};
	void SetDeviceId(int id) {m_iDeviceId=id;}; //Range 0-15 0==>InitStatus
	void SetDeviceReady(bool bReady) { m_bDeviceReady=bReady; if (bReady) AutoConfigCmdWaitTimer();};
	bool IsDeviceReady() {return m_bDeviceReady;};

	void ErrorProcess(long ErrorMsg,long ErrorId);
	ULONGLONG GetEchoTime() {return m_tEchoTime;};
	void ReOpen();
	void CleanReceiveBuffer();
protected:
	virtual void BootLoaderEcho() {};
	virtual void NotifyBootLoaderMode(int nDeviceId=0) {}; //eric chao 20130906
	virtual void ParseSetupCommand(SETUP_COMMAND_PKT *pPkt,int iSize){};
	virtual void ParseActionCommand(ACTION_COMMAND_PKT *pPkt,int iSize){};
	virtual void ParseMassDataCommand(MASS_DATA_COMMAND_HEADER *pMassDataHdr,int iSize){};
	virtual void ParseEchoCommand(AOI_CTRL_HEADER *pHdr){};
	virtual void ParseMultiPurposeCommand(MULTI_PURPOSE_COMMAND_HEADER *pMPDataHdr,int iSize){};
	void SetDeviceConnectType(int DeviceType,int ConnectType);
	virtual CString GetControlName() {return _T("AOIPACKET");};
	virtual long GetWndResourceId() {return (m_lWndResourceId+m_iDeviceId);};
	WORD SWAP(WORD tData);
	DWORD SWAP(DWORD);
	short SWAP(short);
	long SWAP(long);
	UINT64 SWAP(UINT64); //eric chao 20160727
	BOOL IsService() {return m_bService;};
	BOOL StartService();
	BOOL StopService();
	std::vector<WaitItem*>	m_waitItems;
	BOOL sendCommand(BYTE commandType, BYTE *rawData, int dataLength, BOOL isWaitResponse, CString errmsg, int comSequence = -1);
	long m_lWndResourceId;
	
	BOOL sendBootCommand(BYTE *rawData, int dataLength, BOOL isWaitResponse, CString errmsg, int comSequence = -1);		
	BOOL m_bIsSupportInternalMode;
	IBaseNotify *m_pNotify;
	int	m_isBootLoader;		// -1:firmware, 1:BootLoader
	WORD m_wModelType; //InternalMode define by hexparser.h
private:
	ULONGLONG m_tEchoTime;
	bool m_bDeviceReady;
	int	 m_iDeviceId;
	bool m_bServer;
	bool m_bDebugMode;
	void AutoConfigCmdWaitTimer(); //eric chao 20131219
	bool MovePacketToStartCode(); //eric chao 20130902
	void CheckPacketInBuffer();
	void Lock(BOOL bFlag);
	int m_dReceiveBufferIndex;
	BYTE m_dReceiveBuffer[MAX_RECEIVE_BUFFER_SIZE];
	void CleanWaitItem();
	int m_iDeviceType;
	int m_iConnectType;
	BOOL m_bService;
	void Init(); 
	CWnd *m_pAOIPacketWnd;
	void MakeAoiPacket(CByteArray &Packet,BYTE commandType, BYTE *rawData, int dataLength, BOOL isWaitResponse,int comSequence);
	bool CheckSum(BYTE* pData, int DataSize);
	CWnd* m_pProcessWnd;
	IDevice *m_pDevice;
	int m_commandSequence;
	int m_ResponseCmdSeq;
	BYTE m_CheckSum;
	BOOL m_bIsabnormal;
	TimeoutErr m_xFirstTimeoutError;
	TimeoutErr m_xLastTimeoutError;
	CRITICAL_SECTION m_xPackLock; //20180706
};


// CAOIPacketWnd
#define WM_PACKET_CHECKTIME WM_APP+100
#define WM_USB_DATA_TIME WM_APP+101
#define WM_DEVICE_READY WM_APP+102
#define WM_QUERY_ALIVE	WM_APP+103 //eric chao 20150320

// Socket Process---- Merage with IOSocketThread Message
#define WM_SOCKET_PROCESS WM_APP+110
#define WM_SOCKET_RECEIVE WM_APP+111
#define WM_SOCKET_CONNECT WM_APP+112
#define WM_SOCKET_CLOSE	  WM_APP+113
#define WM_SOCKET_PARSER  WM_APP+114

#define WM_SOCKET_ERROR	  WM_APP+120

class CAOIPacketWnd : public CDialog //use for show the detail message
{
	DECLARE_DYNAMIC(CAOIPacketWnd)

public:
	CAOIPacketWnd(AoiControlPacket* pPacket);
	virtual ~CAOIPacketWnd();
	virtual BOOL OnInitDialog();
#ifdef SUPPORT_SERIAL
	afx_msg LRESULT	OnSerialMsg(WPARAM wParam, LPARAM lParam);
#endif //SUPPORT_SERIAL

	void SetInputData(BYTE* pInput,int iSize);
	void SetOutputData(BYTE* pOutput,int iSize);
	void SetRemoveSeq(int seq) {m_iLastRemoveSeq=seq;this->Invalidate();};
protected:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg LRESULT OnChangeTimer(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUSBTimer(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDeviceReady(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnQueryAlive(WPARAM wParam,LPARAM lParam); //eric chao 20150320
	afx_msg LRESULT OnSocketProcess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSocketError(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
private:
	int m_iLastRemoveSeq;
	CString m_xMode;
	CString m_xInputCmd;
	CString m_xOutputCmd;
	enum {TIMER_CHECK_PACKET_IN_QUEUE=500,TIMER_CHECK_DEVICE,TIMER_USB_DEVICE,TIMER_DEVICE_QUERYALIVE}; //TIMER_USB_DEVICE using for get USB data
	AoiControlPacket* m_pLinkPacket;
	long m_lElapseTime;
	long m_lCheckTime;
	long m_lQueryAliveTime;
	long m_lUsbTime;
	UINT_PTR m_lDeviceQueryAliveTimer;
	UINT_PTR m_lCheckWaitingTimer;
	UINT_PTR m_lCheckDeviceTimer;
	UINT_PTR m_lUsbDataTimer;
//********socket receive thread********
    enum
    {
        EV_EXIT,
        EV_RECEIVE,
        EV_REVEIVE_OK,
        EV_COUNT
    };
    HANDLE m_hThread;
    HANDLE m_hEvent[ EV_COUNT ];

    static DWORD __stdcall Thread_SocketReceive( void* pvoid );
//*************************************
};


