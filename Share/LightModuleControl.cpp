#include "Stdafx.h"
#include "LightModuleControl.h"
#include "Iphlpapi.h" //for use arp


/*
IMPLEMENT_DYNAMIC(LightModuleControl, CWinThread)

BEGIN_MESSAGE_MAP(LightModuleControl, CWinThread)
	ON_THREAD_MESSAGE(WM_THREAD_DEVICE_MSG,OnDeviceMessage)
END_MESSAGE_MAP()
*/


DWORD WINAPI DetectDeviceThread(LPVOID lParam)
{
	unsigned long* pDeviceId=(unsigned long*)lParam;
	if (!AfxSocketInit()){
		TRACE("\nInit Socket Error!");
	}
	
	CPing xPing;
	CPingReply xPingRpy;
	xPingRpy.RTT=300;
	xPing.PingUsingICMP(LAN_DEVICE_IPRANGE[*pDeviceId].IpAddress,xPingRpy,10,50);
	TRACE("\nPing%d,%d",*pDeviceId,xPingRpy.RTT);
	if (xPingRpy.EchoReplyStatus == IP_SUCCESS){
		*pDeviceId=xPingRpy.RTT;
	}else {
		*pDeviceId=300;
	}
	return 0;
}

LightModuleControl::LightModuleControl(void)
{ //if IDevice is ready...then attach the device //this class need modify the connection
	if (!AfxSocketInit()){	// added by eric_chao at 20120905
	}
	Init();	
}

LightModuleControl::~LightModuleControl(void)
{
	m_xDeviceList.clear();
	while (m_deviceArray.size()){
		m_deviceArray.erase(m_deviceArray.begin());
	}		
	CloseDevice();
}

void LightModuleControl::OnDeviceMessage(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case WM_DEVICE_QUIT:
		::AfxEndThread(0);
		break;
	};
}

void LightModuleControl::Init()
{
	m_xDeviceList.clear();	
	RemoveDevice(-1);
#ifdef SUPPORT_COM
	m_pComm=NULL;
#endif //SUPPORT_COM
#ifdef SUPPORT_USB
	m_usbHandle=NULL;
#endif //SUPPORT_USB
	m_pSocket=NULL;
	m_pCurrentControl=NULL;
	m_bAutoDetectLan=true;	
	m_bMaster=false;
	m_bServer=false;
}
void LightModuleControl::StartDetect()
{
	m_xDeviceList.clear();
	for (int i=0;i<MAX_DEVICE;i++){
		m_xDeviceList.push_back(i);
	}
}
void LightModuleControl::RemoveDevice(int iDeviceId)
{
	if (iDeviceId==-1){
		//Remove All Device
		while (m_xLightControlList.size()){
			LightSourceController *pDevice=m_xLightControlList[0];
			delete pDevice;
			m_xLightControlList.erase(m_xLightControlList.begin());
		}		
	}
	else {
		int iSize=(int)m_xLightControlList.size();
		for (int i=0;i<iSize;i++){
			if (iDeviceId==m_xLightControlList[i]->GetDeviceId()){
				LightSourceController *pDevice=m_xLightControlList[i];				
				delete pDevice;
				m_xLightControlList.erase(m_xLightControlList.begin()+i);
				break;
			}
		}
	}
}
void LightModuleControl::CleanArpTable() //eric chao 20121212
{
	ULONG xSize=0;
	GetIpNetTable(NULL,&xSize,FALSE);
	if (xSize){
		BYTE* pByte=NULL;
		pByte=new BYTE[xSize];
		GetIpNetTable((PMIB_IPNETTABLE)pByte,&xSize,FALSE);
		int iSize=((PMIB_IPNETTABLE)pByte)->dwNumEntries;
		for (int i=0;i<iSize;i++)
		DeleteIpNetEntry(&((PMIB_IPNETTABLE)pByte)->table[i]);
		delete []pByte;
	}

}
void LightModuleControl::DetectNetworkDevice()
{
	CPing xPing;
	CPingReply xPingRpy;
	m_xDeviceList.clear();
	//WinExec("arp -d",SW_HIDE); //replace by CleanArpTable
	CleanArpTable();
	HANDLE DetectThreadHandle[MAX_DEVICE];
	unsigned long iTemp[MAX_DEVICE];
	for (int i=0;i<MAX_DEVICE;i++){
		DetectThreadHandle[i]=NULL;
		iTemp[i]=i;
		DetectThreadHandle[i]=CreateThread(NULL, 0, DetectDeviceThread, (void*)&iTemp[i], 0, NULL);
	}
	WaitForMultipleObjects(MAX_DEVICE, DetectThreadHandle, TRUE, INFINITE);
	for(int j=0; j<MAX_DEVICE; j++){
		if (iTemp[j] <50)
			m_xDeviceList.push_back(j);
		if (DetectThreadHandle[j]){
			CloseHandle(DetectThreadHandle[j]);
		}
	}
}
void LightModuleControl::OpenNetworkDevice()
{
	if (m_pCurrentControl){
		if (m_pSocket){
			delete m_pSocket;
			m_pSocket=new CAoiConnectSocket(m_pCurrentControl->GetAoiPacketWnd()->GetSafeHwnd());
		}
		else{
			m_pSocket=new CAoiConnectSocket(m_pCurrentControl->GetAoiPacketWnd()->GetSafeHwnd());
		}
		if (m_pSocket){			
			m_pSocket->Create();
#ifdef LIGHT_SIMULATION
			m_pSocket->Connect(LAN_DEVICE_IPRANGE[m_pCurrentControl->GetDeviceId()].IpAddress,LAN_DEVICE_PORT[m_pCurrentControl->GetDeviceId()]);
#else
            m_pSocket->Connect(LAN_DEVICE_IPRANGE[m_pCurrentControl->GetDeviceId()].IpAddress,PORT_LM);
#endif
			m_pCurrentControl->Attach(this);
		}

	}
}

