#pragma once

#define GLEW_STATIC
#include <gl/glew.h>
#include <FTGL/ftgl.h>	// Beagle 20120504 added.
#include <vector>
#include <map>
#include <SharedComponent.h>	// Beagle 20120920 added.
#include <MyTracker.h>		// added by eric at 20130118
#include <LineTracker.h>	// added by eric at 20130208

#include "OpenGLSegmentation.h"
#include "OpenGLRollingMode.h"
// 方向
enum	EMBOSS_DIRECTION
{
	EMBOSS_UP = 0,
	EMBOSS_DOWN = 1,
	EMBOSS_LEFT = 2,
	EMBOSS_RIGHT = 3,
	EMBOSS_LEFT_UP = 4,
	EMBOSS_LEFT_DOWN = 5,
	EMBOSS_RIGHT_UP = 6,
	EMBOSS_RIGHT_DOWN = 7,
	EMBOSS_ALL_DIRECTION = 8,			// 全方向
	EMBOSS_UP_DOWN = 9,			// 同時做上下 取交集
	EMBOSS_LEFT_RIGHT = 10,			// 同時做左右 取交集
	EMBOSS_LU_RD = 11,			// 左上 到 右下
	EMBOSS_LD_RU = 12,			// 左下 到 右上
	EMBOSS_MIDDLE = 13,			// 中間
	EMBOSS_DIRECTION_MAX = 14
};
#define SHOW_INSPECTION_MARK
#define MAX_SPLIT_WINDOW		128		// Beagle 20111101
#define MAX_DEFECTS				16
#define MINIMAP_SHRINK			0.2f
#define MAX_GL_BARCODE_DIGITS	23

#define WM_OPENGL_MSG			WM_APP+1000		// message identifier of OpenGL window
#define	IDM_OPENGL_LBUTTONDOWN	WM_APP+1001
#define IDT_GL_TIMER			WM_APP+1002
#define	IDM_OPENGL_ANCHOR		WM_APP+1003
#define IDM_OPENGL_RECTANGLE	WM_APP+1004
#define	IDM_OPENGL_CIRCLE		WM_APP+1005
#define	IDM_OPENGL_ELLIPSE		WM_APP+1006
#define	IDM_OPENGL_POLYGON		WM_APP+1007		// added by danny
#define IDM_OPENGL_DOWN_ERASE	WM_APP+1008		// added by danny
#define IDM_OPENGL_MOVE_ERASE	WM_APP+1009		// added by danny
#define IDM_OPENGL_UP_ERASE		WM_APP+1010		// added by danny
#define IDM_OPENGL_MOUSEMOVE	WM_APP+1011
#define IDM_OPENGL_ANCHOR_REOPS	WM_APP+1012		// added by danny at 20110218
#define IDM_OPENGL_ANCHOR_WEAKEN	WM_APP+1013		// added by danny at 20110309
#define	IDM_OPENGL_SHOWRGB		WM_APP+1014		// added by Beagle at 20110704
#define IDM_OPENGL_ANCHOR_BARCODE	WM_APP+1015		// added by eric at 20110801
#define IDM_OPENGL_DOWN_ADD		WM_APP+1016		// added by eric at 20110929
#define IDM_OPENGL_MOVE_ADD		WM_APP+1017		// added by eric at 20110929
#define IDM_OPENGL_UP_ADD		WM_APP+1018		// added by eric at 20120330
#define IDM_OPENGL_DOWN_SEL		WM_APP+1019		// added by eric at 20120330
#define IDM_OPENGL_UP_SEL		WM_APP+1020		// added by eric at 20120330
#define IDM_OPENGL_HIDE_REGION	WM_APP+1021		// added by eric at 20120330
#define	IDM_OPENGL_LBUTTONUP	WM_APP+1022		// added by eric at 20120803
#define IDM_OPENGL_ANCHOR_DISTANCE	WM_APP+1023		// added by eric at 20120910
#define IDM_OPENGL_LINE			WM_APP+1024		// added by eric at 20121121
#define IDM_OPENGL_MOUSEWHEEL	WM_APP+1025		// Beagle 20121217 added.
#define IDM_OPENGL_LAYOUT_INDEX	WM_APP+1026		// added by eric at 20130107
#define IDM_OPENGL_LAYOUT_REOPS	WM_APP+1027		// added by eric at 20130107
#define IDM_OPENGL_RBUTTONUP	WM_APP+1028		// added by eric at 20130107
#define IDM_OPENGL_SHAPE_ADD	WM_APP+1029		// added by eric at 20130208
#define IDM_OPENGL_SHAPE_SELECT	WM_APP+1030		// added by eric at 20130208
#define IDM_OPENGL_SHAPE_MODIFY	WM_APP+1031		// added by eric at 20130208
#define IDM_OPENGL_RADIO_INDEX	WM_APP+1032		// added by eric at 20170411
#define	IDM_OPENGL_LBUTTON_DBCLK 	WM_APP+1033
#define IDM_OPENGL_LINE_MOVE    WM_APP+1034

// modified by danny at 20101228, 20130117
// #ifndef MAX_ANCHORS
// #define	MAX_ANCHORS	20
// #endif

#ifndef MAX_RECTS
#define MAX_RECTS		100		// modified by danny at 20100324
#endif

#ifndef MAX_POLY_POINTS
#define	MAX_POLY_POINTS	60		// modified by danny at 20100324
#endif

// Text string alignment -- Beagle 20120703 added.
#define ALIGN_LEFT		0x0001
#define ALIGN_RIGHT		0x0002
#define ALIGN_HCENTER	0x0003
#define ALIGN_TOP		0x0010
#define ALIGN_BOTTOM	0x0020
#define ALIGN_VCENTER	0x0030
#define	ALIGN_WINDOW	0x0100
#define	ALIGN_SHEET		0x0200
#define ALIGN_SPLIT_MASK 0xF000 //eric chao 20131122
#define ALIGN_SPLIT(n)  ((n)<<12)
#define ALIGN_FRAGMENT	0x0300

// Beagle 20120710 added.
#define GRID_STYLE_NOCHANGE		0
#define GRID_STYLE_RECTANGLE	1
#define GRID_STYLE_BILLIARD		2

// Beagle 20130305 added.
#define REGION_SEGMENT_ENABLE_RGB332	(DWORD)(0xFFFFFFFF)
#define REGION_DFSHAPE_ENABLE_RGB332	(DWORD)(0xFFFFFFFF)

// Beagle 20130816
#define LUMINANCE_NONE			0x00
#define LUMINANCE_HORIZONTAL	0x01
#define LUMINANCE_VERTICAL		0x02
#define LUMINANCE_MODE_MASK		0x0F
#define LUMINANCE_GRIDLINE		0x10

// Beagle 20140711
#define DISPLAY_MODE_NORMAL			0
#define DISPLAY_MODE_DEFECT			1
#define DISPLAY_MODE_DEFECT_PIECE	2
#define DISPLAY_MODE_DEFECT_STACK	3

#define OGLSEGMENT 0

enum MatrixFlipMode
{
    FLIP_NONE,
    FLIP_HORIZON,
    FLIP_VERTICAL,
    FLIP_H_AND_V,
};

// added by eric at 20130219
enum TrackerHit {
	hitNothing = -1,
	hitStart,
	hitEnd,
	hitMiddle
};

enum LMKeyMode
{
	LM_IGNORE,			// ignore mouse left button
	LM_TRANSLATE,		// use left button to do translation
	LM_SCALE,			// use left button to scale
	LM_SET_ANCHOR,		// use left button to set anchor
	LM_DRAW_RECTANGLE,	// use left button to draw rectangle
	LM_DRAW_CIRCLE,		// use left button to draw circle
	LM_DRAW_ELLIPSE,	// use left button to draw ellipse
	LM_DRAW_POLYGON,	// use left button to draw polygon
	LM_DRAW_RGBCURVE_X,	// use left button to draw rgb curve
	LM_DRAW_RGBCURVE_Y,	// use left button to draw rgb curve
	LM_DRAW_ERASER,		// use left button to erase the inspection region

