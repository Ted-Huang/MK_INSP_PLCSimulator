#pragma once

//----------------------------------------------------------
// LightModuleControl 燈具控制盒,實體連接層 (COM/USB/LAN)
// 用來串聯多個燈具的控制,但是一次最多只能連上其中一個
// 實體的控制處理,在這層處理
// ---------------------------------------------------------
// 透過 LightSourceController (燈具控制盒,虛擬控制層)
// 針對燈具的Protocol,在這層處理增加
// ---------------------------------------------------------
// 底層處理 AoiControlPacket (資料處理層)
// 通用的資料處理,Parser,在這層處理
// eric chao
//----------------------------------------------------------

#include "lightsourcecontroller.h"
#include "ping.h"

//#define SUPPORT_USB
//#define SUPPORT_COM

#ifdef SUPPORT_USB
#include "siusbxp.h"
//#pragma comment(lib, "siusbxp.lib")
#ifdef _WIN64
#pragma comment(lib, "..\\USBDriver\\x64\\SiUSBXp.lib")
#else
#pragma comment(lib, "..\\USBDriver\\x86\\SiUSBXp.lib")
#endif
#endif //SUPPORT_USB

#ifdef USE_SOCKET_THREAD
#include "IoSocketThread.h"
#endif //USE_SOCKET_THREAD

#ifdef _DEBUG
//#define LIGHT_SIMULATION
#endif

struct IP_MAPINFO
{
	BYTE DeviceId;
	CString IpAddress;
};

#ifdef LIGHT_SIMULATION
const IP_MAPINFO LAN_DEVICE_IPRANGE[]={0,_T("127.0.0.1"),1,_T("10.0.0.1"),2,_T("10.0.0.1")\
									,3,_T("10.0.0.1"),4,_T("10.0.0.1"),5,_T("10.0.0.1")\
									,6,_T("10.0.0.1"),7,_T("10.0.0.1"),8,_T("10.0.0.1")\
									,9,_T("10.0.0.1"),10,_T("10.0.0.1"),11,_T("10.0.0.1")\
									,12,_T("10.0.0.1"),13,_T("10.0.0.1"),14,_T("10.0.0.1")\
									,15,_T("10.0.0.1")};
const int LAN_DEVICE_PORT[]={1000,1001,1002,1003,1004,1005,1006,1007,1008,1009,0110,1011,1012,1013,1014,1015};
#else
//10.0.0.10~10.0.0.25,Mac From 00-04-A3-00-00-00 ~ 00-04-A3-00-00-15
const IP_MAPINFO LAN_DEVICE_IPRANGE[]={0,_T("10.0.0.10"),1,_T("10.0.0.11"),2,_T("10.0.0.12")\
									,3,_T("10.0.0.13"),4,_T("10.0.0.14"),5,_T("10.0.0.15")\
									,6,_T("10.0.0.16"),7,_T("10.0.0.17"),8,_T("10.0.0.18")\
									,9,_T("10.0.0.19"),10,_T("10.0.0.20"),11,_T("10.0.0.21")\
									,12,_T("10.0.0.22"),13,_T("10.0.0.23"),14,_T("10.0.0.24")\
									,15,_T("10.0.0.25")};
#endif
#define PORT_LM	1000

#define COM_LM _T("COM3")
#define MAX_DEVICE 16
#define DEFAULT_DEVICE_ID 0


#define WM_THREAD_DEVICE_REMOVE WM_APP+400
#define WM_THREAD_DEVICE_MSG	WM_APP+600
#define WM_DEVICE_QUIT			WM_APP+601
//detect USB first....Com next..Lan last

class LightModuleControl :	public IDevice//,public CWinThread
{ //in COM USB status,Only One Device... in Lan status,Maybe have more Device to Control
	//need Keep connection
//	DECLARE_DYNAMIC(LightModuleControl)
public:
	LightModuleControl(void);
	~LightModuleControl(void);
	void Init();
	void StartDetect();
//use echo command or ASK_BOOTLOADER_VERSION to detect Device 
	void DetectNetworkDevice();
	void OpenNetworkDevice(); //need pushback DeviceId List
	void OpenUsbDevice();
	void OpenComDevice();

	BOOL UsbSetBaudRate(DWORD dwBaudRate=115200);

	bool DetectUsbDevice();
	
	void RemoveDevice(int iDeviceId);

	void OpenDevice();
	void CloseDevice();

	virtual void OnError(void *pInstance,long ErrorMsg,long ErrorId);
	virtual bool IsOpenDevice(void* pInstance) ;
	virtual long ReceiveData(void* pInstance,unsigned char *pReadBuffer,int ReadDataSize,DWORD &BytesRead) ;
	virtual long SendData(void* pInstance,unsigned char *pSendBuffer,int SendDataSize,int &BytesSend) ;
	virtual bool ReOpen(void* pInstance) ;

	void SetServerMode(bool bServer) {m_bServer=bServer;};
	void SetAutoDetectLan(bool bDetect) {m_bAutoDetectLan=bDetect;};
	void SetMasterMode(bool bMaster) {m_bMaster=bMaster;};
	BOOL AttachDeviceId(int iDeviceId);
	LightSourceController* GetCurrentControl() {return m_pCurrentControl;};
	void GetCurrentAllDeviceId(std::vector<int> &xDeviceList);
	void GetAllDeviceIdInfo(std::vector<int> &vDeviceList, std::vector<int> &vFlashLed);
	void GetAllLightType(std::vector<int> &xDeviceList); //eric chao 20161026
	void GetAllLightParam(std::vector<LIGHT_PARAM> &xParamList); //eric chao 20161026
	int  GetLightType(int nDeviceId); //eric chao 20161027

	void SetDeviceTemperatureLimit(int iDeviceId,unsigned char *limitT,int no,BOOL bUseDefalut); //20140516-01
	void ResetAllLightParamCheck(); //seanchen 20151001
	BOOL GetLightParamCheck(int iDeviceId); //seanchen 20151001
	LIGHT_PARAM LightModuleControl::GetLightParam(int iDeviceId); //seanchen 20151002

private:
	LightSourceController *m_pCurrentControl;
	bool m_bAutoDetectLan;	
	bool m_bMaster;
	bool m_bServer;
#ifdef SUPPORT_COM
	CSerialMFC *m_pComm; //For Com
#endif //SUPPORT_COM
#ifdef SUPPORT_USB
	HANDLE m_usbHandle; //USB BaudRate
#endif //SUPPORT_USB
	CAoiConnectSocket *m_pSocket; //Direct Device
#ifdef USE_SOCKET_THREAD
	CWinThread *m_SocketThread;
#endif //USE_SOCKET_THREAD

	std::vector<CString>	m_deviceArray;
	//----use in multi connect for multi device
	std::vector<LightSourceController*> m_xLightControlList;
	std::vector<int> m_xDeviceList;
	void MakeLightControlList();
	void CleanArpTable(); //eric chao 20121212
protected:
	afx_msg void OnDeviceMessage(WPARAM wParam, LPARAM lParam);
//	DECLARE_MESSAGE_MAP()
};