bool LightModuleControl::DetectUsbDevice()
{
#ifdef SUPPORT_USB
	DWORD dwNumDevices = 0;
	SI_DEVICE_STRING devStr;

	m_deviceArray.clear();
	SI_STATUS res = SI_GetNumDevices(&dwNumDevices);
	if (res == SI_SUCCESS)
	{
		for (DWORD i=0; i<dwNumDevices; i++)
		{
			if (SI_GetProductString(i, devStr, SI_RETURN_DESCRIPTION) == SI_SUCCESS)
			{
				// add the device name to usb array
				CString str = CA2T(devStr);
				m_deviceArray.push_back(str);
			}
		}

		// find at least one device, return true
		// else return false
		if (m_deviceArray.size() > 0)
			return TRUE;
		else 
			return FALSE;
	}
#endif //SUPPORT_USB
	// can not execute SI function !?
	return FALSE;
}
BOOL LightModuleControl::UsbSetBaudRate(DWORD dwBaudRate)
{
#ifdef SUPPORT_USB
	if (m_usbHandle){
	SI_STATUS retStatus;
	retStatus = SI_SetBaudRate(m_usbHandle, dwBaudRate);
		if (retStatus == SI_SUCCESS)
		{
			return TRUE;
		}
		else 
		{
			TRACE0("Failed to set usb baudrate\n");
		}
	}
#endif //SUPPORT_USB
	return FALSE;
}
void LightModuleControl::OpenUsbDevice()
{ //siusbxp.h ==> cp2102
	if (DetectUsbDevice()){
#ifdef SUPPORT_USB
		SI_STATUS retStatus = SI_Open(0, &m_usbHandle);	// always use the first device found in the device array
		if (retStatus == SI_SUCCESS)
		{
			retStatus = SI_FlushBuffers(m_usbHandle, TRUE, TRUE);
			if (retStatus == SI_SUCCESS)
			{
				if (m_pCurrentControl){
					m_pCurrentControl->SetConnectType(CONNECT_USB);
					UsbSetBaudRate();
					::Sleep(100);
					m_pCurrentControl->Attach(this);
				}
			}
			else
			{
				SI_Close(m_usbHandle);
				m_usbHandle = NULL;
			}
		}
		else
		{
			TRACE0("Failed to open usb device\n");
			m_usbHandle = NULL;
		}
#endif //SUPPORT_USB
	}
}
void LightModuleControl::OpenComDevice()
{
	if (GetCurrentControl()){
#ifdef SUPPORT_COM
		if (m_pComm==NULL){
			m_pComm=new CSerialMFC();
		}
		if (m_pComm){
			int tRet=m_pComm->Open(COM_LM,GetCurrentControl()->GetAoiPacketWnd(),0,false);
			if (tRet==ERROR_SUCCESS){
				if (m_pComm->Setup(CSerial::EBaud115200, CSerial::EData8, CSerial::EParNone, CSerial::EStop1) == ERROR_SUCCESS){
					if (m_pCurrentControl){
						m_pCurrentControl->SetConnectType(CONNECT_COM);
						m_pCurrentControl->Attach(this);
					//m_pPacket->AttachNotify(this);					
					}
				}
				else{			
					m_pComm->Close();
				}
			}
			else{
				CString msg;
				msg.Format(_T("Open %s Fail...Error Code:%d"),COM_LM,tRet);				
				TRACE(_T("%s"),msg);
				//AfxMessageBox(msg);
			}
		}
#endif //SUPPORT_COM
	}
}