	// added by danny at 20110309
	LM_SET_WEAKENANCHOR,

	LM_CHECK_DEFECT,		// check the defect image

	LM_SET_BARCODEANCHOR,	// added by eric at 20110801

	LM_SET_WEAKENPAINT,		// added by eric at 20110929

	LM_SET_DISTANCEANCHOR,	// added by eric at 20120910
	LM_DRAW_LINE,			// added by eric at 20121121
	LM_DRAW_LAYOUTSHAPE,	// added by eric at 20130107
	LM_DRAW_SHADOWRECT,		// added by eric at 20130208
	LM_MODIFY_SHADOWRECT,	// added by eric at 20130208
	LM_MODIFY_LINE,			// added by eric at 20130208
	LM_SET_COLORTOOL,		// added by eric at 20130429
	LM_SET_COLORDERIVATE,	// added by eric at 20130923
	LM_SET_NORESIZMOVING,	// added by eric at 20150527
};

enum oglShape
{
	SHAPE_NONE,
	SHAPE_RECT,
	SHAPE_CIRCLE,
	SHAPE_ELLIPSE,
	SHAPE_POLYGON,
	SHAPE_ANCHOR_RECT,		//seanchen 20130820
};

// added by eric at 20110929, 20120330
enum oglPaintMode
{
	PAINT_NONE,
	PAINT_RECT,
	PAINT_BRUSH,
	PAINT_PICK,
	PAINT_FILL,
	PAINT_PAINTBRUSH,
};

// Beagle 20120507 added.
typedef struct _GLPOINT {
	float	x;
	float	y;
} GLPOINT;

// Beagle 20120507 added.
typedef struct _GLRECT {
	int left;
	int top;
	int right;
	int bottom;
} GLRECT;

// *FIXME* 等我有空時再寫說明
// Beagle 20140214
typedef struct _GL_PRINTF_POSITION {
	int	split_id;	// 大分割畫面

	int	align;	// 對齊方式
	int x;
	int	y;

	// texture 之中的小分割
	int	frag_idx;
	int	frag_total;

	// texture 之中的小分割, 和上面二選一即可
	int	frag_wsize;
	int	frag_widx;
	int	frag_hsize;
	int	frag_hidx;
} GLPRINTF_POSITION;

// Beagle 20120507 added.
typedef struct _DEFECT_REGION {
	int			index;
	int			client_no;
	int			type;
	int			alignType;
	float		rot;
	GLRECT		region;
	GLPOINT		shift;
	GLPOINT		center;
	COLORREF	color;
	wchar_t		description[64];	// modified by eric at 20120717
} DEFECT_REGION;

// rectH: 缺陷線框寬度
// rectW: 缺陷線框高度
// 若線框太小，長寬皆小於10.0，則改成顯示綠色圓圈。
// Beagle 20130621 modified.
typedef struct _DEFECT_PIECE {
	int		index;
	int		client_no;
	IMAGE	image;
	IMAGE	shade;
	float	rectW;
	float	rectH;
	wchar_t	description[64];	// modified by eric at 20120717
	COLORREF color;
	float	scale;
	__int64	info;	// Beagle 20140718 added.
} DEFECT_PIECE;

struct _CROSSHAIR_POINT {	// Beagle 20130118 added.
	float x;
	float y;
	COLORREF color;
};

typedef struct _CROSSHAIR_LENGTH	{	
	float fXDis;
	float fYDis;
	float fLDis;
} CROSSHAIR_LENGTH;

struct _CROSSHAIR_PAIR {	// Beagle 20130118 added.
	int	p1, p2;
};

typedef struct _SHADOWINFO {	// added by eric at 20130208
	int direction;	// added by eric at 20130219
	RECT shadowShape;
	RECT lineShape;
	RECT lineShape2;
} SHADOWINFO;

class COpenGLControl : public CWnd
{
public:

private:
	// Window information
    BOOL    m_bNoColorCubeBackground;
	CWnd	*hWnd;
	HDC		hdc;
	HGLRC	hrc;
	HGLRC	hrc_loader;
	int		m_nPixelFormat;
	COLORREF m_glBackColor;
	COLORREF m_glExtColor;
	HCURSOR	m_glCursor;
	// Image sample texture for split window.
	unsigned int m_image_texture[MAX_SPLIT_WINDOW];	// Beagle 20111104
	// Transparent region layer for split window.
	unsigned int m_region_texture[MAX_SPLIT_WINDOW];	// Beagle 20130822
	// font, inspection region, ok, ok_mask, fail, fail_mask, ok_text, ok_text_mask, ng_text, ng_text_mask, region segments
	// 0: Unused. Captured images moved to m_image_texture[].
	// 1: Unused. Alphabet and number font now using TrueType font. -- Beagle 20120504
	// 2: Unused. Inspection region moved to m_region_texture[]. -- Beagle 20130822
	// 3: OK sign
	// 4: OK mask
	// 5: Fail sign
	// 6: Fail mask
	// 7: OK text
	// 8: OK text mask
	// 9: NG text
	// 10: NG text mask
	// 11: region segment									//move to m_region_texture
	// 12: Texture buffer for CUDA.							//Unused
	// 13: Texture buffer for grid texture					//Unused
	// 14: Texture buffer for golden sample grid texture	//Unused
	// 15: Texture buffer for defect shpae texture	
	unsigned int m_texture[23];
	// 0: Texture buffer for CUDA.
	unsigned int m_texBuffer[2];							

	// Textures of 0-9 digits for barcode. Beagle 20120301 added.
	unsigned int m_texBarcodeDigits[10];
	BOOL	m_showBarcodeDigitsImage;
	int		m_width_BarcodeDigits;
	int		m_height_BarcodeDigits;
	DWORD   m_dThreadId; //20170906
	struct {
		int		n;
		float	x;
		float	y;
	} m_pos_BarcodeDigits[MAX_GL_BARCODE_DIGITS];

	BOOL	m_minimap;

	BOOL m_LeftButtonDown;
	CPoint m_LeftDownPos;
	CPoint m_LastLeftDownPos;
	BOOL m_RightButtonDown;
	CPoint m_RightDownPos;
	CPoint m_ScaleCenter;	// Beagle 20130103 modified.

	GLfloat m_xTranslate;
	GLfloat m_yTranslate;
	GLfloat m_glScale;

	struct {	// Beagle 20130816
		BOOL type;
		unsigned char *dataLuminance;
		unsigned char *dataRed;
		unsigned char *dataGreen;
		unsigned char *dataBlue;
		int len;
		int size;
		int x;
		int y;
	} m_xLuminance;

	int m_glwidth;
	int	m_glheight;

	// Split Window
	int	m_wsplit;					// Beagle 20111101 modified.
	int m_hsplit;					// Beagle 20111101 modified.
	int m_width[MAX_SPLIT_WINDOW];	// Beagle 20111101 modified.
	int m_height[MAX_SPLIT_WINDOW];	// Beagle 20111101 modified.
	int m_data_w[MAX_SPLIT_WINDOW];	// Beagle 20120925 added.
	int	m_data_h[MAX_SPLIT_WINDOW];	// Beagle 20120925 added.
	int m_textype[MAX_SPLIT_WINDOW];	// Beagle 20130410 added.
	bool m_splitEnable[MAX_SPLIT_WINDOW];	// Beagle 20140214 added.

	GLfloat m_norm_scale[MAX_SPLIT_WINDOW];

	double m_projection[MAX_SPLIT_WINDOW][16];
	double m_modelview[MAX_SPLIT_WINDOW][16];

    OpenGLSegmentation m_OGLSegment[ MAX_SPLIT_WINDOW ];
    OpenGLRollingMode  m_OGLRolling[ MAX_SPLIT_WINDOW ];

	LMKeyMode	m_lmMode;	//mouse left button mode

	// added by eric at 20150427
	int m_AnchorWidth;
	int m_nAnchorHeight;

