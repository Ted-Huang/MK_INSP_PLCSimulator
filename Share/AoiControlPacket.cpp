#include "StdAfx.h"
#include "AoiControlPacket.h"


#define BL_END_CHARACTER		4
#define BL_START_CHARACTER 		15

const BYTE	BOOTLOADER_ECHO[] = {0x0F, 0x90, 0x00, 0x70, 0x04};
const BYTE	BOOTLOADER_IDEN_TI[] =  {0x0F, 0x00, 0x02, 0x02, 0x00, 0xFC, 0x04};
const BYTE  ASK_BOOTLOADER_VERSION[] = {0x0F, 0x00, 0x02, 0xFE, 0x04};
const BYTE  ASK_LEAVE_BOOTLOADER[] = {0x0F, 0x08, 0x00, 0xF8, 0x04}; //ask boot loader to reset the flag then restart machine

template <typename T>
T swap_endian(T u)
{
	union
	{
		T u;
		unsigned char u8[sizeof(T)];
	} source, dest;

	source.u = u;

	for (size_t k = 0; k < sizeof(T); k++)
		dest.u8[k] = source.u8[sizeof(T) - k - 1];

	return dest.u;
}


IMPLEMENT_DYNAMIC(CAoiConnectSocket, CAsyncSocket)


CAoiConnectSocket::CAoiConnectSocket(HWND pDestWnd)
{
	m_hDestWnd=pDestWnd;
	m_pProcessSocket=NULL;
	m_bConnect=false;
}
CAoiConnectSocket::~CAoiConnectSocket()
{
	if (m_pProcessSocket){
		m_pProcessSocket->ShutDown();
		m_pProcessSocket->Close();
		delete m_pProcessSocket;
	}
}
void CAoiConnectSocket::OnAccept(int nErrorCode)
{
	if (m_pProcessSocket){
		m_pProcessSocket->Close();
		delete m_pProcessSocket;
		m_pProcessSocket=NULL;
	}
	if (m_pProcessSocket==NULL){
		m_pProcessSocket=new CAoiConnectSocket(m_hDestWnd);		
	}
	Accept(*m_pProcessSocket);
	CAsyncSocket::OnAccept(nErrorCode);	
}
void CAoiConnectSocket::OnReceive(int nErrorCode) 
{
	if (nErrorCode==0)
	::SendMessage(m_hDestWnd,WM_SOCKET_PROCESS,WM_SOCKET_RECEIVE,NULL);	
	CAsyncSocket::OnReceive(nErrorCode);
}
void CAoiConnectSocket::OnClose(int nErrorCode)
{
	m_bConnect=false;
	if (m_hDestWnd)
		::PostMessage(m_hDestWnd,WM_SOCKET_PROCESS,WM_SOCKET_CLOSE,NULL);
	if (nErrorCode)
		::PostMessage(m_hDestWnd,WM_SOCKET_ERROR,WM_SOCKET_CLOSE,nErrorCode);
	CAsyncSocket::OnClose(nErrorCode);
}
void CAoiConnectSocket::OnConnect(int nErrorCode)
{
	if (nErrorCode==0){
		m_bConnect=true;
		::PostMessage(m_hDestWnd,WM_SOCKET_PROCESS,WM_SOCKET_CONNECT,NULL);
	}
	else{
		::PostMessage(m_hDestWnd,WM_SOCKET_ERROR,WM_SOCKET_CONNECT,nErrorCode);
	}
	CAsyncSocket::OnConnect(nErrorCode);
}

//----------------------
UINT64 AoiControlPacket::SWAP(UINT64 tData) //eric chao 20160727 Need Check
{
	UINT64 tRet = 0;
	tRet = swap_endian(tData);
	return tRet;
}

long AoiControlPacket::SWAP(long tData)
{
	long tRet= ( (tData&0xFF000000)>>24 ) | ( (tData&0xFF0000)>>8 ) | ( (tData&0xFF00)<<8 ) | ( (tData&0xFF)<<24 );
	return tRet;
}
WORD AoiControlPacket::SWAP(WORD tData)
{
	WORD tRet= ( (tData&0xFF00)>>8 ) | ( (tData&0xFF)<<8 );
	return tRet;
};
DWORD AoiControlPacket::SWAP(DWORD tData)
{
	DWORD tRet= ( (tData&0xFF000000)>>24 ) | ( (tData&0xFF0000)>>8 ) | ( (tData&0xFF00)<<8 ) | ( (tData&0xFF)<<24 );
	return tRet;
}
short AoiControlPacket::SWAP(short tData)
{
	short tRet= ( (tData&0xFF00)>>8 ) | ( (tData&0xFF)<<8 );
	return tRet;
}
AoiControlPacket::AoiControlPacket(void)
{
	Init();
}
void AoiControlPacket::Init()
{
	m_pProcessWnd=NULL;
	m_pDevice=NULL;
	m_commandSequence=0;
	m_ResponseCmdSeq = 0;
	m_pAOIPacketWnd=NULL;
	m_bService=FALSE;
	m_pNotify=NULL;
	m_bDeviceReady=false;
	m_bServer=false;
	m_bIsabnormal=FALSE;
	m_bDebugMode=false;
	m_iDeviceType=DEVICE_BASIC;
	m_iConnectType=CONNECT_COM;
	m_iDeviceId=0;
	m_lWndResourceId=3000;
	m_bIsSupportInternalMode=FALSE;
	m_isBootLoader=MODE_FIRMWARE;
	m_wModelType=0;
	m_CheckSum=0;
	InitializeCriticalSection(&m_xPackLock);
	m_waitItems.empty();
	CleanReceiveBuffer();	
}
void AoiControlPacket::CleanReceiveBuffer()
{
	m_dReceiveBufferIndex=0;
	memset(m_dReceiveBuffer,0,MAX_RECEIVE_BUFFER_SIZE);
}
BOOL AoiControlPacket::StartService()
{	
	StopService(); //After Detect Device,StartService-----
	if (m_pAOIPacketWnd==NULL){
		m_pAOIPacketWnd=new CAOIPacketWnd(this);
		if (m_pAOIPacketWnd){
			RECT trc;
			trc.left=trc.top=0;
			trc.right=300;
			trc.bottom=200;
			RECT rc;
			AfxGetMainWnd()->GetClientRect(&rc);
			CString tWndName;
			tWndName.Format(_T("%s%.2d"),GetControlName(),GetDeviceId());
			m_pAOIPacketWnd->Create(NULL,tWndName,WS_OVERLAPPEDWINDOW,trc,AfxGetMainWnd(),GetWndResourceId()); //Not Yet....Parent Window maybe change other
            ( ( CAOIPacketWnd* )m_pAOIPacketWnd )->OnInitDialog();
			m_pAOIPacketWnd->MoveWindow(rc.right-trc.right,rc.bottom-trc.bottom,300,200);
			if (m_bDebugMode)
				m_pAOIPacketWnd->ShowWindow(SW_SHOW);
			else
				m_pAOIPacketWnd->ShowWindow(SW_HIDE);
			m_bService=TRUE;
			return TRUE;
		}
	}
	return FALSE;
}
void AoiControlPacket::SetDebugMode(bool bDebug)
{
	if (m_bDebugMode!=bDebug){
		m_bDebugMode=bDebug;
		if (m_pAOIPacketWnd)
			if (::IsWindow(m_pAOIPacketWnd->GetSafeHwnd()))
				if (m_bDebugMode){
					m_pAOIPacketWnd->ShowWindow(SW_SHOW);
				}
				else{
					m_pAOIPacketWnd->ShowWindow(SW_HIDE);
				}
	}
}
BOOL AoiControlPacket::StopService()
{	
	if (m_pAOIPacketWnd){
		if (::IsWindow(m_pAOIPacketWnd->m_hWnd))
		m_pAOIPacketWnd->SendMessage(WM_CLOSE,NULL,NULL);
		delete (CAOIPacketWnd*)m_pAOIPacketWnd;
		m_pAOIPacketWnd=NULL;
		m_bService=FALSE;
		return TRUE;
	}
	return FALSE;
}