void LightModuleControl::OpenDevice()
{	//open com,lan,usb
	//detect network first....
	CloseDevice();
	this->StartDetect();	
	if (m_bAutoDetectLan)
		DetectNetworkDevice();
	MakeLightControlList();
	if (!m_bMaster){ //Only Slave Mode,need Detect USB/COM
		this->OpenUsbDevice();
		this->OpenComDevice();
	}
	OpenNetworkDevice(); //DirectConnect or Detect Ip Range------
}
BOOL LightModuleControl::AttachDeviceId(int iDeviceId)
{
	if (m_pCurrentControl && (iDeviceId != m_pCurrentControl->GetDeviceId())){
		bool bFindDevice=false;
		LightSourceController *pControl=NULL;
		int iSize=(int)m_xLightControlList.size();
		for (int i=0;i<iSize;i++){
			if (iDeviceId==m_xLightControlList[i]->GetDeviceId()){
				bFindDevice=true;
				pControl=m_xLightControlList[i];
			}
		}
		if (bFindDevice){
			m_pCurrentControl->Attach(NULL);
			m_pCurrentControl->AttachNotify(NULL);
			m_pCurrentControl=pControl;
			OpenNetworkDevice();
			//need reattach Notify.....
			return TRUE;
		}
	}
	return FALSE;
}
void LightModuleControl::MakeLightControlList()
{
	int iSize=(int)m_xDeviceList.size();
	for (int i=0;i<iSize;i++){
		LightSourceController *pLightControl=new LightSourceController(m_xDeviceList[i]);
		if (pLightControl){			
			//pLightControl->SetDeviceId(m_xDeviceList[i]); //fix bug			
			pLightControl->SetServerMode(m_bServer);		
			m_xLightControlList.push_back(pLightControl);
		}
		if (i==0 && m_pCurrentControl==NULL)
			m_pCurrentControl=pLightControl;
	}
}
void LightModuleControl::CloseDevice()
{	//close com,lan,usb
	if (m_pCurrentControl){
		m_pCurrentControl->Attach(NULL);
		m_pCurrentControl->AttachNotify(NULL);
		m_pCurrentControl=NULL;
	}
#ifdef SUPPORT_COM
	if (m_pComm){		
		//m_pComm->Close();
		delete m_pComm;
		m_pComm=NULL;
	}
#endif //SUPPORT_COM
#ifdef SUPPORT_USB
	if (m_usbHandle){
		//close handle
		SI_Close(m_usbHandle);
		m_usbHandle=NULL;
	}
#endif //SUPPORT_USB
	if (this->m_pSocket){
		delete m_pSocket;
		m_pSocket=NULL;
	}
	RemoveDevice(-1); //Remove all Device and all connection
}
bool LightModuleControl::ReOpen(void* pInstance)
{
	if (pInstance==this){
		CloseDevice();
		OpenDevice();
		return true;
	}
	return false;
}
void LightModuleControl::OnError(void *pInstance,long ErrorMsg,long ErrorId)
{
	if (pInstance==GetCurrentControl()){
		switch(ErrorMsg)
		{
		case WM_SOCKET_CONNECT: //Connect Error
			break;
		case WM_SOCKET_RECEIVE: //Receive Error
			break;
		};
	}
}
bool LightModuleControl::IsOpenDevice(void* pInstance)
{
	int iSize=(int)m_xDeviceList.size();
	if (pInstance==m_pCurrentControl){
		int iType=m_pCurrentControl->GetConnectType();
		switch(iType)
		{
		case CONNECT_COM:
#ifdef SUPPORT_COM
			if (m_pComm)
			return m_pComm->IsOpen();
#endif //SUPPORT_COM
			break;
		case CONNECT_USB:
#ifdef SUPPORT_USB
			if (m_usbHandle)
				return true;
#endif //SUPPORT_USB
			break;
		case CONNECT_LAN:
			if (m_pSocket)
				return m_pSocket->IsConnected();
			break;
		};
	}
	return false;
}
long LightModuleControl::ReceiveData(void* pInstance,unsigned char *pReadBuffer,int ReadDataSize,DWORD &BytesRead)
{
	if (pInstance==m_pCurrentControl){
		int iType=m_pCurrentControl->GetConnectType();
		switch(iType)
		{
		case CONNECT_COM:
#ifdef SUPPORT_COM
			if (m_pComm)
				return m_pComm->Read(pReadBuffer,ReadDataSize,&BytesRead);
#endif //SUPPORT_COM
			break;
		case CONNECT_USB:
#ifdef SUPPORT_USB
			if (m_usbHandle){
				SI_STATUS retStatus;
				// set Timeout
				retStatus=SI_SetTimeouts(10, 20);	// read_timeout = 0.0 sec, write_timeout = 0.01 sec
				// specify a max number byte to be read 64Kbytes
				retStatus = SI_Read(m_usbHandle, (LPVOID)pReadBuffer, ReadDataSize, &BytesRead);				
				//TRACE("\nRead Time:%d,Get Data:%d,First Data:%d",::GetTickCount(),BytesRead,pReadBuffer[0]);	
				if (retStatus == SI_SUCCESS)
					return TRUE;
				else 
					return FALSE;				
			}
#endif //SUPPORT_USB
			break;
		case CONNECT_LAN:
			if (m_pSocket)
				BytesRead=m_pSocket->Receive(pReadBuffer,ReadDataSize);
			break;
		};
	}
	
	return 0;
}
long LightModuleControl::SendData(void* pInstance,unsigned char *pSendBuffer,int SendDataSize,int &BytesSend)
{
	if (pInstance==m_pCurrentControl){
		int iConnectType=m_pCurrentControl->GetConnectType();
		switch(iConnectType)
		{
		case CONNECT_COM:
#ifdef SUPPORT_COM
			if (m_pComm)
				m_pComm->Write(pSendBuffer,SendDataSize,(DWORD*)&BytesSend);
#endif //SUPPORT_COM
			break;
		case CONNECT_USB:
#ifdef SUPPORT_USB
			if (m_usbHandle){
				SI_STATUS retStatus;				
				// set Timeout
				retStatus=SI_SetTimeouts(10, 100);	// read_timeout = 0.01 sec, write_timeout = 0.01 sec

				retStatus = SI_Write(m_usbHandle, (LPVOID)pSendBuffer, SendDataSize,(DWORD*) &BytesSend);
				if (retStatus == SI_SUCCESS){
					DWORD rt1=0,rt2=0;
					SI_CheckRXQueue(m_usbHandle,&rt1,&rt2);
					TRACE("\nQueue Result:%d,%d",rt1,rt2);
					return TRUE;
				}
				else{
					return FALSE;
				}
			}
#endif //SUPPORT_USB
			break;
		case CONNECT_LAN:
			if (m_pSocket)
				BytesSend=m_pSocket->Send(pSendBuffer,SendDataSize);
			break;
		};
	}
	return 0;
}
void LightModuleControl::GetCurrentAllDeviceId(std::vector<int> &xDeviceList)
{
	xDeviceList=m_xDeviceList;
}
void LightModuleControl::GetAllDeviceIdInfo(std::vector<int> &vDeviceList, std::vector<int> &vFlashLed)
{
	vDeviceList = m_xDeviceList;
	vFlashLed.clear();
	int nLight = (int)m_xDeviceList.size();
	int nSize = (int)m_xLightControlList.size();
	for (int i = 0; i < nLight; i++){
		int nDeviceId = m_xDeviceList[i];
		for (int j = 0; j < nSize; j++){
			LightSourceController *pControl = m_xLightControlList[j];
			if (pControl && pControl->GetDeviceId() == nDeviceId){
				int nFlashLed = (int)pControl->GetLedGroupNum();
				vFlashLed.push_back(nFlashLed);
				break;
			}
		}
	}

}
void LightModuleControl::GetAllLightType(std::vector<int> &xTypeList) //eric chao 20161026
{
	int nSize = (int)m_xLightControlList.size();
	for (int i=0;i<nSize;i++){
		if (m_xLightControlList[i]){
			LIGHT_PARAM xParam = m_xLightControlList[i]->GetLightParam();
			xTypeList.push_back(xParam.LightType);
		}
	}
}
void LightModuleControl::GetAllLightParam(std::vector<LIGHT_PARAM> &xParamList) //eric chao 20161026
{
	int nSize = (int)m_xLightControlList.size();
	for (int i=0;i<nSize;i++){
		if (m_xLightControlList[i]){
			LIGHT_PARAM xParam = m_xLightControlList[i]->GetLightParam();
			xParamList.push_back(xParam);
		}
	}
}
int LightModuleControl::GetLightType(int nDeviceId) //eric chao 20161027
{
	int nType = 0;
	int nSize = (int)m_xLightControlList.size();
	for (int i=0;i<nSize;i++){
		if (m_xLightControlList[i]){
			if (nDeviceId == m_xLightControlList[i]->GetDeviceId()){
				LIGHT_PARAM xLight =  m_xLightControlList[i]->GetLightParam();
				if (xLight.LightType == LT_MDL || xLight.LightType == LT_MDL_FLASH){
					nType = 1;
				}
				break;
			}
		}
	}
	return nType;
}

