#pragma once
#ifdef HEXPARSER_EXPORTS
#define HEXPARSER_API __declspec(dllexport)
#else
#define HEXPARSER_API __declspec(dllimport)
#endif


// Type Define enumeration--------------------------------------------------------------------------

typedef enum {
	errNO_ERROR			=  0,
	errRECOARD_MARK		= -1,
	errREC_TYPE			= -2,
	errCHECKSUM			= -3,
	errPARSE			= -4,
	errOPEN_FILE		= -5,
	errUNKNOW_REC_TYPE	= -6,
	errEOF				= -7,	//end of file
} _HEX_ERR_CODE;

typedef enum {
	rtDATA,		//Data Record
	rtEOF,		//End of File Record
	rtESA,		//Extended Segment Address Record
	rtSSA,		//Start Segment Address Record
	rtELA,		//Extended Linear Address Record
	rtSLA,		//Start Linear Address Record
} _RECTPY;

typedef struct {
	UINT			REC_LEN;
	UINT			REC_LDOFFSET;
	_RECTPY			REC_TYPE;
	_HEX_ERR_CODE	ERR_CODE;

	UINT			ULBA;
	UINT			LastWordAddr;	//Word address
	UINT			FirstWordAddr;	//Word address
	UINT			PackageNumber;	//Total package number

	BYTE			BIN_DATA[200];
} _HEX_PROPERTY;

typedef enum {
	tgBOOTLOADER,	//Target to bootloader
	tgMAIN_PROG,	//Target to main program
} _TARGET;

typedef enum {
	cchSTX	= 0x0F,
	cchETX	= 0x04,
	cchDLE	= 0x05,
} _CONTROL_CHARACTERS;

typedef enum {
	cmdRD_VER		= 0x00,
	cmdRD_FLASH		= 0x01,
	cmdWT_FLASH		= 0x02,
	cmdER_FLASH		= 0x03,
	cmdRD_CONFIG	= 0x06,
	cmdWT_CONFIG	= 0x07,
} _CMD_DEF;

typedef enum {
	ftyMCP_DES,
	ftyMCP_HEX,
	ftyTI_DES,
	ftyTI_HEX,
	ftyAT91_DES,
	ftyAT91_HEX,
	ftyBIN_32bit_DES,
	ftyBIN_32bit_HEX,
	ftyAT91_UI_DES,
	ftyAT91_UI_HEX,
	ftySTM4_AOI_STC_001,
	ftySTM4_AOI_IO_002,
	ftySTM4_ID_VERIFICATOR,
	ftyPIC_MLL_15W,
	ftyPIC_MDL_140W,
	ftyPIC67J60_ADL,
	ftyPIC67J60_BSL,
	ftyPIC67J60_SLL,
	ftySTM4_AOI_RS2CAN,
	ftySTM4_IO_002_CAN,
	ftySTM4_ENC_CARD,
	ftyPIC67J60_SPL,
	ftyPIC67J60_MDL_SWITCH,
	ftyOTHER
} _FILE_TYPE;

#pragma pack(push, 1)
struct FW_header {
	char	modeltype[24];		
	UINT16	Version;
	UINT16	Subversion;
	UINT32	Checksum;			
	UINT32	FirmwareLen;	
	BYTE	dummy[28];
};
#pragma pack(pop)
//------------------------------------------------------------------------------

class HEXPARSER_API CHEXParser {
public:
	// Constructor
	CHEXParser(void);


// protected variables and member functions
protected:
	// flag to identify if user has successfully called the OpenFile function
	BOOL	m_isFileOpened;
	
	// internal counter for counting next package to be send
	UINT			m_pCounter;
	UINT			m_D20_Offset;
	UINT			m_D20FileSize;
	BYTE            dleBuffer[4096];
	BYTE			m_LineBuffer[300];
	BYTE			m_dataBuffer[1024 * 128][2];	// internal data buffer (256K bytes) for storing the binary data
	_HEX_PROPERTY	m_HEX_Property;
	UINT			m_BinaryDataSize;

	BOOL	procMCP_File (int);
	BOOL	procTI_File (int);
	BOOL    procARM_File(int);
	BOOL    procBIN_32bit_File (int fileSize);
	BOOL    procStM4_File (int fileSize);
	BOOL    openD20_File (char *filename,char* ModelName,BYTE ModelLen);
	int		PackingData(UINT, BYTE *, BYTE *, int);
	int     PackageNormalCommand(BYTE commandType, BYTE *commandBody, int dataLength,BYTE *dest);
	int	    checkNomalDLE(BYTE *source, int length);
	//_HEX_ERR_CODE	ParesLineHEX(void);
	//int PackingData(UINT wAddr, int wLen);


// public variables and member functions, call be accessed by outside objects
public:

	// open the specified hex file, return true if we successfully open the file
	BOOL	OpenFile(char* filename, _FILE_TYPE fileTpy);	

	// return the total package number
	int		GetAllPackageNumber(void);

	// return the current package number
	int		GetCurPackageNumber(void);

	// give user the pointer of data package that should be send, and return the data length of this package
	// return 0 if there is no data can be send
	int		GetNextDataPackage(BYTE **dataPtr);
	void	SetTarget(_TARGET target = tgMAIN_PROG);

	// reset the package counter
	void	ResetCounter(void);

	// get pure binary data generated from input file,
	// the binary data without any header.
	// return value indicate data size.
	int		GetBinaryDataPackage(BYTE **dataPtr);

};

// stand alone functions
//HEXPARSER_API int HEX_GetDataLength(void);