AoiControlPacket::~AoiControlPacket(void)
{
	if (this->ifWaiting())
		CleanWaitItem();
	if (m_bService)
		StopService();
	Attach(NULL);
	DeleteCriticalSection(&m_xPackLock);
}
void AoiControlPacket::SetDeviceConnectType(int DeviceType,int ConnectType)
{
	bool bDeviceChange=false;
	bool bConnectChange=false;
	if (m_iConnectType!=ConnectType){
		m_iConnectType=ConnectType;
		bConnectChange=true;
	}
	switch(DeviceType)
	{
	case DEVICE_IO:
		if (m_iDeviceType!=DeviceType){
			bDeviceChange=true;
			m_iDeviceType=DeviceType;
		}
		break;
	case DEVICE_LM:
		if (m_iDeviceType!=DeviceType){
			bDeviceChange=true;
			m_iDeviceType=DeviceType;
		}
		break;
	default:
		TRACE("\n UnKnown Control Device!");
		break;
	};

	if (!m_bService)
		StartService();

}
void AoiControlPacket::AutoConfigCmdWaitTimer() //eric chao 20131219
{
	long tTimer=0;
	switch(m_iDeviceType)
	{
	case DEVICE_IO:
		tTimer=IO_PACKET_RESEND_TIMER;
		break;
	case DEVICE_LM:
		tTimer=LM_LAN_PACKET_RESEND_TIMER;
		break;
	}
	switch(m_iConnectType)
	{
	case CONNECT_USB:
		tTimer=LM_USB_PACKET_RESEND_TIMER;
		break;
	case CONNECT_COM:
		tTimer=LM_COM_PACKET_RESEND_TIMER;
		break;
	case CONNECT_LAN:
		tTimer=LM_LAN_PACKET_RESEND_TIMER;
		break;
	}
	if (m_pAOIPacketWnd){
		m_pAOIPacketWnd->PostMessage(WM_PACKET_CHECKTIME,(WPARAM)tTimer,NULL);
	}
}
BOOL AoiControlPacket::ifWaiting(void)
{
	Lock(TRUE);
	int nSize = (int)m_waitItems.size();
	Lock(FALSE);
	if (nSize > 0)
		return TRUE;
	else 
		return FALSE;
}
void AoiControlPacket::updateWaiting(void)
{
	ULONGLONG currTimeTick = GetTickCount64();

	TRACE("\nupdateWaiting----");
	Lock(TRUE);
	for (auto &xWait : m_waitItems){
		if (xWait){
			if ((currTimeTick - xWait->outTimeTick) > REQUESTTIMEOUT){
				if (xWait->timeoutCount < MAXTIMEOUTCOUNT){
					xWait->timeoutCount++;
					xWait->outTimeTick = currTimeTick;
					if (m_pDevice && xWait->outData){
						int iSend = 0;
						m_pDevice->SendData(this, xWait->outData, xWait->outLength, iSend);
						if (!xWait->errorMsg.IsEmpty()){
							TRACE(_T("\n(%d)%s-----"), xWait->commandSequence, xWait->errorMsg);
						}
					}
				}
				else{	// timeout !! 
					TimeoutErr *errMsg = new TimeoutErr;
					errMsg->commandSequence = xWait->commandSequence;
					errMsg->errorMsg = xWait->errorMsg;
					if (!xWait->errorMsg.IsEmpty()){
						TRACE(_T("\nTimeOut-----(%d)%s"), xWait->commandSequence, xWait->errorMsg);
					}
					// release memory and delete item
					delete[] xWait->outData;
					delete xWait;
					xWait = NULL;
					if (!m_bIsabnormal){
						m_bIsabnormal = TRUE;
						m_xFirstTimeoutError.commandSequence = errMsg->commandSequence;
						m_xFirstTimeoutError.errorMsg = errMsg->errorMsg;
					}
					m_xLastTimeoutError.commandSequence = errMsg->commandSequence;
					m_xLastTimeoutError.errorMsg = errMsg->errorMsg;
					// send out timeout message
					//if (m_Parent != NULL)
					//	m_Parent->PostMessage(WM_USBMSG, IDM_TIMEOUT_MSG, (LPARAM)errMsg);
					delete errMsg;
				}
			}
		}
	}
	Lock(FALSE);
	ClearNoUseWaiting();
}
void AoiControlPacket::ClearNoUseWaiting()
{
	Lock(TRUE);
	int nIdx = 0;
	while (nIdx < (int)m_waitItems.size()){
		if (m_waitItems[nIdx] == NULL){
			m_waitItems.erase(m_waitItems.begin() + nIdx);
		}
		else{
			nIdx++;
		}
	}
	Lock(FALSE);
}
void AoiControlPacket::addWaitingItem(BYTE commandSequence, BYTE *outData, DWORD outLength, ULONGLONG timeTick, CString errMsg)
{
	WaitItem *myWait = new WaitItem;

	myWait->commandSequence = commandSequence;
	if (outLength){
		myWait->outData = new BYTE[outLength];
		memcpy(myWait->outData, outData, outLength);
	}
	myWait->outLength = outLength;

	myWait->errorMsg = errMsg;
	
	myWait->outTimeTick = timeTick;
	myWait->timeoutCount = 0;
	
	Lock(TRUE);
	m_waitItems.push_back(myWait);
	Lock(FALSE);
}