void LightModuleControl::SetDeviceTemperatureLimit(int iDeviceId,unsigned char *limitT, int no,BOOL bUseDefalut) //20140516-01
{

	int iSize=(int)m_xLightControlList.size();
	for (int i=0;i<iSize;i++){
		if (iDeviceId==m_xLightControlList[i]->GetDeviceId()){
			m_xLightControlList[i]->SetTemperatureLimit(limitT, no, bUseDefalut);
			break;
		}
	}
}



void LightModuleControl::ResetAllLightParamCheck() //seanchen 20151001
{
	int iSize=(int)m_xLightControlList.size();
	for (int i=0;i<iSize;i++){
		if (m_xLightControlList[i]){
			m_xLightControlList[i]->ResetLightParamStatus();;			
		}
	}

}

BOOL LightModuleControl::GetLightParamCheck(int iDeviceId) //seanchen 20151001
{
	BOOL bRet = FALSE;
	int iSize=(int)m_xLightControlList.size();
	for (int i=0;i<iSize;i++){
		if (iDeviceId==m_xLightControlList[i]->GetDeviceId()){
			bRet = m_xLightControlList[i]->GetLightParamCheck();
			break;
		}
	}

	return bRet;
}

LIGHT_PARAM LightModuleControl::GetLightParam(int iDeviceId) //seanchen 20151002
{
	LIGHT_PARAM xParam;
	memset(&xParam,0,sizeof(xParam));
	int iSize=(int)m_xLightControlList.size();
	for (int i=0;i<iSize;i++){
		if (iDeviceId==m_xLightControlList[i]->GetDeviceId()){
			xParam = m_xLightControlList[i]->GetLightParam();
			break;
		}
	}

	return xParam;
}