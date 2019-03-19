#include "StdAfx.h"
#include "LightSourceController.h"
//#include <DevTemperatureInfo.h>
#include <aoiconst.h>
#ifdef SUPPORT_AOI
#include "..\aoi\AOI.h" //eric chao 20151123 for InsertDebugLog
#include "AoiDefaultSetting.h" //20170828 for g_AoiDefault
#endif //SUPPORT_AOI

//eric chao 20170222
#ifndef MIN
# define MIN(x,y) ((x) > (y) ? (y) : (x))
#endif
CString MakeStrByArray(int *pVal,int nSize)
{
	CString strResult;
	for (int i=0;i<nSize;i++){
		CString strItem;
		strItem.Format(_T("%d,"),pVal[i]);
		strResult += strItem;
	}
	return strResult;
}


LightSourceController::LightSourceController(int iDeviceId)
{
	SetDeviceId(iDeviceId);
	Init();
}
LightSourceController::LightSourceController(void)
{
	Init();
}

LightSourceController::~LightSourceController(void)
{
	AttachNotify(NULL);
}


void LightSourceController::Init()
{
	//m_isBootLoader=MODE_BOOTLOADER;
	m_isBootLoader=MODE_FIRMWARE;
	m_lightUsingTime=0;	
	m_wModelType=ftyMCP_DES;
	m_lWndResourceId=ctLIGHT_CTRL_WND;	//for m_pAOIPacketWnd
	m_bGetLightParam = FALSE;	//seanchen 20151001
	memset(m_cModelName,0,sizeof(m_cModelName));
	memset(&m_xLightParam,0,sizeof(m_xLightParam));
	m_cSessionKey.RemoveAll();

	memset(m_ldLightUsingTime,0,sizeof(m_ldLightUsingTime));
	memset(m_ldLightError,0,sizeof(m_ldLightError));	
	memset(m_UserTypeTempLimit, ctLIGHT_MAX_TEMP, MAX_TEMPERATURE_LIMIT);

	for(int i=0;i<MAX_LIGHTTYPE_CNT;i++){
		memset(m_DefaultTempLimit[i], ctLIGHT_MAX_TEMP, MAX_TEMPERATURE_LIMIT);
	}
	m_bUseDefaultLimit = TRUE;

	SetDeviceConnectType(DEVICE_LM,CONNECT_COM);
}