void AoiControlPacket::RemoveWaitingItem(BYTE commandSequence)
{
	Lock(TRUE);
	for (auto &xWait : m_waitItems){
		if (xWait){
			if (xWait->commandSequence == commandSequence || commandSequence == 0xFF){
				TRACE(_T("\nRemove Wait Item(%d)"), xWait->commandSequence);
				if (xWait->outData){
					delete[]xWait->outData;
				}
				delete xWait;
				xWait = NULL;
			}
		}
	}
	Lock(FALSE);
	ClearNoUseWaiting();

	if (m_pAOIPacketWnd && m_bDebugMode)
		((CAOIPacketWnd*)m_pAOIPacketWnd)->SetRemoveSeq((int)commandSequence);
}
void AoiControlPacket::MakeAoiPacket(CByteArray &Packet,BYTE commandType, BYTE *rawData, int dataLength, BOOL isWaitResponse,int comSequence)
{
// +3 => Command + isEchoing + Command Sequence
// +3 => Start Character + Check Sum + End Character

	AddDle_InCommandBody(rawData,dataLength,&Packet);	
	int outLength = (int)Packet.GetSize() + 3 + 3;
	BYTE chkSum = 0;
	
	Packet.InsertAt(0,START_CHARACTER);
	Packet.InsertAt(1,commandType);
	Packet.InsertAt(2,isWaitResponse==TRUE? 1:0);
	
	if (comSequence == -1)
	{
		Packet.InsertAt(3,m_commandSequence);		
		m_commandSequence++;
		if (m_commandSequence > CHECKSUM_RADIX)
			m_commandSequence = 0;
	}
	else 
		Packet.InsertAt(3,comSequence);
	//TRACE(_T("\nMake Type(%d) Command(%d)------Wait:%d"),commandType,m_commandSequence-1,isWaitResponse);
	BYTE *pData=Packet.GetData();
	int  PacketSize=(int)Packet.GetSize();

	for (int i=1; i<PacketSize; i++)
	{		
		chkSum += pData[i];
	}
	// add check sum byte
	chkSum %= CHECKSUM_RADIX;
	m_CheckSum=chkSum;
	Packet.Add(chkSum);
	Packet.Add(END_CHARACTER);	
}
BOOL AoiControlPacket::sendIsBootLoadRequest(void)
{
	return sendBootCommand((BYTE*)ASK_BOOTLOADER_VERSION,sizeof(ASK_BOOTLOADER_VERSION),FALSE,_T(""));
}
BOOL AoiControlPacket::sendRestartRequest(void)
{
	return sendBootCommand((BYTE*)ASK_LEAVE_BOOTLOADER,sizeof(ASK_LEAVE_BOOTLOADER),TRUE,_T(""));
}
BOOL AoiControlPacket::sendFWPackage(BYTE *rawData, int dataLength, BOOL resend, CString errmsg)
{
	return sendBootCommand(rawData, dataLength, FALSE, errmsg);
}
BOOL AoiControlPacket::sendBootCommand(BYTE *rawData, int dataLength, BOOL isWaitResponse, CString errmsg, int comSequence)
{

	BYTE comSeq = comSequence;
	if (comSequence == -1)
	{
		comSeq = m_commandSequence ;	// command sequence
		m_commandSequence++;
		if (m_commandSequence > CHECKSUM_RADIX)
			m_commandSequence = 0;
	}

	BOOL res = FALSE;
	int dwSendSize=0;
	if (m_pDevice){
		m_pDevice->SendData(this,rawData,dataLength,dwSendSize);
		if (dwSendSize==dataLength)
			res=TRUE;
	}
	/*		
	{
		unsigned char *data = new unsigned char[dataLength + 4];
		memcpy(data + 4, rawData, dataLength);
		memcpy(data, &dataLength, 4);
		m_ioSocketThread->PostThreadMessage(IO_SOCKETMSG, IOS_SENDDATA_MSG, (LPARAM)data);
	}
	*/

	if (isWaitResponse)
	{
		// record the sending time tick
		addWaitingItem(comSeq, rawData, dataLength, GetTickCount64(), errmsg);		
	}
	return res;
}
BOOL AoiControlPacket::sendCommand(BYTE commandType, BYTE *commandBody, int dataLength, BOOL isWaitResponse, CString errmsg, int comSequence)
{

	if (m_pDevice)
		if (!m_pDevice->IsOpenDevice(this))
			return FALSE;

	CByteArray cAoiPacket;
	MakeAoiPacket(cAoiPacket,commandType,commandBody,dataLength,isWaitResponse,comSequence);
	BYTE *outData=cAoiPacket.GetData();
	AOI_CTRL_HEADER *pAoiHdr=(AOI_CTRL_HEADER*)outData;
	int outLength=(int)cAoiPacket.GetSize();

	// set if we are waiting for firmware response
	// add this outdata into waiting item queue
	// modified by eric at 20120413
	BOOL res = FALSE;

	if (m_pDevice){
		int iSend=0;
		m_pDevice->SendData(this,outData,outLength,iSend);

		if (m_pAOIPacketWnd && m_pAOIPacketWnd->IsWindowVisible()){
			((CAOIPacketWnd*)m_pAOIPacketWnd)->SetOutputData(outData,outLength);
			m_pAOIPacketWnd->Invalidate();
		}
		if (iSend==outLength)
			res=TRUE;
	}


	if (isWaitResponse)	{
#ifdef DUMP_WAIT_CMD
		TRACE(_T("\nAdd Wait Command(%d)------%d"),pAoiHdr->CommandSequence,GetTickCount());
#endif //DUMP_WAIT_CMD
		// record the sending time tick
		addWaitingItem(pAoiHdr->CommandSequence, outData, outLength, GetTickCount64(), errmsg);
	}
#if 0 //eric chao 20170817 modify
	if (m_iDeviceType == DEVICE_LM) //eric chao 20121128 avoid LM Firmware Bug
		::Sleep(50);
#endif //
	return res;


}
int AoiControlPacket::AddDle_InCommandBody(BYTE *pCommandBody,int BodyLength,CByteArray *rst)
{
	int count=0;
	if(rst && BodyLength){
		rst->RemoveAll();
		rst->SetSize(BodyLength*2);
		for (int i=0;i<BodyLength;i++){
			if (pCommandBody[i]==START_CHARACTER ||
				pCommandBody[i]==DLE_COMMAND ||
				pCommandBody[i]==END_CHARACTER){
					rst->SetAt(count++,DLE_COMMAND);
			}
			rst->SetAt(count++,pCommandBody[i]);
		}
		rst->SetSize(count);
		rst->FreeExtra();
	}
	return count;
}
void AoiControlPacket::RemoveDLE_InCommandBody(BYTE *pData,int &DataSize)
{
	int iRemoveDLE=0;	
	for (int i=0;i<DataSize;i++){
		if (pData[i]==DLE_COMMAND){
			if ( (i+1)<DataSize ){
				if (pData[i+1]==DLE_COMMAND ||
					pData[i+1]==START_CHARACTER ||
					pData[i+1]==END_CHARACTER){
					iRemoveDLE++;
					memmove(&pData[i],&pData[i+1],DataSize-i-1);
					pData[DataSize-1]=0;
				}
			}
		}
	}
	if (iRemoveDLE)
		DataSize-=iRemoveDLE;
}
bool AoiControlPacket::ParseCommand(BYTE * pData, int DataSize)
{
	WORD tValue=0;
	if (pData && CheckSum(pData, DataSize)){
		m_isBootLoader=MODE_FIRMWARE;
		AOI_CTRL_PACKET *pPacket=(AOI_CTRL_PACKET*)pData;
		RemoveWaitingItem(pPacket->AoiHeader.CommandSequence);
		
		m_ResponseCmdSeq = pPacket->AoiHeader.CommandSequence;
		if (pPacket && pPacket->AoiHeader.IsEchoing==1){
			sendEchoCommand(pPacket->AoiHeader.CommandSequence);
		}
		int iCommandBodySize=DataSize-sizeof(AOI_CTRL_HEADER)-sizeof(AOI_CTRL_TAIL);
		RemoveDLE_InCommandBody((BYTE*)(&pPacket->AoiHeader+1),iCommandBodySize); // In CommandBody, DLE+STARTCODE= DATA(254), DLE+DLE=DATA(242), DLE+ENDCODE= DATA(255)

		switch(pPacket->AoiHeader.CommandType)
		{
		case SETUP_COMMAND:
			ParseSetupCommand(&pPacket->CommandBody.Setup,iCommandBodySize);
			break;
		case ACTION_COMMAND:
			ParseActionCommand(&pPacket->CommandBody.Action,iCommandBodySize);
			break;
		case ECHO_COMMAND:
			m_tEchoTime=::GetTickCount64();
			ParseEchoCommand(&pPacket->AoiHeader);
			SetDeviceReady(true);
			if (m_pAOIPacketWnd)
				m_pAOIPacketWnd->PostMessage(WM_DEVICE_READY,NULL,NULL);
			break;
		case MASS_DATA_COMMAND:
			ParseMassDataCommand(&pPacket->CommandBody.MassData,iCommandBodySize);
			break;
		case MULTI_PURPOSE_COMMAND:			
			ParseMultiPurposeCommand(&pPacket->CommandBody.MPData,iCommandBodySize);
			break;
		default:
			break;
		}
		m_bIsabnormal=FALSE;
		return true;		
	}else{ //CheckSum Error Data~
		DumpErrData(ERR_CMD_CHECKSUM,pData,DataSize);
	}
	return false;
}


