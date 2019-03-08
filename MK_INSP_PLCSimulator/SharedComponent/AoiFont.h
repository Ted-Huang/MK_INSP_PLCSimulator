#pragma once
#include "afxwin.h"

typedef enum tagFontDef
{
	typeT1 = 0,
	typeT2,
	typeT3,
	typeT4,
	typeT5,
	typeT6,
	typeT7,
	typeT7V,
	typeT8,
	typeT9,
	typeT10,
	typeT11,
	typeT12,
	typeT13,
	typeT14,
	typeT15,
	typeTD,
}FontDef;


//segoeUI,·L³n¥¿¶Â,YuGothic
// Font Id
typedef enum tagFontId
{
	typeSegoeUI = 0,    //For English OS
	typeMicroSoftBlod,  //For BIG5,GB OS
	typeYuGothic,		//For JP OS
}FontId;
// Font styles
typedef enum tagEType
{
	typeNormal       = 0x00,
	typeBold         = 0x01,
	typeItalic       = 0x02,
	typeUnderline    = 0x04,
	typeDoubleHeight = 0x08,
} EType;
// Font Color
const int ctFontColor[] = { 
	0x00FFFFFF,
	0x005D5D5D,
	0x00494949,
	0x00808080,
	0x00FF6347,
	0x00000000
};

typedef struct UI_FONT_ITEM_{
	FontDef	nTypeId;
	FontId	nFontId;
	int		nFontSize;
	EType	nFontSubType;
	int		nFontColor;
	int		nAngle;
}UI_FONT_ITEM;

const UI_FONT_ITEM ctFont[] ={
	{ typeT1,	typeSegoeUI,		14,		typeNormal,		ctFontColor[1],		0	},		//T1
	{ typeT2,	typeSegoeUI,		30,		typeNormal,		ctFontColor[0],		0	},		//T2
	{ typeT3,	typeSegoeUI,		14,		typeBold,		ctFontColor[1],		0	},		//T3
	{ typeT4,	typeSegoeUI,		20,		typeBold,		ctFontColor[1],		0	},		//T4
	{ typeT5,	typeSegoeUI,		20,		typeNormal,		ctFontColor[0],		0	},		//T5
	{ typeT6,	typeSegoeUI,		14,		typeNormal,		ctFontColor[0],		0	},		//T6
	{ typeT7,	typeSegoeUI,		20,		typeNormal,		ctFontColor[2],		0	},		//T7
	{ typeT7V,	typeSegoeUI,		20,		typeNormal,		ctFontColor[2],		90	},		//T7V
	{ typeT8,	typeSegoeUI,		14,		typeNormal,		ctFontColor[3],		0	},		//T8
	{ typeT9,	typeSegoeUI,		20,		typeBold,		ctFontColor[0],		0	},		//T9
	{ typeT10,	typeSegoeUI,		50,		typeNormal,		ctFontColor[4],		0	},		//T10
	{ typeT11,	typeSegoeUI,		50,		typeNormal,		ctFontColor[1],		0	},		//T11
	{ typeT12,	typeSegoeUI,		10,		typeNormal,		ctFontColor[1],		0	},		//T12
	{ typeT13,	typeSegoeUI,		26,		typeNormal,		ctFontColor[1],		0	},		//T13
	{ typeT14, typeMicroSoftBlod,	12,		typeNormal,		ctFontColor[1],		0   },		//T14
	{ typeT15, typeSegoeUI,			30,		typeBold,		ctFontColor[1],		0   },		//T15
	{ typeTD, typeSegoeUI,			16,		typeNormal,		ctFontColor[1],		0	},		//TD
};
const int ctMaxFont = sizeof(ctFont)/sizeof(UI_FONT_ITEM);

class CLogFont : public LOGFONT
{
public:
	CLogFont()
	{
		memset(this, 0, sizeof(LOGFONT));        
	}
};
class CAoiFont : public CFont
{
public:
	CAoiFont(void) { m_xColor = ctFontColor[0];};
	~CAoiFont(void) {};
	void SetFontColor(COLORREF xColor) { m_xColor = xColor; };
	COLORREF GetFontColor() { return m_xColor;};
private:
	COLORREF m_xColor;
};

class CAoiFontManager
{
public:
	CAoiFontManager(void);
	~CAoiFontManager(void);

public:
	CFont* GetFont(FontDef xId);
	COLORREF GetFontColor(FontDef xId);
	void SetWindowFont(CWnd *pWnd,FontDef xId,BOOL bRedraw = TRUE);

private:
	void Init();
	void Finalize();
	void DestroyFont();
	void CreateFont();
	CAoiFont *GetFontType(FontDef);
private:
	bool m_bInit;
	CAoiFont m_fFont[ctMaxFont];
};

static CAoiFontManager g_AoiFont;