void LightSourceController::SetTemperatureLimit(unsigned char *limitT, int no,BOOL bUseDefalut) //20140516-01
{
	int nMin = 0;

	if(bUseDefalut)
	{
		m_bUseDefaultLimit = TRUE;
	}
	else
	{
		if(limitT){
			m_bUseDefaultLimit = FALSE;
			nMin = min(no,MAX_TEMPERATURE_LIMIT);
			if(nMin){
				memcpy(m_UserTypeTempLimit,limitT,nMin);
			}
		}
		else{
			m_bUseDefaultLimit = TRUE;
		}

	}
}
void LightSourceController::GetDeviceInfoByLightParam() //eric chao 20161107
{
	if (m_xLightParam.LightType == LT_ADL ||
		m_xLightParam.LightType == LT_ADL_BSL){
		m_xDevInfo.bCheckLEDModel = TRUE;
		m_xDevInfo.bSupportGroupLed = FALSE;
		if (m_xLightParam.HardRev <= 3){ //1==> Old Hardware,2==> 1/2 LED,3==>New Hardware
			m_xDevInfo.cModelType = (BYTE)m_xLightParam.HardRev;
		}
	}
	else{
		m_xDevInfo.bCheckLEDModel = FALSE;
		m_xDevInfo.cModelType = 0;
	}
}
void LightSourceController::GetLightInfoByModelType(WORD wType) //eric chao 20161216
{
	switch(wType)
	{
	case ftyPIC67J60_MDL_SWITCH:
		m_xDevInfo.bSupportGroupLed = TRUE;
		sendGroupLedRequest();
		break;
	case ftyPIC_MLL_15W: //eric chao 20170710, MLL/MLL Switch use same ModelType,use LHT_LED_GROUP_NUM replace
		//m_xDevInfo.bSupportGroupLed = FALSE;
		sendGroupLedRequest();
		break;
	default:
		m_xDevInfo.bSupportGroupLed = FALSE;
		break;
	}
}
void LightSourceController::ParseSetupCommand(SETUP_COMMAND_PKT *pPkt,int iSize)
{
	if (pPkt){
		WORD tValue=SWAP(pPkt->SlaveDeviceValue);
		if (pPkt->SlaveDeviceId==L_MESSAGE){			
			switch(pPkt->SlaveDeviceParam)
			{
			case FAN_ERROR_REPORT:
				if (m_pNotify)
					m_pNotify->OnSetupNotify(FAN_ERROR_REPORT,tValue);
				break;
			case GENERAL_ERROR_REPORT:
				if (m_pNotify)
					m_pNotify->OnSetupNotify(GENERAL_ERROR_REPORT,tValue);
				break;
			case ENCODER_TEST:
				if (m_pNotify)
					m_pNotify->OnSetupNotify(ENCODER_TEST,tValue);
				break;
			case LHT_VERSION:				
				tValue=(pPkt->SlaveDeviceValue&0xFF)*100+((pPkt->SlaveDeviceValue&0xFF00)>>8);
				m_xDevInfo.nFirmwareVer = tValue;
				SetDeviceReady(TRUE);
				if (GetAoiPacketWnd())
					GetAoiPacketWnd()->PostMessage(WM_DEVICE_READY,NULL,NULL);
				if (m_pNotify)
					m_pNotify->OnSetupNotify(LHT_VERSION,tValue);
				break;
			case LHT_SERIALNO:
				if (m_pNotify)
					m_pNotify->OnSetupNotify(LHT_SERIALNO,tValue);
				break;
			}
		}
		if (pPkt->SlaveDeviceId==LHT_SRC){
			switch(pPkt->SlaveDeviceParam)
			{
			case INTERNAL_MODELTYPE:
				m_bIsSupportInternalMode=TRUE;				
				m_wModelType=tValue;
				GetLightInfoByModelType(m_wModelType); //eric chao 20161216
				if (m_pNotify)
					m_pNotify->OnSetupNotify(INTERNAL_MODELTYPE,tValue);
				break;
			case scdLIGHT_PULSE_LENGTH:
				if (m_pNotify)
					m_pNotify->OnSetupNotify(scdLIGHT_PULSE_LENGTH,tValue);
				break;
			case LHT_LED_GROUP_NUM: //eric chao 20161108
				m_xDevInfo.bSupportGroupLed = TRUE;
				m_xDevInfo.cGroupLedNum = (BYTE)tValue;
				if (m_xDevInfo.cGroupLedNum == 0){ //20170710
					m_xDevInfo.bSupportGroupLed = FALSE;
				}
				break;
			default:
				break;
			}
		}
	}
}
void LightSourceController::ParseActionCommand(ACTION_COMMAND_PKT *pPkt,int iSize)
{//Implement firmware to PC
	if (pPkt){
		if (pPkt->SlaveDeviceId==LHT_PROG){
			switch(pPkt->ActionParam)
			{
			case ENTER_LHT_BOOTLOADER:
				break;
			case HAS_ENTER_BOOTLOADER:
				m_isBootLoader=MODE_BOOTLOADER;
				if (m_pNotify)
					m_pNotify->OnActionNotify(HAS_ENTER_BOOTLOADER);
				if (m_pNotify)
					m_pNotify->OnUpdateFirmwareNotify(UPDATE_ENTERBL);
				break;
			case PACKET_FINISH:
			case ERASE_OK:
			case acdSEND_FIRMWARE_ACK:
				if (m_pNotify){
					//m_pNotify->OnActionNotify(pPkt->ActionParam);
					::Sleep(15); //eric chao 20151207 for fix Light Device Problem
					m_pNotify->OnUpdateFirmwareNotify(UPDATE_MSG);
				}
				break;
			case CODE_FINISHED:
				break;
			case SEND_REBOOT_NOTIFY:
				if (m_pNotify){
					m_pNotify->OnActionNotify(pPkt->ActionParam);
				}
				break;
			}
		}		
	}
}
void LightSourceController::ParseEchoCommand(AOI_CTRL_HEADER *pHdr)
{
	if (m_pNotify)
		m_pNotify->OnEchoNotify(pHdr);
}
void LightSourceController::ParseMassDataCommand(MASS_DATA_COMMAND_HEADER *pMassDataHdr,int iSize)
{
	if (pMassDataHdr){
		BYTE* pData=(BYTE*)&pMassDataHdr->DataSize+1;
		switch(pMassDataHdr->DataType)
		{
		case MODEL_RSPN: //需要Session Key(ticktime)
			ASSERT(pMassDataHdr->DataSize==6);
			{
				m_cModelName[MAX_MODELNAME_SIZE-1]='\0';
				if (m_cSessionKey.GetSize()>=(MAX_MODELNAME_SIZE-1) )
					for (int i=0;i<(MAX_MODELNAME_SIZE-1);i++)
					{
						m_cModelName[i]=pData[i]^this->m_cSessionKey[i];
					}			
			}
			break;
		case READ_TEMP:
			m_areaTemperatures.clear();
			for (unsigned int i=0; i<pMassDataHdr->DataSize; i++)
				m_areaTemperatures.push_back(pData[i]);
			break;
		case LIGHT_USE_TIME:
			ASSERT(pMassDataHdr->DataSize==4);
			m_lightUsingTime=SWAP(*(long*)pData);
			break;
		case SET_LSOURCE_STATUS:
			ASSERT(pMassDataHdr->DataSize==4);
			{ //eric chao 20151123 Add Error Log~
			BYTE cStatus = pData[3];
			CString strDes;
			if (cStatus&0x1){ //Device Restart~
				strDes += _T("Device Restart!");
			}
			if (cStatus&0x2){ //Device Temperture Error
				strDes += _T("Device Temperture Limit!");
			}
			m_xDevInfo.cErrorState = cStatus; //eric chao 20161107
			m_xDevInfo.tErrorTime = CTime::GetCurrentTime().GetTime(); //eric chao 20161107
#ifdef SUPPORT_AOI
			if (!strDes.IsEmpty()){
				CString strDeviceId;
				strDeviceId.Format(_T("----Device:%d"),GetDeviceId());
				strDes += strDeviceId;
				theApp.InsertDebugLog(strDes,LOG_LIGHT);
			}
#endif //SUPPORT_AOI
			}
			break;
		case READ_TEMP_LIMIT:
				m_tempLimit.clear();
				for (unsigned int i=0; i<pMassDataHdr->DataSize; i++)
					m_tempLimit.push_back(pData[i]);
			break;
		case REPORT_LHT:
				m_tempLevel.clear();
				for (unsigned int i=0; i<pMassDataHdr->DataSize; i++)
					m_tempLevel.push_back(pData[i]);
			break;
		case DAC_TEST_REPORT:
			break;
		case READ_LD_TEMP:
			{
				int dataSize = pMassDataHdr->DataSize;
				if (pData){
				int idx = pData[0];
				ASSERT(idx < 20);
				for (int i=0; i<dataSize-1; i++)
					m_areaTemperatures.push_back(pData[1+i] + idx * 1000);
				}
			}
			break;
		case READ_LD_TIME:
			{
				ASSERT(pMassDataHdr->DataSize == 5);
				if (pData){
				int idx = pData[0];
				ASSERT(idx < 20);
				if (idx>=0 && idx< MAX_LIGHT_SETTING)
				m_ldLightUsingTime[idx]=SWAP(*(long*)&pData[1]);				
				}
			}
			break;
		case SET_LDERR_STATUS:
			{
				ASSERT(pMassDataHdr->DataSize == 5);
				if (pData){
				int idx = pData[0];
				ASSERT(idx < 20);
				if (idx>=0 && idx< MAX_LIGHT_SETTING)
				m_ldLightError[idx]=SWAP(*(long*)&pData[1]);				
				}
			}
			break;
		case READ_LD_TEMP_LIMIT:
			{
				int dataSize = pMassDataHdr->DataSize;
				if (pData){
				int idx = pData[0];
				for (int i=0; i<dataSize-1; i++)
					m_tempLimit.push_back(pData[1+i] + idx * 1000);
				}
			}
			break;
		case REPORT_LD_LHT:
			{
				m_tempLevel.clear(); //eric chao 20130201 need clear old data
				int dataSize = pMassDataHdr->DataSize;
				ASSERT(dataSize > 1);
				if (pData){
					int idx = pData[0];
					ASSERT(idx < 20);
					for (int i=0; i<dataSize-1; i++){
						m_tempLevel.push_back(pData[1+i] + idx * 1000);
					}
				}
			}
			break;
		case REPORT_LDFAN_ERROR:
			{
				ASSERT(pMassDataHdr->DataSize == 3);
				if (pData){
				int idx = pData[0];
				ASSERT(idx < 20);
				int errorCode = 0;
				errorCode=(int)SWAP(*(WORD*)&pData[1]);
				errorCode += (idx << 16);
				}
			}
			break;
		case LD_LHT_VERSION:
			{
				ASSERT(pMassDataHdr->DataSize == 3);
				if (pData){
				int idx = pData[0];
				ASSERT(idx < 20);
				int version = pData[1] * 100 + pData[2];
				version += (idx << 16);
				m_xDevInfo.nFirmwareVer = version;
				SetDeviceReady(TRUE);
				if (GetAoiPacketWnd())
					GetAoiPacketWnd()->PostMessage(WM_DEVICE_READY,NULL,NULL);				
				}
			}
			break;
		case REPORT_LHT_PARAM: //eric chao 20121120
			{
				ASSERT(pMassDataHdr->DataSize == sizeof(m_xLightParam));
				if (pMassDataHdr->DataSize == sizeof(m_xLightParam)){
					memcpy(&m_xLightParam,pData,sizeof(m_xLightParam));
#ifdef SUPPORT_MDL_FLASH
					if (m_xLightParam.LightType == LT_MDL){
						m_xLightParam.LightType = LT_MDL_FLASH;
					}
#endif //SUPPORT_MDL_FLASH
					m_bGetLightParam = TRUE;	//seanchen 20151001
					SendTemperatureLimit_ByLightType();
				}
				else{
					unsigned char Tmp[MAX_TEMPERATURE_LIMIT];
					memset(Tmp, ctLIGHT_MAX_TEMP, MAX_TEMPERATURE_LIMIT);
					sendSetupTemperatureLimit(Tmp, MAX_TEMPERATURE_LIMIT);
				}
				GetDeviceInfoByLightParam(); //eric chao 20161107
			} 
			break;
		case LD_LHT_SERIALNO:
			{
				ASSERT(pMassDataHdr->DataSize == 3);
				if (pData){
				int idx = pData[0];
				ASSERT(idx < 20);
				int serial = 0;
				serial=SWAP(*(short*)&pData[1]);				
				serial += (idx << 16);
				}
			}
			break;
		}
		if (m_pNotify)
			m_pNotify->OnMassDataNotify(pMassDataHdr,iSize);
	}
}
void LightSourceController::ParseMultiPurposeCommand(MULTI_PURPOSE_COMMAND_HEADER *pMPDataHdr,int iSize)
{

}

