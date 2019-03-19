#pragma once
#include "aoicontrolpacket.h"

//#define _USE_OLD_TEMPLIMIT_PROC_ //20140502-01

//----------------------------------------------------------
// LightModuleControl 燈具控制盒,實體連接層 (COM/USB/LAN)
// 用來串聯多個燈具的控制,但是一次最多只能連上其中一個
// 實體的控制處理,在這層處理
// ---------------------------------------------------------
// 透過 LightSourceController (燈具控制盒,虛擬控制層)
// 針對燈具的Protocol,在這層處理增加
// ---------------------------------------------------------
// 底層處理 AoiControlPacket (資料處理層)  IO卡和燈具共用
// 通用的資料處理,Parser,在這層處理
// eric chao
//----------------------------------------------------------
//		Light source controller
const int ctLIGHT_CTRL_WND = 3450; //IDD_LIGHT_CTRLD_WND
#define MP_SET_IMAGE_INFO			1
// --------------------------------------------------------------------
#define MAX_TEMPERATURE_LIMIT		8
#define MAX_LIGHTTYPE_CNT			1

#define LHT_DEVICE			1
#define LHT_SRC				2
#define L_MESSAGE			3
#define LHT_PROG			5

//---Action Command (LHT_DEVICE)
#define ASK_TEMP			2
#define USE_TIME			3
#define CLEAR_USE_TIME		4
#define ASK_GROUP_LED_NUM	5 //eric chao 20161108
#define ASK_FAN_STATUS		6
#define ASK_LS_STATUS		8
#define ASK_LHT				9
#define ASK_LHT_VERSION		10
#define ASK_SERIALNO		11
#define ASK_TEMP_LIMIT		12
#define ASK_INTERNAL_MODELTYPE	13		// 詢問device 類型(IO board ,72瓦480瓦燈具) (rtn:2-2-8)	// Beagle 20120622 added.
#define acdASK_LIGHT_PARAM	14
#define ASK_MCU_RESET		16
#define ASK_TEST_DAC		40
#define ASK_TEST_ENCODER	41
#define ASK_LED19_BLINK		42

//---Action Command (LHT_PROG)
#define ENTER_LHT_BOOTLOADER	52
#define HAS_ENTER_BOOTLOADER	53
#define PACKET_FINISH			54
#define CODE_FINISHED			55
#define ERASE_OK				56
#define	acdSTART_SEND_FIRMWARE	57
#define	acdEND_SEND_FIRMWARE	58
#define	acdSEND_FIRMWARE_ACK	59
#define SEND_REBOOT_NOTIFY		60

//---Setup Command (LHT_SRC)
#define SET_SERIALNO			3
#define SET_TSENOR_STATUS		4
#define LHT_VERSION				5
#define LHT_SERIALNO			6
#define SET_STROBE_MODE			7	// Beagle 20120622 added.
#define INTERNAL_MODELTYPE		8	// Beagle 20120622 added.
#define scdSET_LIGHT_PULSE_LENGTH  9
#define scdLIGHT_PULSE_LENGTH     10
#define LHT_SET_LED_GROUP_NUM	11 //eric chao 20161108
#define LHT_LED_GROUP_NUM		12 //eric chao 20161108
#define SET_LINKDEVICE_ID		31	// added by danny at 20101109, setup id for linked lighting devices
#define ASK_ENTER_LDBOOT		32	// added by danny at 20101109, ask specified id of linked lighting device to enter bootloader
#define LHT_LAN_DEVICEID		33 //eric chaoe 20120824
//---Setup Command(L_Message) //include LHT_VERSION,LHT_SERIALNO
#define FAN_ERROR_REPORT		1
#define GENERAL_ERROR_REPORT	2 //not in AOI_LA_Protocol document
#define ENCODER_TEST			20


