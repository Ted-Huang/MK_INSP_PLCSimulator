//#include "InspParamDef.h"
#pragma once

#define	_GoldImageFolder_			_T("GoldImage")
#define	_GoldImageName_				_T("GoldImage%02d.bmp")
#define	_GoldDecodeName_			_T("Decode%02d_%02d.dat")
#define	_InspSettingIni_			_T("InspSetting.ini")
#define	_IOSettingIni_				_T("IOSetting.ini")
#define	_GlobalParamIni_			_T("GlobalParam.ini")
#define	_DefaultGlobalParamIni_		_T("DefaultGlobalParam.ini")
#define	_UserInfoDat_				_T("UrUse.dat")
#define _HWCONFIG_FILE_				_T("HWConfig.AOILite")	


#define DEFAULT_ANCHORSIZE			128


#define MIN_INSP_CORRELATION		0
#define MAX_INSP_CORRELATION		100
#define DAFAULT_INSP_CORRELATION	70

#define MIN_DUPLI_CHECKCODE			0
#define MAX_DUPLI_CHECKCODE			100
#define DUPLI_CHECKCODE_VALUE		75

#define MIN_ENHANVE_LEVEL			-10
#define MAX_ENHANVE_LEVEL			5
#define ENHANVE_LEVEL_VALUE			0

#define MIN_INSPRET_SHIFT_VALUE		0
#define MAX_INSPRET_SHIFT_VALUE		100
#define INSPRET_SHIFT_VALUE			50

#define MIN_INSPRET_COLORDEEP_VALUE		0
#define MAX_INSPRET_COLORDEEP_VALUE		10
#define INSPRET_COLORDEEP_VALUE			0

#define DEFAULT_DECODE_W			640 
#define DEFAULT_DECODE_H			480
#define DECODE_SIZE_CHANGE_VAL		50

#define DEFAULT_IMAGE_W				1280
#define DEFAULT_IMAGE_H				960


#define DEFAULT_BLOCK_W				120
#define DEFAULT_BLOCK_H				 90
#define BLOCK_SIZE_CHANGE_VAL_W		 40
#define BLOCK_SIZE_CHANGE_VAL_H		 30

#define DEFAULT_FOV				97	//unit 0.001 mm
#define MIN_FOV					1	//unit 0.001 mm
#define MAX_FOV					200 //unit 0.001 mm

#define DEFAULT_KEEPIO_TIME		1000 //20160919 by jerry
#define CHECK_KEEPIO_TIME		16	

#define DEFAULT_XSHIT				5
#define DEFAULT_YSHIT				6	

#define _USE_INPUT_		2
#define _USE_OUTPUT_	2

#define _DEFAULT_IO_STATUS_		1
#define _TRIGGER_IO_STATUS_		0

#define _DEFAULT_GPIO_STATUS_	0
#define _TRIGGER_GPIO_STATUS_	1		


#define _IO_PLUS_TIME_			16  //ms
#define _TEST_PLUS_TIME_		100  //ms	//500 

#define _Keep_InspRet_Cnt_	150


#define _DEFAULT_COLOR_DISTANCE_	3.0

enum ENUM_MINMAX_IDX
{
	EUL_MIN_IDX = 0,
	EUL_MAX_IDX = 1,
	
	
	EUL_MINMAX_CNT ,
	
};

typedef enum EM_GPIO_PIN_
{
	GPIO_PIN_FIRST = 0,

	GPIO_PIN_RED = 0,
	GPIO_PIN_GREEN = 1,
	GPIO_PIN_STOPMACHINE = 2,


	GPIO_PINMAX,



}EM_GPIO_PIN;

typedef enum EM_INSPJOB_
{
	JOB_FIRST = 0,

	JOB_SHIFT = 0,
	JOB_MIXDEOCDE = 1,
	JOB_SHIFTDEOCDE = 2,
	JOB_COLORCHECK = 3,


	JOB_MAX,

	JOB_TMPEXTRA = 9000,
	JOB_UNKNOW = 9999,

}EM_INSPJOB;


typedef enum EM_INSPRET_TYPE_
{
	INSPRET_FIRST = 0,

	INSPRET_SHIFT = 0,
	INSPRET_DEOCDE = 1,
	INSPRET_COLORCHECK = 2,

	INSPRET_MAX,


}EM_INSPRET_TYPE;

typedef enum EM_DECODETYPE_
{
	DECODETYPE_FIRST = 0,

	DECODETYPE_QR = 0,
	DECODETYPE_DATAMATRIX = 1,
	DECODETYPE_BARCODE = 2,
	DECODETYPE_EAN13 = 3,

	DECODETYPE_MAX,

}EM_DECODETYPE;


typedef struct GEN_INSP_PARAM_{
	RECT rcSearchRange;
	unsigned int uCorrelation; //0-100
	unsigned int uMinCorrelation;
	unsigned int uMaxCorrelation;
	RECT rcDecodeRange;
	int iEnhanceLevel;// -10 ~ +5
	int iMinEnhanceLevel;
	int iMaxEnhanceLevel;
	int iSearchSize;
	//color
	int nColorCnt;
	RECT rcBlockRange;
	COLORREF clrInspColor;

	int nColorLower;
	int nColorUpper;
	float fColorDeep; //0.0-10.0 default 0

}GEN_INSP_PARAM;


typedef struct GEN_INSPRET_RPARAM_{
	int nMinShift;
	int nMaxShift;
	int nXShift;
	int nYShift;
	int nMinDupliChkCode;	
	int nMaxDupliChkCode;	
	int nDupliChkCode;		//0-100
}GEN_INSPRET_RPARAM;


typedef struct GEN_GLOBAL_PARAM_{
	int nLanguage;
	int nUICamMode;
	int nShowDefectView;
	int nIOKeepTime;
	BOOL bMergrResult;
	BOOL nStopMachineEn;
}GEN_GLOBAL_PARAM;


typedef struct GLOBAL_INSP_PARAM_{
	EM_INSPJOB emChInspJob;
	EM_DECODETYPE emDecodeType;
	int nTheFOV;			//unit 0.001 mm
	int nAnchorCnt;
}GLOBAL_INSP_PARAM;