void LightSourceController::SetSessionKey(int iNo,unsigned char *key)
{
	if (key){
		if (m_cSessionKey.GetSize())
			m_cSessionKey.RemoveAll();
		m_cSessionKey.SetSize(iNo);

		for (int i=0;i<iNo;i++){		
			m_cSessionKey.SetAt(i,(BYTE)key[i]);
		}
	}
}


//---Send Action Command---
// 1-1-2, ask light device temperature
int	 LightSourceController::sendTemperatureRequest(CString errmsg)	// action command
{
	return sendActionRequest(LHT_DEVICE,ASK_TEMP,TRUE,errmsg);	
}

// 1-1-3, ask light device using time
int	 LightSourceController::sendUsingTimeRequest(CString errmsg)	// action command
{
	return sendActionRequest(LHT_DEVICE,USE_TIME,TRUE,errmsg);
}

// 1-1-4, reset light device using time
int	 LightSourceController::sendResetUsingTime(CString errmsg)	// action command
{
	return sendActionRequest(LHT_DEVICE,CLEAR_USE_TIME,TRUE,errmsg);	
}
// 1-1-5, ask light group number
int LightSourceController::sendGroupLedRequest(CString errmsg) //eric chao 20161108
{
	return sendActionRequest(LHT_DEVICE, ASK_GROUP_LED_NUM,TRUE, errmsg);
}