	int m_barcodewidth;	// added by eric at 20110801
	int m_barcodeheight;	// added by eric at 20110801
	int m_Numberbarcode;	// added by eric at 20110801
	BOOL m_isShowBarcode;	// added by eric at 20110801
	BOOL m_isShowOnlyBarcode;	// added by eric at 20111024
	BOOL m_showOCRSize;	// added by eric at 20110809

	LONGLONG m_largenumber;

	oglPaintMode m_PaintMode;// added by eric at 20110929
	int m_nWeakenColorBlock;// added by eric at 20110929
	CRect m_rcWeakenColor;	// added by eric at 20110929

	POINT m_Barcodelefttop;			// added by eric at 20111024
	POINT m_Barcoderightbottom;		// added by eric at 20111024
	BOOL m_bShowRGB;	// added by eric at 20120223

	BOOL m_bShowAnchorPoints;	// added by eric at 20120515
	BOOL m_bEnableShapModify;	// added by eric at 20120702
	// double clicked event
	UINT	dcMsgId;
	WPARAM	dcWpara;
	LPARAM	dcLpara;

	// Mouse coordinate on camera image.
	int	m_mouse_x;
	int m_mouse_y;
	int m_mouse_sp;	// Beagle 20130424 -- 滑鼠所在的分割視窗.

	// Keep minimum redraw interval.
	__int64	m_draw_lasttime;	// Beagle 20120510 added.
	__int64	m_draw_interval;	// Beagle 20120510 added.
	BOOL	m_need_redraw;		// Beagle 20120510 added.
	BOOL	m_enable_redraw_interval;	// Beagle 20130621 added.

	// Sprites
	//	Anchor
	int m_anchors;
	int m_anchor_x[MAX_ANCHORS];
	int m_anchor_y[MAX_ANCHORS];
	int m_selAnchor;	// current selected anchor, added by danny
	int m_selAnchorRange;	//seanchen 20130819

	// added by danny at 20110218
	std::vector<POINT> m_reposAnchorPoint;
	int m_selReposAnchor;
	int m_selReposAnchorRange; //seanchen 20130819

	struct _CROSSHAIR {	// Beagle 20130117 added.
		int num;
		int size;
		struct _CROSSHAIR_POINT *p;
		struct _CROSSHAIR_LENGTH *pDistance; //seanchen 20130826-3
		int pair_num;
		int pair_size;
		struct _CROSSHAIR_PAIR	*pair;
	} m_crosshair[MAX_SPLIT_WINDOW];

    struct RECTINFO
    {
        int      rects;
        oglShape rect_type[ MAX_RECTS ];
	    RECT     rect[ MAX_RECTS ];
        COLORREF rect_color[MAX_RECTS];

        RECTINFO() { memset( this, NULL, sizeof( RECTINFO ) ); }
    };
    std::map< int, RECTINFO > m_mapRectInfo;

	BOOL m_showCrossHair;

	//	Rectangle, Circle, Ellipse
	//int m_rects;
	//oglShape m_rect_type[MAX_RECTS];
	//RECT m_rect[MAX_RECTS];

	// 
	BOOL m_bDrawUserExtLY;
	std::vector<RECT>	m_rcUserLayout[MAX_SPLIT_WINDOW]; //seanchen 20151211
	int m_nFocusSplitId;	// added by eric at 20161007
	RECT m_rtFocusUserLayoutRectangle;	// added by eric at 20161007

	// added by amike at 20120907
	//COLORREF m_rect_color[MAX_RECTS];
	BOOL m_show_rect_color;
	//	Polygon
	BOOL m_poly_drawing;
	int m_polygons;
	int m_poly_points[MAX_RECTS];
	CPoint m_polygon[MAX_RECTS][MAX_POLY_POINTS];

	BOOL m_bAnchorRect;	// added by eric at 20150420
	RECT m_rtAnchorRect;	// added by eric at 20150420

	// added by eric at 20121008
	int m_rectsPattern;
	RECT m_rectPattern[200];
	BOOL m_isShowPatternRect;
	BOOL m_isPatternMode;
	// added by eric at 20121017
	BOOL m_isShowPatternSegment;
	CRect m_rtSegment;
	//////////////////////////////////////////////////////////////////////////
	// added by eric at 20121121
	BOOL m_bSetHoriOrVertLine;
	RECT m_lineRect[MAX_RECTS];	// Veritcal 垂直
	int m_lineRects;
	BOOL m_bShowLineShape;
	int m_selLine;

	// added by eric at 20130107
	RECT m_layoutRect[MAX_RECTS];
	int m_layoutRects;
	BOOL m_bShowLayout;
	int m_selLayoutRect;
	int m_XPos;
	int m_YPos;
	std::vector<CMyTracker> m_vRectTrackerList;	// modified by eric at 20130118
	std::vector<int> m_vSelectionList;

	// added by eric at 20130208
	BOOL m_bShowShadowRectShape;
	std::vector<SHADOWINFO> m_vShadowInfo;
	POINT m_ptShadowShape;
	int m_shadowType;
	CLineTracker *m_pLineTracker;
	CLineList m_LineList;
	int m_nBlockDirection;	// added by eric at 20130219
	//////////////////////////////////////////////////////////////////////////

	//int m_selRectangle;
	//int m_selPolygon;
	std::vector<int> m_selRectangle;
	std::vector<int> m_selPolygon;

	BOOL m_bShiftMode;	// added by eric at 20121212
	CRect m_rtShiftShape;	// added by eric at 20121212

	// Defect Region.
	int		m_display_mode;	// Beagle 20140711
	int		m_num_defect;
	int		m_next_defect;	// Beagle 20140711
	bool	m_bShowDefectLabel;	// Beagle 20140711
	DEFECT_REGION	m_defectRegion[MAX_DEFECTS];	// Beagle 20120508 added.
	BOOL	m_show_inspection_text;
	BOOL	m_curr_sample_result;
	CPoint	m_oldPoint;		// for rgb curve mouse move
	BOOL	m_show_cursor_outline;		// Beagle 20130424 added.

	// Defect Region -- Images in pieces.
	unsigned int	m_piece_texture[MAX_DEFECTS];
	unsigned int	m_piece_tex_shade[MAX_DEFECTS];	// Beagle 20130403 added.
	DEFECT_PIECE	m_defectPiece[MAX_DEFECTS];	// Beagle 20120508 added.
	int m_stackInspIndex[MAX_DEFECTS];		//seanchen 20150421-01

	// Inspected Region, added by danny
	BOOL	m_showInspectionRgn;
	BOOL	m_bShowMatrixLine;
	BOOL	m_showRegionSegments;
	BOOL	m_showRegionDFShape;
	BOOL	m_isShowSegment;
	RECT	m_rcMatrix;
    MatrixFlipMode m_eMatrixFlipMode;

	COLORREF	m_RegionSegmentColor;	// Beagle 20130305 added.
	COLORREF	m_RegionDFShapeColor;	

	// Beagle 20120504 added.
	FTGLTextureFont	*m_font;		// Variable-width font.
	FTGLTextureFont	*m_fontFixed;	// Fixed-width font.
	struct text_string {	// Beagle 20120703 added.
		int align;
		float x;
		float y;
		wchar_t str[128];
	} m_textstring[64];
	int	m_num_textstring;	// Beagle 20120703 added.

	struct cut_here_line {	// Beagle 20120817 added.
		BOOL orientation;
		int	start;
		int	period;
	} m_cutHereLine;

	struct unit_divide_line {
		BOOL orientation;
		std::vector<int> vUnitline;
	}m_xUnitDivide; //eric chao 20160527

	char	m_barcode_result[MAX_GL_BARCODE_DIGITS+1];	// Beagle 20120224 added.
	char	m_ocr_result[MAX_GL_BARCODE_DIGITS+1];		// Beagle 20120224 added.
	int		m_barcode_position;							// Beagle 20120326 added.

	BOOL	m_isOnline;

	// added by danny for defect learning
	BOOL	m_showGrid;
	BOOL	m_showGridTexture;
	BOOL	m_showGoldDiffTexture;
	int		m_gridSize;
	BOOL	m_showLearnRegion;
	std::vector<CRect>	m_learnRegion;