BOOL AoiControlPacket::sendEchoCommand(BYTE comSeq,CString errmsg,bool isWaitResponse)
{	
	return sendCommand(ECHO_COMMAND,NULL,0,isWaitResponse,errmsg,comSeq);
}

int  AoiControlPacket::sendActionRequest(BYTE Device,BYTE Param, BOOL isWaitResponse,CString errmsg, int comSequence)
{
	if (sendActionCommand(Device,Param,isWaitResponse,errmsg,comSequence)) 
		return m_commandSequence==0? CHECKSUM_RADIX:(m_commandSequence-1);
	else 
		return -1;	// error
}
int  AoiControlPacket::sendSetupRequest(BYTE Device,BYTE Param,WORD value, BOOL isWaitResponse,CString errmsg, int comSequence)
{
	if (sendSetupCommand(Device,Param,value,isWaitResponse,errmsg,comSequence))
		return m_commandSequence==0? CHECKSUM_RADIX:(m_commandSequence-1);
	else 
		return -1;	// error
}
int  AoiControlPacket::sendMassDataRequest(BYTE DataType,BYTE DataSize,BYTE *pMassData,BOOL isWaitResponse,CString errmsg, int comSequence)
{
	if (sendMassDataCommand(DataType,DataSize,pMassData,isWaitResponse,errmsg,comSequence))
		return m_commandSequence==0? CHECKSUM_RADIX:(m_commandSequence-1);
	else 
		return -1;	// error
}
int  AoiControlPacket::sendMPRequest(BYTE MPCmd,long MPSize,BYTE *pMPData,BOOL isWaitResponse,CString errmsg, int comSequence)
{
	if (sendMPCommand(MPCmd,MPSize,pMPData,isWaitResponse,errmsg,comSequence))
		return m_commandSequence==0? CHECKSUM_RADIX:(m_commandSequence-1);
	else 
		return -1;	// error
}

BOOL AoiControlPacket::sendActionCommand(BYTE Device,BYTE Param, BOOL isWaitResponse,CString errmsg, int comSequence)
{
	ACTION_COMMAND_PKT ActionPacket;
	ActionPacket.SlaveDeviceId=Device;
	ActionPacket.ActionParam=Param;
	return this->sendCommand(ACTION_COMMAND,(BYTE*)&ActionPacket,sizeof(ActionPacket),isWaitResponse,errmsg,comSequence);
}

BOOL AoiControlPacket::sendSetupCommand(BYTE Device,BYTE Param,WORD value, BOOL isWaitResponse,CString errmsg, int comSequence)
{
	SETUP_COMMAND_PKT SetupPacket;
	SetupPacket.SlaveDeviceId=Device;
	SetupPacket.SlaveDeviceParam=Param;
	SetupPacket.SlaveDeviceValue=SWAP(value);
	return this->sendCommand(SETUP_COMMAND,(BYTE*)&SetupPacket,sizeof(SetupPacket),isWaitResponse,errmsg,comSequence);
}
BOOL AoiControlPacket::sendMassDataCommand(BYTE DataType,BYTE DataSize,BYTE *pMassData,BOOL isWaitResponse,CString errmsg, int comSequence)
{
	MASS_DATA_COMMAND_HEADER MassDataHdr;
	MassDataHdr.DataType=DataType;
	MassDataHdr.DataSize=DataSize;
	if (pMassData){
		int xSize=DataSize+2;
		if (DataSize==0)
			xSize=258; //256+2
		return this->sendCommand(MASS_DATA_COMMAND,pMassData,xSize,isWaitResponse,errmsg,comSequence);
	}
	else{		
		return this->sendCommand(MASS_DATA_COMMAND,(BYTE*)&MassDataHdr,2,isWaitResponse,errmsg,comSequence);
	}
}
BOOL AoiControlPacket::sendMPCommand(BYTE MPCmd,long MPSize,BYTE *pMPData,BOOL isWaitResponse,CString errmsg, int comSequence)
{
	MULTI_PURPOSE_COMMAND_HEADER MPDataHdr;
	MPDataHdr.MPCMD=MPCmd;
	MPDataHdr.MPDataSize=SWAP(MPSize);
	if (pMPData){	
		return this->sendCommand(MULTI_PURPOSE_COMMAND,pMPData,MPSize+5,isWaitResponse,errmsg,comSequence);
	}
	else{		
		return this->sendCommand(MULTI_PURPOSE_COMMAND,(BYTE*)&MPDataHdr,5,isWaitResponse,errmsg,comSequence);
	}
}

