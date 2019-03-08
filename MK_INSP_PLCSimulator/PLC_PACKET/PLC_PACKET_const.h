#pragma once

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



#pragma pack(pop) /* restore original alignment from stack */

typedef struct PLC_CMD_FIELD_BODY_{ //4 Byte
	BYTE cCh : 4;					//0==>ROLL,1==>OP,2==>SIDE,15==>ALL
	BYTE cOpCode : 4;				//0==>Query,1==>Set
	BYTE cField;					//FIELD_ID
	WORD wValue;
}PLC_CMD_FIELD_BODY;

enum FIELD_ID_{
	FIELD_INSP_RESULT = 1,		//�����쥻�� COM.VALUE
	FIELD_INSP_ERR,			//�����쥻�� COM.ERROR
	FIELD_INSP_TOGGLEBIT,		//�����쥻�� COM.TOGGLEBIT
	FIELD_INSP_MODE,			//�����쥻�� MODE.SOLL
	FIELD_INSP_VERIFY,			//�����쥻�� COM.VERIFY
	FIELD_CAM_ONLINE,			//�����쥻�� COM.ONLINE
	FIELD_CAM_DIR,			//�����쥻�� COM.DIRECTION
	FIELD_CAM_IMG_RECVBIT,	//�����쥻�� COM.IMGRECEIVEBIT
	FIELD_BAR_WIDTH,			//����	  MARKWIDTH
	FIELD_GOLDEN_RESET,		//�q���t�� Golden Reset
	FIELD_VERIFY_RESET,		//�q���t�� Verify Golden Image Reset
	FIELD_GOLDEN_READY,		//�q��PLC�� Golden Ready
	FIELD_VERIFY_READY,		//�q��PLC�� Verify Golden Ready
	FIELD_INSP_TRIGGER,		//�q���t�� �i���˴�
	FIELD_INSP_VERIFY2		//�q���t�� �i��2������
}FIELD_ID;