//---MassData Command
#define SET_MODEL			1
#define MODEL_RSPN			2
#define SET_TEMP_LIMIT		3
#define READ_TEMP			4
#define LIGHT_USE_TIME		5
#define SET_TMPKEY_TO_ASK_MODEL	6
#define SET_LSOURCE_STATUS	7
#define READ_TEMP_LIMIT		8
#define REPORT_LHT			9
#define ADJ_LHT				10
#define REPORT_LHT_PARAM	11
#define SET_LHT_PARAM		12
#define SET_GROUP_CHANNEL	13		//eric chao 20161214 By Protocol Document
#define DAC_TEST_REPORT		21
#define READ_LD_TEMP		31		// added by danny at 20101109
#define READ_LD_TIME		32		// added by danny at 20101109
#define SET_LDERR_STATUS	33		// added by danny at 20101109
#define READ_LD_TEMP_LIMIT	34		// added by danny at 20101109
#define REPORT_LD_LHT		35		// added by danny at 20101109
#define REPORT_LDFAN_ERROR	36		// added by danny at 20101109
#define LD_LHT_VERSION		37		// added by danny at 20101109
#define LD_LHT_SERIALNO		38		// added by danny at 20101109


//---MULTI_PURPOSE_CMD
#define mpSET_IMAGE_INFO		1
#define mpSEND_FIRMWARE_DATA	2


//-----------------------------------------------------------
#define ASK_GENERAL_STATUS	7


#define LHT_CODE			13


//#define ADJ_LHT1				1
//#define ADJ_LHT2				2


//#define REPORT_LHT1				3
//#define REPORT_LHT2				4

#define	CODE_LENGTH				51


typedef enum _enmLightType
{
	LT_MLL=0,			// Modular Light Line, 15 LEDs per module
	LT_MDL,			// Modular Dome Light
	LT_ADL,			// Area Dome Light
	LT_BSL,			// Beam Splitter
	LT_ADL_BSL,		// Area Dome Light + Beam Splitter
	LT_SLL,			// Side Line Light
	LT_SPL,			// Spot Light
	LT_MLL_FLASH,	// MLL +Flash Type //eric chao 20161107 Not Yet
	LT_MDL_FLASH,   // MDL +Flash Type //eric chao 20161107
	LT_UNKNOWN = 0xFF		// Light type not specified yet
}enmLightType;

typedef enum _enmDiffuserType
{
	DT_NO_DIFFUSER,		// No diffuser
	DT_40x1,			// 40x1deg diffuser
	DT_60x1,			// 60x1deg diffuser
	DT_40x40,			// 40x40deg diffuser
	DT_2d40x40,		// 2 diffusers 40x40deg
	DT_60x60,			// 60x60deg diffuser
	DT_2d60x60,		// 2 diffusers 60x60deg
	DT_UNKNOWN = 0xFF	// Diffuser not specified yet
}enmDiffuserType;

typedef enum _enmReflectorType
{
	RT_NO_REFLECTOR,		// No reflector
	RT_DIFFUSE,		// Diffuse reflector
	RT_SPECULAR,		// Specular reflector (mirror-like reflector)
	RT_UNKNOWN = 0xFF		// Reflector not specified yet
}enmReflectorType;

typedef enum _enmLensType
{
	LT_NO_LENS,		// No lens
	LT_LEN_UNKNOWN = 0xFF		// Lens not specified yet.
}enmLensType;

//For Flash LED Control //eric chao 20161104
#define MIN_LIGHT_GROUP_LED 1
#define MAX_LIGHT_GROUP_LED 4

#define MAX_LIGHT_LEVEL_ITEM 20 
#define MAX_TOTAL_LIGHT_LEVEL 600 //eric chao 20170222


#pragma pack(push,1)
// --------------------------------------------------------------------
typedef struct _LIGHT_PARAM
{
	BYTE	LightType;		// Light type, see enum enmLightType
	unsigned short	MechaRev;	// Mechanic revision
	unsigned short	HardRev;	// Electronic hardware revision
	unsigned short	Dim1;		// Light dimension #1 in mm (length for MLL and MDL, Diameter for ADL, width for BSL)
	unsigned short	Dim2;		// Light dimension #2 in mm (not used for MLL, MDL and ADL, height for BSL)
	unsigned char	NoChannel;	// Number of light channels (MLL, ADL, BSL = 1, MDL = 1 to 14)
	unsigned short	Wavelength;	// LED Wavelength in nm (Cool White = 0)
	unsigned char	FanNum;		// Fan fitted or not (0 = fan not fitted, 1 = fan fitted)
	BYTE	Diffuser;		// Diffuser type, see enmDiffuserType.
	BYTE	Reflector;		// Reflector type, see enmReflectorType.
	BYTE	Lens;		// Lens type, see enmLensType
}LIGHT_PARAM;