bool AoiControlPacket::CheckSum(BYTE* pData, int DataSize)
{
	if (pData[0]!=START_CHARACTER || pData[DataSize-1]!=END_CHARACTER || DataSize<=3) //StartCode,EndCode,CheckSum
		return false;
	BYTE sum=0;
	for (int i=1;i<DataSize-2;i++)
		sum+=pData[i];
	if ( (sum % CHECKSUM_RADIX) == pData[DataSize-2] )
		return true;
	return false;
}

void AoiControlPacket::CleanWaitItem()
{
	Lock(TRUE);
	for (int i=0;i<(int)m_waitItems.size();i++){
			delete []m_waitItems[i]->outData;
			delete m_waitItems[i];
	}
	m_waitItems.clear();
	Lock(FALSE);
}
void AoiControlPacket::ErrorProcess(long ErrorMsg,long ErrorId)
{
	if (m_pDevice){
		m_pDevice->OnError(this,ErrorMsg,ErrorId);
	}
}
void AoiControlPacket::ReceiveAllData()
{
	if (m_pDevice && m_pDevice->IsOpenDevice(this)){
		DWORD dwRead=0;
		do
		{
		dwRead=0;
		m_pDevice->ReceiveData(this,m_dReceiveBuffer+m_dReceiveBufferIndex,sizeof(m_dReceiveBuffer)-m_dReceiveBufferIndex,dwRead);
		if ((int)dwRead>0)
		m_dReceiveBufferIndex+=dwRead;
		}while ((int)dwRead >0); //Not Yet
		
		if (m_dReceiveBufferIndex>0){
		if (m_pAOIPacketWnd && m_pAOIPacketWnd->IsWindowVisible()){			
			((CAOIPacketWnd*)m_pAOIPacketWnd)->SetInputData(m_dReceiveBuffer,m_dReceiveBufferIndex);
			m_pAOIPacketWnd->Invalidate();			
		}
		CheckPacketInBuffer();
		}
	}
}
void AoiControlPacket::Attach(IDevice *pDevice)
{
	m_pDevice=pDevice;
	if (m_pDevice){
		RemoveWaitingItem(-1);
		CheckDeviceMode();
	}
};
void AoiControlPacket::CheckDeviceMode()
{
	if (!m_bServer){
		if (m_iConnectType==CONNECT_COM || m_iConnectType==CONNECT_USB)
			sendIsBootLoadRequest();		
			QueryVerionInfo();
			sendInternalModelTypeRequest(); //Only Version >10 Support this command
	}
}

void AoiControlPacket::SetConnectType(int ConnectType)
{
	m_iConnectType=ConnectType;	
	if (m_iConnectType==CONNECT_USB){
		if (m_pAOIPacketWnd)//==>Start USB Receive Data Timer
			m_pAOIPacketWnd->PostMessage(WM_USB_DATA_TIME,1,NULL);
	}
	else{
		if (m_pAOIPacketWnd)
			m_pAOIPacketWnd->PostMessage(WM_USB_DATA_TIME,0,NULL);
	}
};

