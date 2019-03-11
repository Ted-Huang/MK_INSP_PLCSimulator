#pragma once

#define CMD_START	0x40 //@
#define CMD_END		0x23 //#

#define CMDTYPE_QUERYALIVE	0x01	// 1 ==> Query Alive Command
#define CMDTYPE_OP			0x02	// 2 ==> Field Op Command

#define CAMERA_ROLL		0x00
#define CAMERA_OP		0x01
#define CAMERA_SIDE		0x02
#define CAMERA_ALL		0x0F

#define OPCODE_QUERY	0x00
#define OPCODE_SET		0x01
#define OPCODE_ECHO		0x02

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */
typedef struct PLC_CMDEX_PACKET_{
	BYTE cStart;
	BYTE cCmdType;  //1 ==> Query Alive Command , 2 ==> Field Op Command
	BYTE cBody[4];
	BYTE cEnd;
}PLC_CMDEX_PACKET;

typedef struct PLC_CMDEX_ALIVE_BODY_{
	BYTE cTypeS;
	BYTE cValS;
	BYTE cTypeR;
	BYTE cValR;
}PLC_CMDEX_ALIVE_BODY;

typedef struct PLC_CMD_FIELD_BODY_{ //4 Byte
	BYTE cCh : 4;					//0==>ROLL,1==>OP,2==>SIDE,15==>ALL
	BYTE cOpCode : 4;				//0==>Query,1==>Set,2==>echo
	BYTE cField;					//FIELD_ID
	WORD wValue;
}PLC_CMD_FIELD_BODY;

#pragma pack(pop) /* restore original alignment from stack */



enum FIELD_ID_{
	FIELD_INSP_RESULT = 1,		//�����쥻�� COM.VALUE
	FIELD_VERIFY_RESULT,		//
	FIELD_INSP_ERR,				//�����쥻�� COM.ERROR
	FIELD_INSP_TOGGLEBIT,		//�����쥻�� COM.TOGGLEBIT
	FIELD_INSP_MODE,			//�����쥻�� MODE.SOLL
	FIELD_INSP_VERIFY,			//�����쥻�� COM.VERIFY
	FIELD_CAM_ONLINE,			//�����쥻�� COM.ONLINE
	FIELD_CAM_DIR,				//�����쥻�� COM.DIRECTION
	FIELD_CAM_IMG_RECVBIT,		//�����쥻�� COM.IMGRECEIVEBIT
	FIELD_BAR_WIDTH,			//����	  MARKWIDTH
	FIELD_GOLDEN_RESET,			//�q���t�� Golden Reset
	FIELD_VERIFY_RESET,			//�q���t�� Verify Golden Image Reset
	FIELD_GOLDEN_READY,			//�q��PLC�� Golden Ready
	FIELD_VERIFY_READY,			//�q��PLC�� Verify Golden Ready
	FIELD_INSP_TRIGGER,			//�q���t�� �i���˴�
	FIELD_INSP_VERIFY2			//�q���t�� �i��2������
};