// 1-1-6, ask light device fan status
int	 LightSourceController::sendFanErrorRequest(CString errmsg)	// action command
{
	return sendActionRequest(LHT_DEVICE,ASK_FAN_STATUS,TRUE,errmsg);	
}

// 1-1-7, ask light device general error
int	 LightSourceController::sendGeneralErrorRequest(CString errmsg)	// action command
{
	return sendActionRequest(LHT_DEVICE,ASK_GENERAL_STATUS,TRUE,errmsg);
}

// 1-1-8, ask light source led error
int	LightSourceController::sendLEDErrorRequest(CString errmsg)
{
	return sendActionRequest(LHT_DEVICE,ASK_LS_STATUS,TRUE,errmsg);	
}

// 1-1-9, ask lighting level
int	LightSourceController::sendLightLevelRequest(CString errmsg)
{
	return sendActionRequest(LHT_DEVICE,ASK_LHT,TRUE,errmsg);	
}

// 1-1-10, ask light device version
int	LightSourceController::sendLightVersionRequest(CString errmsg)
{
	return sendActionRequest(LHT_DEVICE,ASK_LHT_VERSION,TRUE,errmsg);	
}

// 1-1-11, ask light device serial number
int	LightSourceController::sendLightSerialRequest(CString errmsg)
{
	return sendActionRequest(LHT_DEVICE,ASK_SERIALNO,TRUE,errmsg);	
}