typedef struct _LIGHT_GROUP_LIGHT_LEVEL_ITEM
{
	int nLightNo;
	int nLevel[MAX_LIGHT_LEVEL_ITEM];
	BYTE cMapMask[MAX_LIGHT_LEVEL_ITEM];
}LIGHT_GROUP_LIGHT_LEVEL_ITEM;

typedef struct _LIGHT_GROUP_MAP //eric chao 20161214
{
	BYTE cMap[16]; //Each Channel(4 BYTE),Total 4 Channel
}LIGHT_GROUP_MAP;

typedef struct _LIGHT_DEVICE_INFO //eric chao 20161104
{
	int nFirmwareVer;
	BYTE cGroupLedNum; // Value Range 1-4
	BOOL bSupportGroupLed;
	BOOL bCheckLEDModel;
	BYTE cModelType;	//type : O Model   (1.X) ,N Model   (3.X), S (LED/2)  (2.X)  //By Light Param
	BYTE cErrorState;	// 0 ==>no Error 1 ==> Reboot 2==> temp Limit shutdown
	UINT64 tErrorTime;
	LIGHT_GROUP_LIGHT_LEVEL_ITEM xGroupInfo[MAX_LIGHT_GROUP_LED]; //Save all Group Light Level
	_LIGHT_DEVICE_INFO()
	{
		nFirmwareVer = 0;
		cGroupLedNum = 0;
		bSupportGroupLed = FALSE;
		cModelType = 0;
		cErrorState = 0;
		tErrorTime = 0;
		memset(xGroupInfo, 0, sizeof(xGroupInfo));
	}
}LIGHT_DEVICE_INFO;

#pragma pack(pop)

#define MAX_MODELNAME_SIZE 7
#define MAX_LIGHT_SETTING 20

class ILmNotify
{
public:
	virtual void OnActionNotify(BYTE ActionParam)=0;
	virtual void OnSetupNotify(BYTE DeviceParam,WORD tValue)=0;
};