	//added by leo for QCtool
	BOOL	m_showChessboard;
	std::vector<CPoint> m_pointList;
	std::vector<CPoint> m_dispointList;
	std::vector<CPoint> m_avgdisptrList;

	// Inspection mark for debugging and machine integration.
#ifdef	SHOW_INSPECTION_MARK
#define	MAX_INSPECTION_MARK	10
	BOOL	m_show_inspection_mark;
	BOOL	m_inspection[MAX_INSPECTION_MARK];
	int		m_curr_mark;
	BOOL	m_split_inspection_mark[MAX_SPLIT_WINDOW];
#endif /* SHOW_INSPECTION_MARK */

	// Beagle 20120704 added.
	int		m_FrameNumber;
	int		m_FrameGrid[2];			// X, Y
	float	m_FrameColor[MAX_SPLIT_WINDOW][4];	// BOOL, R, G, B
	BOOL	m_showFrameIndex;		// Beagle 20120705
	int		m_FrameGridStyle;		// Beagle 20120710

	// danny's triangular pyramid
	//GLfloat	m_dannyXRot;

	BOOL	m_showDefectShift;	// Beagle 20120630

	BOOL	m_bDrawTriangle;	// added by eric at 20120910
	BOOL	m_bDrawPatternShape;	// added by eric at 20121008
	CRect	m_rtPatternShape;	// added by eric at 20121008

	BOOL m_bShowAngleString;	// added by eric at 20130128
	// added by eric at 20161217
	BOOL m_bShowLineString;
	CString m_tLineLabel;
	int m_nShootFormatDist;
	int m_nEnableMap;
	CString m_tDisable;
	BOOL m_bCrossOverLineOrder;
	//////////////////////////////////////////////////////////////////////////

	CString m_AngleString;	// added by eric at 20130123
	float m_AngleX;	// added by amike at 20130123
	float m_AngleY;	// added by amike at 20130123
	int m_AngleAlignType;

	int	m_nRadioButton;		// Beagle 20130807
	int m_nRadioSelected;	// Beagle 20130807

	BOOL m_bEnableColorDerivateBox;	// added by eric at 20130923
	std::vector<CRect> m_vColorDerivateBox;	// added by eric at 20130923

	std::vector<CPoint> m_triangleptrList;	// added by eric at 20120910
	std::vector<std::vector<CPoint>> m_multiTrianglePtrList;	// added by eric at 20130107
	std::vector<CROSSHAIR_LENGTH> m_multiTriangleDisList;	// seanchen 20130827-6

	BOOL m_bShowSpecialRectBox;	// added by eric at 20150211
	std::vector<CRect> m_vSpecialRectBox;	// added by eric at 20150211


	void oglSetNormalTranslate(void);
	void oglDrawLargeNumber(void);
	void oglDrawNumbers(float x, float y, int number, float wmax=20.0f, float hmax=20.0f);
	void oglDrawMinimap(void);
	void DrawDefect(void);		// Beagle 20140711
	void DrawDefectPiece(void);	// Beagle 20140711
	void DrawDefectStack(void);	// Beagle 20140711
	void DrawSingleDefectPiece(int split_x, int split_y, int split_id);	// Beagle 20140711
	void DrawDefectLabel(int client_no, COLORREF color, wchar_t *description);	// Beagle 20140711
	void oglDrawInspectionMark(void);
	void oglDrawTextString(void);
	void DrawSplit(int split_x, int split_y);		// Beagle 20111101 modified
	void oglDrawSplitMark(int split_x, int split_y);	// Beagle 20111101 modified
	void oglDrawStackInspIndex(int nHistogram,int nXIndex = 0,int nYIndex = 0);				//seanchen 20150421-01
	void oglDrawBarcodeResult(void);		// Beagle 20120224 added.
	void oglDrawBarcodeDigits(void);		// Beagle 20120301 added.
public:
	void oglDrawBilliardBall(COLORREF color, int number);	// Beagle 20120424 added.
private:
    void DrawMaxtrixLine();
	void DrawCircle(float radius);						// Beagle 20130807 modified.
	void DrawScene(BOOL isPaint = FALSE);				// Beagle 20120510 added.
	void PostDrawScene(BOOL isPaint = FALSE);			// Beagle 20130423 added.
	void DrawFrame(int split_id);						// Beagle 20120704 added.
	void AddIndexText(void);							// Beagle 20120705 added.
	void DrawCrosshair(int s);							// Beagle 20130117 added.
	void ExtendCrosshairPoints(int split, int space=1);	// Beagle 20130118 added.
	void ExtendCrosshairPairs(int split, int space=1);	// Beagle 20130118 added.
	void DrawSingleString(int split_id, int align, float x, float y, const wchar_t *str);	// Beagle 20130121 added.
	void DrawPrintf(int split_id, int align, float x, float y, const wchar_t *fmt, ...);	// Beagle 20130121 added.
	void SetTexture(IMAGE *TexImg, unsigned int texture, int flag=0);	// Beagle 20130305 added.
	BOOL UpdateTexture(IMAGE *TexImg, unsigned int texture, int xoffset, int yoffset);	// Beagle 20130409 added.
	void DrawCursorOutline(int split_id);			// Beagle 20130424 added.
	void DrawRadioButton(void);						// Beagle 20130807
	void DrawRing(float radius1, float radius2);	// Beagle 20130807
	void MousePick(int x, int y);					// Beagle 20130807
	void DrawLuminanceGraph(int split_id);			// Beagle 20130814
	void DrawCurve(int split_id, bool isHorizontal,
		const unsigned char *data, int len);		// Beagle 20130815
	void DrawGrid(bool isHorizontal);				// Beagle 20130815
	void DrawDashedLine(bool isHorizontal, float position);	// Beagle 20130815
	void TransformImage2Window(int split_id, float ix, float iy,
		float &wx, float &wy);	// Beagle 20130816

	void oglGetCenteredRect(RECT &rect, int iCenterX, int iCenterY, int iWidth, int iHeight);	// Beagle 20130425 rewrite.
	void DrawUserLayoutRectangle(int split_id);	//seanchen 20151211

	// added by eric at 20160819
	BOOL IsHitLineRect(CPoint pt, int &idx);
	RECT m_rtOldRect;
	//////////////////////////////////////////////////////////////////////////

	// added by danny at 20091019
	int m_oglEraserX;
	int m_oglEraserY;
	int m_oglEraserRadius;
	void ComputeEraserSize(int mx, int my, int mRadius = 16);	// modified by eric at 20120330

	////////////////// added by eric at 20120330 /////////////////////////
	int m_regions;
	std::vector<CRgn *> m_region;
	int m_inspectRegions;
	std::vector<CRgn *> m_inspectRegion;
	std::vector<int> m_selRegion;
	BOOL m_bLMouseDown;
	CPoint m_ptDown;
	CPoint m_ptMoveOffset;
	//CRectTracker* m_tracker;
	CMyTracker *m_tracker;	// modified by eric at 20130118
	BOOL m_bTracking;
	int m_nSelectedIndex;
	CRect m_rtSelected;
	int m_nHitHandle;
	BoundedType m_nBoundedType;
	BOOL m_bPolygonPointHit;
	int m_nPolygonHitIndex;
	BOOL m_bHideSelectedRegion;

	unsigned char *m_pMovingRgn;
	int m_nMovingRgnCntBuf;
	void OnMouseDownShape(CPoint point);
	void OnMouseMoveShape(CPoint point);
	void OnMouseUpShape(CPoint point);
	void OnMoveResizing(int nHandle, CPoint ptMouse, CPoint ptOffset);	// modified by eric at 20120521
	void OnPolygonResizing(CPoint ptMouse, CPoint ptOffset);
	BOOL OnRegionMoving(CPoint ptOffset);
	int GetSelectedShapeIndex(CPoint point, int TexNum=0);
	void SetBoundedTracker();
	//////////////////////////////////////////////////////////////////////////
	