// 1-1-12, ask light device temp limit
int	LightSourceController::sendLightTLimitRequest(CString errmsg)
{
	return sendActionRequest(LHT_DEVICE,ASK_TEMP_LIMIT,TRUE,errmsg);
}

int LightSourceController::sendInternalModelTypeRequest(CString errmsg)
{	
	return sendActionRequest(LHT_DEVICE,ASK_INTERNAL_MODELTYPE,FALSE,errmsg);
}

// 5-1-1, ask device to turn into bootloader mode
int	LightSourceController::sendUpgradeRequest(CString errmsg)
{
	return sendActionRequest(LHT_PROG,ENTER_LHT_BOOTLOADER,TRUE,errmsg);
}

// 5-1-4, send finish upgrading command
BOOL LightSourceController::sendFinishUpgrade(CString errmsg)
{
	return sendActionRequest(LHT_PROG,CODE_FINISHED,FALSE,errmsg);
}

BOOL LightSourceController::sendNewStartUpgrade(void)
{
	return sendActionRequest(LHT_DEVICE,acdSTART_SEND_FIRMWARE,FALSE,_T(""));	
}
// 1-1-14
BOOL LightSourceController::sendAskLightParam(void)
{
	return sendActionRequest(LHT_DEVICE,acdASK_LIGHT_PARAM,FALSE,_T(""));	
}
// 1-1-16
BOOL LightSourceController::sendMcuReset(void)
{
	return sendActionRequest(LHT_DEVICE,ASK_MCU_RESET,FALSE,_T(""));
}
BOOL LightSourceController::sendNewFinishUpgrade(void)
{
	return sendActionRequest(LHT_DEVICE,acdEND_SEND_FIRMWARE,FALSE,_T(""));
}
//----Send Setup Command------------------------
// 2-2-3, setup light device serial number
int	LightSourceController::sendSetupLightSerial(int serialNo, CString errmsg)
{
	//short tValue=SWAP((short)serialNo); //fix bug
	short tValue=(short)serialNo;
	return sendSetupRequest(LHT_SRC,SET_SERIALNO,(WORD)tValue,TRUE,errmsg);
}
// 2-2-4, setup light device sensor status
int LightSourceController::sendSetupLightSensor(int status, CString errmsg)
{
	//short tValue=SWAP((short)status);
	short tValue=(short)status;
	return sendSetupRequest(LHT_SRC,SET_TSENOR_STATUS,tValue,FALSE,errmsg);
}
// 2-2-31, setup linked light device id
int	 LightSourceController::sendSetupLDLightID(CString errmsg)
{
	short tValue=0;
	return sendSetupRequest(LHT_SRC,SET_LINKDEVICE_ID,tValue,TRUE,errmsg);
}

// 2-2-32, ask linked light device to turn into bootloader mode
int LightSourceController::sendUpgradeLDRequest(int idx, CString errmsg)
{
	//short tValue=SWAP((short)idx);
	short tValue=(short)idx;
	return sendSetupRequest(LHT_SRC,ASK_ENTER_LDBOOT,tValue,FALSE,errmsg);
}
// 2-2-33, change device ipaddress
int	LightSourceController::sendDeviceId(int NewDeviceID, CString errmsg)	// setup command
{
	short tValue=(short)NewDeviceID;
	return sendSetupRequest(LHT_SRC,LHT_LAN_DEVICEID,tValue,TRUE,errmsg);
}

// 5-2-1, setup machine to know the code length
int	LightSourceController::sendSetupCodeLength(int codeLength, CString errmsg)
{
	//short tValue=SWAP((short)codeLength);
	short tValue=(short)codeLength;
	return sendSetupRequest(LHT_PROG,CODE_LENGTH,tValue,TRUE,errmsg);	
}
//eric chao 20170126 modify
int LightSourceController::sendGroupLed(int nGroupNum, CString errmsg)
{
	short tValue = (short)nGroupNum;
	sendSetupRequest(LHT_SRC, LHT_SET_LED_GROUP_NUM, tValue,TRUE, errmsg);
	return sendGroupLedRequest();
}