void AoiControlPacket::ParseBLCommand(BYTE *pData,int DataSize)
{
	switch (DataSize)
	{
	case 5:		// should be the package received echo
		if (memcmp(pData, BOOTLOADER_ECHO, 5) == 0)
		{
			if (m_pNotify)
				m_pNotify->OnUpdateFirmwareNotify(UPDATE_BLECHO);
			BootLoaderEcho();
			// clear the first element in waiting queue
			if (ifWaiting())
			{
				Lock(TRUE);
				if (m_waitItems.size()){
					if (m_waitItems[0]){
					delete[] m_waitItems[0]->outData;
					delete m_waitItems[0];
					}
					m_waitItems.erase(m_waitItems.begin());
				}
				Lock(FALSE);
			}			
		}
		break;
	case 7:		// should be the isBootLoader echo
		if (memcmp(pData, BOOTLOADER_IDEN_TI, 7) == 0){	// TI (1200/1300) boot loader
			m_isBootLoader = MODE_BOOTLOADER;
			SetDeviceReady(true);
			NotifyBootLoaderMode(m_iDeviceType); //eric chao 20130923
			if (m_pAOIPacketWnd)
			m_pAOIPacketWnd->PostMessage(WM_DEVICE_READY,NULL,NULL);
			if (ifWaiting())
				RemoveWaitingItem(255); //Remove All Waiting Item
		}
		break;	
	}
}
void AoiControlPacket::ReOpen()
{
	if (m_pDevice)
		m_pDevice->ReOpen(this);
}
bool AoiControlPacket::MovePacketToStartCode() //eric chao 20130902
{
	BYTE *pStart=m_dReceiveBuffer;
	int iStartPos=0;
	do{
		if (*(pStart+iStartPos) == START_CHARACTER)
			break;
		iStartPos++;
	}while(iStartPos<m_dReceiveBufferIndex);
	if (iStartPos){
		if ((m_dReceiveBufferIndex-iStartPos)>0)
			memmove(m_dReceiveBuffer,pStart+iStartPos,m_dReceiveBufferIndex-iStartPos);
		m_dReceiveBufferIndex=m_dReceiveBufferIndex-iStartPos;
		memset(&m_dReceiveBuffer[m_dReceiveBufferIndex],0,sizeof(m_dReceiveBuffer)-m_dReceiveBufferIndex);
		return true;
	}
	return false;
}
void AoiControlPacket::DumpCurrentBuf()
{
	DumpErrData(ERR_BUF_DUMP,m_dReceiveBuffer,m_dReceiveBufferIndex);
}
void AoiControlPacket::CheckPacketInBuffer()
{
	BYTE *pStart=m_dReceiveBuffer;
	//if (m_iConnectType==CONNECT_LAN){
	if (*pStart != START_CHARACTER && *pStart!= BL_START_CHARACTER){
		MovePacketToStartCode();
	}
	//}
	//MODE_FIRMWARE
	for (int i=0;i<m_dReceiveBufferIndex;i++){
		if (m_dReceiveBuffer[i]==END_CHARACTER){
			if (i>0 && m_dReceiveBuffer[i-1]!=DLE_COMMAND && *pStart==START_CHARACTER){
				int iCmdSize=(int)(&m_dReceiveBuffer[i]-pStart+1);
				ParseCommand(pStart,iCmdSize);
				pStart=&m_dReceiveBuffer[i+1];
			}
		}
	}
	int iSize=(int)(&m_dReceiveBuffer[m_dReceiveBufferIndex]-pStart);
	if (iSize>=0){
		if (pStart!=m_dReceiveBuffer){
			memmove(m_dReceiveBuffer,pStart,iSize);
			m_dReceiveBufferIndex=iSize;
			memset(&m_dReceiveBuffer[m_dReceiveBufferIndex],0,sizeof(m_dReceiveBuffer)-m_dReceiveBufferIndex);
		}
	}
	//MODE_BOOTLOADER ... StartCode 0x0F,EndCode 0x04 CheckSum XOR+1
	if (m_dReceiveBuffer[0] == BL_START_CHARACTER && m_dReceiveBufferIndex >= sizeof(BOOTLOADER_ECHO))
	{
		for (int i=0;i<m_dReceiveBufferIndex;i++){
			if (m_dReceiveBuffer[i]==BL_END_CHARACTER){
				int iBLCmdSize=(int)(&m_dReceiveBuffer[i]-pStart+1);
				ParseBLCommand(pStart,iBLCmdSize);
				pStart=&m_dReceiveBuffer[i+1];
			}
		}
		int iBLCmdSize=(int)(&m_dReceiveBuffer[m_dReceiveBufferIndex]-pStart);
		if (iBLCmdSize>=0){
			if (pStart!=m_dReceiveBuffer){
			memmove(m_dReceiveBuffer,pStart,iBLCmdSize);
			m_dReceiveBufferIndex=iBLCmdSize;
			memset(&m_dReceiveBuffer[m_dReceiveBufferIndex],0,sizeof(m_dReceiveBuffer)-m_dReceiveBufferIndex);
			}
		}
	}	
	if (m_dReceiveBufferIndex >= MAX_RECEIVE_BUFFER_SIZE){
		DumpErrData(ERR_BUF_FULL,m_dReceiveBuffer,MAX_RECEIVE_BUFFER_SIZE); //eric chao 20150316
		CleanReceiveBuffer();
	}
}
void AoiControlPacket::Lock(BOOL bFlag)
{
	if (bFlag){
		EnterCriticalSection(&m_xPackLock);
	}
	else{
		LeaveCriticalSection(&m_xPackLock);
	}
}
#if 0
void AoiControlPacket::CheckPacketInBuffer()
{
	BYTE *pStart=m_dReceiveBuffer;
	if (m_isBootLoader==MODE_FIRMWARE){
	for (int i=0;i<m_dReceiveBufferIndex;i++){
		if (m_dReceiveBuffer[i]==END_CHARACTER){
			if (i>0 && m_dReceiveBuffer[i-1]!=DLE_COMMAND){
				int iCmdSize=&m_dReceiveBuffer[i]-pStart+1;
				ParseCommand(pStart,iCmdSize);
				pStart=&m_dReceiveBuffer[i+1];
			}
		}
	}
	int iSize=&m_dReceiveBuffer[m_dReceiveBufferIndex]-pStart;
	if (iSize>=0){
		if (pStart!=m_dReceiveBuffer){
			memmove(m_dReceiveBuffer,pStart,iSize);
			m_dReceiveBufferIndex=iSize;
			memset(&m_dReceiveBuffer[m_dReceiveBufferIndex],0,sizeof(m_dReceiveBuffer)-m_dReceiveBufferIndex);
		}
	}
	}
	else if (m_isBootLoader==MODE_BOOTLOADER){
		//BootLoader Mode Check
			switch (m_dReceiveBufferIndex)
			{
			case 5:		// should be the package received echo
				if (memcmp(m_dReceiveBuffer, BOOTLOADER_ECHO, 5) == 0)
				{
					if (m_pNotify)
						m_pNotify->OnUpdateFirmwareNotify(UPDATE_BLECHO);
					//if (m_Parent != NULL)
					//	m_Parent->PostMessage(WM_USBMSG, IDM_BLECHO_MSG);	// modified by danny at 20120412
					// clear the first element in waiting queue
					if (ifWaiting())
					{
						delete [] m_waitItems[0]->outData;
						delete m_waitItems[0];
						m_waitItems.erase(m_waitItems.begin());
					}
					memset(m_dReceiveBuffer,0,m_dReceiveBufferIndex);
					m_dReceiveBufferIndex=0;
				}
				break;
			case 7:		// should be the isBootLoader echo
				if (memcmp(m_dReceiveBuffer, BOOTLOADER_IDEN_TI, 7) == 0){	// TI (1200/1300) boot loader
					m_isBootLoader = MODE_BOOTLOADER;
					memset(m_dReceiveBuffer,0,m_dReceiveBufferIndex);
					m_dReceiveBufferIndex=0;
				}
				break;
			default:	// error !?
				updateWaiting();
				break;
			}
			if (m_dReceiveBufferIndex > 7)
				TRACE("\nUnKnown Data In~");
	}
}
#endif //0
#ifdef SUPPORT_SOCKET
void AoiControlPacket::ReceiveSocketData(BYTE *pBuffer,int dwSize)
{
	if (dwSize <= (int)(sizeof(m_dReceiveBuffer)-m_dReceiveBufferIndex)){
	memcpy(m_dReceiveBuffer+m_dReceiveBufferIndex,pBuffer,dwSize);
	if (dwSize>0)
		m_dReceiveBufferIndex+=dwSize;

	CheckPacketInBuffer();
	}
}
#endif //SUPPORT_SOCKET

// CAOIPacketWnd

IMPLEMENT_DYNAMIC(CAOIPacketWnd, CDialog)

CAOIPacketWnd::CAOIPacketWnd(AoiControlPacket* pPacket)
{
	m_lCheckWaitingTimer=0;
	m_lUsbDataTimer=0;
	m_iLastRemoveSeq=-1;
	m_pLinkPacket=pPacket;
	m_xMode=_T("COM---");	
	m_lElapseTime=DEFAULT_PACKET_RESEND_TIMER;
	m_lCheckTime=DEFAULT_CHECK_DEVICE_TIMER;
	//m_lUsbTime=DEFAULT_USB_NORMAL_CHECK;	
	m_lUsbTime=DEFAULT_USB_HIGHTSPEED_CHECK;
	m_lQueryAliveTime=DEFAULT_QUERYALIVE_TIMER;
	m_lCheckDeviceTimer=0;
	m_lCheckWaitingTimer=0;

    m_hThread = NULL;

    memset( m_hEvent, NULL, sizeof( m_hEvent ) );
}