	// added by eric at 20130208
	void OnUpdateShadowLine();
	void OnGet45DegreeRectagleCenterLine(CRect &rt, CRect rtShape, BOOL bClockwise);

	BOOL m_ShowOCRInspArea;	// added by eric at 20120801
	CRect m_rtOCR;	// added by eric at 20120801
	BOOL m_bShowOKNGText;	// added by eric at 20120821
	BOOL m_bShowOKText;	// added by eric at 20120821

	float m_XYRatio; // added by amike at 20121206

	std::vector<CString> m_vDistanceStrings;	// added by eric at 20130311
	CString m_strDistanceUnitString;//seanchen 20130821
	double m_fDistance_XRatio;		//seanchen 20130821
	double m_fDistance_YRatio;		//seanchen 20130821

	std::vector<LMKeyMode> m_vSingleClickEvent;	// added by eric at 20130827

    std::vector< CString > m_vecMatrixNormalSequence;
    std::vector< CString > m_vecMatrixHorizonSequence;
    std::vector< CString > m_vecMatrixVerticalSequence;
    std::vector< CString > m_vecMatrixH_and_VSequence;

public:
	COpenGLControl(void);
	virtual ~COpenGLControl(void);

	// added by danny at 20110218
	void oglClearReposAllAnchor();
	void oglDeleteReposAnchor(int x, int y);
	void oglAddReposAnchor(int x, int y);
	void oglGetReposAnchor(int num, int &x, int &y);
	void oglSetReposAnchorMark(int x, int y);
	void oglSetReposAnchorRangeMark(int nRectIdx); //seanchen 20130819
	int	 oglGetNumReposAnchors(void) { return (int)m_reposAnchorPoint.size(); };
	//---------------------------


	void oglCreate(CRect rect, CWnd *parent, UINT id);
	void oglInitialize(void);
	void oglDrawScene(BOOL isPaint = FALSE);
	void oglSetBackgroundColor(COLORREF color) {m_glBackColor = color;};
	void oglSetExtColor(COLORREF color) {m_glExtColor = color;};
	void oglSetCursor(HCURSOR cursor) {m_glCursor = cursor;};
	void oglScaleChange(float rate, float center_x, float center_y);	// Beagle 20130103 added.
	void oglScaleSet(float scale);	// Beagle 20130103 added.
	void oglScaleReset(void) {oglScaleSet(1.0f);}
	void oglScaleEqual(void) {oglScaleReset();}
	void oglSetPosition(float pos_x, float pos_y, int TexNum=0);	// Beagle 20140605.

	void oglSetTexture(IMAGE *TexImg, int TexNum=0);	// Beagle 20120920 added.
	void oglSetResult(BOOL bResult) {	m_curr_sample_result = bResult;}; //eric chao 20130313
	void oglSetTexture(int w, int h, unsigned char *buffer, BOOL result=FALSE, int TexNum=0, BOOL shrink=FALSE, int shrink_w=0, int shrink_h=0);	// texnum is for master / slave, split window
	void oglSetTextureGrey(int w, int h, unsigned char *buffer, int TexNum=0);	// added by danny at 20100601, // texnum is for master / slave, split window
	void oglSetTextureRGB332(int w, int h, unsigned char *buffer, int TexNum=0);
	BOOL oglUpdateTexture(IMAGE *TexImg, int xoffset, int yoffset, int TexNum=0);	// Beagle 20130409 added.
	BOOL oglUpdateTextureBySplit(IMAGE *TexImg, int index, int total, int TexNum=0);	// Beagle 20140212 added.
	void oglClearTexture(int data_w, int data_h, COLORREF color, int TexNum, int orig_w=0, int orig_h=0);	// Beagle 20140212 added.

	void oglResetStackDefectPiece(void); //seanchen 20150820
	void oglResetTexture();		// added by danny at 20091015
	void oglClearAllAnchor();	// added by danny at 20091015
	void oglGetImageFrame(int &w,int &h,int nTextNum) 
	{ 
		if (nTextNum>=0 && nTextNum < MAX_SPLIT_WINDOW){
			w = m_width[nTextNum]; h = m_height[nTextNum];
		}else{
			w = m_width[0]; h = m_height[0];
		}
	}; //eric chao 20151218
	void oglSetImageFrame(int w, int h) {m_width[0] = w; m_height[0] = h;};	// Beagle 20111101 modified.
	void oglSetLuminanceData(int type, unsigned char *lData=NULL, unsigned char *rData=NULL, unsigned char *gData=NULL, unsigned char *bData=NULL, int length=0, int x=-1, int y=-1);	// Beagle 20130816 modified.
	void oglProjectPoint(int& mouseX, int& mouseY, int imageX, int imageY);
	void oglTranslatePoint(int mouseX, int mouseY, int& imageX, int& imageY, int *window=NULL, int *winX=NULL, int *winY=NULL); // Beagle 20111101 modified.
	void oglShowLargeNumber(LONGLONG number) {m_largenumber = number;};
	void oglGetMouse(int &x, int &y) {x = m_mouse_x; y = m_mouse_y; };
	void oglLoadBMP(TCHAR *Filename);
	int	 oglGetNumAnchors(void) { return m_anchors; };
	void oglGetAnchor(int num, int &x, int &y);
	void oglDeleteAnchor(int index);
	void oglAddAnchor(int x, int y);
	void oglShowMinimap() { m_minimap = TRUE; };
	void oglHideMinimap() { m_minimap = FALSE; };
	void oglSetDisplayMode(int mode) { m_display_mode = mode; }	// Beagle 20140711
	int oglSetDefectPiece(DEFECT_PIECE *pDefect);	// for master / slave, split window // Beagle 20120508 modified.
	void oglShowDefectLabel(bool show=true) { m_bShowDefectLabel = show; }	// Beagle 20140711
	void oglHideDefectLabel(void) { oglShowDefectLabel(false); }	// Beagle 20140711
	void oglSetNumDefect(int num) { m_num_defect = num;};
	void oglSetDefect(DEFECT_REGION *pDefect);	// Beagle 20120508 modified.
	int	 oglGetRectCount(int TexNum=0) { return ( m_mapRectInfo.find( TexNum ) == m_mapRectInfo.end() ) ? NULL : m_mapRectInfo[ TexNum ].rects; };
	void oglSetRectCount(int count)
	{
		for (std::map< int, RECTINFO >::iterator itr = m_mapRectInfo.begin(); itr != m_mapRectInfo.end(); itr++)
		{
			itr->second.rects = count;
		};
	}
	void oglClearUserLayoutRect(); //seanchen 20151211
	void oglSetRectShape(int index, oglShape shape, int left, int top, int right, int bottom, int TexNum=0, BOOL redraw=TRUE);
	void oglGetRectShape(int index, oglShape &shape, int &left, int &top, int &right, int &bottom, int TexNum=0);
	void oglClearShapeBySize(RECT rt, int TexNum=0);
	void oglClearShape(bool bRedraw=true); //eric chao 20131122
	void oglSetInspectionMark(BOOL mark, BOOL* split_mark=NULL, int n=0);		// for master / slave, split window
	void oglShowInspectionMark(void) { m_show_inspection_mark = TRUE; };
	void oglResetInspectionMark(void); 
	void oglShowInspectionText(BOOL show) { m_show_inspection_text = show; };
	void oglClearText(void);										// Beagle 20120703 added.
	void oglPrintf(wchar_t *fmt, ...);								// Beagle 20120703 changed.
	void oglPrintf(int align, float x, float y, wchar_t *fmt, ...);	// Beagle 20120703 changed.
	void oglPrintfFrag(int split_id, int align, int frag_idx, int frag_total, float x, float y, wchar_t *fmt, ...);	// Beagle 20140214
	void oglPrintf(GLPRINTF_POSITION *gp, wchar_t *fmt, ...);	// Beagle 20140214
	void oglDrawCorona(float cx, float cy, float scale);
	void oglDrawInspectionText();
	void oglSetShowSegment(BOOL show) {m_isShowSegment = show; oglClearBoundedTracker();};	// modified by eric at 20120330
	void oglSetShowGrid(BOOL show) {m_showGrid = show;};
	void oglSetShowGridTexture(BOOL show) {m_showGridTexture = show;};
	void oglSetShowGoldDiff(BOOL show) {m_showGoldDiffTexture = show;};
	void oglSetGridSize(int size) {m_gridSize = size;};
	void oglSetShowLearnRgn(BOOL show) {m_showLearnRegion = show;};
	void oglSetLearnRegion(std::vector<CRect> *list) {m_learnRegion.clear(); m_learnRegion.assign(list->begin(), list->end());};
	void oglSetShowBarcode(BOOL show) {m_isShowBarcode = show;};	// added by eric at 20110801
	void oglSetLMKeyBarcodeSize(int w, int h) {m_barcodewidth = w; m_barcodeheight = h;};	// added by eric at 20110801
	void oglSetAnchorMaxSize(int w, int h) {m_AnchorWidth=w; m_nAnchorHeight=h;};		// added by eric at 20150427
	void oglMaxBarcodeNumber(int count) {m_Numberbarcode = count;};	// added by eric at 20110801
	void oglClearBarcodeShape(void);	// added by eric at 20110801
	void oglSetShowOCRSize(BOOL show) {m_showOCRSize = show;};	// added by eric at 20110809
	//void oglSetUndoPointBarcode(void) {m_rects--;};	// added by eric at 20110902
	void oglSetShowChessboard(BOOL show) {m_showChessboard = show;}; //added by leo 20120728
	void COpenGLControl::oglSetChessboardMark(std::vector<CPoint> *pointList,  std::vector<CPoint> *distortionptr, std::vector<CPoint> *avgdistortionptr); //add by leo 20120728