//---Send Mass Data Command----------------
//3-1-6
int	 LightSourceController::sendLightModelRequest(unsigned char *key, int keyNo, CString errmsg)	// mass data command
{
	SetSessionKey(keyNo,key);
	CByteArray cBody;
	int iSize=0;
	iSize=keyNo+2;
	cBody.SetSize(iSize);
	cBody.SetAt(0,SET_TMPKEY_TO_ASK_MODEL);
	cBody.SetAt(1,(BYTE)keyNo);
	for (int i=0;i<keyNo;i++){
		cBody.SetAt(i+2,(BYTE)key[i]);		
	}
	return sendMassDataRequest(SET_TMPKEY_TO_ASK_MODEL,(BYTE)(cBody.GetSize()-2),cBody.GetData(),TRUE,errmsg);
}
// 3-10, setup lighting level
int	 LightSourceController::sendSetupLightLevel(int lightNo, int *level, CString errmsg)
{
    ASSERT(lightNo>=0 && lightNo<=255);
	if (lightNo>=0){
		if (m_xLightValue.size()){
			int iValue=(int)m_xLightValue.size();
			if (GetConnectType() != CONNECT_LAN){ //eric chao 21061108 Only,RS232 or USB Device Need check Same Light Level Value
				if (iValue == lightNo){
					BOOL bCompare = true;
					for (int i = 0; i<iValue; i++){
						if (m_xLightValue[i] != level[i])
							bCompare = false;
					}
					if (bCompare) //Same Light Value,don't need change
						return -1;
				}
			}
			m_xLightValue.clear();						
		}
		//eric chao 20170222 For RS232 Light Device,NoChannel always 0
		int DEV_LIGHT_LEVEL[MAX_LIGHT_LEVEL_ITEM];
		memset(DEV_LIGHT_LEVEL,0,sizeof(DEV_LIGHT_LEVEL));
		for (int i=0;i<lightNo;i++){
			if (i < MAX_LIGHT_LEVEL_ITEM){
				if (m_xLightParam.NoChannel!=0){
					if (i< m_xLightParam.NoChannel){
						DEV_LIGHT_LEVEL[i] = level[i];
					}
				}else{
					DEV_LIGHT_LEVEL[i] = level[i];
				}
			}
		}
		
		int nLightNum = lightNo;
		if (m_xLightParam.NoChannel != 0){
			nLightNum = MIN(lightNo,m_xLightParam.NoChannel);
		}
		int nTotalLightLevel = 0;
		for (int i=0;i<nLightNum;i++){
			nTotalLightLevel += level[i];
		}
#ifdef SUPPORT_AOI
		if (g_AoiDefault.IsLightLevelLimit()){
			if (nTotalLightLevel > MAX_TOTAL_LIGHT_LEVEL){
				for (int i = 0; i<nLightNum; i++){
					DEV_LIGHT_LEVEL[i] = (level[i] * MAX_TOTAL_LIGHT_LEVEL) / nTotalLightLevel;
				}
				CString strLevelO = MakeStrByArray(level, nLightNum);
				CString strLevelN = MakeStrByArray(DEV_LIGHT_LEVEL, nLightNum);
				CString strLog;
				strLog.Format(_T("ReRange Light Level Value!(%s),(%s)"), strLevelO, strLevelN);
				theApp.InsertDebugLog(strLog, LOG_LIGHT);
			}
		}
#endif //SUPPORT_AOI
//eric chao 20170222避免燈具的異常處理,固定都傳送20個燈條的亮度值
		int nSetupLightNum = MAX_LIGHT_LEVEL_ITEM;
		for (int i=0;i<nSetupLightNum;i++)
			m_xLightValue.push_back(DEV_LIGHT_LEVEL[i]);
		CByteArray cBody;
		int iSize=0;
		iSize=nSetupLightNum+2;
		cBody.SetSize(iSize);
		cBody.SetAt(0,ADJ_LHT);
		cBody.SetAt(1,(BYTE)nSetupLightNum);
		for (int i=0;i<nSetupLightNum;i++)
			cBody.SetAt(i+2,(BYTE)DEV_LIGHT_LEVEL[i]);
		return sendMassDataRequest(ADJ_LHT,(BYTE)(cBody.GetSize()-2),cBody.GetData(),TRUE,errmsg);
	}

	return -1;	
}
// 1-3-1, setup light device model
int	 LightSourceController::sendSetupLightModel(CString model, CString errmsg)	// mass data command
{
	CByteArray cBody;
	int iSize=0;
	int size=model.GetLength()+1;
	char name[100];
	memset(name,0,sizeof(name));
	strcpy_s(name, 100, CW2A(model.GetBuffer()));
	iSize=size+2;
	cBody.SetSize(iSize);
	cBody.SetAt(0,SET_MODEL);
	cBody.SetAt(1,(BYTE)size);
	for (int i=0;i<size;i++)
		cBody.SetAt(i+2,(BYTE)name[i]);

	return sendMassDataRequest(SET_MODEL,(BYTE)(cBody.GetSize()-2),cBody.GetData(),TRUE,errmsg);
}

