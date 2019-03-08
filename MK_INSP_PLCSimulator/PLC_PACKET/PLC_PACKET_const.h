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
	FIELD_INSP_RESULT = 1,		//對應原本的 COM.VALUE
	FIELD_INSP_ERR,			//對應原本的 COM.ERROR
	FIELD_INSP_TOGGLEBIT,		//對應原本的 COM.TOGGLEBIT
	FIELD_INSP_MODE,			//對應原本的 MODE.SOLL
	FIELD_INSP_VERIFY,			//對應原本的 COM.VERIFY
	FIELD_CAM_ONLINE,			//對應原本的 COM.ONLINE
	FIELD_CAM_DIR,			//對應原本的 COM.DIRECTION
	FIELD_CAM_IMG_RECVBIT,	//對應原本的 COM.IMGRECEIVEBIT
	FIELD_BAR_WIDTH,			//對應	  MARKWIDTH
	FIELD_GOLDEN_RESET,		//通知系統 Golden Reset
	FIELD_VERIFY_RESET,		//通知系統 Verify Golden Image Reset
	FIELD_GOLDEN_READY,		//通知PLC端 Golden Ready
	FIELD_VERIFY_READY,		//通知PLC端 Verify Golden Ready
	FIELD_INSP_TRIGGER,		//通知系統 進行檢測
	FIELD_INSP_VERIFY2		//通知系統 進行2次校驗
}FIELD_ID;