	void oglSetDrawWeakenColorBlockMode(oglPaintMode nMode) { m_PaintMode = nMode; /*m_LeftButtonDown = 0;*/ oglClearBoundedTracker();};	// added by eric at 20110929, 20120330
	void oglGetWeankenColorRect(CRect &rt) {rt = m_rcWeakenColor;};	// added by eric at 20110929
	void oglReleaseWeakenColorBlock() {m_nWeakenColorBlock = 0; m_rcWeakenColor = CRect(0,0,0,0); oglDrawScene();};	// added by eric at 20110929
	void oglGetPaintMode(oglPaintMode &paint) { paint = m_PaintMode; }; // added by eric at 20110929
	void oglSetShowOnlyBarcode(BOOL show) {m_isShowOnlyBarcode = show;};	// added by eric at 20111024
	void oglSetBarcodeRectShape(POINT lefttop, POINT rightbottom) {m_Barcodelefttop = lefttop; m_Barcoderightbottom = rightbottom; oglDrawScene();};	// added by eric at 20111024
	// added by amike at 20120907
	void oglSetRectInfo(int index, oglShape shape, int left, int top, int right, int bottom, COLORREF color, int TexNum=0);
	void oglShowRectColor() { m_show_rect_color = TRUE; }
	void oglHideRectColor() { m_show_rect_color = FALSE; }
    BOOL oglIsShowRectColor() { return m_show_rect_color; }
#ifdef CUDA_OPENGL_INTEROP
	void oglCreateDynamicBuffer(int width, int height, HDC *pDC, HGLRC *pGLRC, int *pBufNum);
#endif
	// Beagle 20120921 added.
	void oglGetWindowSize(int &w, int &h);
	void oglGetSplitWinSize(int &w, int &h);

	void oglSetPolygonCount(int count) {m_polygons = count;};	// added by danny
	void oglGetPolygonVertex(int index, std::vector<CPoint> *vertexList);		// added by danny
	void oglSetPolygonVertex(int index, std::vector<CPoint> *vertexList);		// added by danny

	// added by danny
	BOOL oglIsShowInspectionRgn() {return m_showInspectionRgn;}; //eric chao 20130624
	void oglViewInspectionRgn(BOOL view) {m_showInspectionRgn = view; InvalidateRect(NULL, FALSE);};
	void oglShowInspectionRgn(BOOL show=TRUE) {m_showInspectionRgn=show;}	// Beagle 20130403 added.
	void oglShowMatrixLine(BOOL bShow,LPRECT rcMatrix);//20170717
    void oglSetMatrixFlipMode( MatrixFlipMode eMatrixFlipMode );
    void oglSetMatrixDisplaySequence( const std::vector< int >& vecSequence );
	void oglHideInspectionRgn(void) {oglShowInspectionRgn(FALSE);}	// Beagle 20130403 added.
	void oglSetInspectionRgn(int w, int h, unsigned char *buffer);
	void oglSetInspectionRgn(IMAGE *RegionImg, int RegionNum);	// Beagle 20130822 added.
	BOOL oglUpdateInspectionRgn(int w, int h, int x, int y, unsigned char *buffer);	// Beagle 20130409 added.
	BOOL oglUpdateInspectionRgn(IMAGE *RegionImg, int RegionNum, int x, int y);	// Beagle 20130822 added.
	void OnKeyDown(UINT nChar);
	BOOL oglIsInPolygon() {return m_poly_drawing;};
	void oglSetAnchorMark(int idx);
	int oglGetAnchorMark(){return m_selAnchor;};//seanchen 20130819
	void oglSetAnchorRangeMark(int nRectIdx);//seanchen 20130819
	void oglMoveMarkedSegment(int type, int idx, POINT offset);	// type:0 => rect, type:1 => poly

	void oglSetRectangleMark(std::vector<int> *idxList, BOOL bDrawTracker);	// modified by eric at 20120330
	void oglSetPolygonMark(std::vector<int> *idxList, BOOL bDrawTracker);	// modified by eric at 20120330
	void oglClearRPMark(void) {m_selRectangle.clear(); m_selPolygon.clear(); m_selRegion.clear();};	// modified by eric at 20120330

	// For master-slave architecture.
	void oglSetSplit(int split_window, BOOL origin=FALSE);
	void oglSetSplit_New(int split_w, int split_h, BOOL clearimage);	// Beagle 20140730 modified.
	void oglSetSplitInspectionMark(BOOL mark, int window) { m_split_inspection_mark[window] = mark; };

	void oglSetRegionSegmentTexture(int w, int h, unsigned char *buffer);
	void oglSetRegionSegmentTexture(int w, int h, COLORREF clr, unsigned char *buffer);	// Beagle 20130305 modified.
	void oglSetRegionDefectShapeTexture(int w, int h, unsigned char *buffer);	
	void oglSetRegionDefectShapeTexture(int w, int h, COLORREF clr, unsigned char *buffer);
	void oglEmptyRegionSegment() {m_showRegionSegments = FALSE;};
	void oglEmptyRegionDfShape() {m_showRegionDFShape = FALSE;};

	// added by danny at 20100412
	void oglSetGridTexture(int w, int h, unsigned char *buffer);
	void oglSetGoldGridTexture(int w, int h, unsigned char *buffer);

	void oglSetOnline(BOOL isOnline) {m_isOnline = isOnline;};

	// set inspection result image
	void oglSetInspectionSignImg(CImage *signList);

	float	oglGetNormalScale(int split_id) { return m_norm_scale[split_id]; };

	void		oglSetLMKeyMode(LMKeyMode mode) {m_lmMode = mode;};
	LMKeyMode	oglGetLMKeyMode(void) {return m_lmMode;};
	void	oglMakeCurrent(void) {wglMakeCurrent(hdc, hrc);};

	void	oglSetDClickEvent(UINT msgId, WPARAM wpara, LPARAM lpara)
	{
		dcMsgId = msgId;
		dcWpara = wpara;
		dcLpara = lpara;
	};

	void	oglGetEraserSize(int &x, int &y, int &r) {x=m_oglEraserX; y=m_oglEraserY; r=m_oglEraserRadius;};