class LightSourceController :
	public AoiControlPacket
{
public:
	LightSourceController(int DeviceId); //eric chao 20121130
	LightSourceController(void);
	~LightSourceController(void);

	std::vector<int>* GetLightTemperature() {return &m_areaTemperatures;};
	void ClearLightTemperature() {m_areaTemperatures.clear();};
	std::vector<int>* GetLimitTemperature() {return &m_tempLimit;};
	void ClearLimitTemperature() {m_tempLimit.clear();};
	std::vector<int>* GetLightLevel() {return &m_tempLevel;};
	void ClearLightLevel() {m_tempLevel.clear();};
	void SetTemperatureLimit(unsigned char *limitT, int no,BOOL bDefalut); //20140516-01
	void SendTemperatureLimit_ByLightType(); //20140516-01
	int GetLDUsingTimeById(int idx) 
	{
		if (idx<MAX_LIGHT_SETTING)
		return m_ldLightUsingTime[idx];
		else
		return -1;
	};
	int GetLDErrorById(int idx) 
	{
		if (idx<MAX_LIGHT_SETTING)
		return m_ldLightError[idx];
		else 
		return -1;
	};

	void AskLightModelName();
	void ResetGroupLightValue() { OpGroupLightLevel(OP_GROUP_LIGHT_RESET); }; //eric chao 20161108
	void SendGroupLightValue() { OpGroupLightLevel(OP_GROUP_LIGHT_FINISH); }; //eric chao 20161108
//----Send Action Command
	int		sendTemperatureRequest(CString errmsg = _T("Ask device temperature fail..."));	
	// 1-1-3, ask light device using time
	int		sendUsingTimeRequest(CString errmsg = _T("Ask device using time fail..."));	// action command
	// 1-1-4, reset light device using time
	int		sendResetUsingTime(CString errmsg = _T("Reset device using time fail..."));	// action command
	// 1-1-5, ask light group number
	int		sendGroupLedRequest(CString errmsg = _T("Ask device group number...")); //eric chao 20161108
	// 1-1-6, ask light device fan status
	int		sendFanErrorRequest(CString errmsg = _T("Ask device fan status fail..."));	// action command
	// 1-1-7, ask light device general error
	int		sendGeneralErrorRequest(CString errmsg = _T("Ask device general error fail..."));	// action command
	// 1-1-8, ask light source led error
	int		sendLEDErrorRequest(CString errmsg = _T("Ask light source status fail..."));	// action command
	// 1-1-9, ask lighting level
	int		sendLightLevelRequest(CString errmsg = _T("Ask light level fail..."));	// action command
	// 1-1-10, ask light device version
	int		sendLightVersionRequest(CString errmsg = _T("Ask device version fail..."));	// action command
	// 1-1-11, ask light device serial number
	int		sendLightSerialRequest(CString errmsg = _T("Ask device serial number fail..."));	// action command
	// 1-1-12, ask light device temp limit
	int		sendLightTLimitRequest(CString errmsg = _T("Ask device temperature limit fail..."));	// action command
	// 1-1-13 ask Device Type
	int		sendInternalModelTypeRequest(CString errmsg=_T("Light source internal model type failed..."));
	// 5-1-1, ask device to turn into bootloader mode
	int		sendUpgradeRequest(CString errmsg = _T("Ask device to turn into bootloader fail..."));
	// 5-1-4, send finish upgrading command
	BOOL	sendFinishUpgrade(CString errmsg=_T("Send device restart fail..."));
	// 1-5-57
	BOOL    sendNewStartUpgrade(void);
	// 1-1-14
	BOOL    sendAskLightParam(void);
	// 1-1-16
	BOOL	sendMcuReset(void);
	// 1-5-58
	BOOL    sendNewFinishUpgrade(void);
//----Send Setup Command--------------------
	// 2-2-3, setup light device serial number
	int		sendSetupLightSerial(int serialNo, CString errmsg = _T("Setup device serial number fail..."));	// setup command
	// 2-2-4, setup light device sensor status
	int		sendSetupLightSensor(int status, CString errmsg = _T("Setup device sensor status fail..."));	// setup command	
	// 2-2-31, setup linked light device id
	int		sendSetupLDLightID(CString errmsg = _T("Setup linked device id number fail..."));	// setup command
	// 2-2-32, ask linked light device to turn into bootloader mode
	int		sendUpgradeLDRequest(int idx, CString errmsg = _T("Ask device to turn into bootloader fail..."));	// setup command
	// 2-2-33, change device ipaddress
	int		sendDeviceId(int NewDeviceID, CString errmsg = _T("Set device ipaddress fail..."));	// setup command
	// 5-2-1, setup machine to know the code length
	int		sendSetupCodeLength(int codeLength, CString errmsg = _T("Setup device code length fail..."));

	// 2-2-11,setup group led number //eric chao 20161107
	int		sendGroupLed(int nGroupNum, CString errmsg = _T("Set Group LED fail..."));
//----Send Mass Data Command-----------------------------------------
	// 3-1-6, ask light device model
	int		sendLightModelRequest(unsigned char *key, int keyNo, CString errmsg = _T("Ask device model fail..."));	// mass data command
	// 3-10
	int		sendSetupLightLevel(int lightNo, int *level, CString errmsg=_T("Setup device light level fail..."));
	// 3-1, setup light device model
	int		sendSetupLightModel(CString model, CString errmsg = _T("Setup device model fail..."));	// mass data command
	// 3-3, setup stop working temperature limits
	int		sendSetupTemperatureLimit(unsigned char *limitT, int no, CString errmsg = _T("Setup temperature limit fail..."));	// mass data command
// 5-3-1, send firmware package to device
	int		sendSetupFirmware(BYTE *firmwareData, int dataLength, CString errmsg = _T("Setup device firmware fail..."));
	// 3-12, setup Light Param
	int		sendSetupLightParam(BYTE *pLightParam,int ParamSize,CString errmsg = _T("Setup device firmware fail...")); //mass data command
	// 3-13, setup Group Light
	int		sendSetupGroupChannelMap(LIGHT_GROUP_MAP xMap,CString errmsg = _T("Setup device group map Fail")); //eric chao 20161214
//----Send MultiPurpose Command--------------------
	// 4-1 setup Image Info
	//int		sendSetupCamImageInfo(std::vector<CAMIMAGEINFO> *ciInfo);

//eric chao 20161104
	int		SetupGroupLightLevel(int nGroupId, int lightNo, int *level, CString errmsg = _T("Setup device group light level fail..."));
//----------------------

	virtual void QueryVerionInfo() ;
	char *GetModelName() {return (char *)m_cModelName;};
	int GetVersion() {return m_xDevInfo.nFirmwareVer;};
	int GetLightUsingTime() {return m_lightUsingTime;};
	void SetSessionKey(int iNo,unsigned char *key);

	int GetLightChannel() {return m_xLightParam.NoChannel;};
	int GetFanNumber() {return m_xLightParam.FanNum;};
	BYTE GetLightType() {return m_xLightParam.LightType;};
	BYTE GetLedGroupNum() { return m_xDevInfo.cGroupLedNum; }; //eric chao 20161104
	LIGHT_PARAM GetLightParam() {return m_xLightParam;};

	void ResetLightParamStatus(){m_bGetLightParam = FALSE;}; //seanchen 20151001
	BOOL GetLightParamCheck(){return m_bGetLightParam;}; //seanchen 20151001
	BOOL IsSupportGroupLed() { return m_xDevInfo.bSupportGroupLed;};
protected:
	virtual void ParseSetupCommand(SETUP_COMMAND_PKT *pPkt,int iSize);
	virtual void ParseActionCommand(ACTION_COMMAND_PKT *pPkt,int iSize);
	virtual void ParseEchoCommand(AOI_CTRL_HEADER *pHdr);
	virtual void ParseMassDataCommand(MASS_DATA_COMMAND_HEADER *pMassDataHdr,int iSize);
	virtual void ParseMultiPurposeCommand(MULTI_PURPOSE_COMMAND_HEADER *pMPDataHdr,int iSize);
	virtual CString GetControlName() {return _T("LightSourceController");};
	std::vector<int>	m_areaTemperatures;
	std::vector<int>	m_tempLimit;
	unsigned char m_UserTypeTempLimit[MAX_TEMPERATURE_LIMIT] ;	//20140516-01
	unsigned char m_DefaultTempLimit[MAX_LIGHTTYPE_CNT][MAX_TEMPERATURE_LIMIT] ; //20140516-01
	BOOL m_bUseDefaultLimit; //20140516-01
	int				m_lightUsingTime;
	std::vector<int>	m_tempLevel;
private:
	BOOL m_bGetLightParam;	//seanchen 20151001
	std::vector<int> m_xLightValue;
	LIGHT_PARAM m_xLightParam;
	LIGHT_DEVICE_INFO m_xDevInfo; //eric chao 20161104
	int	m_ldLightUsingTime[MAX_LIGHT_SETTING];
	int	m_ldLightError[MAX_LIGHT_SETTING];
	char m_cModelName[MAX_MODELNAME_SIZE];
	CByteArray m_cSessionKey;
	enum {
		OP_GROUP_LIGHT_RESET =0,
		OP_GROUP_LIGHT_FINISH,
	};
private:
	void Init();
	void OpGroupLightLevel(int nOpCode); //eric chao 20161108
	void GetDeviceInfoByLightParam(); //eric chao 20161107
	void GetLightInfoByModelType(WORD wType); //eric chao 20161216
};