CAOIPacketWnd::~CAOIPacketWnd()
{
	m_pLinkPacket=NULL;
}


BEGIN_MESSAGE_MAP(CAOIPacketWnd, CDialog)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_MESSAGE(WM_PACKET_CHECKTIME,OnChangeTimer)
	ON_MESSAGE(WM_USB_DATA_TIME,OnUSBTimer)
	ON_MESSAGE(WM_DEVICE_READY,OnDeviceReady)
	ON_MESSAGE(WM_QUERY_ALIVE,OnQueryAlive) //eric chao 20150320
#ifdef SUPPORT_SOCKET	
	ON_MESSAGE(WM_SOCKET_PROCESS,OnSocketProcess)
	ON_MESSAGE(WM_SOCKET_ERROR,OnSocketError)
#endif //SUPPORT_SOCKET
#ifdef SUPPORT_SERIAL
	ON_WM_SERIAL(&CAOIPacketWnd::OnSerialMsg)
#endif //SUPPORT_SERIAL
END_MESSAGE_MAP()

void CAOIPacketWnd::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==TIMER_CHECK_PACKET_IN_QUEUE){
		if (m_pLinkPacket && m_pLinkPacket->IsDeviceExist())
			if (m_pLinkPacket->ifWaiting())
				m_pLinkPacket->updateWaiting();
	}
	else if (nIDEvent==TIMER_CHECK_DEVICE){
		if (m_pLinkPacket)
			m_pLinkPacket->CheckDeviceMode();
	}
	else if (nIDEvent==TIMER_USB_DEVICE){
		if (m_pLinkPacket)
			m_pLinkPacket->ReceiveAllData();
	}
	else if (nIDEvent==TIMER_DEVICE_QUERYALIVE){
		ULONGLONG tCheckTime=::GetTickCount64();
		if (m_pLinkPacket && m_pLinkPacket->GetConnectType()==CONNECT_LAN){
			if ( (tCheckTime-m_pLinkPacket->GetEchoTime()) > 3*DEFAULT_QUERYALIVE_TIMER )
				m_pLinkPacket->ReOpen(); // No Response...Need Re Connect
			else if ( (tCheckTime-m_pLinkPacket->GetEchoTime()) > DEFAULT_QUERYALIVE_TIMER )
				m_pLinkPacket->sendEchoCommand(-1,L"Send ECHO command fail...",true);
		}

	}
}
void CAOIPacketWnd::OnDestroy()
{
	if (m_lDeviceQueryAliveTimer){
		KillTimer(m_lDeviceQueryAliveTimer);
		m_lDeviceQueryAliveTimer=0;
	}
	if (m_lCheckWaitingTimer){
		KillTimer(m_lCheckWaitingTimer);
		m_lCheckWaitingTimer=0;
		m_pLinkPacket=NULL;
	}
	if (m_lCheckDeviceTimer){
		KillTimer(m_lCheckDeviceTimer);
		m_lCheckDeviceTimer=0;
	}
	if (m_lUsbDataTimer){
		KillTimer(m_lUsbDataTimer);
		m_lUsbDataTimer=0;
	}
    ::SetEvent( m_hEvent[ EV_EXIT ] );
    ::WaitForSingleObject( m_hThread, INFINITE );
}
void CAOIPacketWnd::OnPaint()
{
	CDC *pDC=this->GetDC();
	pDC->SetBkMode(OPAQUE);
	if (m_xInputCmd.GetLength()){
		pDC->TextOut(10,10,m_xMode+_T("INPUT---"));
		pDC->TextOut(10,30,m_xInputCmd);
	}
	if (m_xOutputCmd.GetLength()){
		pDC->TextOut(10,50,_T("OUTPUT---"));
		pDC->TextOut(10,70,m_xOutputCmd);
	}
	if (m_iLastRemoveSeq>=0){
		CString temp;
		temp.Format(_T("Remove Seq:%d"),this->m_iLastRemoveSeq);
		pDC->TextOut(10,90,temp);
	}
	
	this->ReleaseDC(pDC);

	CDialog::OnPaint();
}

LRESULT CAOIPacketWnd::OnSocketError(WPARAM wParam, LPARAM lParam)
{ //lParam ==> error code
	if (wParam==WM_SOCKET_RECEIVE){
		if (m_pLinkPacket){

		}
	}
	else if (wParam==WM_SOCKET_CONNECT){
		switch(lParam)
		{
		case WSAEADDRINUSE: //The specified address is already in use.
			break;
		case WSAEADDRNOTAVAIL: //The specified address is not available from the local machine
			break;
		case WSAEAFNOSUPPORT: //Addresses in the specified family cannot be used with this socket.
			break;
		case WSAECONNREFUSED: //The attempt to connect was forcefully rejected.
			break;
		case WSAEDESTADDRREQ: // A destination address is required.
			break;
		case WSAEFAULT: //The lpSockAddrLen argument is incorrect.
			break;
		case WSAEINVAL: //The socket is already bound to an address.
			break;
		case WSAEISCONN: //The socket is already connected.
			break;
		case WSAEMFILE: //No more file descriptors are available.
			break;
		case WSAENETUNREACH: //The network cannot be reached from this host at this time.
			break;
		case WSAENOBUFS: //No buffer space is available. The socket cannot be connected.
			break;
		case WSAENOTCONN: //The socket is not connected
			break;
		case WSAENOTSOCK: //The descriptor is a file, not a socket.
			break;
		case WSAETIMEDOUT: //The attempt to connect timed out without establishing a connection
			break;
		};
		if (this->m_pLinkPacket)
			m_pLinkPacket->ErrorProcess(WM_SOCKET_CONNECT,(long)lParam);
	}
	return 0;
}