	////////////////// added by eric at 20120330 /////////////////////////
	void oglUndoShape(int TexNum=0) { if ( m_mapRectInfo.find( TexNum ) != m_mapRectInfo.end() && m_mapRectInfo[ TexNum ].rects > 0 ) m_mapRectInfo[ TexNum ].rects--; };
	void oglClearBoundedTracker();
	BoundedType oglGetBoundedType() {return m_nBoundedType;};
	void oglSetRegionCount(int count) {m_regions = count;};
	void oglSetRegionSize(int index, CRgn* rgn);
	void oglGetRegionSize(int index, CRgn* rgn);
	void oglSetRegionMark(std::vector<int> *idxList, BOOL bDrawTracker);
	void oglReleaseRegionList() {		
		for (unsigned int i=0; i<m_region.size(); i++)
		{
			m_region[i]->DeleteObject();
			delete m_region[i];
			m_region[i] = NULL;
		}
		m_region.clear();
		m_regions=0;	// added by eric at 20150911
	};
	void oglSetInspectionRegionCount(int count);
	void oglAddInspectionRegion(std::vector<CRgn *> vList);	// added by eric at 20130111
	void oglGetInspectionRegion(std::vector<CRgn *> &vList) {vList.assign(m_inspectRegion.begin(), m_inspectRegion.end());};	// added by eric at 20130111
	void oglSetInspectionSelection(int index);	// added by eric at 20130111
	//////////////////////////////////////////////////////////////////////////

	void oglClearLayoutRectangle(int split_id, BOOL redraw);	// added by eric at 20161007
	void oglSetUserLayoutRectangle(int split_id, RECT RcUserLayout, BOOL redraw=FALSE, BOOL bFocus=FALSE);	// added by eric at 20161007

	// added by eric at 20120130, 20130124
	void oglUndoAddedShape(BoundedType type,int TexNum=0) 
	{ 
		if (type == TYPE_POLYGON) { 
			m_poly_points[m_polygons] = 0;
		} else if (type == TYPE_NORMAL) {
			oglUndoShape( TexNum );//m_rects--;
		}
	};

	// Beagle 20120326 modified.
	// position = 0: Upper left, 1: Upper right
	void oglSetOCRResult(char *barcode_result, char *ocr_result=NULL, int position=0);
	void oglShowRGBChannel(BOOL enable) {m_bShowRGB = enable;};	// added by eric at 20120223

	// Beagle 20120301 added.
	void oglSetBarcodeDigitImage(int n, int x, int y, unsigned char *ptr, int depth);
	void oglShowBarcodeDigit(BOOL show=TRUE) { m_showBarcodeDigitsImage=show; };
	void oglBarcodeDigitPosition(int n, int *nArray, float *xArray, float *yArray);

	void oglShowAnchorPoints(BOOL bShow=TRUE) {m_bShowAnchorPoints = bShow;};	// added by eric at 20120515
	void oglShowDefectShift() { m_showDefectShift = TRUE; };	// Beagle 20120630
	void oglHideDefectShift() { m_showDefectShift = FALSE; };	// Beagle 20120630
	void oglSetFrameColor(int num, BOOL show, COLORREF color);	// Beagle 20120704 added.
	void oglClearFrameColor();	// added by eric at 20120713
	void oglShowFrameColor(bool bShow);
	void oglSetFrameGrid(int gridX, int gridY, int style=GRID_STYLE_NOCHANGE);	// Beagle 20120710 modified.
	BOOL oglIsShowIndex() {return m_showFrameIndex;};
	void oglShowIndex(BOOL show=TRUE) { m_showFrameIndex=show; oglClearText(); };	// Beagle 20120716 modified.
	void oglSetUnitLine(BOOL orientation,std::vector<int> vUnit); //eric chao 20160527
	void oglDrawUnitLine(int split_id); //eric chao 20160527
	void oglClearUnitLine(void) { std::vector<int> vEmpty; oglSetUnitLine(FALSE,vEmpty);}; //eric chao 20160527
	void oglSetCutHereLine(BOOL orientation, int start, int period);	// Beagle 20120817 added.
	void oglClearCutHereLine(void) { oglSetCutHereLine(TRUE, 0, 0); };	// Beagle 20120817 added.
	void oglDrawCutHereLine(int split_id);								// Beagle 20120817 added.

	void oglSetShapModification(BOOL bEnable = TRUE) { m_bEnableShapModify = bEnable; };	// added by eric at 20120702
	void oglSetShowOCRInspArea(BOOL bShow) {m_ShowOCRInspArea = bShow;};	// added by eric at 20120801
	void oglSetOCRInspArea(CRect rt) {m_rtOCR.CopyRect(rt);};	// added by eric at 20120801
	void oglSetOKNGText(BOOL bShowText, BOOL bIsOK) {m_bShowOKNGText = bShowText; m_bShowOKText = bIsOK;};	// added by eric at 20120821
	void oglDrawOKNGText();	// added by eric at 20120821
	void oglSetDrawTriangle(BOOL bDraw) {m_bDrawTriangle = bDraw; if(!bDraw) {oglClearText(); m_multiTrianglePtrList.clear(); m_multiTriangleDisList.clear();}};	// added by eric at 20120910, 20130107
	void oglDrawTriangle(std::vector<CPoint>vertexList) { m_triangleptrList.clear(); m_triangleptrList.assign(vertexList.begin(), vertexList.end());};	// added by eric at 20120910
	void oglDrawTriangleList(std::vector<std::vector<CPoint>>vList) {m_multiTrianglePtrList.assign(vList.begin(), vList.end());};	// added by eric at 20130107
	void oglDrawTriangleDisList(std::vector<CROSSHAIR_LENGTH>vList) {m_multiTriangleDisList.assign(vList.begin(), vList.end());};	// seanchen 20130827-6
	void oglSetDrawPattern(BOOL bEnable) {m_bDrawPatternShape = bEnable;};	// added by eric at 20121008
	BOOL oglGetDrawPattern() {return m_bDrawPatternShape;};	// added by eric at 20140610
	void oglSetPatternShape(CRect rt) { m_rtPatternShape.SetRectEmpty(); m_rtPatternShape.CopyRect(rt);};	// added by eric at 20121008
	void oglSetPatternRectCount(int count) { m_rectsPattern = count; };	// added by eric at 20121008
	void oglSetPatternShapeMode(BOOL bPatternMode = FALSE) {m_isPatternMode = bPatternMode;};	// added by eric at 20121008
	void oglSetShowPatternRectShape(BOOL show) {m_isShowPatternRect = show;};	// added by eric at 20121008
	void oglClearPatternShape();	// added by eric at 20121008
	void oglSetPatternRectShape(int index, int left, int top, int right, int bottom);	// added by eric at 20121008
	void oglSetPatternSegmentShape(BOOL bShow, CRect rt=CRect(0,0,0,0)) {m_isShowPatternSegment=bShow; m_rtSegment.CopyRect(rt);};	// added by eric at 20121017

	void oglDrawHoriVertLine(BOOL enable) {m_bSetHoriOrVertLine = enable;};
	void oglClearLineShape(void);	// added by eric at 20121121
	void oglSetLineShape(int index, int left, int top, int right, int bottom, BOOL redraw=TRUE);	// added by eric at 20121121
	void oglGetLineShape(int index, int &left, int &top, int &right, int &bottom);	// added by eric at 20121121
	void oglShowLineShape(BOOL bShow) {
		m_bShowLineShape = bShow; 
		m_bSetHoriOrVertLine = FALSE;
		//m_bShowAngleString = FALSE; 
		//m_bShowLineString = FALSE; 
		m_tLineLabel.Empty();
		m_tDisable.Empty();
		m_nEnableMap=0xFF;
		m_bCrossOverLineOrder=FALSE;
		oglClearLineShape();
	};	// added by eric at 20121121, 20130328