// 1-3-3, setup stop working temperature limits
int	 LightSourceController::sendSetupTemperatureLimit(unsigned char *limitT, int no, CString errmsg)	// mass data command
{
	if (no>=0){
	CByteArray cBody;
	int iSize=0;
	iSize=no+2;
	cBody.SetSize(iSize);
	cBody.SetAt(0,SET_TEMP_LIMIT);
	cBody.SetAt(1,(BYTE)no);
	for (int i=0;i<no;i++)
		cBody.SetAt(i+2,(BYTE)(limitT[i]));

	return sendMassDataRequest(SET_TEMP_LIMIT,(BYTE)(cBody.GetSize()-2),cBody.GetData(),TRUE,errmsg);	
	}
	return -1;
}
// 5-3-1, send firmware package to device
int	LightSourceController::sendSetupFirmware(BYTE *firmwareData, int dataLength, CString errmsg)
{
	CByteArray cBody;
	int iSize=dataLength+2;
	cBody.SetSize(iSize);
	cBody.SetAt(0,LHT_CODE);
	if (dataLength == 256)          //==>bug
		cBody.SetAt(1,0);		// 256
	else 
		cBody.SetAt(1,(BYTE)dataLength);
	BYTE *pData=cBody.GetData();
	memcpy(pData+2,firmwareData,dataLength);
	
	return sendMassDataRequest(LHT_CODE,(BYTE)(cBody.GetSize()-2),cBody.GetData(),FALSE,errmsg);
}
// 3-12, setup Light Param
int	LightSourceController::sendSetupLightParam(BYTE *pLightParam,int ParamSize,CString errmsg)
{//eric chao 20121120
	CByteArray cBody;
	int iSize=ParamSize+2;
	cBody.SetSize(iSize);
	cBody.SetAt(0,SET_LHT_PARAM);
	cBody.SetAt(1,(BYTE)ParamSize);
	BYTE *pData=cBody.GetData();
	memcpy(pData+2,pLightParam,ParamSize);
	return sendMassDataRequest(SET_LHT_PARAM,(BYTE)(cBody.GetSize()-2),cBody.GetData(),FALSE,errmsg);
}
// 3-13, setup Group Light
int LightSourceController::sendSetupGroupChannelMap(LIGHT_GROUP_MAP xMap,CString errmsg) 
{//eric chao 20161214
	CByteArray cBody;
	int nParamSize = sizeof(xMap);
	int iSize = nParamSize + 2;
	cBody.SetSize(iSize);
	cBody.SetAt(0, SET_GROUP_CHANNEL);
	cBody.SetAt(1, (BYTE)nParamSize);
	BYTE *pData = cBody.GetData();
	memcpy(pData + 2, &xMap, sizeof(xMap) );
	return sendMassDataRequest(SET_GROUP_CHANNEL, (BYTE)(cBody.GetSize() - 2), cBody.GetData(), FALSE, errmsg);
}
#if 0
int LightSourceController::sendSetupFlahLightLevel(LIGHT_GROUP_LIGHT_LEVEL_ITEM xItem, CString errmsg) //eric chao 20161108
{
	CByteArray cBody;
	int nParamSize = sizeof(xItem);
	int iSize = nParamSize + 2;
	cBody.SetSize(iSize);
	cBody.SetAt(0, SET_FLASH_LET);
	cBody.SetAt(1, (BYTE)nParamSize);
	BYTE *pData = cBody.GetData();
	memcpy(pData + 2, &xItem, sizeof(xItem) );
	return sendMassDataRequest(SET_FLASH_LET, (BYTE)(cBody.GetSize() - 2), cBody.GetData(), FALSE, errmsg);
}
#endif //0
int	LightSourceController::SetupGroupLightLevel(int nGroupId, int lightNo, int *level, CString errmsg) //eric chao 20161108
{
	if ((nGroupId+1) >= MIN_LIGHT_GROUP_LED  &&
		(nGroupId+1) <= MAX_LIGHT_GROUP_LED){
		if (m_xDevInfo.bSupportGroupLed){
			if (lightNo > 0 && level){
				int nIdx = nGroupId;
				m_xDevInfo.xGroupInfo[nIdx].nLightNo = lightNo;
				for (int i = 0; i < lightNo; i++){
					m_xDevInfo.xGroupInfo[nIdx].nLevel[i] = level[i];
				}
			}
		}
	}
	return 0;
}
void LightSourceController::OpGroupLightLevel(int nOpCode) //eric chao 20161108
{
	if (nOpCode == OP_GROUP_LIGHT_RESET){
		memset(m_xDevInfo.xGroupInfo, 0, sizeof(m_xDevInfo.xGroupInfo));
	}
	else if (nOpCode == OP_GROUP_LIGHT_FINISH){
		if (m_xDevInfo.bSupportGroupLed){
			//Merge Result & Send Protocol to Device
			int nGroup = (int)m_xDevInfo.cGroupLedNum;
			//For Current Device,Only can process one level
			int nTotalLevel = 0;
			LIGHT_GROUP_LIGHT_LEVEL_ITEM xCurLed;
			memset(&xCurLed, 0, sizeof(xCurLed));
			for (int i = 0; i < nGroup; i++){
				if (m_xDevInfo.xGroupInfo[i].nLightNo > nTotalLevel){
					nTotalLevel = m_xDevInfo.xGroupInfo[i].nLightNo;
				}
				for (int j = 0; j < m_xDevInfo.xGroupInfo[i].nLightNo; j++){
					if (m_xDevInfo.xGroupInfo[i].nLevel[j]){
						xCurLed.nLevel[j] = m_xDevInfo.xGroupInfo[i].nLevel[j];
						xCurLed.cMapMask[j] |= (1 << i);
					}
				}
			}
			xCurLed.nLightNo = nTotalLevel;
			//eric chao 20161214 modify....Need Check Map
			//sendSetupFlahLightLevel(xCurLed);
			sendSetupLightLevel(xCurLed.nLightNo,(int*)xCurLed.nLevel);
			LIGHT_GROUP_MAP xMap;
			memset(&xMap,0,sizeof(xMap));
			for (int i=0;i<xCurLed.nLightNo;i++){
				for (int j=0;j<MAX_LIGHT_GROUP_LED;j++){
					if (xCurLed.cMapMask[i]&(1<<j)){
						int nMapPos = j*4+(i/8);
						xMap.cMap[nMapPos] |= (1<<(i%8));
					}
				}
			}
			sendSetupGroupChannelMap(xMap);
		}
	}
}