LRESULT CAOIPacketWnd::OnSocketProcess(WPARAM wParam, LPARAM lParam)
{
    if ( wParam == WM_SOCKET_PARSER )
    {
        if ( m_pLinkPacket )
        {
		    m_pLinkPacket->ReceiveAllData();

            ::SetEvent( m_hEvent[ EV_REVEIVE_OK ] );
		}
    }
	else if (wParam==WM_SOCKET_RECEIVE){
        ::SetEvent( m_hEvent[ EV_RECEIVE ] );
		//if (m_pLinkPacket){
		//		m_pLinkPacket->ReceiveAllData();
		//}
	}
	else if (wParam==WM_SOCKET_CONNECT){
		m_xMode=_T("LAN--");
		if (m_pLinkPacket){
			m_pLinkPacket->CleanReceiveBuffer();
			m_pLinkPacket->SetConnectType(3); //CONNECT_LAN
			m_pLinkPacket->CheckDeviceMode();
		}
	}
	else if (wParam==WM_SOCKET_CLOSE){
		if (m_pLinkPacket){
			m_pLinkPacket->CleanReceiveBuffer();
		}
	}
	return 0;
}
LRESULT CAOIPacketWnd::OnQueryAlive(WPARAM wParam,LPARAM lParam) //eric chao 20150320
{
	if (m_pLinkPacket){
		m_pLinkPacket->SendQueryAlive();
	}
	return TRUE;
}
LRESULT CAOIPacketWnd::OnDeviceReady(WPARAM wParam, LPARAM lParam)
{
	if (m_lCheckDeviceTimer){
		KillTimer(m_lCheckDeviceTimer);
		m_lCheckDeviceTimer=0;
		if (m_pLinkPacket)
			m_pLinkPacket->SetDeviceReady(true);
#if 0 //don't need always check connection
		KillTimer(m_lDeviceQueryAliveTimer);
		m_lDeviceQueryAliveTimer=SetTimer(TIMER_DEVICE_QUERYALIVE,m_lQueryAliveTime,NULL);
#endif //0
		return TRUE;
	}
	if (m_pLinkPacket){
		m_pLinkPacket->SetDeviceReady(true);
		return TRUE;
	}
	return FALSE;
}
LRESULT CAOIPacketWnd::OnUSBTimer(WPARAM wParam, LPARAM lParam)
{
	if (wParam==1){
		if (m_lUsbDataTimer==0){
			if ((int)lParam==0){
				m_lUsbDataTimer=this->SetTimer(TIMER_USB_DEVICE,m_lUsbTime,NULL);
			}
			else{
				m_lUsbTime=(int)lParam;
				m_lUsbDataTimer=this->SetTimer(TIMER_USB_DEVICE,m_lUsbTime,NULL);
			}
		}
	}
	else {
		if (m_lUsbDataTimer){
			KillTimer(m_lUsbDataTimer);
			m_lUsbDataTimer=0;
		}
	}

	return FALSE;
}
LRESULT CAOIPacketWnd::OnChangeTimer(WPARAM wParam, LPARAM lParam)
{
	long tNewTimer=(long)wParam;
	if ( (tNewTimer>0 && m_lElapseTime!=tNewTimer) || m_lCheckWaitingTimer==0 ){
	m_lElapseTime=tNewTimer;
	KillTimer(m_lCheckWaitingTimer);
	m_lCheckWaitingTimer=SetTimer(TIMER_CHECK_PACKET_IN_QUEUE,m_lElapseTime,NULL);
	return TRUE;
	}
	return FALSE;
}
BOOL CAOIPacketWnd::OnInitDialog()
{
	m_lCheckWaitingTimer=this->SetTimer(TIMER_CHECK_PACKET_IN_QUEUE,m_lElapseTime,NULL);
	if (m_pLinkPacket && m_pLinkPacket->IsDeviceReady()){
		if (m_lCheckDeviceTimer){
			KillTimer(m_lCheckDeviceTimer);			
		}
		m_lCheckDeviceTimer=0;
	}
	else{
		m_lCheckDeviceTimer=SetTimer(TIMER_CHECK_DEVICE,m_lCheckTime,NULL);
	}
    m_hThread = ::CreateThread( NULL, NULL, Thread_SocketReceive, this, NULL, NULL );

	return true;
}
void CAOIPacketWnd::SetInputData(BYTE* pInput,int iSize)
{
	CString temp;
	temp.Preallocate(iSize*4+2);
	for (int i=0;i<iSize;i++){
		if (temp.IsEmpty())
			temp.Format(_T("%.2X"),pInput[i]);
		else
			temp.Format(_T("%s-%.2X"),temp,pInput[i]);
	}
	m_xInputCmd.Format(_T("%s"),temp);	
}
void CAOIPacketWnd::SetOutputData(BYTE* pOutput,int iSize)
{
	CString temp;
	temp.Preallocate(iSize*4+2);
	for (int i=0;i<iSize;i++){
		if (temp.IsEmpty())
			temp.Format(_T("%.2X"),pOutput[i]);
		else
			temp.Format(_T("%s-%.2X"),temp,pOutput[i]);
	}
	m_xOutputCmd.Format(_T("%s"),temp);	
}
#ifdef SUPPORT_SERIAL
LRESULT	CAOIPacketWnd::OnSerialMsg(WPARAM wParam, LPARAM lParam) //For Comport Data Notify
{//Receive CSerialMFC
	const CSerialMFC::EEvent eEvent = CSerialMFC::EEvent(LOWORD(wParam));

	DWORD dwBytesRead = 0;
	
	switch (eEvent)
	{
	case CSerialMFC::EEventRecv:
		if (m_pLinkPacket){
			m_pLinkPacket->ReceiveAllData();
		}
		break;
	}	
	// Return successful
	return 0;

}
#endif //SUPPORT_SERIAL
// CAOIPacketWnd 訊息處理常式

DWORD CAOIPacketWnd::Thread_SocketReceive( void* pvoid )
{
    CAOIPacketWnd* pAoiPacketWnd = ( CAOIPacketWnd* )pvoid;

    for ( int i = NULL; i < EV_COUNT; i++ ) pAoiPacketWnd->m_hEvent[ i ] = ::CreateEvent( NULL, TRUE, FALSE, NULL );

    BOOL bRun = TRUE;

    while ( bRun )
    {
        switch ( ::WaitForMultipleObjects( EV_COUNT, pAoiPacketWnd->m_hEvent, FALSE, INFINITE ) )
        {
        case WAIT_OBJECT_0 + 1:
            {
                ::ResetEvent( pAoiPacketWnd->m_hEvent[ EV_RECEIVE    ] );
                ::ResetEvent( pAoiPacketWnd->m_hEvent[ EV_REVEIVE_OK ] );

			    pAoiPacketWnd->PostMessage( WM_SOCKET_PROCESS, WM_SOCKET_PARSER );

                ::WaitForSingleObject( pAoiPacketWnd->m_hEvent[ EV_REVEIVE_OK ], INFINITE ); // wait parser command complete, bug: double parse command

                ::ResetEvent( pAoiPacketWnd->m_hEvent[ EV_REVEIVE_OK ] );
            }
            break;
        case WAIT_OBJECT_0:
            {
                bRun = FALSE;
            }
            break;
        }
    }
    for ( int i = NULL; i < EV_COUNT; i++ ) ::CloseHandle( pAoiPacketWnd->m_hEvent[ i ] );

    return NULL;
}