	void oglSetLineMark(int idx);	// added by eric at 20121121
	void oglSetLineMark(CRect rt);	// added by eric at 20130107
	int	 oglGetLineNumbers() {return m_lineRects;};	// added by eric at 20121122
	void oglRemoveLine(int index);	// added by eric at 20121122
	void oglSetXYRatio(float XYRatio){
		XYRatio = (float)XYRatio;
		m_XYRatio = XYRatio;
	}; // added by amike at 20121206
	void transByXYRatio(GLfloat *trans_x, GLfloat *trans_y, GLfloat angle, GLfloat cx, GLfloat cy, GLfloat sx, GLfloat sy, GLfloat orig_x, GLfloat orig_y);
	void oglSetShiftMode(BOOL bEnable) {m_bShiftMode = bEnable;};	// added by eric at 20121212
	void oglSetShiftShape(CRect rt) {m_rtShiftShape.CopyRect(rt);};	// added by eric at 20121212

	// added by eric at 20130107
	void oglClearLayoutRect(void);
	void oglClearTracker() {m_vSelectionList.clear(); m_vRectTrackerList.clear();};
	void oglShowLayout(BOOL bShow) {m_bShowLayout = bShow; if (m_tracker != NULL) {delete m_tracker; m_tracker = NULL;}};
	void oglGetSelLayoutIndex(int x, int y, int &nIndex);
	void oglGetSelLayoutIndex(std::vector<int> &vIndexList) {vIndexList.assign(m_vSelectionList.begin(), m_vSelectionList.end());};
	void oglAddLayoutShape(int index, int left, int top, int right, int bottom, BOOL redraw=TRUE);
	void oglGetLayoutShape(int index, int &left, int &top, int &right, int &bottom);
	void oglSetLayoutShape(int index, BOOL bCtrl = FALSE);
	void oglGetSelectionLayout(int &num) {num = (int)m_vSelectionList.size();};
	void oglMergeLayoutShape(std::vector<RECT> &vRectList, int &nBaseIndex);
	void oglGetAllLayoutShape(std::vector<RECT> &vRectList);
	int GetSelectedShapeIndex() {return m_nSelectedIndex;};
	void oglShowAngleString(BOOL enable) {m_bShowAngleString = enable;};	// added by eric at 20130128
	void oglShowLineString(BOOL enable) {m_bShowLineString=enable;};
	void oglDrawLineString(BOOL enable, CString tLabel, int nDist, int enableMap=0xFF, CString tDisable=_T("")) {
		m_bShowLineString = enable; 
		m_tLineLabel = tLabel; 
		m_nShootFormatDist=nDist;
		m_nEnableMap=enableMap;
		m_tDisable=tDisable;
	};	// added by eric at 20161217
	void oglCanCrossoverLineOrder(BOOL bEnable=FALSE) {m_bCrossOverLineOrder=bEnable;};	// added by eric at 20161217
	void oglSetRotationAngleString(float x, float y, CString angle, int type) {m_AngleX = x; m_AngleY = y; m_AngleString = angle; m_AngleAlignType = type;};

	void oglShowCrosshair(void) {m_showCrossHair = TRUE;}	// Beagle 20130117 added.
	void oglHideCrosshair(void) {m_showCrossHair = FALSE;}	// Beagle 20130117 added.
	void oglAddCrosshair(int split, float x, float y, COLORREF color);		// Beagle 20130610 modified.
	void oglAddCrosshairPair(int split, float x1, float y1, float x2, float y2, COLORREF color);	// Beagle 20130611 modified.
	void oglAddCrosshairPair_AndDis(int split, float x1, float y1, float x2, float y2, COLORREF color,float fXDis,float fYDis,float fLDis); //Seanchen 20130826-3
	void oglClearCrosshair(int split = -1);					// Beagle 20130117 added.
	void oglShowCursorOutline(BOOL show=TRUE) { m_show_cursor_outline=show; }	// Beagle 20130424 added.
	void oglHideCursorOutline(void) {oglShowCursorOutline(FALSE);}	// Beagle 20130424 added.

	// added by eric at 20130208
	void oglShowShadowRectShape(BOOL bShow);	
	void oglSetSizeShadowShape(POINT pt) {m_ptShadowShape.x = pt.x; m_ptShadowShape.y = pt.y;};
	void oglSetShadowType(int nType) {m_shadowType = nType;};
	void oglAddShadowShape(RECT shadowShape, RECT lineShape, RECT lineShape2, int direction);	// modified by eric at 20130219
	void oglClearShadowShape();
	void oglUndoShadowShape();
	void oglGetShadowShape(int index, CRect &shadowShape, CRect &lineShape, CRect &lineShape2);
	void oglSetShadowShape(int index, CRect shadowShape, CRect lineShape, CRect lineShape2);	// added by eric at 20130318
	void oglClearSelection();
	void oglGetSelection(int &index) {index = m_nSelectedIndex;};
	void oglDeleteSelection();
	void oglSetShadowBlockDirection(int direct) {m_nBlockDirection = direct;};	// modified by eric at 20130219
	void oglGetShadowBlockSize(int &size) { size = (int)m_vShadowInfo.size();};	// added by eric at 20130221
	//////////////////////////////////////////////////////////////////////////
	void oglSetDistanceStrings(std::vector<CString>vList) {m_vDistanceStrings.assign(vList.begin(), vList.end());};	// added by eric at 20130311
	void oglSetDistanceUnitStrings(CString strUnit){m_strDistanceUnitString = strUnit;};//seanchen 20130821
	void oglSetDistanceXRatio(double fDistance_Ratio){m_fDistance_XRatio = fDistance_Ratio;};//seanchen 20130821
	void oglSetDistanceYRatio(double fDistance_Ratio){m_fDistance_YRatio = fDistance_Ratio;};//seanchen 20130821
	double oglGetDistanceXRatio(){return m_fDistance_XRatio;};//seanchen 20130823-2
	double oglGetDistanceYRatio(){return m_fDistance_YRatio;};//seanchen 20130823-2

	void oglEnableMinimumRedrawInterval(BOOL enable);	// Beagle 20130621 added.
	void oglSetRadioButtonNumber(int n);	// Beagle 20130807

	void oglSetColorDerivateBox(BOOL bEnable) {m_bEnableColorDerivateBox = bEnable; if (bEnable==FALSE) m_vColorDerivateBox.clear(); m_lmMode=(bEnable==TRUE)?LM_SET_COLORDERIVATE:LM_IGNORE;};	// added by eric at 20130923
	void oglSetColorDerivateBoxList(std::vector<CRect> vList) {
		m_vRectTrackerList.clear();
		m_nSelectedIndex = -1;
		if (m_vColorDerivateBox.size() == 0) {
			m_vColorDerivateBox.assign(vList.begin(), vList.end());
		} else {
			for (int i=1; i<(int)vList.size(); i++) {
				m_vColorDerivateBox[i].right = m_vColorDerivateBox[i].left + (vList[i].right-vList[i].left);
				m_vColorDerivateBox[i].bottom = m_vColorDerivateBox[i].top + (vList[i].bottom-vList[i].top);
			}
		}
	};	// added by eric at 20130923

	void oglSetSpecialRectBox(BOOL bShow) {m_bShowSpecialRectBox=bShow;};	// added by eric at 20150211
	void oglSetSpecialRectBoxList(std::vector<CRect> vList) {m_vSpecialRectBox.assign(vList.begin(), vList.end());};	// added by eric at 20150211
	void oglInitStackModeDrawPos(int nPos){m_next_defect = nPos;}//seanchen 20150407-01
	void oglSetStackInspIndex(int nIdx){m_stackInspIndex[m_next_defect] = nIdx;}; //call it before oglSetDefectPiece //seanchen 20150421-01
	void oglInitStackInspIndex(){ //seanchen 20150421-01
		for (int i=0; i<MAX_DEFECTS; i++) {
			m_stackInspIndex[i] = -1;
		}
	};
    void oglEnableRollingMode( const BOOL bEnable, const int iRollingMilliSeconds, const int iTextureNum = 0 );
    void oglRunRollingMode( const BOOL bRun, const int iTextureNum = 0 );
    void oglNoColorCubeBackground( const BOOL bNoColorCubeBackground );
	// Added message classes:
	afx_msg void OnPaint();
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	//afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);	// added by eric at 20130208

private:
	// GDI objects
	CPen m_redPen;	// Beagle 20130423 added.
	CPen m_bluePen;	// Beagle 20130423 added.
};