void LightSourceController::QueryVerionInfo() 
{
	sendGroupLedRequest(); //eric chao 20170622 fix Flash LED Error
	sendAskLightParam(); //for get LM Descript  //must ask to get light type for setting Temperature limit
	sendLEDErrorRequest(); //eric chao 20151123 //for get error report from LM
	sendLightVersionRequest(); //for get LM Version Info
}
void LightSourceController::AskLightModelName()
{
	CString key;
	key.Format(_T("%d"), (int)GetTickCount64());
	key = key.Right(6);
	char sessionKey[7];
	memset(sessionKey,0,sizeof(sessionKey));
	for (int i=0; i<6; i++)
		sessionKey[i] = (unsigned char)key.GetAt(i);
	sendLightModelRequest((unsigned char *)sessionKey, 6);
}


void LightSourceController::SendTemperatureLimit_ByLightType() 
{
	int nDevID = GetDeviceId();
	if(m_bUseDefaultLimit){//20140516-01
		if((m_xLightParam.LightType >=0)&&((m_xLightParam.LightType <MAX_LIGHTTYPE_CNT))){
			sendSetupTemperatureLimit(m_DefaultTempLimit[m_xLightParam.LightType], MAX_TEMPERATURE_LIMIT);
		}
		else{
				unsigned char Tmp[MAX_TEMPERATURE_LIMIT];
				memset(Tmp, ctLIGHT_MAX_TEMP, MAX_TEMPERATURE_LIMIT);
				sendSetupTemperatureLimit(Tmp, MAX_TEMPERATURE_LIMIT);
		}

	}
	else{
		sendSetupTemperatureLimit(m_UserTypeTempLimit, MAX_TEMPERATURE_LIMIT);
	}
}

