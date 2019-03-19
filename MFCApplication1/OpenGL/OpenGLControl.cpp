#include "stdafx.h"
#include <OpenGLControl.h>
#include <stdarg.h>
#include <math.h>
#include <algorithm>	// added by eric at 20120330
#include <GL/wglew.h>

// Beagle 20130305 added -- For SetTexture()
#define STFLAG_SIZE_NO_CHANGE	0x01
#define STFLAG_SMOOTH			0x02
#define STFLAG_TRANSPARENT		0x04 //eric chao 20150415

#define	PICKBUFFER	512

#pragma comment(lib, "opengl32.lib")
#ifdef _WIN64
#pragma comment(lib, "..\\Share\\freetype2\\x64\\freetype2410MT.lib")
#else
#pragma comment(lib, "..\\Share\\freetype2\\win32\\freetype2410MT.lib")
#endif

#ifdef CUDA_OPENGL_INTEROP
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#endif

#define	GL_CHECK_ERROR() {											\
	GLenum err = glGetError();										\
	if (err != GL_NO_ERROR) {										\
		_D(L"%S(%d): Error %d: %S\n", __FILE__, __LINE__, err,		\
		gluErrorString(err));										\
	}																\
}

static void _D(const wchar_t *fmt, ...)
{
	wchar_t text[1024];
	va_list	ap;

	va_start(ap, fmt);
	vswprintf_s(text, 1024, fmt, ap);
	va_end(ap);

	OutputDebugStringW(text);
}

COpenGLControl::COpenGLControl(void) : m_bNoColorCubeBackground( FALSE )
{
	m_dThreadId = 0; //20170906
	m_anchors = 0;
	for (int c=0; c<MAX_ANCHORS; c++) {
		m_anchor_x[c] = -1;
		m_anchor_y[c] = -1;
	}

	// added by danny at 20110218
	m_selReposAnchor = -1;
	m_selReposAnchorRange = -100; //seanchen 20130819

    m_mapRectInfo[ NULL ];
	//m_rects = 0;
	//for (int c=0; c<MAX_RECTS; c++) {
	//	m_rect_type[c] = SHAPE_NONE;
	//	m_rect_color[c] = RGB(0,0,0);
	//}
	m_poly_drawing = FALSE;
	m_polygons = 0;
	for (int c=0; c<MAX_RECTS; c++) {
		m_poly_points[c] = 0;
	}

	// added by eric at 20121008
	m_rectsPattern = 0;
	for (int c=0; c<MAX_RECTS; c++) {
		m_rectPattern[c].left = m_rectPattern[c].top = m_rectPattern[c].right = m_rectPattern[c].bottom = 0;
	}

	m_bAnchorRect=FALSE;	// added by eric at 20150420

	// added by eric at 20121121
	m_lineRects = 0;
	for (int c=0; c<MAX_RECTS; c++) {
		memset(&m_lineRect[c],0,sizeof(RECT));
	}

	m_glBackColor = RGB(0, 0, 0);
	m_glExtColor = RGB(0, 0, 255);
	//m_glBackColor = ::GetSysColor(COLOR_3DFACE);
	m_glCursor = ::LoadCursor(NULL, IDC_HAND);		// default cursor

	m_LeftButtonDown = FALSE;
	m_RightButtonDown = FALSE;

	m_xTranslate = 0.0f;
	m_yTranslate = 0.0f;
	m_glScale = 1.0f;

	m_showInspectionRgn = FALSE;

	memset(&m_xLuminance, 0, sizeof(m_xLuminance));	// Beagle 20130816

	// Beagle 20120925 modified.
	for (int c=0; c<MAX_SPLIT_WINDOW; c++) {
		m_width[c] = 0;
		m_height[c] = 0;
		m_data_w[c] = 0;
		m_data_h[c] = 0;
		m_textype[c] = IMAGE_TYPE_NOTYPE;	// Beagle 20130410 added.
		m_norm_scale[c] = 1.0f;
		m_splitEnable[c] = false;
	}
	m_width[0] = m_bNoColorCubeBackground ? 0 : 32;
	m_height[0] = m_bNoColorCubeBackground ? 0 : 32;
	m_splitEnable[0] = true;

	m_glwidth = 0;
	m_glheight = 0;

	m_lmMode = LM_IGNORE;

	m_largenumber = -1;
	m_display_mode = DISPLAY_MODE_NORMAL;	// Beagle 20140711
	m_show_rect_color = FALSE; //added by amike at 20120907

	dcMsgId = 0;
	dcWpara = 0;
	dcLpara = 0;

	for (int c=0; c<MAX_INSPECTION_MARK; c++) {
		m_inspection[c] = TRUE;
	}
	m_curr_mark = MAX_INSPECTION_MARK-1;
	m_show_inspection_mark = FALSE;
	m_num_textstring = 0;

	m_selAnchorRange = -100; //seanchen 20130819
	m_selAnchor = -1;
	m_show_inspection_text = FALSE;
	m_curr_sample_result = FALSE;
	//m_selRectangle = -1;
	//m_selPolygon = -1;
	m_showRegionSegments = FALSE;
	m_showRegionDFShape = FALSE; //Seanchen 20160808

	m_isOnline = FALSE;
	m_isShowSegment = TRUE;
	m_LeftDownPos = CPoint(0, 0);
	m_oldPoint = CPoint(0, 0);
	m_ScaleCenter = CPoint(0, 0);	// Beagle 20130103 added.

	m_showGrid = FALSE; 
	m_showGridTexture = FALSE;
	m_showGoldDiffTexture = FALSE;
	m_gridSize = 80;
	m_showLearnRegion = FALSE;

	// added by eric at 20150427
	m_AnchorWidth=m_nAnchorHeight=0;

	m_barcodewidth = 0;	// added by eric at 20110801
	m_barcodeheight = 0;	// added by eric at 20110801
	m_Numberbarcode = 0;	// added by eric at 20110801
	m_isShowBarcode = FALSE;	// added by eric at 20110801
	m_showOCRSize = FALSE;	// added by eric at 20110809

	m_PaintMode = PAINT_NONE;	// added by eric at 20110929
	m_nWeakenColorBlock = 0;	// added by eric at 20110929
	m_isShowOnlyBarcode = FALSE;	// added by eric at 20111024

	m_wsplit = 1;
	m_hsplit = 1;
	m_bShowRGB = FALSE;	// added by eric at 20120223

	// Beagle 20120302 added.
	memset(m_barcode_result, 0, MAX_GL_BARCODE_DIGITS+1);
	memset(m_ocr_result, 0, MAX_GL_BARCODE_DIGITS+1);
	// added by eric at 20120330
	m_bLMouseDown = FALSE;
	m_tracker = NULL;
	m_bTracking = FALSE;
	m_nSelectedIndex = -1;
	m_nHitHandle = -1;
	m_bPolygonPointHit = FALSE;
	m_nPolygonHitIndex = -1;
	m_nBoundedType = TYPE_NONE; 
	m_bHideSelectedRegion = FALSE;

	m_pMovingRgn = NULL;
	m_nMovingRgnCntBuf = 0;
	// added by eric at 20120417
	m_regions = 0;
	m_inspectRegions = 0;

	// Beagle 20120712 added.
	m_font = NULL;
	m_fontFixed = NULL;

	// Beagle 20120508 added.
	memset(m_defectRegion, 0, MAX_DEFECTS*sizeof(DEFECT_REGION));
	memset(m_defectPiece, 0, MAX_DEFECTS*sizeof(DEFECT_PIECE));
	oglInitStackInspIndex(); //seanchen 20150421-01
	m_next_defect = 0;	// Beagle 20140711
	m_bShowDefectLabel = true;	// Beagle 20140711

	// Beagle 20120510 added.
	LARGE_INTEGER f;
	m_draw_lasttime = 0;
	QueryPerformanceFrequency(&f);
	m_draw_interval = 40 * f.QuadPart / 1000;	// 40ms
	m_need_redraw = FALSE;
	m_enable_redraw_interval = FALSE;	// Beagle 20130621 added.

	m_bShowAnchorPoints = TRUE;	// added by eric at 20120515
	m_showDefectShift = FALSE;
	m_bEnableShapModify = FALSE;	// added by eric at 20120702, 20120918
	m_FrameNumber = 0;			// Beagle 20120704 added.
	m_showFrameIndex = FALSE;	// Beagle 20120705 added.
	m_FrameGridStyle = GRID_STYLE_BILLIARD;	// Beagle 20120710 added.

	m_ShowOCRInspArea = FALSE;	// added by eric at 20120801
	m_rtOCR.SetRectEmpty();		// added by eric at 20120801
	oglClearCutHereLine();		// Beagle 20120817 added.
	m_showChessboard = FALSE;	// Beagle 20120820 added.
	oglClearUnitLine(); //eric chao 20160527

	m_bShowOKNGText = FALSE;	// added by eric at 20120821
	m_bShowOKText = TRUE;	// added by eric at 20120821
	m_bDrawTriangle = FALSE;	// added by eric at 20120910
	m_bDrawPatternShape = FALSE;	// added by eric at 20121008
	m_isPatternMode = FALSE;	// added by eric at 20121008
	m_isShowPatternRect = FALSE;	// added by eric at 20121008
	m_isShowPatternSegment = FALSE;	// added by eric at 20121017
	m_bShowLineShape = FALSE;	// added by eric at 20121121
	m_selLine = -1;	// added by eric at 20121121
	m_XYRatio = 1.0f; // added by amike at 20121206
	m_bShiftMode = FALSE;	// added by eric at 20121212

	m_bShowLayout = FALSE;	// added by eric at 20130107
	m_selLayoutRect = -1;	// added by eric at 20130107
	m_layoutRects = 0;		// added by eric at 20130107
	for (int c=0; c<MAX_RECTS; c++) {
		m_layoutRect[c].left = m_layoutRect[c].top = m_layoutRect[c].right = m_layoutRect[c].bottom = 0;
	}
	m_XPos = m_YPos = 1;	// added by eric at 20130107
	m_vRectTrackerList.clear();	// added by eric at 20130107
	m_vSelectionList.clear();	// added by eric at 20130107

	// added by eric at 20130208
	m_bShowShadowRectShape = FALSE;
	m_vShadowInfo.clear();
	m_shadowType = 0;
	m_pLineTracker = NULL;
	m_nBlockDirection = EMBOSS_UP;	// modified by eric at 20130306
	//////////////////////////////////////////////////////////////////////////

	m_showCrossHair = FALSE;	// Beagle 20130117 added.
	memset(m_crosshair, 0, MAX_SPLIT_WINDOW*sizeof(struct _CROSSHAIR));
	m_show_cursor_outline = FALSE;	// Beagle 20130424 added.
	m_mouse_x = 0;
	m_mouse_y = 0;
	m_mouse_sp = 0;

	m_bShowAngleString = FALSE;	// added by eric at 20130128
	// added by eric at 20161217
	m_bShowLineString = FALSE;
	m_nShootFormatDist=1;
	m_nEnableMap=0xFF;
	m_bCrossOverLineOrder = FALSE;
	//////////////////////////////////////////////////////////////////////////

	m_AngleX = 0.0f;
	m_AngleY = 0.0f;
	m_AngleAlignType = ALIGN_LEFT;

	// Beagle 20130423 added.
	m_redPen.CreatePen(PS_SOLID, 3, RGB(255,0,0));
	m_bluePen.CreatePen(PS_DOT,3,RGB(0,0,255));

	m_nRadioButton = 0;		// Beagle 20130807
	m_nRadioSelected = 0;	// Beagle 20130807
	m_bDrawUserExtLY = TRUE;

	m_strDistanceUnitString = _T(""); // Seanchen 20130821
	m_fDistance_XRatio = 1.0;	// Seanchen 20130821
	m_fDistance_YRatio = 1.0;	// Seanchen 20130821
	m_minimap = FALSE;			// Beagle 20131016
	m_num_defect = 0;
    m_bShowMatrixLine = FALSE;
    m_eMatrixFlipMode = FLIP_NONE;

	// added by eric at 20130827
	const LMKeyMode ctSingleClickEvent[] = {LM_SET_ANCHOR, LM_SET_WEAKENANCHOR,
		LM_SET_BARCODEANCHOR, LM_SET_DISTANCEANCHOR};
	int nCount = sizeof(ctSingleClickEvent)/sizeof(LMKeyMode);
	for (int i =0;i<nCount;i++){
		m_vSingleClickEvent.push_back(ctSingleClickEvent[i]);
	}

	m_bEnableColorDerivateBox = FALSE;	// added by eric at 20130923
	m_bShowSpecialRectBox=FALSE;	// added by eric at 20150211

	memset(&m_cutHereLine,0,sizeof(m_cutHereLine)); //eric chao 20160527
	m_xUnitDivide.orientation = FALSE; //eric chao 20160527
	m_nFocusSplitId=0;	// added by eric at 20161007
	memset(&m_rtFocusUserLayoutRectangle,0,sizeof(RECT));	// added by eric at 20161007
	m_bSetHoriOrVertLine = FALSE;
}

COpenGLControl::~COpenGLControl(void)
{
	// Beagle 20120504 added.
	delete m_font;
	delete m_fontFixed;

	// added by eric at 20120330
	if (m_tracker != NULL)
	{
		delete m_tracker;
		m_tracker = NULL;
	}

	// added by eric at 20130208
	if (m_pLineTracker) {
		delete m_pLineTracker;
		m_pLineTracker = NULL;
	}

	for (unsigned int i=0; i<m_region.size(); i++)
	{
		m_region[i]->DeleteObject();
		delete m_region[i];
	}

	for (unsigned int i=0; i<m_inspectRegion.size(); i++)
	{
		m_inspectRegion[i]->DeleteObject();
		delete m_inspectRegion[i];
	}

	if(m_pMovingRgn)
	{
		delete [] m_pMovingRgn;
		m_pMovingRgn = NULL;
	}

	for (int s=0; s<MAX_SPLIT_WINDOW; s++) {	// Beagle 20130118 added.
		if (m_crosshair[s].size > 0 && m_crosshair[s].p != NULL) {
			free(m_crosshair[s].p);
		}
		if (m_crosshair[s].pair_size > 0 && m_crosshair[s].pair != NULL) {
			free(m_crosshair[s].pair);
		}

		if (m_crosshair[s].pair_size > 0 && m_crosshair[s].pDistance != NULL) {	//seanchen 20130827-2
			free(m_crosshair[s].pDistance);
		}
	}

	if (m_xLuminance.dataLuminance != NULL) {	// Beagle 20130816
		delete m_xLuminance.dataLuminance;
	}
	if (m_xLuminance.dataRed != NULL) {			// Beagle 20130816
		delete m_xLuminance.dataRed;
	}
	if (m_xLuminance.dataGreen != NULL) {		// Beagle 20130816
		delete m_xLuminance.dataGreen;
	}
	if (m_xLuminance.dataBlue != NULL) {		// Beagle 20130816
		delete m_xLuminance.dataBlue;
	}
}
void COpenGLControl::oglShowMatrixLine(BOOL bShow, LPRECT pMatrix)//20170717
{
	m_bShowMatrixLine = bShow;
	if (pMatrix){
		m_rcMatrix = *pMatrix;
	}
	else{
		memset(&m_rcMatrix, 0, sizeof(m_rcMatrix));
	}
}
void COpenGLControl::oglSetMatrixFlipMode( MatrixFlipMode eMatrixFlipMode )
{
    m_eMatrixFlipMode = eMatrixFlipMode;

    oglDrawScene();
}
void COpenGLControl::oglSetMatrixDisplaySequence( const std::vector< int >& vecSequence )
{
    if ( 9 != vecSequence.size() )
    {
        ASSERT( FALSE );

        return;
    }
    auto swap = []( std::vector< int >& vecData, const int a, const int b )
    {
        vecData[ a ] ^= vecData[ b ];
        vecData[ b ] ^= vecData[ a ];
        vecData[ a ] ^= vecData[ b ];
    };
    auto toString = []( std::vector< CString >& vecDst, const std::vector< int >& vecSrc )
    {
        vecDst.resize( vecSrc.size() );

        for ( UINT i = NULL; i < vecSrc.size(); i++ )
        {
            vecDst[ i ].Format( _T( "%d" ), vecSrc[ i ] );
        }
    };
    std::vector< int > vecHorizon  = vecSequence;
    std::vector< int > vecVertical = vecSequence;

    swap( vecHorizon, 0, 2 );
    swap( vecHorizon, 3, 5 );
    swap( vecHorizon, 6, 8 );

    std::vector< int > vecH_and_V = vecHorizon;

    swap( vecVertical, 0, 6 );
    swap( vecVertical, 1, 7 );
    swap( vecVertical, 2, 8 );

    swap( vecH_and_V, 0, 6 );
    swap( vecH_and_V, 1, 7 );
    swap( vecH_and_V, 2, 8 );

    toString( m_vecMatrixHorizonSequence,  vecHorizon  );
    toString( m_vecMatrixVerticalSequence, vecVertical );
    toString( m_vecMatrixNormalSequence,   vecSequence );
    toString( m_vecMatrixH_and_VSequence,  vecH_and_V  );
}

BEGIN_MESSAGE_MAP(COpenGLControl, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_NCHITTEST()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	//ON_WM_MBUTTONDOWN()
	//ON_WM_MBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEWHEEL()
	ON_WM_SETCURSOR()	// added by eric at 20130208
END_MESSAGE_MAP()

void COpenGLControl::OnPaint()
{
	oglDrawScene(TRUE);
	ValidateRect(NULL);
}

int COpenGLControl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1) 
		return -1;

	oglInitialize();

	try {	// Beagle 20120712 modified.
		char sys_dir[256], path[1024];
		GetWindowsDirectoryA(sys_dir, 256);
		sprintf_s(path, 1024, "%s\\Fonts\\ARIALUNI.TTF", sys_dir);
		m_font = new FTGLTextureFont(path);	// Arial Unicode MS
		m_font->FaceSize(18);
		sprintf_s(path, 1024, "%s\\Fonts\\MSGOTHIC.TTC", sys_dir);
		m_fontFixed = new FTGLTextureFont(path);	// MS Gothic
		m_fontFixed->FaceSize(18);
	} catch (CException *e) {
		AfxMessageBox(L"OpenGL: Create font error.", MB_OK|MB_ICONERROR);
		e->Delete();
	}
    oglSetMatrixDisplaySequence( { 1, 2, 3, 8, 9, 4, 7, 6, 5 } );

	return 0;
}

void COpenGLControl::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
		case IDT_GL_TIMER:
			// Beagle 20120510 added.
			LARGE_INTEGER now_time;
			QueryPerformanceCounter(&now_time);
			if (m_need_redraw && now_time.QuadPart - m_draw_lasttime > m_draw_interval) {
				m_draw_lasttime = now_time.QuadPart;
				DrawScene(FALSE);
				PostDrawScene(FALSE);
				m_need_redraw = FALSE;
			}
			break;

		default:
			break;
	}
	CWnd::OnTimer(nIDEvent);
}

void COpenGLControl::oglCreate(CRect rect, CWnd *parent, UINT id)
{
	// Beagle added CS_DBLCLKS to receive double click event. 20110704
	CString className = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_OWNDC|CS_DBLCLKS, NULL, (HBRUSH)GetStockObject(NULL_BRUSH/*BLACK_BRUSH*/), NULL);

	CreateEx(0, className, _T("OpenGL"), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, rect, parent, id);

	// Set initial variables' values
	hWnd = parent;
}

void COpenGLControl::oglInitialize(void)
{
	// Initial Setup:
	//
	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32, // bit depth
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		16, // z-buffer depth
		0, 0, 0, 0, 0, 0, 0,
	};

	// Get device context only once.
	CDC *myCDC = GetDC();
	hdc = myCDC->m_hDC;
	ReleaseDC(myCDC);

	// Pixel format.
	m_nPixelFormat = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, m_nPixelFormat, &pfd);

	//Record Create Thread Id
	m_dThreadId = GetCurrentThreadId();

	// Create the OpenGL Rendering Context.
	hrc = wglCreateContext(hdc);
	hrc_loader = wglCreateContext(hdc);
	wglShareLists(hrc, hrc_loader);
	wglMakeCurrent(hdc, hrc);

	GLenum rc = glewInit();
	if (rc != GLEW_OK) {
		_D(L"%s(%d) glewInit() failed!\n", __FILE__, __LINE__);
	}

	wglSwapIntervalEXT(0);

	glGenTextures(23, m_texture);
	GL_CHECK_ERROR();
	glGenTextures(MAX_SPLIT_WINDOW, m_image_texture);
	GL_CHECK_ERROR();
	glGenTextures(MAX_SPLIT_WINDOW, m_region_texture);	// Beagle 20130822
	GL_CHECK_ERROR();
	glGenTextures(MAX_DEFECTS, m_piece_texture);
	GL_CHECK_ERROR();
	glGenTextures(MAX_DEFECTS, m_piece_tex_shade);
	GL_CHECK_ERROR();

	// Beagle 20120301 added.
	m_showBarcodeDigitsImage = FALSE;
	m_width_BarcodeDigits = 0;
	m_height_BarcodeDigits = 0;
	memset(m_pos_BarcodeDigits, 0, sizeof(m_pos_BarcodeDigits));
	glGenTextures(10, m_texBarcodeDigits);
	GL_CHECK_ERROR();

#ifdef CUDA_OPENGL_INTEROP
	glGenBuffers(2, m_texBuffer);
	GL_CHECK_ERROR();
#endif

	oglResetTexture();

	//glShadeModel(GL_FLAT);

	// Set color to use when clearing the background.
	glClearColor((float)GetRValue(m_glBackColor)/255.0f,
				(float)GetGValue(m_glBackColor)/255.0f,
				(float)GetBValue(m_glBackColor)/255.0f, 1.0f);
	glClearDepth(1.0f);

	// Turn on back face culling
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	// Turn on depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Setup blending function.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void COpenGLControl::OnDestroy()
{
	CWnd::OnDestroy();

	KillTimer(IDT_GL_TIMER);	// Beagle 20120510 added.

	// OpenGL release
	if (wglGetCurrentContext() != NULL)
		wglMakeCurrent(NULL, NULL);

	if (hrc != NULL)
	{
		wglDeleteContext(hrc);
		hrc = NULL;
	}

	if (hrc_loader != NULL)
	{
		wglDeleteContext(hrc_loader);
		hrc_loader = NULL;
	}

	// added by eric at 20120330
	if (m_tracker)
	{
		delete m_tracker;
		m_tracker = NULL;
	}

	// added by eric at 20130208
	if (m_pLineTracker) {
		delete m_pLineTracker;
		m_pLineTracker = NULL;
	}

	if (m_xLuminance.dataLuminance!=NULL) {	// Beagle 20130816
		delete m_xLuminance.dataLuminance;
		m_xLuminance.dataLuminance = NULL;
	}
	if (m_xLuminance.dataRed!=NULL) {		// Beagle 20130816
		delete m_xLuminance.dataRed;
		m_xLuminance.dataRed = NULL;
	}
	if (m_xLuminance.dataGreen!=NULL) {		// Beagle 20130816
		delete m_xLuminance.dataGreen;
		m_xLuminance.dataGreen = NULL;
	}
	if (m_xLuminance.dataBlue!=NULL) {		// Beagle 20130816
		delete m_xLuminance.dataBlue;
		m_xLuminance.dataBlue = NULL;
	}
    for ( int i = NULL; i < MAX_SPLIT_WINDOW; i++ ) m_OGLSegment[ i ].UnsetSegmentTexture();
    for ( int i = NULL; i < MAX_SPLIT_WINDOW; i++ ) m_OGLRolling[ i ].RunRollingMode( FALSE );
}

// Beagle 20130621 modified.
void COpenGLControl::oglDrawScene(BOOL isPaint)
{
	//20170906
	int nProcessId = GetCurrentThreadId();
	if (m_dThreadId != nProcessId){
		_D(L"[ERROR] Process in Other Thread!");
		return;
	}
	if (m_enable_redraw_interval == TRUE) {
		LARGE_INTEGER now_time;
		QueryPerformanceCounter(&now_time);
		if (now_time.QuadPart-m_draw_lasttime>m_draw_interval) {
			m_draw_lasttime = now_time.QuadPart;
			DrawScene(isPaint);
			PostDrawScene(isPaint);
			m_need_redraw = FALSE;
		} else {
			m_need_redraw = TRUE;
		}
	} else {	// m_enable_redraw_interval == FALSE
		DrawScene(isPaint);
		PostDrawScene(isPaint);
	}
}

void COpenGLControl::DrawScene(BOOL isPaint)
{
	oglMakeCurrent();

	// Clear color and depth buffer bits
	glClearColor((float)GetRValue(m_glBackColor)/255.0f,
				(float)GetGValue(m_glBackColor)/255.0f,
				(float)GetBValue(m_glBackColor)/255.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	switch (m_display_mode) {	// Beagle 20140711
	case DISPLAY_MODE_NORMAL:
	default:
		if (m_wsplit == 1 && m_hsplit == 1) {	// No split.
			glDisable(GL_SCISSOR_TEST);
			glLoadIdentity();
			DrawSplit(0, 0);
		} else {						// Split window.
			glEnable(GL_SCISSOR_TEST);
			for (int h=0; h<m_hsplit; h++) for (int w=0; w<m_wsplit; w++) {
				int split_id = h*m_wsplit+w;
				if (m_splitEnable[split_id] == true) {
					glLoadIdentity();
					glScissor(m_glwidth*w/m_wsplit, m_glheight*(m_hsplit-h-1)/m_hsplit,
						m_glwidth/m_wsplit, m_glheight/m_hsplit);
					glTranslatef(m_glwidth*((0.5f+w)/m_wsplit-0.5f), m_glheight*((0.5f+h)/m_hsplit-0.5f), 0.0f);
					DrawSplit(w, h);
				}
			}
			glDisable(GL_SCISSOR_TEST);
		}
		break;
	case DISPLAY_MODE_DEFECT:
		DrawDefect();
		break;
	case DISPLAY_MODE_DEFECT_PIECE:
		DrawDefectPiece();
		break;
	case DISPLAY_MODE_DEFECT_STACK:
		DrawDefectStack();
		break;
	}

	// Unique objects.
	oglDrawLargeNumber();
	oglDrawInspectionMark();
	oglDrawInspectionText();
	oglDrawOKNGText();	// added by eric at 20120821
	DrawRadioButton();	// Beagle 20130807

	// End
	SwapBuffers(hdc);
}

void COpenGLControl::oglSetSplit(int split_window, BOOL origin)
{
	switch (split_window) {
	case 2:
		if (origin==TRUE) oglSetSplit_New(1, 2, FALSE);
		else oglSetSplit_New(2, 1, FALSE);
		break;
	case 3:
	case 4:
		oglSetSplit_New(2, 2, FALSE);
		break;
	case 5: //eric chao 20130703
	case 6:
		oglSetSplit_New(3, 2, FALSE);
		break;
	case 7:
	case 8:
		oglSetSplit_New(3, 3, FALSE);
		break;
	case 1:
	default:
		oglSetSplit_New(1, 1, FALSE);
		break;
	}

	for (int c=0; c<MAX_SPLIT_WINDOW; c++) {
		m_splitEnable[c] = (c<split_window ? true : false);
	}
}

// Beagle 20111103
void COpenGLControl::oglSetSplit_New(int split_w, int split_h, BOOL clearImage)
{
	int split_window = split_w * split_h;
	if (split_window >= MAX_SPLIT_WINDOW) return;

	m_wsplit = split_w;
	m_hsplit = split_h;

	if (clearImage) {
		for (int c=0; c<MAX_SPLIT_WINDOW; c++) {
			m_width[c] = 0;
			m_height[c] = 0;
			m_splitEnable[c] = false;
		}
	}

	oglSetNormalTranslate();
}

void COpenGLControl::oglDrawSplitMark(int split_x, int split_y)
{
	int split_id = split_y * m_wsplit + split_x;

	if (!m_show_inspection_mark) return;	// Beagle 20111108
	if (split_x >= m_wsplit || split_y >= m_hsplit) return;

	// Beagle 20111104 modified.
	glLoadIdentity();
	glTranslatef(((split_x+1.0f)/m_wsplit-0.5f)*m_glwidth-16.0f,
		((split_y+1.0f)/m_hsplit-0.5f)*m_glheight-16.0f, 0.0f);

	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glBindTexture(GL_TEXTURE_2D, m_texture[6]);
	glBlendFunc(GL_DST_COLOR,GL_ZERO);  // mask
	if (m_split_inspection_mark[split_id])
		{ glBindTexture(GL_TEXTURE_2D, m_texture[4]); }	// ok mask
	else
		{ glBindTexture(GL_TEXTURE_2D, m_texture[6]); }	// fail mask
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex3f(-16.0f, -16.0f, 0.0f);
		glTexCoord2f(1, 0);
		glVertex3f( 16.0f, -16.0f, 0.0f);
		glTexCoord2f(1, 1);
		glVertex3f( 16.0f,  16.0f, 0.0f);
		glTexCoord2f(0, 1);
		glVertex3f(-16.0f,  16.0f, 0.0f);
	glEnd();
	glBlendFunc(GL_ONE, GL_ONE);
	if (m_split_inspection_mark[split_id]) 
		glBindTexture(GL_TEXTURE_2D, m_texture[3]);	// ok sign
	else
		glBindTexture(GL_TEXTURE_2D, m_texture[5]);	// fail sign
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex3f(-16.0f, -16.0f, 0.0f);
		glTexCoord2f(1, 0);
		glVertex3f( 16.0f, -16.0f, 0.0f);
		glTexCoord2f(1, 1);
		glVertex3f( 16.0f,  16.0f, 0.0f);
		glTexCoord2f(0, 1);
		glVertex3f(-16.0f,  16.0f, 0.0f);
	glEnd();
}

void COpenGLControl::DrawSplit(int split_x, int split_y)
{
	int split_id;

	if (split_x >= m_wsplit || split_y >= m_hsplit) return;

	if (m_nRadioButton > 0) {	// Beagle 20130807
		split_id = m_nRadioSelected;
	} else {
		split_id = split_y * m_wsplit + split_x;
	}

	if (m_width[split_id] <= 0 || m_height[split_id] <= 0) return;	// Beagle 20111101 modified.

    if ( m_OGLRolling[ split_id ].IsRollingMode() )
    {
        m_OGLRolling[ split_id ].DrawRolling();
    }
    else if ( m_OGLSegment[ split_id ].IsSegmentation() )
    {
        m_OGLSegment[ split_id ].DrawSegment();
    }
    else
    {
	    glPushMatrix();

	    // Beagle 20111104 modified.
	    glScalef(m_glScale, m_glScale, 1.0f);
	    glScalef(m_norm_scale[split_id], m_norm_scale[split_id], 1.0f);
	    glTranslatef(m_xTranslate/m_norm_scale[split_id], m_yTranslate/m_norm_scale[split_id], 0.0f);
	    glTranslatef(-0.5f*m_width[split_id], -0.5f*m_height[split_id], 0.0f);

	    glGetDoublev(GL_MODELVIEW_MATRIX, m_modelview[split_id]);
	    glGetDoublev(GL_PROJECTION_MATRIX, m_projection[split_id]);

	    glPushMatrix();

	    // If the current view is perspective...
	    glDisable(GL_BLEND);
	    glEnable(GL_TEXTURE_2D);

	    glBindTexture(GL_TEXTURE_2D, m_image_texture[split_id]);
	    glBegin(GL_QUADS);
		    glColor3f(1, 1, 1);
		    glTexCoord2f(0.0f, 0.0f);
		    glVertex3f(0.0f, 0.0f, 0.0f);
		    glTexCoord2f(0.0f, 1.0f);
		    glVertex3f(0.0f, m_height[split_id]*1.0f, 0.0f);	// Beagle 20111101 modified.
		    glTexCoord2f(1.0f, 1.0f);
		    glVertex3f(m_width[split_id]*1.0f, m_height[split_id]*1.0f, 0.0f);	// Beagle 20111101 modified.
		    glTexCoord2f(1.0f, 0.0f);
		    glVertex3f(m_width[split_id]*1.0f, 0.0f, 0.0f);	// Beagle 20111101 modified.
	    glEnd();
    }
	// added by danny, show inspection region
	if (m_showInspectionRgn == TRUE)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
		//glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
		glBindTexture(GL_TEXTURE_2D, m_region_texture[split_id]);	// Beagle 20130822 modified.
		GLenum error = glGetError();
		glBegin(GL_QUADS);
		glColor3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(0.0f, m_height[split_id]*1.0f, 0.0f);	// Beagle 20111101 modified.
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(m_width[split_id]*1.0f, m_height[split_id]*1.0f, 0.0f);	// Beagle 20111101 modified.
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(m_width[split_id]*1.0f, 0.0f, 0.0f);	// Beagle 20111101 modified.
		glEnd();
	}
	else{
		if (m_nBoundedType == TYPE_INSPREGION){//moving region
			if (m_nSelectedIndex >=0 &&m_nSelectedIndex < m_inspectRegion.size()) {
				CRgn *pCurRgn = m_inspectRegion[m_nSelectedIndex];
				RECT rcCurRgn;
				pCurRgn->GetRgnBox(&rcCurRgn);
				int nXShift = rcCurRgn.left - m_rtSelected.left;
				int nYShift = rcCurRgn.top -  m_rtSelected.top;					
				if(nXShift!=0||nYShift!=0){	
					glColor3f(1.0f, 0.0f, 0.0f); 
					glEnable(GL_COLOR_LOGIC_OP); 
					glDisable(GL_TEXTURE_2D);
					glLogicOp(GL_OR_REVERSE);
					int iCount = pCurRgn->GetRegionData(NULL,0);					
					if(m_pMovingRgn==NULL||iCount>m_nMovingRgnCntBuf){
						if(m_pMovingRgn){
							delete [] m_pMovingRgn;
							m_pMovingRgn = NULL;
						}
						m_pMovingRgn = new unsigned char[iCount];
						m_nMovingRgnCntBuf = iCount;
					}
					pCurRgn->GetRegionData((LPRGNDATA)m_pMovingRgn,iCount);
					LPRGNDATA pInfo = (LPRGNDATA)m_pMovingRgn;
					LPRECT pRect = (LPRECT)pInfo->Buffer;
					int iTotalRgn = pInfo->rdh.nCount;
					for (int i=0;i<iTotalRgn;i++){
						LPRECT pCurrnet = pRect+i;
						int left = pCurrnet->left - nXShift;
						int right = pCurrnet->right - nXShift;
						int top = pCurrnet->top - nYShift;
						int bottom = pCurrnet->bottom - nYShift;
						glBegin(GL_QUADS);
						glVertex3f(left +0.0f, top+0.0f, 0);
						glVertex3f(right-0.0f, top+0.0f, 0);
						glVertex3f(right-0.0f, bottom-0.0f, 0);
						glVertex3f(left +0.0f, bottom-0.0f, 0);
						glEnd();
					}
				}
			}
		}
	
	}

	
	// added by eric at 20121008
	if (m_isPatternMode == TRUE) {
		if (m_isShowPatternRect == TRUE) {
			glEnable(GL_COLOR_LOGIC_OP); 
			glDisable(GL_TEXTURE_2D);
			glLineWidth(1.0f);
			glColor3f(0.2f, 0.8f, 0.2f);
			for (int c=0; c<m_rectsPattern; c++) {
				BOOL bfind = FALSE;

                for ( int i = NULL; i < m_mapRectInfo[ NULL ].rects; i++ )
                {
                    if ( NULL == memcmp( &m_mapRectInfo[ NULL ].rect[ i ], &m_rectPattern[ i ], sizeof( RECT ) ) )
                    {
                        bfind = TRUE;
                        break;
                    }
                }
				//for (int c1=0; c1<m_rects; c1++) {
				//	if (m_rect[c1].left == m_rectPattern[c].left &&
				//		m_rect[c1].top == m_rectPattern[c].top &&
				//		m_rect[c1].right == m_rectPattern[c].right &&
				//		m_rect[c1].bottom == m_rectPattern[c].bottom) {
				//			bfind = TRUE;
				//			break;
				//	}
				//}
				if (bfind == FALSE) {
					glBegin(GL_LINE_LOOP);
					glVertex3f((GLfloat)m_rectPattern[c].left, (GLfloat)m_rectPattern[c].top, 0);
					glVertex3f((GLfloat)m_rectPattern[c].right, (GLfloat)m_rectPattern[c].top, 0);
					glVertex3f((GLfloat)m_rectPattern[c].right, (GLfloat)m_rectPattern[c].bottom, 0);
					glVertex3f((GLfloat)m_rectPattern[c].left, (GLfloat)m_rectPattern[c].bottom, 0);
					glEnd();
				}
			}
		}
	}

	// added by eric at 20121017
	if (m_isShowPatternSegment == TRUE && m_rtSegment.IsRectEmpty() == FALSE) {
		glEnable(GL_COLOR_LOGIC_OP); 
		glDisable(GL_TEXTURE_2D);
		glLineWidth(3.0f);
		glColor3f(0.0f,0.0f,1.0f);
		glBegin(GL_LINE_LOOP);
		glVertex3f((GLfloat)m_rtSegment.left, (GLfloat)m_rtSegment.top, 0);
		glVertex3f((GLfloat)m_rtSegment.right, (GLfloat)m_rtSegment.top, 0);
		glVertex3f((GLfloat)m_rtSegment.right, (GLfloat)m_rtSegment.bottom, 0);
		glVertex3f((GLfloat)m_rtSegment.left, (GLfloat)m_rtSegment.bottom, 0);
		glEnd();
	}
	if ( m_bShowMatrixLine ) //20170717
    {
        DrawMaxtrixLine();
	}
	// added by eric at 20121121
	if (m_bShowLineShape == TRUE) {
		glEnable(GL_COLOR_LOGIC_OP); 
		glDisable(GL_TEXTURE_2D);

		for (int c=0; c<m_lineRects; c++) {
			if (c == m_selLine)
			{
				glLineWidth(3.2f);
				glColor3f(0.0f, 1.0f, 0.0f);
			}
			else
			{
				glLineWidth(2.0f);
				glColor3f(1.0f, 0.0f, 1.0f);
			}
			glBegin(GL_LINES);
			glVertex3d(m_lineRect[c].left, m_lineRect[c].top, 0);
			glVertex3d(m_lineRect[c].right, m_lineRect[c].bottom, 0);
			glEnd();
			// added by eric at 20161217
			if (m_bShowLineString) {
				if (m_nShootFormatDist != 1) {
					CString mm;
					if (m_nEnableMap & (1 << c)){ //if Enable InkJet
						mm.Format(_T("%s %d"), m_tLineLabel, c + 1);
					}
					else {
						mm.Format(_T("%s %d (%s)"), m_tLineLabel, c + 1, m_tDisable);
					}
					int x = 0, y = (c % 2 == 0) ? 40 : 60;
					oglTranslatePoint(x, y, x, y);
					if (m_nEnableMap & (1 << c)){ //if Enable InkJet
						x = m_lineRect[c].right - 100;
					}
					else {
						x = m_lineRect[c].right - 250;
					}
					DrawPrintf(split_id, ALIGN_SHEET | ALIGN_LEFT | ALIGN_TOP, x, y, mm);
				}
			}
		}

		if (m_selLine!=-1) {
			CString tString;
			int dist=m_lineRect[m_selLine].left;
			if (m_bShowLineString) {
				if (m_nShootFormatDist != 1) {
					if (m_width[0]!=0) {
						dist=m_nShootFormatDist*m_lineRect[m_selLine].left/m_width[0];
						tString.Format(_T("%s %d:%d Piexls, %d mm"), m_tLineLabel, m_selLine+1, m_lineRect[m_selLine].left, dist);
					} else {
						tString.Format(_T("%d Piexls"), dist);
					}
				}
				else {
					tString.Format(_T("%d Piexls"), dist);
				}
				oglPrintf(ALIGN_RIGHT | ALIGN_TOP | ALIGN_WINDOW, 0, 0, _T("%s"), tString);
			}
		}
		// added by eric at 20130128
		if (m_bShowAngleString) {
			if (m_AngleX>0.5f && m_AngleY>0.5f) {
				if(m_AngleAlignType==ALIGN_LEFT)
					DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP, m_AngleX, m_AngleY, m_AngleString);
				else
					DrawPrintf(split_id, ALIGN_SHEET|ALIGN_RIGHT|ALIGN_TOP, m_AngleX-m_width[split_id], m_AngleY, m_AngleString);
			}
		}
	}

	// added by eric at 20130923
	if (m_bEnableColorDerivateBox) {
		glEnable(GL_COLOR_LOGIC_OP); 
		glDisable(GL_TEXTURE_2D);
		glLineWidth(3.2f);
		for (int i=0; i<(int)m_vColorDerivateBox.size(); i++) {
			if (i == m_nSelectedIndex) {
				continue;
			}
			if (i==0) {
				glColor3f(0.2f, 0.8f, 0.2f);
			} else if (i==1) {
				glColor3f(0.0f, 1.0f, 1.0f);
			} else if (i==2) {
				glColor3f(1.0f, 0.0f, 1.0f);
			} else if (i==3) {
				glColor3f(1.0f, 1.0f, 0.0f);
			}
			glBegin(GL_LINE_LOOP);
			glVertex3f((GLfloat)m_vColorDerivateBox[i].left, (GLfloat)m_vColorDerivateBox[i].top, 0);
			glVertex3f((GLfloat)m_vColorDerivateBox[i].right, (GLfloat)m_vColorDerivateBox[i].top, 0);
			glVertex3f((GLfloat)m_vColorDerivateBox[i].right, (GLfloat)m_vColorDerivateBox[i].bottom, 0);
			glVertex3f((GLfloat)m_vColorDerivateBox[i].left, (GLfloat)m_vColorDerivateBox[i].bottom, 0);
			glEnd();
		}
	}

	// added by eric at 20150211
	if (m_bShowSpecialRectBox) {
		glDisable(GL_TEXTURE_2D);
		glLineWidth(2.0f);
		for (int i=0; i<(int)m_vSpecialRectBox.size(); i++) {
			glColor3f(1.0f, 0.0f, 0.0f);
			glBegin(GL_LINE_LOOP);
			glVertex3f((GLfloat)m_vSpecialRectBox[i].left, (GLfloat)m_vSpecialRectBox[i].top, 0);
			glVertex3f((GLfloat)m_vSpecialRectBox[i].right, (GLfloat)m_vSpecialRectBox[i].top, 0);
			glVertex3f((GLfloat)m_vSpecialRectBox[i].right, (GLfloat)m_vSpecialRectBox[i].bottom, 0);
			glVertex3f((GLfloat)m_vSpecialRectBox[i].left, (GLfloat)m_vSpecialRectBox[i].bottom, 0);
			glEnd();
		}
	}

	// added by eric at 20121212
	if (m_bShiftMode == TRUE) {
		glEnable(GL_COLOR_LOGIC_OP); 
		glDisable(GL_TEXTURE_2D);
		glLineWidth(3.0f);
		glColor3f(0.0f,0.0f,1.0f);
		glBegin(GL_LINE_LOOP);
		glVertex3f((GLfloat)m_rtShiftShape.left, (GLfloat)m_rtShiftShape.top, 0);
		glVertex3f((GLfloat)m_rtShiftShape.right, (GLfloat)m_rtShiftShape.top, 0);
		glVertex3f((GLfloat)m_rtShiftShape.right, (GLfloat)m_rtShiftShape.bottom, 0);
		glVertex3f((GLfloat)m_rtShiftShape.left, (GLfloat)m_rtShiftShape.bottom, 0);
		glEnd();
	}

	// added by eric at 20130107
	if (m_bShowLayout == TRUE) {
		glDisable(GL_TEXTURE_2D);
		glLineWidth(3.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glLogicOp(GL_OR_REVERSE);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x00AA);
		for (int i=0; i<m_layoutRects; i++) {
			glBegin(GL_LINE_LOOP);
			glVertex3f((GLfloat)m_layoutRect[i].left, (GLfloat)m_layoutRect[i].top, 0);
			glVertex3f((GLfloat)m_layoutRect[i].right, (GLfloat)m_layoutRect[i].top, 0);
			glVertex3f((GLfloat)m_layoutRect[i].right, (GLfloat)m_layoutRect[i].bottom, 0);
			glVertex3f((GLfloat)m_layoutRect[i].left, (GLfloat)m_layoutRect[i].bottom, 0);
			glEnd();
		}
		glDisable(GL_LINE_STIPPLE);
	}

	// added by eric at 20130208, Beagle 20130423 modified.
	if (m_bShowShadowRectShape == TRUE) {
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_COLOR_LOGIC_OP);
		glLogicOp(GL_OR_REVERSE);
		glLineWidth(3.0f);
		for (int i=0; i<(int)m_vShadowInfo.size(); i++) {
			RECT rt = m_vShadowInfo[i].shadowShape;
			int direction = m_vShadowInfo[i].direction;

			if (m_nBlockDirection == direction) { glColor3f(0.0f, 1.0f, 0.0f); }
			else { glColor3f(0.9f, 0.9f, 0.9f); }

			if (direction == EMBOSS_LEFT_UP || direction == EMBOSS_RIGHT_DOWN ||
				direction == EMBOSS_LU_RD || direction == EMBOSS_LEFT_DOWN ||
				direction == EMBOSS_RIGHT_UP || direction == EMBOSS_LD_RU)
			{
				float cx = (rt.left+rt.right)/2.0f+0.5f;
				float cy = (rt.top+rt.bottom)/2.0f+0.5f;

				glBegin(GL_LINE_LOOP);
					glVertex3f(cx, rt.top+0.5f, 0);
					glVertex3f(rt.right+0.5f, cy, 0);
					glVertex3f(cx, rt.bottom+0.5f, 0);
					glVertex3f(rt.left+0.5f, cy, 0);
				glEnd();
			} else {
				glBegin(GL_LINE_LOOP);
					glVertex3f(rt.left+0.5f,  rt.top+0.5f,    0);
					glVertex3f(rt.right+0.5f, rt.top+0.5f,    0);
					glVertex3f(rt.right+0.5f, rt.bottom+0.5f, 0);
					glVertex3f(rt.left+0.5f,  rt.bottom+0.5f, 0);
				glEnd();
			}
		}
	}
	// added by eric at 20150420
	if (m_bAnchorRect) {
		glEnable(GL_COLOR_LOGIC_OP); 
		glDisable(GL_TEXTURE_2D);
		glLineWidth(1.2f);
		glColor3f(0.2f, 0.2f, 0.8f);
		glLogicOp(GL_OR_REVERSE);
		glBegin(GL_LINE_LOOP);
		glVertex3f((GLfloat)m_rtAnchorRect.left, (GLfloat)m_rtAnchorRect.top, 0);
		glVertex3f((GLfloat)m_rtAnchorRect.right, (GLfloat)m_rtAnchorRect.top, 0);
		glVertex3f((GLfloat)m_rtAnchorRect.right, (GLfloat)m_rtAnchorRect.bottom, 0);
		glVertex3f((GLfloat)m_rtAnchorRect.left, (GLfloat)m_rtAnchorRect.bottom, 0);
		glEnd();
	}

	if (m_isShowSegment == TRUE && m_isShowBarcode == FALSE && !m_OGLRolling[ split_id ].IsRollingMode() )
	{
		glEnable(GL_COLOR_LOGIC_OP); 
		glDisable(GL_TEXTURE_2D);
		// Render rectangles, circles, ellipses.
		BOOL found;
		BOOL bFindReposAnchorRange,bFindAnchorRange; //seanchen 20130820

        if ( m_mapRectInfo.find( split_id ) != m_mapRectInfo.end() )
        {
		    for (int c=0; c<m_mapRectInfo[ split_id ].rects; c++) 
		    {	
                //if ( ::IsRectEmpty( &m_mapRectInfo[ split_id ].rect[c] ) )
                //{
                //    continue;
                //}
			    bFindReposAnchorRange = FALSE;	//seanchen 20130820
			    bFindAnchorRange = FALSE;		//seanchen 20130820
			    found = FALSE;
			    for (int r=0; r<(int)m_selRectangle.size(); r++)
			    {
				    if (m_selRectangle[r] == c)
				    {
					    found = TRUE;
					    break;
				    }
			    }

			    if((c==m_selReposAnchorRange)||(c==m_selReposAnchorRange+1)) //seanchen 20130820
			    {
				    bFindReposAnchorRange = TRUE;
				    found = FALSE;
			    }

			    if((c==m_selAnchorRange)||(c==m_selAnchorRange+1)) //seanchen 20130820
			    {
				    bFindAnchorRange = TRUE;
				    found = FALSE;
			    }

			    if (m_lmMode == LM_SET_BARCODEANCHOR)	// added by eric at 20110801
			    {
				    glLineWidth(1.2f);
				    glColor3f(1.0f, 0.2f, 0.2f);
				    glLogicOp(GL_OR_REVERSE);
			    }
			    else
			    {
				    if (found == TRUE)	//(c == m_selRectangle)
				    {
					    glLineWidth(3.2f);
					    //glColor3f(1.0f, 1.0f, 0.0f);
					    glColor3f(0.2f, 0.8f, 0.2f);
					    glLogicOp(GL_OR_REVERSE);
	
				    }
				    else if(bFindAnchorRange){ //seanchen 20130820
					    glLineWidth(3.2f);
					    glColor3f(1.0f, 1.0f, 0.0f);
					    glLogicOp(GL_OR_REVERSE);
				    }
				    else if(bFindReposAnchorRange){ //seanchen 20130820
					    glLineWidth(3.2f);
					    glColor3f(0.0f, 1.0f, 0.0f);
				    }
				    else
				    {
					    if (m_isPatternMode == TRUE) {	// modified by eric at 20121008
						    glLineWidth(3.0f);
						    glColor3f(1.0f, 0.0f, 1.0f);
					    }else{
						    glLineWidth(1.2f);
						    glColor3f(0.2f, 0.2f, 0.8f);
						    glLogicOp(GL_OR_REVERSE);
					    }
				    }
			    }

			    switch (m_mapRectInfo[ split_id ].rect_type[c]) {
			    case SHAPE_ANCHOR_RECT:	//seanchen 20130820
			    case SHAPE_RECT:
				    if (m_showOCRSize && c > 0)	// added by eric at 20110809
					    continue;

				    if(m_mapRectInfo[ split_id ].rect_type[c] == SHAPE_ANCHOR_RECT)
				    {  			
					    glLineWidth(1.7f);
					    glColor3f(1.0f, 1.0f, 0.0f);
				    }

				    {
					    int	left, right, top, bottom;
					    if(m_mapRectInfo[ split_id ].rect[c].bottom<m_mapRectInfo[ split_id ].rect[c].top){
						    top = m_mapRectInfo[ split_id ].rect[c].bottom;
						    bottom = m_mapRectInfo[ split_id ].rect[c].top;
					    } else {
						    top = m_mapRectInfo[ split_id ].rect[c].top;
						    bottom = m_mapRectInfo[ split_id ].rect[c].bottom;
					    }
					    if(m_mapRectInfo[ split_id ].rect[c].right<m_mapRectInfo[ split_id ].rect[c].left){
						    right = m_mapRectInfo[ split_id ].rect[c].left;
						    left = m_mapRectInfo[ split_id ].rect[c].right;
					    } else {
						    right = m_mapRectInfo[ split_id ].rect[c].right;
						    left = m_mapRectInfo[ split_id ].rect[c].left;
					    }
					    // modified by amike at 20130109
					    glBegin(GL_LINE_LOOP);
						    glVertex3f(left +0.1f, top+0.1f, 0);
						    glVertex3f(right-0.1f, top+0.1f, 0);
						    glVertex3f(right-0.1f, bottom-0.1f, 0);
						    glVertex3f(left +0.1f, bottom-0.1f, 0);
					    glEnd();
					    // added by amike at 20120907
					    if(m_show_rect_color){
						    glPushMatrix();
						    glTranslatef((float)left, (float)top, 0.0f);
						    if(m_glScale<10.0f) {
							    glScalef(10.0f/m_glScale/m_norm_scale[0],
								    10.0f/m_glScale/m_norm_scale[0], 1.0f);
						    } else {
							    glScalef(1.0f/m_norm_scale[0],
								    1.0f/m_norm_scale[0], 1.0f);
						    }
						    oglDrawBilliardBall(m_mapRectInfo[ split_id ].rect_color[c], c+1);
						    glPopMatrix();
					    }
				    }

				    // modified by danny at 20100208
				    //if (fabsf((m_rect[c].right-m_rect[c].left)*m_glScale*m_norm_scale)<30.0f && fabsf((m_rect[c].bottom-m_rect[c].top)*m_glScale*m_norm_scale)<30.0f)
				    //oglDrawCorona((m_rect[c].left+m_rect[c].right+1)/2.0f, (m_rect[c].top+m_rect[c].bottom+1)/2.0f, m_glScale*m_norm_scale);
				    if(!m_show_rect_color){
					    if ((fabsf((float)(m_mapRectInfo[ split_id ].rect[c].right-m_mapRectInfo[ split_id ].rect[c].left))<10.0f) && (fabsf((float)(m_mapRectInfo[ split_id ].rect[c].bottom-m_mapRectInfo[ split_id ].rect[c].top))<10.0f))
					    {
						    int x1, y1, x2, y2;
						    oglProjectPoint(x1, y1, m_mapRectInfo[ split_id ].rect[c].left, m_mapRectInfo[ split_id ].rect[c].top);
						    oglProjectPoint(x2, y2, m_mapRectInfo[ split_id ].rect[c].right, m_mapRectInfo[ split_id ].rect[c].bottom);

						    if ((abs(x1-x2)<10) && (abs(y1-y2)<10)) {
							    oglDrawCorona((m_mapRectInfo[ split_id ].rect[c].left+m_mapRectInfo[ split_id ].rect[c].right+1)/2.0f,
								    (m_mapRectInfo[ split_id ].rect[c].top+m_mapRectInfo[ split_id ].rect[c].bottom+1)/2.0f, 2);
						    }
					    }
				    }
				    break;
			    case SHAPE_CIRCLE:
			    case SHAPE_ELLIPSE:
				    {
					    int x, y, dx, dy;

					    x = (m_mapRectInfo[ split_id ].rect[c].left + m_mapRectInfo[ split_id ].rect[c].right)/2;
					    y = (m_mapRectInfo[ split_id ].rect[c].top + m_mapRectInfo[ split_id ].rect[c].bottom)/2;
					    dx = abs(m_mapRectInfo[ split_id ].rect[c].left - m_mapRectInfo[ split_id ].rect[c].right)/2;
					    dy = abs(m_mapRectInfo[ split_id ].rect[c].top - m_mapRectInfo[ split_id ].rect[c].bottom)/2;
					    // modified by danny
					    if (m_mapRectInfo[ split_id ].rect_type[c] == SHAPE_CIRCLE)
					    {
						    if (dx > dy)
							    dx = dy;
						    else
							    dy = dx;
					    }

					    // modified at 20100208
					    //x = m_rect[c].left + dx;
					    //y = m_rect[c].top + dy;

					    //glBegin(GL_LINE_LOOP);
					    glBegin(GL_POLYGON);
					    glVertex3f(x+dx*1.000000f, y+dy*0.000000f, 0.0f);
					    glVertex3f(x+dx*0.980785f, y+dy*0.195090f, 0.0f);
					    glVertex3f(x+dx*0.923880f, y+dy*0.382683f, 0.0f);
					    glVertex3f(x+dx*0.831470f, y+dy*0.555570f, 0.0f);
					    glVertex3f(x+dx*0.707107f, y+dy*0.707107f, 0.0f);
					    glVertex3f(x+dx*0.555570f, y+dy*0.831470f, 0.0f);
					    glVertex3f(x+dx*0.382683f, y+dy*0.923880f, 0.0f);
					    glVertex3f(x+dx*0.195090f, y+dy*0.980785f, 0.0f);
					    glVertex3f(x+dx*-0.000000f, y+dy*1.000000f, 0.0f);
					    glVertex3f(x+dx*-0.195090f, y+dy*0.980785f, 0.0f);
					    glVertex3f(x+dx*-0.382683f, y+dy*0.923880f, 0.0f);
					    glVertex3f(x+dx*-0.555570f, y+dy*0.831470f, 0.0f);
					    glVertex3f(x+dx*-0.707107f, y+dy*0.707107f, 0.0f);
					    glVertex3f(x+dx*-0.831470f, y+dy*0.555570f, 0.0f);
					    glVertex3f(x+dx*-0.923880f, y+dy*0.382683f, 0.0f);
					    glVertex3f(x+dx*-0.980785f, y+dy*0.195090f, 0.0f);
					    glVertex3f(x+dx*-1.000000f, y+dy*-0.000000f, 0.0f);
					    glVertex3f(x+dx*-0.980785f, y+dy*-0.195090f, 0.0f);
					    glVertex3f(x+dx*-0.923880f, y+dy*-0.382683f, 0.0f);
					    glVertex3f(x+dx*-0.831470f, y+dy*-0.555570f, 0.0f);
					    glVertex3f(x+dx*-0.707107f, y+dy*-0.707107f, 0.0f);
					    glVertex3f(x+dx*-0.555570f, y+dy*-0.831469f, 0.0f);
					    glVertex3f(x+dx*-0.382684f, y+dy*-0.923879f, 0.0f);
					    glVertex3f(x+dx*-0.195090f, y+dy*-0.980785f, 0.0f);
					    glVertex3f(x+dx*0.000000f, y+dy*-1.000000f, 0.0f);
					    glVertex3f(x+dx*0.195090f, y+dy*-0.980785f, 0.0f);
					    glVertex3f(x+dx*0.382684f, y+dy*-0.923879f, 0.0f);
					    glVertex3f(x+dx*0.555570f, y+dy*-0.831470f, 0.0f);
					    glVertex3f(x+dx*0.707107f, y+dy*-0.707107f, 0.0f);
					    glVertex3f(x+dx*0.831470f, y+dy*-0.555570f, 0.0f);
					    glVertex3f(x+dx*0.923880f, y+dy*-0.382683f, 0.0f);
					    glVertex3f(x+dx*0.980785f, y+dy*-0.195090f, 0.0f);
					    glEnd();
				    }
				    break;
			    default:
				    break;
			    }
		    }
        }
		// Render polygon.
		for (int c=0; c<m_polygons; c++) 
		{
			found = FALSE;
			for (int r=0; r<(int)m_selPolygon.size(); r++)
			{
				if (m_selPolygon[r] == c)
				{
					found = TRUE;
					break;
				}
			}

			if (found == TRUE) //if (c == m_selPolygon)
			{
				glLineWidth(3.2f);
				//glColor3f(1.0f, 1.0f, 0.0f);
				glColor3f(0.2f, 0.8f, 0.2f);
				glLogicOp(GL_OR_REVERSE);
			}
			else
			{
				glLineWidth(1.2f);
				//glColor3f(0.0f, 0.8f, 0.8f);
				glColor3f(0.2f, 0.2f, 0.8f);
				glLogicOp(GL_OR_REVERSE);
				//glLogicOp(GL_INVERT);
			}

			// modified by danny at 20100414, for convex/concave polygon in opengl,
			// opengl"GL_POLYGON" only supports convex type 
			/*
			//glBegin(GL_LINE_LOOP);
			glBegin(GL_POLYGON);
			for (int d=0; d<m_poly_points[c]; d++)
			{ glVertex3i(m_polygon[c][d].x, m_polygon[c][d].y, 0); }
			glEnd();
			*/
			{  
				double temp[100][3];
				for (int i=0; i<m_poly_points[c]; i++)
				{
					temp[i][0] = m_polygon[c][i].x;
					temp[i][1] = m_polygon[c][i].y;
					temp[i][2] = 0;
				}

				glPushMatrix();
				GLUtesselator *tess; 
				tess = gluNewTess();
				gluTessCallback(tess, GLU_BEGIN , (void (__stdcall *)())glBegin);
				gluTessCallback(tess, GLU_VERTEX, (void (__stdcall *)())glVertex3dv);
				gluTessCallback(tess, GLU_END, (void (__stdcall *)())glEnd);
				gluTessBeginPolygon(tess, (GLvoid *)0);
				gluTessBeginContour(tess);
				for (int d=0; d<m_poly_points[c]; d++)
					gluTessVertex(tess, temp[d], temp[d]);

				gluTessEndContour(tess);
				gluTessEndPolygon(tess);
				gluDeleteTess(tess);
				glPopMatrix();
			}			

		}

		// added by eric at 20130529
		for (int i=0; i<(int)m_region.size();i++) {
			if (m_bHideSelectedRegion == TRUE && m_nSelectedIndex == i) {
				continue;
			} else {
				CRect rt;
				m_region[i]->GetRgnBox(&rt);
				glLineWidth(1.2f);
				glColor3f(0.2f, 0.8f, 0.2f);
				glLogicOp(GL_OR_REVERSE);
				glBegin(GL_LINE_LOOP);
				glVertex3f(rt.left +0.1f, rt.top+0.1f, 0);
				glVertex3f(rt.right-0.1f, rt.top+0.1f, 0);
				glVertex3f(rt.right-0.1f, rt.bottom-0.1f, 0);
				glVertex3f(rt.left +0.1f, rt.bottom-0.1f, 0);
				glEnd();
			}
		}

		// show region segments
		// Beagle 20130306 modified.
		if (m_showRegionSegments == TRUE){
			if (m_nBoundedType == TYPE_REGION){ //moving region 
				if (m_nSelectedIndex >=0 &&m_nSelectedIndex < m_region.size()){
					CRgn *pCurRgn = m_region[m_nSelectedIndex];
					RECT rcCurRgn;
					pCurRgn->GetRgnBox(&rcCurRgn);
					int nXShift = rcCurRgn.left - m_rtSelected.left;
					int nYShift = rcCurRgn.top -  m_rtSelected.top;
					if(nXShift!=0||nYShift!=0){
						glColor3f(0.0f, 1.0f, 0.0f); 
						int iCount = pCurRgn->GetRegionData(NULL,0);
						if(m_pMovingRgn==NULL||iCount>m_nMovingRgnCntBuf){
							if(m_pMovingRgn){
								delete [] m_pMovingRgn;
								m_pMovingRgn = NULL;
							}
							m_pMovingRgn = new unsigned char[iCount];
							m_nMovingRgnCntBuf = iCount;
						}
						pCurRgn->GetRegionData((LPRGNDATA)m_pMovingRgn,iCount);
						LPRGNDATA pInfo = (LPRGNDATA)m_pMovingRgn;
						LPRECT pRect = (LPRECT)pInfo->Buffer;
						int iTotalRgn = pInfo->rdh.nCount;
						for (int i=0;i<iTotalRgn;i++){
							LPRECT pCurrnet = pRect+i;
							int left = pCurrnet->left - nXShift;
							int right = pCurrnet->right - nXShift;
							int top = pCurrnet->top - nYShift;
							int bottom = pCurrnet->bottom - nYShift;
							glBegin(GL_QUADS);
							glVertex3f(left +0.0f, top+0.0f, 0);
							glVertex3f(right-0.0f, top+0.0f, 0);
							glVertex3f(right-0.0f, bottom-0.0f, 0);
							glVertex3f(left +0.0f, bottom-0.0f, 0);
							glEnd();
						}										
					}
				}
			}

			glDisable(GL_COLOR_LOGIC_OP); 
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			glBindTexture(GL_TEXTURE_2D, m_texture[11]);
			GLenum error = glGetError();
			if (m_RegionSegmentColor == REGION_SEGMENT_ENABLE_RGB332) {
				glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
				glColor3f(1.0f, 1.0f, 1.0f);
			} else {
				// Mask
				glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
				glBegin(GL_QUADS);
					glColor3f(1.0f, 1.0f, 1.0f);
					glTexCoord2f(0.0f, 0.0f);
					glVertex3f(0.0f, 0.0f, 0.0f);
					glTexCoord2f(0.0f, 1.0f);
					glVertex3f(0.0f, m_height[split_id]*1.0f, 0.0f);
					glTexCoord2f(1.0f, 1.0f);
					glVertex3f(m_width[split_id]*1.0f, m_height[split_id]*1.0f, 0.0f);
					glTexCoord2f(1.0f, 0.0f);
					glVertex3f(m_width[split_id]*1.0f, 0.0f, 0.0f);
				glEnd();
				glBlendFunc(GL_ONE, GL_ONE);
				GLfloat r, g, b;
				r = GetRValue(m_RegionSegmentColor)/255.0f;
				g = GetGValue(m_RegionSegmentColor)/255.0f;
				b = GetBValue(m_RegionSegmentColor)/255.0f;
				glColor3f(r, g, b);
			}
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(0.0f, 0.0f, 0.0f);
				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(0.0f, m_height[split_id]*1.0f, 0.0f);	// Beagle 20111101 modified.
				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(m_width[split_id]*1.0f, m_height[split_id]*1.0f, 0.0f);	// Beagle 20111101 modified.
				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(m_width[split_id]*1.0f, 0.0f, 0.0f);	// Beagle 20111101 modified.
			glEnd();

	

		}
	}
	else
	{
		// added by danny at 20101014, fix render problem
		BOOL needDraw = FALSE;
		if (m_lmMode==LM_DRAW_RECTANGLE || m_lmMode==LM_DRAW_CIRCLE ||
			m_lmMode==LM_DRAW_ELLIPSE || m_lmMode==LM_DRAW_POLYGON)
			needDraw = TRUE;

		if (m_LeftButtonDown==TRUE && needDraw==TRUE)	// modified by danny at 20101014
		{
			if (m_poly_drawing == FALSE && m_mapRectInfo.find( split_id ) != m_mapRectInfo.end() )
			{
				glEnable(GL_COLOR_LOGIC_OP); 
				glDisable(GL_TEXTURE_2D);
				glLineWidth(1.2f);
				glColor3f(0.2f, 0.2f, 0.8f);
				glLogicOp(GL_OR_REVERSE);

				int c = m_mapRectInfo[ split_id ].rects-1;
				switch (m_mapRectInfo[ split_id ].rect_type[c]) 
				{
				case SHAPE_RECT:
					// modified by amike at 20130109
					glBegin(GL_LINE_LOOP);
					glVertex3f(m_mapRectInfo[ split_id ].rect[c].left +0.1f, m_mapRectInfo[ split_id ].rect[c].top+0.1f, 0);
					glVertex3f(m_mapRectInfo[ split_id ].rect[c].right-0.1f, m_mapRectInfo[ split_id ].rect[c].top+0.1f, 0);
					glVertex3f(m_mapRectInfo[ split_id ].rect[c].right-0.1f, m_mapRectInfo[ split_id ].rect[c].bottom-0.1f, 0);
					glVertex3f(m_mapRectInfo[ split_id ].rect[c].left +0.1f, m_mapRectInfo[ split_id ].rect[c].bottom-0.1f, 0);
					glEnd();
					break;
				case SHAPE_CIRCLE:
				case SHAPE_ELLIPSE:
					{
						int x, y, dx, dy;

						x = (m_mapRectInfo[ split_id ].rect[c].left + m_mapRectInfo[ split_id ].rect[c].right)/2;
						y = (m_mapRectInfo[ split_id ].rect[c].top + m_mapRectInfo[ split_id ].rect[c].bottom)/2;
						dx = abs(m_mapRectInfo[ split_id ].rect[c].left - m_mapRectInfo[ split_id ].rect[c].right)/2;
						dy = abs(m_mapRectInfo[ split_id ].rect[c].top - m_mapRectInfo[ split_id ].rect[c].bottom)/2;
						// modified by danny
						if (m_mapRectInfo[ split_id ].rect_type[c] == SHAPE_CIRCLE)
						{
							if (dx > dy)
								dx = dy;
							else
								dy = dx;
						}

						glBegin(GL_POLYGON);
						glVertex3f(x+dx*1.000000f, y+dy*0.000000f, 0.0f);
						glVertex3f(x+dx*0.980785f, y+dy*0.195090f, 0.0f);
						glVertex3f(x+dx*0.923880f, y+dy*0.382683f, 0.0f);
						glVertex3f(x+dx*0.831470f, y+dy*0.555570f, 0.0f);
						glVertex3f(x+dx*0.707107f, y+dy*0.707107f, 0.0f);
						glVertex3f(x+dx*0.555570f, y+dy*0.831470f, 0.0f);
						glVertex3f(x+dx*0.382683f, y+dy*0.923880f, 0.0f);
						glVertex3f(x+dx*0.195090f, y+dy*0.980785f, 0.0f);
						glVertex3f(x+dx*-0.000000f, y+dy*1.000000f, 0.0f);
						glVertex3f(x+dx*-0.195090f, y+dy*0.980785f, 0.0f);
						glVertex3f(x+dx*-0.382683f, y+dy*0.923880f, 0.0f);
						glVertex3f(x+dx*-0.555570f, y+dy*0.831470f, 0.0f);
						glVertex3f(x+dx*-0.707107f, y+dy*0.707107f, 0.0f);
						glVertex3f(x+dx*-0.831470f, y+dy*0.555570f, 0.0f);
						glVertex3f(x+dx*-0.923880f, y+dy*0.382683f, 0.0f);
						glVertex3f(x+dx*-0.980785f, y+dy*0.195090f, 0.0f);
						glVertex3f(x+dx*-1.000000f, y+dy*-0.000000f, 0.0f);
						glVertex3f(x+dx*-0.980785f, y+dy*-0.195090f, 0.0f);
						glVertex3f(x+dx*-0.923880f, y+dy*-0.382683f, 0.0f);
						glVertex3f(x+dx*-0.831470f, y+dy*-0.555570f, 0.0f);
						glVertex3f(x+dx*-0.707107f, y+dy*-0.707107f, 0.0f);
						glVertex3f(x+dx*-0.555570f, y+dy*-0.831469f, 0.0f);
						glVertex3f(x+dx*-0.382684f, y+dy*-0.923879f, 0.0f);
						glVertex3f(x+dx*-0.195090f, y+dy*-0.980785f, 0.0f);
						glVertex3f(x+dx*0.000000f, y+dy*-1.000000f, 0.0f);
						glVertex3f(x+dx*0.195090f, y+dy*-0.980785f, 0.0f);
						glVertex3f(x+dx*0.382684f, y+dy*-0.923879f, 0.0f);
						glVertex3f(x+dx*0.555570f, y+dy*-0.831470f, 0.0f);
						glVertex3f(x+dx*0.707107f, y+dy*-0.707107f, 0.0f);
						glVertex3f(x+dx*0.831470f, y+dy*-0.555570f, 0.0f);
						glVertex3f(x+dx*0.923880f, y+dy*-0.382683f, 0.0f);
						glVertex3f(x+dx*0.980785f, y+dy*-0.195090f, 0.0f);
						glEnd();
					}
					break;
				default:
					break;
				}
			}
		}
	}

	if (m_showRegionDFShape == TRUE){
			glDisable(GL_COLOR_LOGIC_OP); 
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			glBindTexture(GL_TEXTURE_2D, m_texture[15]);
			GLenum error = glGetError();
			if (m_RegionSegmentColor == REGION_DFSHAPE_ENABLE_RGB332) {
				glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
				glColor3f(1.0f, 1.0f, 1.0f);
			} else {
				// Mask
				glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
				glBegin(GL_QUADS);
					glColor3f(1.0f, 1.0f, 1.0f);
					glTexCoord2f(0.0f, 0.0f);
					glVertex3f(0.0f, 0.0f, 0.0f);
					glTexCoord2f(0.0f, 1.0f);
					glVertex3f(0.0f, m_height[split_id]*1.0f, 0.0f);
					glTexCoord2f(1.0f, 1.0f);
					glVertex3f(m_width[split_id]*1.0f, m_height[split_id]*1.0f, 0.0f);
					glTexCoord2f(1.0f, 0.0f);
					glVertex3f(m_width[split_id]*1.0f, 0.0f, 0.0f);
				glEnd();
				glBlendFunc(GL_ONE, GL_ONE);
				GLfloat r, g, b;
				r = GetRValue(m_RegionDFShapeColor)/255.0f;
				g = GetGValue(m_RegionDFShapeColor)/255.0f;
				b = GetBValue(m_RegionDFShapeColor)/255.0f;
				glColor3f(r, g, b);
			}
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(0.0f, 0.0f, 0.0f);
				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(0.0f, m_height[split_id]*1.0f, 0.0f);	
				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(m_width[split_id]*1.0f, m_height[split_id]*1.0f, 0.0f);
				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(m_width[split_id]*1.0f, 0.0f, 0.0f);
			glEnd();
	}

	// added by eric at 20120910
	if (m_bDrawTriangle == TRUE) {
		glLineWidth(3.0f);
		glColor3f(1.0f, 0.0f, 1.0f);

		for (size_t c=0; c<m_multiTrianglePtrList.size(); c++) {
			glEnable(GL_COLOR_LOGIC_OP); 
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_LINE_LOOP);
			glVertex3f((GLfloat)m_multiTrianglePtrList[c][0].x, (GLfloat)m_multiTrianglePtrList[c][0].y, 0);
			glVertex3f((GLfloat)m_multiTrianglePtrList[c][1].x, (GLfloat)m_multiTrianglePtrList[c][1].y, 0);
			glVertex3f((GLfloat)m_multiTrianglePtrList[c][2].x, (GLfloat)m_multiTrianglePtrList[c][2].y, 0);
			glVertex3f((GLfloat)m_multiTrianglePtrList[c][0].x, (GLfloat)m_multiTrianglePtrList[c][0].y, 0);
			glEnd();

			// added by amike at 20120912
			float x_dis, y_dis, l_dis;
			float base_cx = (m_multiTrianglePtrList[c][2].x+m_multiTrianglePtrList[c][1].x)/2.0f;
			float base_cy = (m_multiTrianglePtrList[c][2].y+m_multiTrianglePtrList[c][1].y)/2.0f;
			float leg_cx = (m_multiTrianglePtrList[c][0].x+m_multiTrianglePtrList[c][2].x)/2.0f;
			float leg_cy = (m_multiTrianglePtrList[c][0].y+m_multiTrianglePtrList[c][2].y)/2.0f;
			float hypotenuse_cx = (m_multiTrianglePtrList[c][0].x+m_multiTrianglePtrList[c][1].x)/2.0f;
			float hypotenuse_cy = (m_multiTrianglePtrList[c][0].y+m_multiTrianglePtrList[c][1].y)/2.0f;
			x_dis = fabs(m_multiTrianglePtrList[c][1].x*1.0f-m_multiTrianglePtrList[c][2].x*1.0f);
			y_dis = fabs(m_multiTrianglePtrList[c][0].y*1.0f-m_multiTrianglePtrList[c][2].y*1.0f);
			l_dis = sqrtf(x_dis*x_dis+y_dis*y_dis);
			
			float fXDis = m_multiTriangleDisList[c].fXDis;
			float fYDis = m_multiTriangleDisList[c].fYDis;
			float fLDis = m_multiTriangleDisList[c].fLDis;

			if(x_dis*m_norm_scale[0]*m_glScale>100.0f
				|| y_dis*m_norm_scale[0]*m_glScale>100.0f
				|| l_dis*m_norm_scale[0]*m_glScale>100.0f)
			{
				// oglPrintf() -> DrawPrintf() -- Beagle 20130121 modified.
				// modified by eric at 20130311
				if(x_dis<5.0f || y_dis<5.0f){
					float line_sp = 20.0f/(m_glScale*m_norm_scale[split_id]);
					
					//if((m_fDistance_XRatio == 1.0)&&(m_fDistance_YRatio == 1.0))
					if(
						((fXDis == 0.0)&& (fYDis == 0.0) && (fLDis == 0.0))||
						(m_strDistanceUnitString == _T(""))
						)
					{
						DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
							base_cx, base_cy, L"%s:%.1f", m_vDistanceStrings[0], x_dis);
						DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
							base_cx, base_cy+line_sp, L"%s:%.1f", m_vDistanceStrings[1], y_dis);
						DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
							base_cx, base_cy+line_sp*2, L"%s:%.1f", m_vDistanceStrings[2], l_dis);
					}
					else //seanchen 20130821
					{									
						CString strT1;
						strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[0], fXDis,m_strDistanceUnitString);
						DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
							base_cx, base_cy, strT1);
						strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[1], fYDis,m_strDistanceUnitString);
						DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
							base_cx, base_cy+line_sp, strT1);
						strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[2], fLDis,m_strDistanceUnitString);
						DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
							base_cx, base_cy+line_sp*2, strT1);

						//double fDistance_LRatio = sqrtf((float)m_fDistance_XRatio*(float)m_fDistance_XRatio+(float)m_fDistance_YRatio*(float)m_fDistance_YRatio);
						//CString strT1;
						//strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[0], x_dis*m_fDistance_XRatio,m_strDistanceUnitString);
						//DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
						//	base_cx, base_cy, strT1);
						//strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[1], y_dis*m_fDistance_YRatio,m_strDistanceUnitString);
						//DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
						//	base_cx, base_cy+line_sp, strT1);
						//strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[2], l_dis*fDistance_LRatio,m_strDistanceUnitString);
						//DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
						//	base_cx, base_cy+line_sp*2, strT1);
					}
				} else {
					//if((m_fDistance_XRatio == 1.0)&&(m_fDistance_YRatio == 1.0))
					if(
						((fXDis == 0.0)&& (fYDis == 0.0) && (fLDis == 0.0))||
						(m_strDistanceUnitString == _T(""))
						)
					{
						DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
							base_cx, base_cy, L"%s:%.1f", m_vDistanceStrings[0], x_dis);
						DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
							leg_cx, leg_cy+40.0f/m_glScale, L"%s:%.1f", m_vDistanceStrings[1], y_dis);
						DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_BOTTOM,
							hypotenuse_cx, -m_height[split_id]+hypotenuse_cy,
							L"%s:%.1f", m_vDistanceStrings[2], l_dis);
					}
					else //seanchen 20130821
					{					
						CString strT1;
						strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[0], fXDis,m_strDistanceUnitString);
						DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
							base_cx, base_cy,strT1);
						strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[1], fYDis,m_strDistanceUnitString);
						DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
							leg_cx, leg_cy+40.0f/m_glScale, strT1);
						strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[2], fLDis,m_strDistanceUnitString);
						DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_BOTTOM,
							hypotenuse_cx, -m_height[split_id]+hypotenuse_cy, strT1);

						//double fDistance_LRatio = sqrtf((float)m_fDistance_XRatio*(float)m_fDistance_XRatio+(float)m_fDistance_YRatio*(float)m_fDistance_YRatio);
						//CString strT1;
						//strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[0], x_dis*m_fDistance_XRatio,m_strDistanceUnitString);
						//DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
						//	base_cx, base_cy,strT1);
						//strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[1], y_dis*m_fDistance_YRatio,m_strDistanceUnitString);
						//DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
						//	leg_cx, leg_cy+40.0f/m_glScale, strT1);
						//strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[2], l_dis*fDistance_LRatio,m_strDistanceUnitString);
						//DrawPrintf(split_id, ALIGN_SHEET|ALIGN_LEFT|ALIGN_BOTTOM,
						//	hypotenuse_cx, -m_height[split_id]+hypotenuse_cy, strT1);
	
					}
				}
			}
		//	else{
		//		oglClearText();
		//	}
		}
	}

	// added by eric at 20121008
	if (m_bDrawPatternShape) {
		glEnable(GL_COLOR_LOGIC_OP); 
		glDisable(GL_TEXTURE_2D);
		glLineWidth(3.0f);
		glColor3f(1.0f, 0.0f, 1.0f);
		glBegin(GL_LINE_LOOP);
		glVertex3f((GLfloat)m_rtPatternShape.left, (GLfloat)m_rtPatternShape.top, 0);
		glVertex3f((GLfloat)m_rtPatternShape.right, (GLfloat)m_rtPatternShape.top, 0);
		glVertex3f((GLfloat)m_rtPatternShape.right, (GLfloat)m_rtPatternShape.bottom, 0);
		glVertex3f((GLfloat)m_rtPatternShape.left, (GLfloat)m_rtPatternShape.bottom, 0);
		glEnd();
	}

	// added by eric at 20110801
	if (m_isShowBarcode)
	{
		glEnable(GL_COLOR_LOGIC_OP); 
		glDisable(GL_TEXTURE_2D);
		//glLogicOp(GL_OR_REVERSE);
		glLineWidth(3.2f);
		glColor3f(0.2f, 0.8f, 0.2f);
		// modified by eric at 20111020
		int nStart = 0;
		if (!m_isShowOnlyBarcode)
			nStart = 1;

        if ( m_mapRectInfo.find( split_id ) != m_mapRectInfo.end() )
        {
		    for (int c=nStart; c<m_mapRectInfo[ split_id ].rects; c++) 
		    {
			    if (c >= m_Numberbarcode)	// modified by eric at 20120302
				    break;
			    glBegin(GL_LINE_LOOP);
			    // modified by amike at 20130109
			    glVertex3f(m_mapRectInfo[ split_id ].rect[c].left +0.1f, m_mapRectInfo[ split_id ].rect[c].top+0.1f, 0);
			    glVertex3f(m_mapRectInfo[ split_id ].rect[c].right-0.1f, m_mapRectInfo[ split_id ].rect[c].top+0.1f, 0);
			    glVertex3f(m_mapRectInfo[ split_id ].rect[c].right-0.1f, m_mapRectInfo[ split_id ].rect[c].bottom-0.1f, 0);
			    glVertex3f(m_mapRectInfo[ split_id ].rect[c].left +0.1f, m_mapRectInfo[ split_id ].rect[c].bottom-0.1f, 0);
			    glEnd();

			    if ((fabsf((float)(m_mapRectInfo[ split_id ].rect[c].right-m_mapRectInfo[ split_id ].rect[c].left))<10.0f) && (fabsf((float)(m_mapRectInfo[ split_id ].rect[c].bottom-m_mapRectInfo[ split_id ].rect[c].top))<10.0f))
			    {
				    int x1, y1, x2, y2;
				    oglProjectPoint(x1, y1, m_mapRectInfo[ split_id ].rect[c].left, m_mapRectInfo[ split_id ].rect[c].top);
				    oglProjectPoint(x2, y2, m_mapRectInfo[ split_id ].rect[c].right, m_mapRectInfo[ split_id ].rect[c].bottom);

				    if ((abs(x1-x2)<10) && (abs(y1-y2)<10))
					    oglDrawCorona((m_mapRectInfo[ split_id ].rect[c].left+m_mapRectInfo[ split_id ].rect[c].right+1)/2.0f, (m_mapRectInfo[ split_id ].rect[c].top+m_mapRectInfo[ split_id ].rect[c].bottom+1)/2.0f, 2);
			    }

			    // Beagle 20120504 modified.
			    // Shrink by 0.2 pixel to avoid covering the green frame.
			    if (m_isShowOnlyBarcode) {
				    oglDrawNumbers(0.2f+m_mapRectInfo[ split_id ].rect[c].left, 0.2f+m_mapRectInfo[ split_id ].rect[c].top, c+1,
					    m_mapRectInfo[ split_id ].rect[c].right-m_mapRectInfo[ split_id ].rect[c].left+0.6f,
					    (m_mapRectInfo[ split_id ].rect[c].bottom-m_mapRectInfo[ split_id ].rect[c].top)/3.0f);
			    } else {
				    oglDrawNumbers(0.2f+m_mapRectInfo[ split_id ].rect[c].left, 0.2f+m_mapRectInfo[ split_id ].rect[c].top, c,
					    m_mapRectInfo[ split_id ].rect[c].right-m_mapRectInfo[ split_id ].rect[c].left+0.6f,
					    (m_mapRectInfo[ split_id ].rect[c].bottom-m_mapRectInfo[ split_id ].rect[c].top)/3.0f);
			    }
		    }
        }
	}

	// added by eric at 20111024
	if (m_isShowSegment)
	{
		if ((m_Barcoderightbottom.x - m_Barcodelefttop.x > 0) && (m_Barcoderightbottom.y - m_Barcodelefttop.y > 0))
		{
			glEnable(GL_COLOR_LOGIC_OP);
			glDisable(GL_TEXTURE_2D);
			glLineWidth(3.2f);
			glColor3f(0.2f, 0.8f, 0.2f);
			glBegin(GL_LINE_LOOP);
				// modified by amike at 20130109
				glVertex3f(m_Barcodelefttop.x+0.1f, m_Barcodelefttop.y+0.1f, 0);
				glVertex3f(m_Barcoderightbottom.x-0.1f, m_Barcodelefttop.y+0.1f, 0);
				glVertex3f(m_Barcoderightbottom.x-0.1f, m_Barcoderightbottom.y-0.1f, 0);
				glVertex3f(m_Barcodelefttop.x+0.1f, m_Barcoderightbottom.y-0.1f, 0);
			glEnd();
		}
	}

	// added by eric at 20110929
	if (m_lmMode == LM_SET_WEAKENPAINT)
	{
		if (m_PaintMode == PAINT_RECT || m_PaintMode == PAINT_PICK)
		{
			if (m_nWeakenColorBlock > 0)
			{
				glEnable(GL_COLOR_LOGIC_OP);
				glDisable(GL_TEXTURE_2D);
				//glLogicOp(GL_OR_REVERSE);
				glLineWidth(1.2f);
				glColor3f(0.4f, 0.4f, 0.4f);
				glBegin(GL_LINE_LOOP);
				// modified by amike at 20130109
				glVertex3f(m_rcWeakenColor.left +0.1f, m_rcWeakenColor.top+0.1f, 0);
				glVertex3f(m_rcWeakenColor.right-0.1f, m_rcWeakenColor.top+0.1f, 0);
				glVertex3f(m_rcWeakenColor.right-0.1f, m_rcWeakenColor.bottom-0.1f, 0);
				glVertex3f(m_rcWeakenColor.left +0.1f, m_rcWeakenColor.bottom-0.1f, 0);
				glEnd();
			}
		}
	}

	// Render anchors.
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_COLOR_LOGIC_OP);
	// modified by eric at 20120515
	if (m_bShowAnchorPoints == TRUE)
	{
		for (int c=0; c<m_anchors; c++)
		{
			if (c == m_selAnchor)
			{
				glLineWidth(3.2f);
				glColor3f(1.0f, 1.0f, 0.0f);
				glLogicOp(GL_OR_REVERSE);
			}
			else
			{
				glLineWidth(1.2f);
				//glColor3f(0.0f, 0.8f, 0.8f);
				glColor3f(0.0f, 0.0f, 1.0f);
				glLogicOp(GL_OR_REVERSE);
				//glLogicOp(GL_INVERT);
			}

			glBegin(GL_LINES);
			glVertex3f((m_anchor_x[c]-50)*1.0f, m_anchor_y[c]*1.0f, 0.0f);
			glVertex3f((m_anchor_x[c]+50)*1.0f, m_anchor_y[c]*1.0f, 0.0f);
			glVertex3f(m_anchor_x[c]*1.0f, (m_anchor_y[c]-50)*1.0f, 0.0f);
			glVertex3f(m_anchor_x[c]*1.0f, (m_anchor_y[c]+50)*1.0f, 0.0f);
			glEnd();
		}
	}

	// render anchors
	for (size_t t=0; t<m_reposAnchorPoint.size(); t++) {	// modified by eric at 20140102, 20140106
		if (m_reposAnchorPoint[t].x == 0) {
			continue;
		}

		int iLength = 0;
		if (m_showCrossHair) {
			glLineWidth(3.2f);
			glColor3f(0.0f, 1.0f, 0.0f);
			iLength = 16;
		} else {
			if (t == m_selReposAnchor) {
				glLineWidth(3.2f);
				glColor3f(0.0f, 1.0f, 0.0f);
				glLogicOp(GL_OR_REVERSE);
			} else {
				glLineWidth(1.2f);
				glColor3f(1.0f, 0.0f, 1.0f);
				glLogicOp(GL_OR_REVERSE);
			}
			iLength = 50;
		}

		glBegin(GL_LINES);
		glVertex3f((m_reposAnchorPoint[t].x-iLength)*1.0f, m_reposAnchorPoint[t].y*1.0f, 0.0f);
		glVertex3f((m_reposAnchorPoint[t].x+iLength)*1.0f, m_reposAnchorPoint[t].y*1.0f, 0.0f);
		glVertex3f(m_reposAnchorPoint[t].x*1.0f, (m_reposAnchorPoint[t].y-iLength)*1.0f, 0.0f);
		glVertex3f(m_reposAnchorPoint[t].x*1.0f, (m_reposAnchorPoint[t].y+iLength)*1.0f, 0.0f);
		glEnd();
	}
	// ---------------------------

	DrawCrosshair(split_id);	// Beagle 20130118 added.

	// Render incomplete polygon.
	glLineWidth(1.0f);
	if (m_poly_drawing == TRUE)
	{
		glLogicOp(GL_INVERT);

		glBegin(GL_LINE_STRIP);
			for (int c=0; c<m_poly_points[m_polygons]; c++)
			{ 
				glVertex3i(m_polygon[m_polygons][c].x, m_polygon[m_polygons][c].y, 0);
			}
			glVertex3i(m_mouse_x, m_mouse_y, 0);
		glEnd();
	}

	// render the anchor computing area, added at 20100210
	if (m_lmMode == LM_SET_ANCHOR)
	{
		// modified by eric at 20111101, Beagle 20130425
		RECT rect;
		oglGetCenteredRect(rect, m_mouse_x, m_mouse_y, m_barcodewidth, m_barcodeheight);

		glColor3f(0.8f, 0.0f, 0.0f);
		glLogicOp(GL_OR_REVERSE);
		glBegin(GL_LINE_LOOP);
			glVertex3f(rect.left+0.1f,  rect.top+0.1f,    0);
			glVertex3f(rect.right+0.9f, rect.top+0.1f,    0);
			glVertex3f(rect.right+0.9f, rect.bottom+0.9f, 0);
			glVertex3f(rect.left+0.1f,  rect.bottom+0.9f, 0);
		glEnd();
	}

	// Modified by eric at 20130306. Modified by Beagle at 20130425.
	if (m_lmMode == LM_DRAW_SHADOWRECT) {
		RECT rect;
		oglGetCenteredRect(rect, m_mouse_x, m_mouse_y, m_ptShadowShape.x, m_ptShadowShape.y);

		glDisable(GL_TEXTURE_2D);
		glEnable(GL_COLOR_LOGIC_OP);
		glLogicOp(GL_OR_REVERSE);
		glLineWidth(1.0f);
		glColor3f(0.8f, 0.0f, 0.0f);
		//glEnable(GL_POINT_SMOOTH);
		// modified by eric at 20130306
		if (m_nBlockDirection == EMBOSS_LEFT_UP || m_nBlockDirection == EMBOSS_LEFT_DOWN ||
			m_nBlockDirection == EMBOSS_RIGHT_UP || m_nBlockDirection == EMBOSS_RIGHT_DOWN ||
			m_nBlockDirection == EMBOSS_LU_RD || m_nBlockDirection == EMBOSS_LD_RU)
		{
			// 45度斜正方形◇
			float cx = (rect.left+rect.right)/2.0f+0.5f;
			float cy = (rect.top+rect.bottom)/2.0f+0.5f;

			glBegin(GL_LINE_LOOP);
				glVertex3f(rect.left+0.5f, cy, 0);
				glVertex3f(cx, rect.top+0.5f, 0);
				glVertex3f(rect.right+0.5f, cy, 0);
				glVertex3f(cx, rect.bottom+0.5f, 0);
			glEnd();
		} else {
			// 正方形□
			glBegin(GL_LINE_LOOP);
				glVertex3f(rect.left+0.5f, rect.top+0.5f, 0);
				glVertex3f(rect.right+0.5f, rect.top+0.5f, 0);
				glVertex3f(rect.right+0.5f, rect.bottom+0.5f, 0);
				glVertex3f(rect.left+0.5f, rect.bottom+0.5f, 0);
			glEnd();
		}
	}

	// render the anchor computing area, added at 20110310
	if ((m_lmMode==LM_SET_WEAKENANCHOR) || (m_lmMode==LM_SET_DISTANCEANCHOR)) // modified by eric at 20120910
	{
		// modified by eric at 20111101, 20130326, Beagle 20130425
		RECT rect;
		oglGetCenteredRect(rect, m_mouse_x, m_mouse_y, m_barcodewidth, m_barcodeheight);

		glColor3f(0.8f, 0.0f, 0.0f);
		glLogicOp(GL_OR_REVERSE);
		glBegin(GL_LINE_LOOP);
			glVertex3f(rect.left+0.1f,  rect.top+0.1f,    0);
			glVertex3f(rect.right+0.9f, rect.top+0.1f,    0);
			glVertex3f(rect.right+0.9f, rect.bottom+0.9f, 0);
			glVertex3f(rect.left+0.1f,  rect.bottom+0.9f, 0);
		glEnd();
	}

	// added by eric at 20110801
	if (m_lmMode == LM_SET_BARCODEANCHOR)
	{
		// modified by eric at 20111101, Beagle 20130425
		RECT rect;
		oglGetCenteredRect(rect, m_mouse_x, m_mouse_y, m_barcodewidth, m_barcodeheight);

		// Beagle 20120316 modified -- Correct the rectangle position.
		glColor3f(0.8f, 0.0f, 0.0f);
		glLogicOp(GL_OR_REVERSE);
		glBegin(GL_LINE_LOOP);
		// modified by amike at 20130109
			glVertex3f(rect.left+0.1f,  rect.top+0.1f,    0);
			glVertex3f(rect.right+0.9f, rect.top+0.1f,    0);
			glVertex3f(rect.right+0.9f, rect.bottom+0.9f, 0);
			glVertex3f(rect.left+0.1f,  rect.bottom+0.9f, 0);
		glEnd();
	}

	// added by danny at 20100412 ------------------------------------------------
	if (m_showGoldDiffTexture == TRUE)
	{
		glDisable(GL_COLOR_LOGIC_OP); 
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);

		// gold grid texture
		//glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
		glBlendFunc(GL_ONE, GL_ONE);
		glBindTexture(GL_TEXTURE_2D, m_texture[14]);
		glBegin(GL_QUADS);
		glColor3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(0.0f, m_height[split_id]*1.0f, 0.0f);	// Beagle 20111101 modified.
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(m_width[split_id]*1.0f, m_height[split_id]*1.0f, 0.0f);	// Beagle 20111101 modified.
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(m_width[split_id]*1.0f, 0.0f, 0.0f);	// Beagle 20111101 modified.
		glEnd();
	}

	if (m_showGridTexture == TRUE)
	{
		glDisable(GL_COLOR_LOGIC_OP); 
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);

		// grid texture
		glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
		glBindTexture(GL_TEXTURE_2D, m_texture[13]);
		glBegin(GL_QUADS);
		glColor3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(0.0f, m_height[split_id]*1.0f, 0.0f);	// Beagle 20111101 modified.
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(m_width[split_id]*1.0f, m_height[split_id]*1.0f, 0.0f);	// Beagle 20111101 modified.
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(m_width[split_id]*1.0f, 0.0f, 0.0f);	// Beagle 20111101 modified.
		glEnd();
	}

	if (m_showGrid == TRUE)
	{
		// grid...
		glDisable(GL_TEXTURE_2D);
		glLineWidth(0.5f);
		glColor3f(1.0f, 0.0f, 0.0f);
		glEnable(GL_COLOR_LOGIC_OP); 
		glLogicOp(GL_OR_REVERSE);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x0F0F);
		glBegin(GL_LINES);
		// Beagle 20111101 modified.
		for (int h=0; h<m_height[split_id]; h+=m_gridSize)
		{
			glVertex3f(0.0f, h*1.0f, 0.0f);
			glVertex3f(1.0f*m_width[split_id], h*1.0f, 0.0f);
		}
		// Beagle 20111101 modified.
		for (int w=0; w<m_width[split_id]; w+=m_gridSize)
		{
			glVertex3f(1.0f*w, 0.0f, 0.0f);
			glVertex3f(1.0f*w, m_height[split_id]*1.0f, 0.0f);
		}
		glEnd();
		glDisable(GL_LINE_STIPPLE);
	}

	if (m_showLearnRegion == TRUE)
	{
		glDisable(GL_TEXTURE_2D);
		glLineWidth(1.0f);
		//glColor3f(0.0f, 1.0f, 0.0f);
		glEnable(GL_COLOR_LOGIC_OP); 
		//glLogicOp(GL_XOR);
		glLogicOp(GL_INVERT);
		for (unsigned int i=0; i<m_learnRegion.size(); i++)
		{
			glBegin(GL_LINE_LOOP);
			glVertex3i(m_learnRegion[i].left, m_learnRegion[i].top, 0);
			glVertex3i(m_learnRegion[i].right, m_learnRegion[i].top, 0);
			glVertex3i(m_learnRegion[i].right, m_learnRegion[i].bottom, 0);
			glVertex3i(m_learnRegion[i].left, m_learnRegion[i].bottom, 0);
			glEnd();
		}
	}

	// added by eric at 20120801
	if (m_ShowOCRInspArea)
	{
		if (m_rtOCR.Width()*m_rtOCR.Height() != 0)	
		{
			glDisable(GL_TEXTURE_2D);
			// Beagle 20130610 --
			//	glLineWidth() should never put between glBegin() and glEnd()
			glLineWidth(1.2f);
			glBegin(GL_LINE_LOOP);
				glColor3f(1.0f, 1.0f, 0.0f);
				// modified by amike at 20130109
				glVertex3f(m_rtOCR.left +0.1f, m_rtOCR.top+0.1f, 0);
				glVertex3f(m_rtOCR.right-0.1f, m_rtOCR.top+0.1f, 0);
				glVertex3f(m_rtOCR.right-0.1f, m_rtOCR.bottom-0.1f, 0);
				glVertex3f(m_rtOCR.left +0.1f, m_rtOCR.bottom-0.1f, 0);
			glEnd();
		}
	}
	// -----------------------------------------------------------------------
	if (m_showBarcodeDigitsImage) { oglDrawBarcodeDigits(); }	// Beagle 20120301 added.
	if (m_FrameNumber > 0) DrawFrame(split_id);	// Beagle 20120704 added.

	DrawUserLayoutRectangle(split_id);//seanchen 20151218 

	glPushMatrix();
	glDisable(GL_COLOR_LOGIC_OP); 
	glColor3f(1.0f, 1.0f, 1.0f);

	oglDrawMinimap();
	oglDrawCutHereLine(split_id);	// Beagle 20120817 added.
	oglDrawUnitLine(split_id); //eric chao 20160527
	oglDrawTextString();
	oglDrawBarcodeResult();
	DrawCursorOutline(split_id);	// Beagle 20130424 added.
	DrawLuminanceGraph(split_id);	// Beagle 20130814

	if(m_showChessboard)
	{
		// grid...
		glDisable(GL_TEXTURE_2D);
		glLineWidth(1.0f);
		glColor3f(0.0f, 0.8f, 0.0f);
		//glEnable(GL_COLOR_LOGIC_OP); 
		//glLogicOp(GL_OR_REVERSE);
		glEnable(GL_LINE_STIPPLE);
		//glLineStipple(1, 0x0F0F);
		glEnable( GL_POINT_SMOOTH );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glPointSize( 4.0 );
#if 1
		glBegin(GL_LINES);
		//glBegin(GL_POINTS);
		//glColor3f( 0.95f, 0.05f, 0.031f );
		for (int i=0; i<(int)m_pointList.size()-1; i++)
		{
			glVertex3d(m_pointList[i].x, m_pointList[i].y, 0);
			glVertex3d(m_pointList[i+1].x, m_pointList[i+1].y, 0);
		}
		glEnd();
		glDisable(GL_LINE_STIPPLE);

		glBegin( GL_POINTS );
		glColor3f( 0.75f, 0.05f, 0.031f );
		for ( int i = 0; i < (int)m_dispointList.size(); ++i )
		{
			glVertex3d( m_dispointList[i].x, m_dispointList[i].y, 0 );
		}
		glEnd();
#endif 

		glLineWidth(3.0f);
		glColor3f(1.0f, 0.0f, 0.0f); 
		glBegin(GL_LINES);

		for (int i=0; i<(int)m_avgdisptrList.size(); i+=2)
		{    
			glVertex3d( m_avgdisptrList[i].x,  m_avgdisptrList[i].y, 0);
			glVertex3d( m_avgdisptrList[i+1].x,  m_avgdisptrList[i+1].y, 0);
		}
		glEnd();

		/*
		for (int h=0; h<m_height[split_id]; h+=m_gridSize)
		{
			glVertex3f(0.0f, h*1.0f, 0.0f);
			glVertex3f(1.0f*m_width[split_id], h*1.0f, 0.0f);
		}
		// Beagle 20111101 modified.
		for (int w=0; w<m_width[split_id]; w+=m_gridSize)
		{
			glVertex3f(1.0f*w, 0.0f, 0.0f);
			glVertex3f(1.0f*w, m_height[split_id]*1.0f, 0.0f);
		}*/
		//glEnd();
	}
    if ( !m_OGLSegment[ split_id ].IsSegmentation() && !m_OGLRolling[ split_id ].IsRollingMode() )
    {
	    glPopMatrix();
	    glPopMatrix();
    }
	glPopMatrix();
	if (m_wsplit > 1 || m_hsplit > 1) { oglDrawSplitMark(split_x, split_y); }
}

LRESULT COpenGLControl::OnNcHitTest(CPoint point)
{
	::SetCursor(m_glCursor);
	
	return CWnd::OnNcHitTest(point);
}
// added by eric at 20130208
BOOL COpenGLControl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: 在此加入您的訊息處理常式程式碼和 (或) 呼叫預設值
	if (m_pLineTracker) {
		m_pLineTracker->SetCursor(pWnd, nHitTest);
		return TRUE;
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void COpenGLControl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
if ( cx <= 0 || cy <= 0 ) return;
	oglMakeCurrent();

	glViewport(0, 0, cx, cy);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	gluPerspective(45.0f, (GLfloat)cx/(GLfloat)cy, 0.1f, 100.0f);
	glOrtho(-0.5f*cx, 0.5f*cx, 0.5f*cy, -0.5f*cy, 0.0f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	m_glwidth = cx;
	m_glheight = cy;

	oglSetNormalTranslate();
}

// Beagle 20130425 modified.
void COpenGLControl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_RightButtonDown) {	// added by eric at 20131223
		return;
	}

	BOOL bNeedRedraw = FALSE;
	POINT ptTranslated = {0,0};

	SetFocus();

	m_LeftButtonDown = TRUE;
	m_LeftDownPos = point;
	// added by eric at 20130827
	BOOL bSetCapture = TRUE;
	for (int i=0; i<(int)m_vSingleClickEvent.size(); i++) {
		if (m_vSingleClickEvent[i] == m_lmMode) {
			bSetCapture = FALSE;
			break;
		}
	}
	if (bSetCapture) {
		SetCapture();
	}
	oglTranslatePoint(point.x, point.y, m_mouse_x, m_mouse_y, &m_mouse_sp);
	ptTranslated.x = m_mouse_x;
	ptTranslated.y = m_mouse_y;

	MousePick(point.x, point.y);	// Beagle 20130807

	switch (m_lmMode)
	{
	case LM_IGNORE:
		OnMouseDownShape(point);	// added by eric at 20120330
		break;
	case LM_TRANSLATE:
		break;
	case LM_SCALE:
		m_ScaleCenter = point;	// Beagle 20130103 added.
		break;
	case LM_SET_ANCHOR:
	case LM_SET_WEAKENANCHOR:
	case LM_SET_DISTANCEANCHOR:
		// added by eric at 20150420
		m_bAnchorRect=TRUE;
		m_rtAnchorRect.right = m_rtAnchorRect.left = m_mouse_x;
		m_rtAnchorRect.bottom = m_rtAnchorRect.top = m_mouse_y;
		break;

		// added by eric at 20110801
	case LM_SET_BARCODEANCHOR:
		if (!m_showOCRSize)	{	// added by eric at 20110809
			if (m_mapRectInfo[ NULL ].rects < m_Numberbarcode) {	// modified by eric at 20120314
				RECT rect;
				oglGetCenteredRect(rect, m_mouse_x, m_mouse_y,
					m_barcodewidth, m_barcodeheight);
				m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects].left = rect.left;
				m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects].top = rect.top;
				m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects].right = rect.right;
				m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects].bottom = rect.bottom;
				m_mapRectInfo[ NULL ].rect_type[m_mapRectInfo[ NULL ].rects] = SHAPE_RECT;
				m_mapRectInfo[ NULL ].rects++;

				POINT *pt = new POINT(ptTranslated);
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_ANCHOR_BARCODE,
					(LPARAM)pt);
				bNeedRedraw = TRUE;
			}
		} else {
			POINT *pt = new POINT(ptTranslated);
			hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_ANCHOR_BARCODE,
				(LPARAM)pt);
			bNeedRedraw = TRUE;
		}
		break;

	case LM_SET_WEAKENPAINT:	// added by eric at 20110929
		if (m_PaintMode == PAINT_RECT)
		{
			m_nWeakenColorBlock++;
			m_rcWeakenColor.right = m_rcWeakenColor.left = m_mouse_x;
			m_rcWeakenColor.bottom = m_rcWeakenColor.top = m_mouse_y;
		}
		else if (m_PaintMode == PAINT_BRUSH)
		{
			ComputeEraserSize(point.x, point.y);
			hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_DOWN_ADD);
		}
		else if (m_PaintMode == PAINT_PAINTBRUSH)		// added by eric at 20120330
		{
			ComputeEraserSize(point.x, point.y);	// modified by eric at 20121022
			hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_DOWN_ADD);
		}
		break;

		// added by eric at 20130107
	case LM_DRAW_LAYOUTSHAPE:
		{
			m_nHitHandle = -1;
			m_bLMouseDown = TRUE;

			m_ptDown.x = m_mouse_x;
			m_ptDown.y = m_mouse_y;

			if (m_vRectTrackerList.size() == 1)
			{
				m_nHitHandle = m_vRectTrackerList[0].HitTest(point);
				if (m_nHitHandle == 4 || m_nHitHandle == 6)
					::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
				else if (m_nHitHandle == 5 || m_nHitHandle == 7)
					::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
				else
					::SetCursor(::LoadCursor(NULL, IDC_ARROW));

				if (m_nHitHandle >= 4 && m_nHitHandle <= 7) {
					//m_bShowLayout = FALSE;
					//InvalidateRect(NULL, FALSE);
					m_XPos = m_YPos = -1;
					if (m_nHitHandle == 4) {
						m_YPos = m_layoutRect[m_selLayoutRect].top;
					} else if (m_nHitHandle == 6) {
						m_YPos = m_layoutRect[m_selLayoutRect].bottom;
					} else if (m_nHitHandle == 5) {
						m_XPos = m_layoutRect[m_selLayoutRect].right;
					} else if (m_nHitHandle == 7) {
						m_XPos = m_layoutRect[m_selLayoutRect].left;
					}
				}
			}
		}
		break;

		// added by eric at 20130208
	case LM_DRAW_SHADOWRECT:
		{
			RECT rect;
			SHADOWINFO shadowInfo;
			memset(&shadowInfo.shadowShape,0,sizeof(shadowInfo.shadowShape));
			memset(&shadowInfo.lineShape,0,sizeof(shadowInfo.lineShape));
			memset(&shadowInfo.lineShape2,0,sizeof(shadowInfo.lineShape2));

			oglGetCenteredRect(rect, m_mouse_x, m_mouse_y,
				m_ptShadowShape.x, m_ptShadowShape.y);

			shadowInfo.direction = m_nBlockDirection;
			shadowInfo.shadowShape = rect;

			// modified by eric at 20130219, 20130306
			if (m_nBlockDirection == EMBOSS_LEFT || m_nBlockDirection == EMBOSS_RIGHT || m_nBlockDirection == EMBOSS_LEFT_RIGHT) {
				if (m_shadowType == 0) {	// One Way has two lines
					shadowInfo.lineShape.top = shadowInfo.shadowShape.top;
					shadowInfo.lineShape.bottom = shadowInfo.shadowShape.bottom;
					shadowInfo.lineShape.left = shadowInfo.lineShape.right = m_mouse_x - 1;
					shadowInfo.lineShape2.top = shadowInfo.shadowShape.top;
					shadowInfo.lineShape2.bottom = shadowInfo.shadowShape.bottom;
					shadowInfo.lineShape2.left = shadowInfo.lineShape2.right = m_mouse_x + 1;
				} else if (m_shadowType == 1) {	// Two Way has one line
					shadowInfo.lineShape.top = shadowInfo.shadowShape.top;
					shadowInfo.lineShape.bottom = shadowInfo.shadowShape.bottom;
					shadowInfo.lineShape.left = shadowInfo.lineShape.right = m_mouse_x;
				}
			} else if (m_nBlockDirection == EMBOSS_UP || m_nBlockDirection == EMBOSS_DOWN || m_nBlockDirection == EMBOSS_UP_DOWN) {
				if (m_shadowType == 0) {	// One Way has two lines
					shadowInfo.lineShape.left = shadowInfo.shadowShape.left;
					shadowInfo.lineShape.right = shadowInfo.shadowShape.right;
					shadowInfo.lineShape.top = shadowInfo.lineShape.bottom = m_mouse_y - 1;
					shadowInfo.lineShape2.left = shadowInfo.shadowShape.left;
					shadowInfo.lineShape2.right = shadowInfo.shadowShape.right;
					shadowInfo.lineShape2.top = shadowInfo.lineShape2.bottom = m_mouse_y + 1;
				} else if (m_shadowType == 1) {	// Two Way has one line
					shadowInfo.lineShape.left = shadowInfo.shadowShape.left;
					shadowInfo.lineShape.right = shadowInfo.shadowShape.right;
					shadowInfo.lineShape.top = shadowInfo.lineShape.bottom = m_mouse_y;
				}
			} else if (m_nBlockDirection == EMBOSS_LEFT_UP || m_nBlockDirection == EMBOSS_LEFT_DOWN || m_nBlockDirection == EMBOSS_RIGHT_UP || m_nBlockDirection == EMBOSS_RIGHT_DOWN ||
					m_nBlockDirection == EMBOSS_LU_RD || m_nBlockDirection == EMBOSS_LD_RU) {
				BOOL bClockwise = FALSE;
				if (m_nBlockDirection == EMBOSS_LEFT_DOWN || m_nBlockDirection == EMBOSS_RIGHT_UP || m_nBlockDirection == EMBOSS_LD_RU) {
					bClockwise = TRUE;
				}

				CRect rtCenterLine;
				OnGet45DegreeRectagleCenterLine(rtCenterLine, shadowInfo.shadowShape, bClockwise);
				if (m_shadowType == 0) {	// One Way has two lines
					shadowInfo.lineShape = shadowInfo.lineShape2 = rtCenterLine;
					if (bClockwise) {
						shadowInfo.lineShape.left-=1;
						shadowInfo.lineShape.right-=1;
						shadowInfo.lineShape.top+=1;
						shadowInfo.lineShape.bottom+=1;
						shadowInfo.lineShape2.left+=1;
						shadowInfo.lineShape2.right+=1;
						shadowInfo.lineShape2.top-=1;
						shadowInfo.lineShape2.bottom-=1;
					} else {
						shadowInfo.lineShape.left+=1;
						shadowInfo.lineShape.right+=1;
						shadowInfo.lineShape.top+=1;
						shadowInfo.lineShape.bottom+=1;
						shadowInfo.lineShape2.left-=1;
						shadowInfo.lineShape2.right-=1;
						shadowInfo.lineShape2.top-=1;
						shadowInfo.lineShape2.bottom-=1;
					}
				} else if (m_shadowType == 1) {	// Two Way has one line
					shadowInfo.lineShape = rtCenterLine;
				}
			}

			m_vShadowInfo.push_back(shadowInfo);
			int nIndex = (int)(m_vShadowInfo.size()-1);
			if (nIndex >= 0) {
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_SHAPE_ADD, nIndex);
			}
		}
		break;

		// added by eric at 20130208
	case LM_MODIFY_SHADOWRECT:
		{
			m_vRectTrackerList.clear();
			m_nHitHandle = -1;
			m_bLMouseDown = TRUE;
			m_nSelectedIndex = -1;	// added by eric at 20130301
			m_ptDown.x = m_mouse_x;
			m_ptDown.y = m_mouse_y;
			for (int i=0; i<(int)m_vShadowInfo.size(); i++) {
				CRect rt = m_vShadowInfo[i].shadowShape;
				if (rt.PtInRect(m_ptDown) > 0) {
					m_nSelectedIndex = i;
					m_nHitHandle = 8;
					::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
					CMyTracker tracker;
					int left, top, right, bottom;
					// modified by eric at 20130306, Beagle 20130424 modified.
					oglProjectPoint(left, top, rt.left, rt.top);
					oglProjectPoint(right, bottom, rt.right, rt.bottom);
					CRect rtTracker = CRect(left, top, right, bottom);
					tracker.m_rect = rtTracker;
					tracker.m_nStyle = CRectTracker::dottedLine|CRectTracker::hatchedBorder|CRectTracker::resizeInside;
					m_vRectTrackerList.push_back(tracker);
					m_rtSelected = rt;
					break;
				}
			}
			bNeedRedraw = TRUE;

			hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_SHAPE_SELECT, m_nSelectedIndex);	// added by eric at 20130301
		}
		break;

		// added by eric at 20130208
	case LM_MODIFY_LINE:
		{
			if (m_pLineTracker) {
				if (m_pLineTracker->OnLButtonDown(this, nFlags, point, &m_LineList)) {
					CLineItem* pLine;
					int i;
					m_pLineTracker->GetSelection(pLine, i);

					if (pLine == NULL && i < 0) {
						break;
					}
					// modified by eric at 20130219
					int nHandle = m_pLineTracker->GetHandle();
					if (nHandle == hitNothing) {
						break;
					}

					int left, top, right, bottom;
					oglTranslatePoint(pLine->m_rcPoints.left, pLine->m_rcPoints.top, left, top);
					oglTranslatePoint(pLine->m_rcPoints.right, pLine->m_rcPoints.bottom, right, bottom);

					int nIndex = i;
					if (m_shadowType == 0) {	// One Way has two lines
						nIndex = (i/2);
					}

					int direction = m_vShadowInfo[nIndex].direction;	// added by eric at 20130306
					// modified by eric at 20130219
					CRect rt;
					if (direction == EMBOSS_LEFT || direction == EMBOSS_RIGHT || direction == EMBOSS_LEFT_RIGHT || 
						direction == EMBOSS_UP || direction == EMBOSS_DOWN || direction == EMBOSS_UP_DOWN) {
						if (m_shadowType == 0) {
							if (i%2 == 0) {
								rt = m_vShadowInfo[nIndex].lineShape;
							} else {
								rt = m_vShadowInfo[nIndex].lineShape2;
							}
						} else {
							rt = m_vShadowInfo[nIndex].lineShape;
						}

						if (direction == EMBOSS_LEFT || direction == EMBOSS_RIGHT || direction == EMBOSS_LEFT_RIGHT) {
							if (nHandle == hitStart) {
								if (m_vShadowInfo[nIndex].shadowShape.top != top) {
									rt.top = m_vShadowInfo[nIndex].shadowShape.top;
								}

								if (m_vShadowInfo[nIndex].shadowShape.left > left)
									rt.left = m_vShadowInfo[nIndex].shadowShape.left;
								else if (m_vShadowInfo[nIndex].shadowShape.right < left)
									rt.left = m_vShadowInfo[nIndex].shadowShape.right;
								else
									rt.left = left;
							} else if (nHandle == hitEnd) {
								if (m_vShadowInfo[nIndex].shadowShape.bottom != bottom) {
									rt.bottom = m_vShadowInfo[nIndex].shadowShape.bottom;
								}

								if (m_vShadowInfo[nIndex].shadowShape.right < right)
									rt.right = m_vShadowInfo[nIndex].shadowShape.right;
								else if (m_vShadowInfo[nIndex].shadowShape.left > right)
									rt.right = m_vShadowInfo[nIndex].shadowShape.left;
								else
									rt.right = right;
							}
						} else {
							if (nHandle == hitStart) {
								if (m_vShadowInfo[nIndex].shadowShape.left != left) {
									rt.left = m_vShadowInfo[nIndex].shadowShape.left;
								}

								if (m_vShadowInfo[nIndex].shadowShape.top > top)
									rt.top = m_vShadowInfo[nIndex].shadowShape.top;
								else if (m_vShadowInfo[nIndex].shadowShape.bottom < top)
									rt.top = m_vShadowInfo[nIndex].shadowShape.bottom;
								else
									rt.top = top;
							} else if (nHandle == hitEnd) {
								if (m_vShadowInfo[nIndex].shadowShape.right != right) {
									rt.right = m_vShadowInfo[nIndex].shadowShape.right;
								}

								if (m_vShadowInfo[nIndex].shadowShape.bottom < bottom)
									rt.bottom = m_vShadowInfo[nIndex].shadowShape.bottom;
								else if (m_vShadowInfo[nIndex].shadowShape.top > bottom)
									rt.bottom = m_vShadowInfo[nIndex].shadowShape.top;
								else
									rt.bottom = bottom;
							}
						}

						oglProjectPoint(left, top, rt.left, rt.top);
						oglProjectPoint(right, bottom, rt.right, rt.bottom);
					} else if (direction == EMBOSS_LEFT_DOWN || direction == EMBOSS_RIGHT_UP || direction == EMBOSS_LD_RU || 
						direction == EMBOSS_LEFT_UP || direction == EMBOSS_RIGHT_DOWN || direction == EMBOSS_LU_RD) {
						// 45 degree
						rt = m_vShadowInfo[nIndex].shadowShape;

						CPoint ptCenter = rt.CenterPoint();
						std::vector<CPoint>vertexTopList;
						std::vector<CPoint>vertexBottomList;
						int leftW = rt.Width()/2;
						int rightW = rt.Width() - leftW;
						int topH = rt.Height()/2;
						int bottomH = rt.Height() - topH;
						CPoint pt, pt1;
						if (direction == EMBOSS_LEFT_UP || direction == EMBOSS_RIGHT_DOWN || direction == EMBOSS_LU_RD) {
							pt.x = rt.left + leftW;
							pt.y = rt.top;
							for (int j=0; j<topH+1; j++) {
								pt1.x = pt.x + j;
								pt1.y = pt.y + j;
								vertexTopList.push_back(pt1);
							}

							pt.x = rt.left;
							pt.y = rt.top + topH;
							for (int j=0; j<bottomH+1; j++) {
								pt1.x = pt.x + j;
								pt1.y = pt.y + j;
								vertexBottomList.push_back(pt1);
							}
						} else {
							pt.x = rt.left + leftW;
							pt.y = rt.top;
							for (int j=0; j<topH+1; j++) {
								pt1.x = pt.x - j;
								pt1.y = pt.y + j;
								vertexTopList.push_back(pt1);
							}

							pt.x = rt.right;
							pt.y = rt.top + topH;
							for (int j=0; j<bottomH+1; j++) {
								pt1.x = pt.x - j;
								pt1.y = pt.y + j;
								vertexBottomList.push_back(pt1);
							}
						}

						if (m_shadowType == 0) {
							if (i%2 == 0) {
								rt = m_vShadowInfo[nIndex].lineShape;
							} else {
								rt = m_vShadowInfo[nIndex].lineShape2;
							}
						} else {
							rt = m_vShadowInfo[nIndex].lineShape;
						}

						if (direction == EMBOSS_LEFT_UP || direction == EMBOSS_RIGHT_DOWN || direction == EMBOSS_LU_RD) {
							if (nHandle == hitStart) {
								if (top < ptCenter.y) {
									rt.top = vertexBottomList[0].y;
									rt.left = vertexBottomList[0].x;
								} else if (top >= m_vShadowInfo[nIndex].shadowShape.bottom) {
									int size = (int)vertexBottomList.size()-1;
									rt.top = vertexBottomList[size].y;
									rt.left = vertexBottomList[size].x;
								} else {
									for (int i=0; i<(int)vertexBottomList.size()-1; i++) {
										if (top == vertexBottomList[i].y) {
											rt.top = vertexBottomList[i].y;
											rt.left = vertexBottomList[i].x;
											break;
										}
									}
								}
							} else if (nHandle == hitEnd) {
								if (bottom > ptCenter.y) {
									int size = (int)vertexTopList.size()-1;
									rt.bottom = vertexTopList[size].y;
									rt.right = vertexTopList[size].x;
								} else if (bottom < m_vShadowInfo[nIndex].shadowShape.top) {
									rt.bottom = vertexTopList[0].y;
									rt.right = vertexTopList[0].x;
								} else {
									for (int i=0; i<(int)vertexTopList.size()-1; i++) {
										if (bottom == vertexTopList[i].y) {
											rt.bottom = vertexTopList[i].y;
											rt.right = vertexTopList[i].x;
											break;
										}
									}
								}	
							}
						} else {
							if (nHandle == hitStart) {
								if (top < m_vShadowInfo[nIndex].shadowShape.top) {
									rt.top = vertexTopList[0].y;
									rt.left = vertexTopList[0].x;
								} else if (top >= ptCenter.y) {
									int size = (int)vertexTopList.size()-1;
									rt.top = vertexTopList[size].y;
									rt.left = vertexTopList[size].x;
								} else {
									for (int i=0; i<(int)vertexTopList.size()-1; i++) {
										if (top == vertexTopList[i].y) {
											rt.top = vertexTopList[i].y;
											rt.left = vertexTopList[i].x;
											break;
										}
									}
								}
							} else if (nHandle == hitEnd) {
								if (bottom > m_vShadowInfo[nIndex].shadowShape.bottom) {
									int size = (int)vertexBottomList.size()-1;
									rt.bottom = vertexBottomList[size].y;
									rt.right = vertexBottomList[size].x;
								} else if (bottom < ptCenter.y) {
									rt.bottom = vertexBottomList[0].y;
									rt.right = vertexBottomList[0].x;
								} else {
									for (int i=0; i<(int)vertexBottomList.size()-1; i++) {
										if (bottom == vertexBottomList[i].y) {
											rt.bottom = vertexBottomList[i].y;
											rt.right = vertexBottomList[i].x;
											break;
										}
									}
								}	
							}
						}
					}

					if (m_shadowType == 0) {	// One Way has two lines
						if (i%2 == 0) {
							m_vShadowInfo[nIndex].lineShape = rt;
						} else {
							m_vShadowInfo[nIndex].lineShape2 = rt;
						}

						int nTemp = 0;
						if (m_vShadowInfo[nIndex].lineShape.top > m_vShadowInfo[nIndex].lineShape2.top) {
							nTemp = m_vShadowInfo[nIndex].lineShape.top;
							m_vShadowInfo[nIndex].lineShape.top = m_vShadowInfo[nIndex].lineShape2.top;
							m_vShadowInfo[nIndex].lineShape2.top = nTemp;
							nTemp = m_vShadowInfo[nIndex].lineShape.left;
							m_vShadowInfo[nIndex].lineShape.left = m_vShadowInfo[nIndex].lineShape2.left;
							m_vShadowInfo[nIndex].lineShape2.left = nTemp;
						}
						if (m_vShadowInfo[nIndex].lineShape.bottom > m_vShadowInfo[nIndex].lineShape2.bottom) {
							nTemp = m_vShadowInfo[nIndex].lineShape.bottom;
							m_vShadowInfo[nIndex].lineShape.bottom = m_vShadowInfo[nIndex].lineShape2.bottom;
							m_vShadowInfo[nIndex].lineShape2.bottom = nTemp;
							nTemp = m_vShadowInfo[nIndex].lineShape.right;
							m_vShadowInfo[nIndex].lineShape.right = m_vShadowInfo[nIndex].lineShape2.right;
							m_vShadowInfo[nIndex].lineShape2.right = nTemp;
						}
						if (m_vShadowInfo[nIndex].lineShape.left > m_vShadowInfo[nIndex].lineShape2.left) {
							nTemp = m_vShadowInfo[nIndex].lineShape.left;
							m_vShadowInfo[nIndex].lineShape.left = m_vShadowInfo[nIndex].lineShape2.left;
							m_vShadowInfo[nIndex].lineShape2.left = nTemp;
							nTemp = m_vShadowInfo[nIndex].lineShape.top;
							m_vShadowInfo[nIndex].lineShape.top = m_vShadowInfo[nIndex].lineShape2.top;
							m_vShadowInfo[nIndex].lineShape2.top = nTemp;
						}
						if (m_vShadowInfo[nIndex].lineShape.right > m_vShadowInfo[nIndex].lineShape2.right) {
							nTemp = m_vShadowInfo[nIndex].lineShape.right;
							m_vShadowInfo[nIndex].lineShape.right = m_vShadowInfo[nIndex].lineShape2.right;
							m_vShadowInfo[nIndex].lineShape2.right = nTemp;
							nTemp = m_vShadowInfo[nIndex].lineShape.bottom;
							m_vShadowInfo[nIndex].lineShape.bottom = m_vShadowInfo[nIndex].lineShape2.bottom;
							m_vShadowInfo[nIndex].lineShape2.bottom = nTemp;
						}
					} else {	// Two Way has one line
						m_vShadowInfo[nIndex].lineShape = rt;
					}

					pLine = m_LineList.GetLineItem(i);
					m_pLineTracker->SetSelection(pLine, i);

					hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_SHAPE_MODIFY, nIndex);
				} else {
					hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_SHAPE_MODIFY, -1);	// added by eric at 20130306
					oglClearSelection();
				}
				bNeedRedraw = TRUE;

				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_LBUTTONDOWN, MAKELPARAM(m_mouse_x, m_mouse_y));
			} else {
				int x, y;
				oglTranslatePoint(point.x, point.y, x, y);
				m_ptDown.x = x;
				m_ptDown.y = y;
				m_selLine=-1;
				BOOL bHit=IsHitLineRect(m_ptDown, m_selLine);
				if (bHit) {
					m_bLMouseDown = TRUE;
					m_rtOldRect=m_lineRect[m_selLine];
					::SetCursor(::LoadCursor(NULL, IDC_ARROW));

					bNeedRedraw = TRUE;
				}
			}
		}
		break;

		// added by eric at 20121121
	case LM_DRAW_LINE:
		m_lineRect[m_lineRects].right =  m_lineRect[m_lineRects].left = m_mouse_x;
		m_lineRect[m_lineRects].bottom =  m_lineRect[m_lineRects].top = m_mouse_y;
		m_lineRects++;
		hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_LBUTTONDOWN, MAKELPARAM(m_mouse_x, m_mouse_y));
		break;

	case LM_DRAW_RECTANGLE:
		// modified by danny
		m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects].right = m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects].left = m_mouse_x;
		m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects].bottom = m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects].top = m_mouse_y;
		m_mapRectInfo[ NULL ].rect_type[m_mapRectInfo[ NULL ].rects] = SHAPE_RECT;
		m_mapRectInfo[ NULL ].rects++;
		break;
	case LM_DRAW_CIRCLE:
		m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects].right = m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects].left = m_mouse_x;
		m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects].bottom = m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects].top = m_mouse_y;
		m_mapRectInfo[ NULL ].rect_type[m_mapRectInfo[ NULL ].rects] = SHAPE_CIRCLE;
		m_mapRectInfo[ NULL ].rects++;
		break;
	case LM_DRAW_ELLIPSE:
		m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects].right = m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects].left = m_mouse_x;
		m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects].bottom = m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects].top = m_mouse_y;
		m_mapRectInfo[ NULL ].rect_type[m_mapRectInfo[ NULL ].rects] = SHAPE_ELLIPSE;
		m_mapRectInfo[ NULL ].rects++;
		break;
	case LM_DRAW_POLYGON:
		if (m_poly_drawing == FALSE) 
		{
			m_poly_drawing = TRUE;
			m_LastLeftDownPos = m_LeftDownPos;	//added by danny at 20100204
			m_poly_points[m_polygons] = 1;
			m_polygon[m_polygons][0].x = m_mouse_x;
			m_polygon[m_polygons][0].y = m_mouse_y;
		} 
		else 
		{
			int diffx1, diffy1, diffx2, diffy2;
			diffx1 = abs(m_mouse_x - m_polygon[m_polygons][0].x);
			diffy1 = abs(m_mouse_y - m_polygon[m_polygons][0].y);
			//diffx2 = abs(m_LastLeftDownPos.x - point.x);
			//diffy2 = abs(m_LastLeftDownPos.y - point.y);
			int fx, fy;
			oglProjectPoint(fx, fy, m_polygon[m_polygons][0].x, m_polygon[m_polygons][0].y);
			diffx2 = abs(fx - point.x);
			diffy2 = abs(fy - point.y);
			if ((diffx1<2 && diffy1<2) || (diffx2<10 && diffy2<10))
			{
				// End of polygon drawing.
				m_poly_drawing = FALSE;
				//m_polygons++;	// removed by danny at 20091214
			} 
			else 
			{
				m_polygon[m_polygons][m_poly_points[m_polygons]].x = m_mouse_x;
				m_polygon[m_polygons][m_poly_points[m_polygons]].y = m_mouse_y;
				m_poly_points[m_polygons]++;
			}
		}
		bNeedRedraw = TRUE;
		break;
	case LM_CHECK_DEFECT:
		if (dcMsgId != 0 && m_num_defect > 0) {
			if((m_mouse_sp>=0)&&(m_mouse_sp<m_num_defect)){ //seanchen 20140918-01
				GetParent()->SendMessage(dcMsgId, dcWpara, (LPARAM)&m_defectPiece[m_mouse_sp]); //eric chao 20160125
			}
		}
		break;
	case LM_DRAW_ERASER:
		{
			ComputeEraserSize(point.x, point.y);
			hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_DOWN_ERASE);
		}
		break;

	case LM_DRAW_RGBCURVE_X:
	case LM_DRAW_RGBCURVE_Y:
		hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_LBUTTONDOWN,
			MAKELPARAM(m_mouse_x, m_mouse_y));	// Beagle 20130816
		bNeedRedraw = TRUE;
		break;

		// added by eric at 20130429
	case LM_SET_COLORTOOL:
		hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_LBUTTONDOWN,
			MAKELPARAM(m_mouse_x, m_mouse_y));	// Beagle 20130816 modified.
		break;

		// added by eric at 20130923
	case LM_SET_COLORDERIVATE:
		{
			m_vRectTrackerList.clear();
			m_nHitHandle = -1;
			m_bLMouseDown = TRUE;
			m_nSelectedIndex = -1;	// added by eric at 20130301
			int x, y;
			oglTranslatePoint(point.x, point.y, x, y);
			m_ptDown.x = x;
			m_ptDown.y = y;

			for (int i=1; i<(int)m_vColorDerivateBox.size(); i++) {
				CRect rt = m_vColorDerivateBox[i];
				if (rt.PtInRect(m_ptDown) > 0) {
					m_nSelectedIndex = i;
					m_nHitHandle = 8;
					::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
					CMyTracker tracker;
					int left, top, right, bottom;
					oglProjectPoint(left, top, rt.left, rt.top);
					oglProjectPoint(right, bottom, rt.right, rt.bottom);
					CRect rtTracker = CRect(left, top, right, bottom);
					tracker.m_rect = rtTracker;
					tracker.m_nStyle = CRectTracker::solidLine|CRectTracker::hatchedBorder;
					m_vRectTrackerList.push_back(tracker);
					m_rtSelected = rt;
					break;
				}
			}
			oglDrawScene();
		}
		break;
	default:
		break;
	}

	if (bNeedRedraw) { oglDrawScene(FALSE); }

	CWnd::OnLButtonDown(nFlags, point);
}

// Beagle 20130425 rewrite this function.
void COpenGLControl::OnGet45DegreeRectagleCenterLine(CRect &rt, CRect rtShape, BOOL bClockwise)
{
	if (bClockwise) {
		// 左上-右下方向
		rt.left   = (long int) ceilf((rtShape.left*3.0f+rtShape.right)/4.0f);
		rt.right  = (long int) ceilf((rtShape.left+rtShape.right*3.0f)/4.0f);
		rt.top    = (long int) floorf((rtShape.top*3.0f+rtShape.bottom)/4.0f);
		rt.bottom = (long int) floorf((rtShape.top+rtShape.bottom*3.0f)/4.0f);
	} else {
		// 左下-右上方向
		rt.left   = (long int) floorf((rtShape.left*3.0f+rtShape.right)/4.0f);
		rt.right  = (long int) floorf((rtShape.left+rtShape.right*3.0f)/4.0f);
		rt.top    = (long int) floorf((rtShape.top+rtShape.bottom*3.0f)/4.0f);
		rt.bottom = (long int) floorf((rtShape.top*3.0f+rtShape.bottom)/4.0f);
	}
}

void COpenGLControl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_LeftButtonDown == FALSE) {	// added by eric at 20140724
		if (m_RightButtonDown) {	// added by eric at 20131223
			return;
		}
	}

	ReleaseCapture();

	LMKeyMode lmMode = m_lmMode;	// added by eric at 20120803

	switch (m_lmMode)
	{
	case LM_IGNORE:
		OnMouseUpShape(point);	// added by eric at 20120330
		break;
	case LM_TRANSLATE:
		break;
	case LM_SCALE:
		break;
	case LM_SET_ANCHOR:
	case LM_SET_WEAKENANCHOR:
	case LM_SET_DISTANCEANCHOR:
		{
			// added by eric at 20150420
			if (m_LeftButtonDown) {
				if(m_rtAnchorRect.bottom<m_rtAnchorRect.top){
					LONG temp = m_rtAnchorRect.bottom;
					m_rtAnchorRect.bottom = m_rtAnchorRect.top;
					m_rtAnchorRect.top = temp;
				}
				if(m_rtAnchorRect.right<m_rtAnchorRect.left){
					LONG temp = m_rtAnchorRect.right;
					m_rtAnchorRect.right = m_rtAnchorRect.left;
					m_rtAnchorRect.left = temp;
				}

				int position[4];
				position[0] = m_rtAnchorRect.left;
				position[1] = m_rtAnchorRect.top;
				position[2] = m_rtAnchorRect.right;
				position[3] = m_rtAnchorRect.bottom;

				if (m_lmMode==LM_SET_ANCHOR) {
					hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_ANCHOR, (LPARAM)position);
				} else if (m_lmMode==LM_SET_WEAKENANCHOR) {
					hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_ANCHOR_WEAKEN, (LPARAM)position);
				} else if (m_lmMode==LM_SET_DISTANCEANCHOR) {
					hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_ANCHOR_DISTANCE, (LPARAM)position);
				}

				m_bAnchorRect=FALSE;
				m_rtAnchorRect.left=m_rtAnchorRect.right=m_rtAnchorRect.top=m_rtAnchorRect.bottom=0;
				oglDrawScene();
			}
		}
		break;
		// added by eric at 20130107
	case LM_DRAW_LAYOUTSHAPE:
		{
			if (m_bLMouseDown) {
				if (m_nHitHandle >= 4 && m_nHitHandle <= 7) {

					int nTemp = 0;
					if (m_rtSelected.left > m_rtSelected.right)
					{
						nTemp = m_rtSelected.left;
						m_layoutRect[m_selLayoutRect].left = m_rtSelected.left = m_rtSelected.right;
						m_layoutRect[m_selLayoutRect].right = m_rtSelected.right = nTemp;
					}
					if (m_rtSelected.top >m_rtSelected.bottom)
					{
						nTemp = m_rtSelected.top;
						m_layoutRect[m_selLayoutRect].top = m_rtSelected.top = m_rtSelected.bottom;
						m_layoutRect[m_selLayoutRect].bottom = m_rtSelected.bottom = nTemp;
					}

					int w = abs(m_rtSelected.right - m_rtSelected.left);
					int h = abs(m_rtSelected.bottom - m_rtSelected.top);
					if (w == 0 && h == 0){
						w += 1; h += 1;
					} else if (w == 0){
						w += 1;
					} else if (h == 0){
						h += 1;
					}

					m_rtSelected.right = m_rtSelected.left + w;
					m_rtSelected.bottom = m_rtSelected.top + h;

					int width = abs((m_rtSelected.right - m_rtSelected.left) - (m_layoutRect[m_selLayoutRect].right - m_layoutRect[m_selLayoutRect].left));
					int height = abs((m_rtSelected.bottom - m_rtSelected.top) - (m_layoutRect[m_selLayoutRect].bottom - m_layoutRect[m_selLayoutRect].top));

					for (int c=0; c<m_layoutRects; c++) {
						if (m_XPos != -1) {
							if (m_nHitHandle == 5) {	// Right
								if (m_layoutRect[c].right == m_XPos) {
									m_layoutRect[c].right = m_rtSelected.right;
								}
								if (m_layoutRect[c].left == m_XPos) {
									if (m_rtSelected.right > m_layoutRect[c].left)
										m_layoutRect[c].left += width;
									else
										m_layoutRect[c].left -= width;
								}

							}

							if (m_nHitHandle == 7) {	// Left
								if (m_layoutRect[c].right == m_XPos) {
									if (m_rtSelected.left > m_layoutRect[c].right)
										m_layoutRect[c].right += width;
									else
										m_layoutRect[c].right -= width;
								}
								if (m_layoutRect[c].left == m_XPos) {
									m_layoutRect[c].left = m_rtSelected.left;
								}
							}
						}
						if (m_YPos != -1) {
							if (m_nHitHandle == 4) {	// Top
								if (m_layoutRect[c].bottom == m_YPos) {
									if (m_rtSelected.top > m_layoutRect[c].bottom)
										m_layoutRect[c].bottom += height;
									else
										m_layoutRect[c].bottom -= height;
								}
								if (m_layoutRect[c].top == m_YPos) {
									m_layoutRect[c].top = m_rtSelected.top;
								}
							}
							if (m_nHitHandle == 6) {	// Bottom
								if (m_layoutRect[c].bottom == m_YPos) {
									m_layoutRect[c].bottom = m_rtSelected.bottom;
								}
								if (m_layoutRect[c].top == m_YPos) {
									if (m_rtSelected.bottom > m_layoutRect[c].top)
										m_layoutRect[c].top += height;
									else
										m_layoutRect[c].top -= height;
								}
							}
						}
					}

					hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_LAYOUT_REOPS);
					InvalidateRect(NULL, FALSE);
				} else {
					oglTranslatePoint(point.x, point.y, m_mouse_x, m_mouse_y);
					hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_LAYOUT_INDEX);
				}

				m_nHitHandle = -1;
				m_bLMouseDown = FALSE;
			}
		}
		break;

		// added by eric at 20130208
	case LM_MODIFY_SHADOWRECT:
		{
			m_bLMouseDown = FALSE;
			m_nHitHandle = -1;
			hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_SHAPE_MODIFY, m_nSelectedIndex);
		}
		break;

		// added by eric at 20130208
	case LM_MODIFY_LINE:
		{
			if (m_pLineTracker) {
			} else {
				if (m_bLMouseDown) {

					BOOL bDrop=TRUE;
					if (m_bCrossOverLineOrder==FALSE) {
						for (int i=0; i<m_lineRects-1; i++) {
							if (m_lineRect[i].right>m_lineRect[i+1].right) {
								bDrop=FALSE;
								break;
							}
						}
						if (bDrop==FALSE) {	
							m_lineRect[m_selLine]=m_rtOldRect;
							m_lineRect[m_selLine].right=m_rtOldRect.right;
							m_lineRect[m_selLine].left=m_rtOldRect.left;
						}
					}

					// added by eric at 20161217
					if (bDrop==TRUE) {
						hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_LINE, m_selLine);
					}

					::SetCursor(::LoadCursor(NULL, IDC_ARROW));
					m_selLine=-1;
					m_bLMouseDown=FALSE;
					memset(&m_rtOldRect,0,sizeof(RECT));

					oglDrawScene();
				}
			}
		}
		break;

		// added by eric at 20121121, 20121122
	case LM_DRAW_LINE:
		if (m_LeftButtonDown){
			// modified by amike at 20130328
			// Vertical Line
			if (m_bSetHoriOrVertLine) {
				int w = abs(m_lineRect[m_lineRects-1].right-m_lineRect[m_lineRects-1].left);
				int h = abs(m_lineRect[m_lineRects-1].top-m_lineRect[m_lineRects-1].bottom);
				if(w>=h){
					if (m_lineRect[m_lineRects-1].top != m_lineRect[m_lineRects-1].bottom) {
						m_lineRect[m_lineRects-1].bottom = m_lineRect[m_lineRects-1].top;
					}
				}
				else{
					if (m_lineRect[m_lineRects-1].left != m_lineRect[m_lineRects-1].right) {
						m_lineRect[m_lineRects-1].right = m_lineRect[m_lineRects-1].left;
					}
				}
			}
			hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_LINE, m_lineRects-1);
		}
		break;
	case LM_DRAW_RECTANGLE:
		if (m_LeftButtonDown){
			// added by amike at 20110905
			if(m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].bottom<m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].top){
				LONG temp = m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].bottom;
				m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].bottom = m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].top;
				m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].top = temp;
			}
			if(m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].right<m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].left){
				LONG temp = m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].right;
				m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].right = m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].left;
				m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].left = temp;
			}
			hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_RECTANGLE, m_mapRectInfo[ NULL ].rects-1);	// modified by danny
		}
		break;
	case LM_DRAW_CIRCLE:
		if (m_LeftButtonDown)
			hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_CIRCLE, m_mapRectInfo[ NULL ].rects-1);		// modified by danny
		break;
	case LM_DRAW_ELLIPSE:
		if (m_LeftButtonDown)
			hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_ELLIPSE, m_mapRectInfo[ NULL ].rects-1);	// modified by danny
		break;
	case LM_DRAW_POLYGON:
		if (m_poly_drawing == FALSE)
			hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_POLYGON, m_polygons);	// added by danny
		break;
	case LM_DRAW_ERASER:	// added at 20091019 by danny
		hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_UP_ERASE);
		break;
	case LM_SET_WEAKENPAINT:	// added by eric at 20110929
		if (m_LeftButtonDown)
		{
			if (m_PaintMode == PAINT_RECT)
			{
				int x, y;
				oglTranslatePoint(point.x, point.y, x, y);
				m_rcWeakenColor.right = x;
				m_rcWeakenColor.bottom = y;

				if(m_rcWeakenColor.bottom<m_rcWeakenColor.top){
					LONG temp = m_rcWeakenColor.bottom;
					m_rcWeakenColor.bottom = m_rcWeakenColor.top;
					m_rcWeakenColor.top = temp;
				}
				if(m_rcWeakenColor.right<m_rcWeakenColor.left){
					LONG temp = m_rcWeakenColor.right;
					m_rcWeakenColor.right = m_rcWeakenColor.left;
					m_rcWeakenColor.left = temp;
				}
			}
			else if (m_PaintMode == PAINT_PAINTBRUSH)	// added by eric at 20120330
			{
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_UP_ADD);
			}
		}
		break;
		// added by eric at 20130923
	case LM_SET_COLORDERIVATE:
		{
			m_bLMouseDown = FALSE;
			m_nHitHandle = -1;
		}
		break;
	default:
		break;
	}

	// added by eric at 20120803
	if (m_LeftButtonDown == TRUE) {
		if (lmMode == LM_DRAW_RECTANGLE || lmMode == LM_DRAW_CIRCLE || lmMode == LM_DRAW_ELLIPSE || lmMode == LM_DRAW_ERASER){
			hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_LBUTTONUP);
		} else if (lmMode == LM_DRAW_POLYGON && m_poly_drawing == FALSE) {
			hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_LBUTTONUP);
		}
	}

	m_LeftButtonDown = FALSE;

	CWnd::OnLButtonUp(nFlags, point);
}

// Beagle 20110704
void COpenGLControl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    if (m_lmMode == LM_MODIFY_LINE) {
        int x, y;
        oglTranslatePoint(point.x, point.y, x, y);
        m_ptDown.x = x;
        m_ptDown.y = y;
        int idx = -1;
        BOOL bHit = IsHitLineRect(m_ptDown, idx);
        if (bHit == TRUE) {
            m_selLine = idx;
            hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_LBUTTON_DBCLK, m_selLine);
        }
    }
    else {
        int imgX, imgY;

        oglMakeCurrent();
        oglTranslatePoint(point.x, point.y, imgX, imgY);
        hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_SHOWRGB, MAKELPARAM(imgX, imgY));
    }
}
// modified by eric at 20120330
void COpenGLControl::ComputeEraserSize(int mx, int my, int mRadius)	// input mouse position, convert monitor 32x32 size to the real image size
{
	// eraser circle center
	oglTranslatePoint(mx, my, m_oglEraserX, m_oglEraserY);
	m_oglEraserX++;
	m_oglEraserY++;
	m_oglEraserRadius = (int)(mRadius / (m_norm_scale[0] * m_glScale));	// modified by eric at 20120330
}

void COpenGLControl::OnMouseMove(UINT nFlags, CPoint point)
{
	BOOL need_redraw = FALSE;
	CSize dist;
	int	x, y;

	oglTranslatePoint(point.x, point.y, m_mouse_x, m_mouse_y, &m_mouse_sp);	// Beagle 20130424 added.

	if (m_show_cursor_outline) { need_redraw = TRUE; }	// Beagle 20130424 added.

	switch (m_lmMode)
	{
	case LM_IGNORE:
		OnMouseMoveShape(point);	// added by eric at 20120330
		break;
	case LM_MODIFY_LINE:
		if (m_bLMouseDown) {
			if (m_selLine<m_lineRects) {
                if ( m_lineRect[m_selLine].left == m_lineRect[m_selLine].right)
                {
				    m_lineRect[m_selLine].right=m_mouse_x;
				    m_lineRect[m_selLine].left=m_mouse_x;
                }
                else
                {
                    m_lineRect[m_selLine].top=m_mouse_y;
				    m_lineRect[m_selLine].bottom=m_mouse_y;
                }
				need_redraw = TRUE;

                hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_LINE_MOVE, m_selLine);
			}
		} else {
			CPoint pt;
			pt.x=m_mouse_x;
			pt.y=m_mouse_y;
			int idx=0;
			BOOL bHit = IsHitLineRect(pt, idx);
			if (bHit) {
				::SetCursor(::LoadCursor(NULL, bHit == 1 ? IDC_SIZEWE : IDC_SIZENS));
			}
		}
		break;
	case LM_TRANSLATE:
		if (m_LeftButtonDown)
		{
			dist = m_LeftDownPos - point;
			m_LeftDownPos = point;
			m_xTranslate -= (GLfloat)(dist.cx)/m_glScale;	// Beagle 20111104 modified.
			m_yTranslate -= (GLfloat)(dist.cy)/m_glScale;
			need_redraw = TRUE;	// Beagle 20130424 modified.
		}
		break;
	case LM_SCALE:
		if (m_LeftButtonDown)	// Beagle 20130103 modified.
		{
			dist = point - m_LeftDownPos;
			m_LeftDownPos = point;

			if (abs(dist.cx) > abs(dist.cy)) {
				oglScaleChange(1.0f+dist.cx/1000.0f,
					(float)m_ScaleCenter.x, (float)m_ScaleCenter.y);
			} else {
				oglScaleChange(1.0f+dist.cy/1000.0f,
					(float)m_ScaleCenter.x, (float)m_ScaleCenter.y);
			}
			need_redraw = TRUE;	// Beagle 20130424 modified.
		}
		break;
	case LM_SET_ANCHOR:
	case LM_SET_WEAKENANCHOR:
	case LM_SET_DISTANCEANCHOR:
		// added by eric at 20150420
		if (m_LeftButtonDown) {
			oglTranslatePoint(point.x, point.y, x, y);
			m_rtAnchorRect.right = x;
			// modified by eric at 20150427
			int w=m_rtAnchorRect.right-m_rtAnchorRect.left;
			if (w>m_AnchorWidth) {
				m_rtAnchorRect.right=m_rtAnchorRect.left+m_AnchorWidth;
			}	
			m_rtAnchorRect.bottom = y;
			int h=m_rtAnchorRect.bottom-m_rtAnchorRect.top;
			if (h>m_nAnchorHeight) {
				m_rtAnchorRect.bottom=m_rtAnchorRect.top+m_nAnchorHeight;
			}
		}
		need_redraw = TRUE;
		break;
	case LM_SET_BARCODEANCHOR:	// added by eric at 20110801
	case LM_DRAW_SHADOWRECT:	// added by eric at 20130208
		// added by danny at 20100210
		{
			need_redraw = TRUE;	// Beagle 20130424 modified.
		}
		break;

		// added by eric at 20130208
	case LM_MODIFY_SHADOWRECT:
		{
			if (m_vRectTrackerList.size() == 1) {
				int x, y;
				oglTranslatePoint(point.x, point.y, x, y);
				CPoint ptMouse;
				ptMouse.x = x;
				ptMouse.y = y;
// 				TRACE(_T("left down  = %d, Hit handle = %d\n"), m_bLMouseDown, m_nHitHandle);
				if (m_bLMouseDown && (m_nHitHandle == 8)) {
					CPoint ptOffset;
					ptOffset.x = x - m_ptDown.x;
					ptOffset.y = y - m_ptDown.y;

					if (ptOffset.x == 0 && ptOffset.y == 0)
						return;

					m_ptDown.x = x;
					m_ptDown.y = y;

					if (m_rtSelected.left + ptOffset.x >= 0 && m_rtSelected.top + ptOffset.y >= 0 &&
						m_rtSelected.right + ptOffset.x < m_width[0] && m_rtSelected.bottom + ptOffset.y < m_height[0]) {
						m_rtSelected.left += ptOffset.x;
						m_rtSelected.right += ptOffset.x;
						m_rtSelected.top += ptOffset.y;
						m_rtSelected.bottom += ptOffset.y;

						m_vShadowInfo[m_nSelectedIndex].shadowShape.left += ptOffset.x;
						m_vShadowInfo[m_nSelectedIndex].shadowShape.right += ptOffset.x;
						m_vShadowInfo[m_nSelectedIndex].shadowShape.top += ptOffset.y;
						m_vShadowInfo[m_nSelectedIndex].shadowShape.bottom += ptOffset.y;

						// modified by eric at 20130219
						if (m_vShadowInfo[m_nSelectedIndex].lineShape.left != 0 && m_vShadowInfo[m_nSelectedIndex].lineShape.top != 0 &&
							m_vShadowInfo[m_nSelectedIndex].lineShape.right != 0 && m_vShadowInfo[m_nSelectedIndex].lineShape.bottom != 0) {
							m_vShadowInfo[m_nSelectedIndex].lineShape.left += ptOffset.x;
							m_vShadowInfo[m_nSelectedIndex].lineShape.right += ptOffset.x;
							m_vShadowInfo[m_nSelectedIndex].lineShape.top += ptOffset.y;
							m_vShadowInfo[m_nSelectedIndex].lineShape.bottom += ptOffset.y;
						}

						// modified by eric at 20130219
						if (m_shadowType == 0) {	// One Way has two lines
							if (m_vShadowInfo[m_nSelectedIndex].lineShape2.left != 0 && m_vShadowInfo[m_nSelectedIndex].lineShape2.top != 0 &&
								m_vShadowInfo[m_nSelectedIndex].lineShape2.right != 0 && m_vShadowInfo[m_nSelectedIndex].lineShape2.bottom != 0) {
								m_vShadowInfo[m_nSelectedIndex].lineShape2.left += ptOffset.x;
								m_vShadowInfo[m_nSelectedIndex].lineShape2.right += ptOffset.x;
								m_vShadowInfo[m_nSelectedIndex].lineShape2.top += ptOffset.y;
								m_vShadowInfo[m_nSelectedIndex].lineShape2.bottom += ptOffset.y;
							}
						}
						OnUpdateShadowLine();
					}

					int width = m_rtSelected.Width();
					int height = m_rtSelected.Height();
					if (ptMouse.x >= m_width[0]-1) {
						m_rtSelected.right = m_vShadowInfo[m_nSelectedIndex].shadowShape.right = m_width[0]-1;
						m_rtSelected.left = m_vShadowInfo[m_nSelectedIndex].shadowShape.left = m_width[0]-1-width;
					} else if (ptMouse.x == 0) {
						m_rtSelected.left = m_vShadowInfo[m_nSelectedIndex].shadowShape.left = 0;
						m_rtSelected.right = m_vShadowInfo[m_nSelectedIndex].shadowShape.right = width;
					}
					if (ptMouse.y >= m_height[0]-1) {
						m_rtSelected.bottom = m_vShadowInfo[m_nSelectedIndex].shadowShape.bottom = m_height[0]-1;
						m_rtSelected.top = m_vShadowInfo[m_nSelectedIndex].shadowShape.top = m_height[0]-1-height;
					} else if (ptMouse.y == 0) {
						m_rtSelected.top = m_vShadowInfo[m_nSelectedIndex].shadowShape.top = 0;
						m_rtSelected.bottom = m_vShadowInfo[m_nSelectedIndex].shadowShape.bottom = height;
					}

					m_vRectTrackerList.clear();
					int left, top, right, bottom;
					// modified by eric at 20130219, 20130306
					int direction = m_vShadowInfo[m_nSelectedIndex].direction;
					if (direction == EMBOSS_LEFT_UP || direction == EMBOSS_RIGHT_DOWN || direction == EMBOSS_LU_RD ||
						direction == EMBOSS_LEFT_DOWN || direction == EMBOSS_RIGHT_UP || direction == EMBOSS_LD_RU) {
							oglProjectPoint(left, top, m_rtSelected.left, m_rtSelected.top+1);
							oglProjectPoint(right, bottom, m_rtSelected.right, m_rtSelected.bottom+1);
					} else {
						oglProjectPoint(left, top, m_rtSelected.left, m_rtSelected.top);
						oglProjectPoint(right, bottom, m_rtSelected.right, m_rtSelected.bottom);
					}

					CRect rtTracker = CRect(left, top, right, bottom);
					CMyTracker tracker;
					tracker.m_rect = rtTracker;
					tracker.m_nStyle = CRectTracker::dottedLine|CRectTracker::hatchedBorder|CRectTracker::resizeInside;
					m_vRectTrackerList.push_back(tracker);
				} else {
					int nHitHandle = m_vRectTrackerList[0].HitTest(point);
					if (nHitHandle == 8)
						::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
					else
						::SetCursor(::LoadCursor(NULL, IDC_ARROW));	
				}
				need_redraw = TRUE;	// Beagle 20130424 modified.
			}
		}
		break;

		// added by eric at 20130107
	case LM_DRAW_LAYOUTSHAPE:
		{
			if (m_vRectTrackerList.size() == 1) {
				int x, y;
				oglTranslatePoint(point.x, point.y, x, y);
				CPoint ptMouse;
				ptMouse.x = x;
				ptMouse.y = y;

				if (m_bLMouseDown && (m_nHitHandle >= 4 && m_nHitHandle <=7)) {
					CPoint ptOffset;
					ptOffset.x = x - m_ptDown.x;
					ptOffset.y = y - m_ptDown.y;

					if (ptOffset.x == 0 && ptOffset.y == 0)
						return;

					m_ptDown.x = x;
					m_ptDown.y = y;

					if (m_nHitHandle == 4)	// Top
					{
						if (m_ptDown.y == 0 && ptOffset.y == 0) {
							m_rtSelected.top = 0;
						}
						else {
							if (m_rtSelected.top + ptOffset.y < m_rtSelected.bottom-10) {
								if (m_rtSelected.top + ptOffset.y < 0) {	// modified by eric at 20130128
									m_rtSelected.top = 0;
								} else {
									m_rtSelected.top += ptOffset.y;
								}
							}
						}
					}
					else if (m_nHitHandle == 5)	// Right
					{
						if (m_ptDown.x == m_width[0]-1 && ptOffset.x == 0){
							m_rtSelected.right = m_width[0]-1;
						}
						else{
							if (m_rtSelected.right + ptOffset.x > m_rtSelected.left+10) {
								if (m_rtSelected.right + ptOffset.x > m_width[0]) {	// modified by eric at 20130128
									m_rtSelected.right = m_width[0];
								} else {
									m_rtSelected.right += ptOffset.x;
								}
							}
						}
					}
					else if (m_nHitHandle == 6)	// Bottom
					{
						if (m_ptDown.y == m_height[0]-1 && ptOffset.y == 0) {
							m_rtSelected.bottom = m_height[0]-1;
						}
						else {
							if (m_rtSelected.bottom + ptOffset.y > m_rtSelected.top+10) {
								if (m_rtSelected.bottom + ptOffset.y > m_height[0]) {	// modified by eric at 20130128
									m_rtSelected.bottom = m_height[0];
								} else {
									m_rtSelected.bottom += ptOffset.y;
								}
							}
						}
					}
					else if (m_nHitHandle == 7)	// Left
					{
						if (m_ptDown.x == 0 && ptOffset.x == 0) {
							m_rtSelected.left = 0;
						}
						else {
							if (m_rtSelected.left + ptOffset.x < m_rtSelected.right-10) {
								if (m_rtSelected.left + ptOffset.x < 0) {	// modified by eric at 20130128
									m_rtSelected.left = 0;
								} else {
									m_rtSelected.left += ptOffset.x;
								}
							}
						}
					}

					need_redraw = TRUE;	// Beagle 20130424 modified.

					if (m_tracker)
					{
						delete m_tracker;
						m_tracker = NULL;
					}

					int left, top, right, bottom;
					oglProjectPoint(left, top, m_rtSelected.left, m_rtSelected.top);
					oglProjectPoint(right, bottom, m_rtSelected.right,m_rtSelected.bottom);

					m_vRectTrackerList.clear();

					CRect rtTracker = CRect(left, top, right, bottom);
					// modified by eric at 20130118
					CMyTracker tracker;
					//CRectTracker tracker;
					tracker.m_rect = rtTracker;
					tracker.m_nStyle = CRectTracker::dottedLine|CRectTracker::hatchedBorder|CRectTracker::resizeInside;
					m_vRectTrackerList.push_back(tracker);
				} else {
					int nHitHandle = m_vRectTrackerList[0].HitTest(point);
					// modified by eric at 20130128
					if (nHitHandle == 4 || nHitHandle == 6)
						::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
					else if (nHitHandle == 5 || nHitHandle == 7)
						::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
					else
						::SetCursor(::LoadCursor(NULL, IDC_ARROW));	
				}
			}
		}
		break;

		// added by eric at 20121121
	case LM_DRAW_LINE:
		if (m_LeftButtonDown)
		{
			oglTranslatePoint(point.x, point.y, x, y);
			m_lineRect[m_lineRects-1].right = x;
			m_lineRect[m_lineRects-1].bottom = y;
			need_redraw = TRUE;	// Beagle 20130424 modified.
		}
		break;
	case LM_DRAW_RECTANGLE:
		if (m_LeftButtonDown)
		{
			oglTranslatePoint(point.x, point.y, x, y);
			m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].right = x;
			m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].bottom = y;
			need_redraw = TRUE;	// Beagle 20130424 modified.
		}
		break;
	case LM_DRAW_CIRCLE:
		if (m_LeftButtonDown)
		{
			oglTranslatePoint(point.x, point.y, x, y);
			m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].right = x;
			m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].bottom = y;
			need_redraw = TRUE;	// Beagle 20130424 modified.
		}
		break;
	case LM_DRAW_ELLIPSE:
		if (m_LeftButtonDown)
		{
			oglTranslatePoint(point.x, point.y, x, y);
			m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].right = x;
			m_mapRectInfo[ NULL ].rect[m_mapRectInfo[ NULL ].rects-1].bottom = y;
			need_redraw = TRUE;	// Beagle 20130424 modified.
		}
		break;
	case LM_DRAW_POLYGON:
		if (m_poly_drawing) 
		{
			need_redraw = TRUE;	// Beagle 20130424 modified.
		}
		break;
	case LM_DRAW_ERASER:	// added at 20091019 by danny
		{
			if (m_LeftButtonDown)
			{
				ComputeEraserSize(point.x, point.y);
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_MOVE_ERASE);
			}
		}
		break;
	case LM_DRAW_RGBCURVE_X:
	case LM_DRAW_RGBCURVE_Y:
		{	// OnMouseMove()
			if (m_oldPoint != point)
			{
				m_oldPoint = point;
				oglMakeCurrent();
				unsigned char rgb[3];
				glReadPixels(point.x, m_glheight-point.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, (void*)rgb);
				int info[5];
				info[0] = point.x;
				info[1] = point.y;
				info[2] = rgb[0];
				info[3] = rgb[1];
				info[4] = rgb[2];
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_MOUSEMOVE, (LPARAM)info);
			}
		}
		break;
	case  LM_SET_WEAKENPAINT:	// added by eric at 20110929
		if (m_LeftButtonDown)
		{
			if (m_PaintMode == PAINT_RECT)
			{
				oglTranslatePoint(point.x, point.y, x, y);
				m_rcWeakenColor.right = x;
				m_rcWeakenColor.bottom = y;
				InvalidateRect(NULL, FALSE);
			}
			else if (m_PaintMode == PAINT_BRUSH)
			{
				ComputeEraserSize(point.x, point.y);
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_MOVE_ADD);
			}
			else if (m_PaintMode == PAINT_PAINTBRUSH)	// added by eric at 20120330
			{
				ComputeEraserSize(point.x, point.y);	// modified by eric at 20121022
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_MOVE_ADD);
			}
		}
		break;

		// added by eric at 20130923
	case LM_SET_COLORDERIVATE:
		{
			if (m_vRectTrackerList.size() == 1) {
				int x, y;
				oglTranslatePoint(point.x, point.y, x, y);
				CPoint ptMouse;
				ptMouse.x = x;
				ptMouse.y = y;

				if (m_bLMouseDown && (m_nHitHandle == 8)) {
					CPoint ptOffset;
					ptOffset.x = x - m_ptDown.x;
					ptOffset.y = y - m_ptDown.y;

					if (ptOffset.x == 0 && ptOffset.y == 0)
						return;

					m_ptDown.x = x;
					m_ptDown.y = y;

					m_rtSelected.left += ptOffset.x;
					m_rtSelected.right += ptOffset.x;
					m_rtSelected.top += ptOffset.y;
					m_rtSelected.bottom += ptOffset.y;

					m_vColorDerivateBox[m_nSelectedIndex].left += ptOffset.x;
					m_vColorDerivateBox[m_nSelectedIndex].right += ptOffset.x;
					m_vColorDerivateBox[m_nSelectedIndex].top += ptOffset.y;
					m_vColorDerivateBox[m_nSelectedIndex].bottom += ptOffset.y;

					m_vRectTrackerList.clear();
					int left, top, right, bottom;
					oglProjectPoint(left, top, m_rtSelected.left, m_rtSelected.top);
					oglProjectPoint(right, bottom, m_rtSelected.right, m_rtSelected.bottom);

					CRect rtTracker = CRect(left, top, right, bottom);
					CMyTracker tracker;
					tracker.m_rect = rtTracker;
					tracker.m_nStyle = CRectTracker::solidLine|CRectTracker::hatchedBorder;
					m_vRectTrackerList.push_back(tracker);

					need_redraw = TRUE;
				}
			}
		}
		break;

	default:
		break;
	}

	// added by eric at 20120223
	if (m_bShowRGB) {
		oglMakeCurrent();
		unsigned char rgb[3];
		glReadPixels(point.x, m_glheight-point.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, (void*)rgb);
		int info[5];
		info[0] = point.x;
		info[1] = point.y;
		info[2] = rgb[0];
		info[3] = rgb[1];
		info[4] = rgb[2];
		hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_MOUSEMOVE, (LPARAM)info);
	}

	if (m_RightButtonDown)
	{
		// modified by eric at 20150527
		if (m_lmMode==LM_SET_NORESIZMOVING) {
			need_redraw = FALSE;
		} else {
			// for translation
			dist = m_RightDownPos - point;
			m_RightDownPos = point;

            if ( !m_OGLRolling[ NULL ].IsRollingMode() )
            {
			    m_xTranslate -= (GLfloat)(dist.cx)/m_glScale;
            }
			m_yTranslate -= (GLfloat)(dist.cy)/m_glScale;
			OnUpdateShadowLine();	// added by eric at 20130208
			need_redraw = TRUE;	// Beagle 20130424 modified.
			oglClearBoundedTracker();	// added by eric at 20120330
		}
	}

	if (need_redraw) { oglDrawScene(); }

	CWnd::OnMouseMove(nFlags, point);
}

// added by danny
//eric chao 20130302 changed to RGB332 format.
// Beagle 20130409 modified.
void COpenGLControl::oglSetInspectionRgn(int w, int h, unsigned char *buffer)
{
	IMAGE img;

	wglMakeCurrent(hdc, hrc);
	ImageInit(&img, IMAGE_TYPE_RGB8_332, buffer, w, h, w, h);
	SetTexture(&img, m_region_texture[0], 0);
}

// Beagle 20130822 added.
void COpenGLControl::oglSetInspectionRgn(IMAGE *RegionImg, int RegionNum)
{
	wglMakeCurrent(hdc, hrc);
	SetTexture(RegionImg, m_region_texture[RegionNum]);
}

// Beagle 20130409 added.
BOOL COpenGLControl::oglUpdateInspectionRgn(int w, int h, int x, int y, unsigned char *buffer)
{
	IMAGE img;

	wglMakeCurrent(hdc, hrc);
	ImageInit(&img, IMAGE_TYPE_RGB8_332, buffer, w, h, w, h);
	return UpdateTexture(&img, m_region_texture[0], x, y);
}

// Beagle 20130822 added.
BOOL COpenGLControl::oglUpdateInspectionRgn(IMAGE *RegionImg, int RegionNum, int x, int y)
{
	wglMakeCurrent(hdc, hrc);
	return UpdateTexture(RegionImg, m_region_texture[RegionNum], x, y);
}

// Beagle 20130305 modified.
void COpenGLControl::oglSetRegionSegmentTexture(int w, int h, unsigned char *buffer)
{
	IMAGE ImgTex;

	if (buffer == NULL) return; 

	ImageInit(&ImgTex, IMAGE_TYPE_RGB24, buffer, w, h);

	wglMakeCurrent(hdc, hrc);
	SetTexture(&ImgTex, m_texture[11]);

	m_showRegionSegments = TRUE;
	m_RegionSegmentColor = REGION_SEGMENT_ENABLE_RGB332;
}

// Beagle 20130306 modified.
void COpenGLControl::oglSetRegionSegmentTexture(int w, int h, COLORREF clr, unsigned char *buffer)
{
	IMAGE ImgTex;
	int	type;

	if (buffer == NULL) return; 

	type = (clr == REGION_SEGMENT_ENABLE_RGB332 ?
		IMAGE_TYPE_RGB8_332 : IMAGE_TYPE_MONO8);
	ImageInit(&ImgTex, type, buffer, w, h);

	wglMakeCurrent(hdc, hrc);
	SetTexture(&ImgTex, m_texture[11]);

	m_showRegionSegments = TRUE;
	m_RegionSegmentColor = clr;
}

void COpenGLControl::oglSetRegionDefectShapeTexture(int w, int h, unsigned char *buffer)
{
	oglSetRegionDefectShapeTexture(w,h,REGION_SEGMENT_ENABLE_RGB332,buffer);
}

void COpenGLControl::oglSetRegionDefectShapeTexture(int w, int h, COLORREF clr, unsigned char *buffer)
{
	IMAGE ImgTex;
	int	type;

	if (buffer == NULL) return; 

	type = (clr == REGION_SEGMENT_ENABLE_RGB332 ?
		IMAGE_TYPE_RGB8_332 : IMAGE_TYPE_MONO8);
	ImageInit(&ImgTex, type, buffer, w, h);

	wglMakeCurrent(hdc, hrc);
	SetTexture(&ImgTex, m_texture[15]);

	m_showRegionDFShape = TRUE;
	m_RegionDFShapeColor = clr;
}

void COpenGLControl::oglSetGridTexture(int w, int h, unsigned char *buffer)
{
	wglMakeCurrent(hdc, hrc);
	glBindTexture(GL_TEXTURE_2D, m_texture[13]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void COpenGLControl::oglSetGoldGridTexture(int w, int h, unsigned char *buffer)
{
	wglMakeCurrent(hdc, hrc);
	glBindTexture(GL_TEXTURE_2D, m_texture[14]);
	//glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void COpenGLControl::oglResetStackDefectPiece(void) //seanchen 20150820
{
	memset(m_defectPiece, 0, MAX_DEFECTS*sizeof(DEFECT_PIECE));
}
// Beagle 20130621 modified.
void COpenGLControl::oglResetTexture(void)
{


	IMAGE TexImg;

	TexImg.type = IMAGE_TYPE_RGB24;
	TexImg.orig_w = 32;
	TexImg.orig_h = 32;
	TexImg.data_w = 32;
	TexImg.data_h = 32;
	TexImg.ptr = (unsigned char *)malloc(32*32*3);

	for (int c=0; c<32; c++) for (int d=0; d<32; d++) {
		if (d%4 == 0) {
			TexImg.ptr[(c*32+d)*3]   = 255 - c*7;
			TexImg.ptr[(c*32+d)*3+1] = 0;
			TexImg.ptr[(c*32+d)*3+2] = 0;
		} else if (d%4 == 1) {
			TexImg.ptr[(c*32+d)*3]   = 255 - c*7;
			TexImg.ptr[(c*32+d)*3+1] = 255 - c*7;
			TexImg.ptr[(c*32+d)*3+2] = 0;
		} else if (d%4 == 2) {
			TexImg.ptr[(c*32+d)*3]   = 0;
			TexImg.ptr[(c*32+d)*3+1] = 255 - c*7;
			TexImg.ptr[(c*32+d)*3+2] = 0;
		} else if (d%4 == 3) {
			TexImg.ptr[(c*32+d)*3]   = 0;
			TexImg.ptr[(c*32+d)*3+1] = 0;
			TexImg.ptr[(c*32+d)*3+2] = 255 - c*7;
		}
	}

	wglMakeCurrent(hdc, hrc_loader);
	// Beagle 20140117 initialize all image textures to default pattern.
	for (int c=0; c<MAX_SPLIT_WINDOW; c++) {
		SetTexture(&TexImg, m_image_texture[c]);
		m_width[c] = m_bNoColorCubeBackground ? 0 : TexImg.data_w;
		m_height[c] = m_bNoColorCubeBackground ? 0 : TexImg.data_h;
	}

	// Set textures to black. -- Beagle 20130807
	memset(TexImg.ptr, 0, 32*32*3); //20150302-01
	for (int c=0; c<MAX_SPLIT_WINDOW; c++) {
		SetTexture(&TexImg, m_region_texture[c]);	// Beagle 20130822
	}
	for (int c=0; c<MAX_DEFECTS; c++) {
		SetTexture(&TexImg, m_piece_texture[c]);
		SetTexture(&TexImg, m_piece_tex_shade[c]);
	}

	free(TexImg.ptr);
	TexImg.ptr = NULL;

	// Reset to 1:1 centered view.
	oglSetNormalTranslate();
	m_glScale = 1.0f;
	m_xTranslate = 0.0f;
	m_yTranslate = 0.0f;
}

// Beagle 20130306 modified.
void COpenGLControl::oglSetTexture(int w, int h, unsigned char *buffer, BOOL result, int TexNum, BOOL shrink, int shrink_w, int shrink_h)
{
	IMAGE TexImg;
	int	type;

#ifdef USE_BGR_IMGDATA
	type = IMAGE_TYPE_BGR24;
#else
	type = IMAGE_TYPE_RGB24;
#endif

	if (shrink) ImageInit(&TexImg, type, buffer, shrink_w, shrink_h, w, h);
	else ImageInit(&TexImg, type, buffer, w, h);

	oglSetTexture(&TexImg, TexNum);
	m_curr_sample_result = result;
}

// Beagle 20130306 modified.
void COpenGLControl::oglSetTextureGrey(int w, int h, unsigned char *buffer, int TexNum)
{
	IMAGE TexImg;

	ImageInit(&TexImg, IMAGE_TYPE_MONO8, buffer, w, h);
	oglSetTexture(&TexImg, TexNum);
}

// Beagle 20130306 modified.
void COpenGLControl::oglSetTextureRGB332(int w, int h, unsigned char *buffer, int TexNum)
{
	IMAGE TexImg;

	ImageInit(&TexImg, IMAGE_TYPE_RGB8_332, buffer, w, h);
	oglSetTexture(&TexImg, TexNum);
}

// Beagle 20130410 modified.
void COpenGLControl::oglSetTexture(IMAGE *TexImg, int TexNum)
{
	int flag = 0;

	//20170906
	int nProcessId = GetCurrentThreadId();
	if (m_dThreadId != nProcessId){
		_D(L"[ERROR] Process in Other Thread!");
		return;
	}

	if (TexNum<0 || TexNum>MAX_SPLIT_WINDOW ||
		TexImg==NULL /*|| TexImg->ptr==NULL*/)
	{
		return;	// Invalid parameter.
	}

	if (m_width[TexNum]==TexImg->orig_w && m_height[TexNum]==TexImg->orig_h &&
		m_data_w[TexNum]==TexImg->data_w && m_data_h[TexNum]==TexImg->data_h &&
		m_textype[TexNum]==TexImg->type)
	{
		flag |= STFLAG_SIZE_NO_CHANGE;
	} else {	// Size changed.
		m_width[TexNum] = TexImg->orig_w;
		m_height[TexNum] = TexImg->orig_h;
		m_data_w[TexNum] = TexImg->data_w;
		m_data_h[TexNum] = TexImg->data_h;
		m_textype[TexNum] = TexImg->type;
	}
	if (TexImg->data_w < TexImg->orig_w || TexImg->data_h < TexImg->orig_h) {
		flag |= STFLAG_SMOOTH;
	}

	m_splitEnable[TexNum] = true;

	wglMakeCurrent(hdc, hrc_loader);

    int iMaxTextureSize = 0;

    glGetIntegerv( GL_MAX_TEXTURE_SIZE, &iMaxTextureSize );

    if ( m_OGLRolling[ TexNum ].IsRollingMode() )
    {
        m_OGLRolling[ TexNum ].SetRollingTexture( TexImg );
    }
#if OGLSEGMENT
    else if ( TexImg->data_w > iMaxTextureSize || 
              TexImg->data_h > iMaxTextureSize )
#else
    else if ( FALSE )
#endif
    {
        m_OGLSegment[ TexNum ].AttachData( m_glwidth / m_wsplit, m_glheight / m_hsplit, &m_glBackColor, &m_glScale, &m_xTranslate, &m_yTranslate );
        m_OGLSegment[ TexNum ].SetSegmentTexture( TexImg, MODE_SEGMENTATION, this );
    }
	else
    {
        m_OGLSegment[ TexNum ].UnsetSegmentTexture();

        SetTexture(TexImg, m_image_texture[TexNum], flag);
    }
	oglSetNormalTranslate();
}

// Beagle 20130103 modified.
BOOL COpenGLControl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (m_bLMouseDown) {	// added by eric at 20131223
		return TRUE;
	}

	unsigned char rgb[3];
	int info[5];
	CPoint localPoint = pt;

	oglMakeCurrent();
	ScreenToClient(&localPoint);
	oglScaleChange(zDelta>0?1.1f:0.9f, (float)localPoint.x, (float)localPoint.y);

	switch (m_lmMode) {
	case LM_SET_ANCHOR:
	case LM_SET_WEAKENANCHOR:
	case LM_SET_BARCODEANCHOR:
		oglTranslatePoint(localPoint.x, localPoint.y, m_mouse_x, m_mouse_y);
		break;
	case LM_DRAW_RGBCURVE_X:
	case LM_DRAW_RGBCURVE_Y:
		glReadPixels(localPoint.x, m_glheight-localPoint.y, 1, 1,
			GL_RGB, GL_UNSIGNED_BYTE, (void*)rgb);
		info[0] = localPoint.x;
		info[1] = localPoint.y;
		info[2] = rgb[0];
		info[3] = rgb[1];
		info[4] = rgb[2];
		hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_MOUSEWHEEL, (LPARAM)info);
		break;
	// added by eric at 20150527
	case LM_SET_NORESIZMOVING:
		oglScaleSet(1.0f);
		break;
	}

	InvalidateRect(NULL, FALSE);
	oglClearBoundedTracker();	// added by eric at 20120330
	OnUpdateShadowLine();	// added by eric at 20130208
	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void COpenGLControl::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_bLMouseDown) {	// added by eric at 20131223
		return;
	}

	SetFocus();

	m_RightButtonDown = TRUE;
	m_RightDownPos = point;
	SetCapture();

	CWnd::OnRButtonDown(nFlags, point);
}

void COpenGLControl::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_bLMouseDown) {	// added by eric at 20131223
		return;
	}

	m_RightButtonDown = FALSE;
	ReleaseCapture();

	// added by eric at 20130107
	if (m_bShowLayout) {
		hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_RBUTTONUP);
	}
	
	OnUpdateShadowLine();
	InvalidateRect(NULL, FALSE);

	CWnd::OnRButtonUp(nFlags, point);
}

/*
void COpenGLControl::OnMButtonDown(UINT nFlags, CPoint point)
{
	m_RightButtonDown = TRUE;
	m_RightDownPos = point;
	SetCapture();

	CWnd::OnMButtonDown(nFlags, point);
}

void COpenGLControl::OnMButtonUp(UINT nFlags, CPoint point)
{
	m_RightButtonDown = FALSE;
	ReleaseCapture();

	CWnd::OnMButtonUp(nFlags, point);
}
*/

// Beagle 20130814 modified.
void COpenGLControl::oglSetLuminanceData(int type, unsigned char *lData,
	unsigned char *rData, unsigned char *gData, unsigned char *bData, int length,
	int x, int y)
{
	// Check parameters.
	if ((type&LUMINANCE_MODE_MASK)==LUMINANCE_NONE || length<=0) {
		m_xLuminance.type = LUMINANCE_NONE;
		m_xLuminance.len = 0;
		return;
	}

	if (length > m_xLuminance.size) {	// Need reallocate.
		if (m_xLuminance.dataLuminance != NULL) { delete m_xLuminance.dataLuminance; }
		if (m_xLuminance.dataRed != NULL) { delete m_xLuminance.dataRed; }
		if (m_xLuminance.dataGreen != NULL) { delete m_xLuminance.dataGreen; }
		if (m_xLuminance.dataBlue != NULL) { delete m_xLuminance.dataBlue; }

		m_xLuminance.size = length;
		m_xLuminance.dataLuminance = new unsigned char [length];
		m_xLuminance.dataRed = new unsigned char [length];
		m_xLuminance.dataGreen = new unsigned char [length];
		m_xLuminance.dataBlue = new unsigned char [length];
	}

	m_xLuminance.type = type;
	m_xLuminance.len = length;
	m_xLuminance.x = x;
	m_xLuminance.y = y;

	if (lData != NULL) { memcpy(m_xLuminance.dataLuminance, lData, length); }
	else { memset(m_xLuminance.dataLuminance, 0, length); }
	if (rData != NULL) { memcpy(m_xLuminance.dataRed, rData, length); }
	else { memset(m_xLuminance.dataRed, 0, length); }
	if (gData != NULL) { memcpy(m_xLuminance.dataGreen, gData, length); }
	else { memset(m_xLuminance.dataGreen, 0, length); }
	if (bData != NULL) { memcpy(m_xLuminance.dataBlue, bData, length); }
	else { memset(m_xLuminance.dataBlue, 0, length); }
}

void COpenGLControl::oglProjectPoint(int& mouseX, int& mouseY, int imageX, int imageY)
{
	GLfloat depth[1];
	GLdouble x, y, z;
	GLint view[4];

	oglMakeCurrent();

	glGetIntegerv(GL_VIEWPORT, view);
	glReadPixels(imageX, imageY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, depth);

    if ( m_OGLRolling[ 0 ].IsRollingMode() )
    {
        m_OGLRolling[ 0 ].GetImageToWindow( view, depth[ 0 ], imageX, imageY, x, y );
    }
    else if ( m_OGLSegment[ 0 ].IsSegmentation() )
    {
        m_OGLSegment[ 0 ].GetImageToWindow( view, depth[ 0 ], imageX, imageY, x, y );
    }
    else
    {
	    gluProject(imageX+0.5, imageY+0.5, depth[0], m_modelview[0],
		    m_projection[0], view, &x, &y, &z);
    }
	mouseX = (int)x;
	mouseY = m_glheight - (int)y;
}

void COpenGLControl::oglTranslatePoint(int mouseX, int mouseY, int& imageX, int& imageY, int *window, int *winX, int *winY)
{
	GLfloat depth[1];
	GLdouble x, y, z;
	GLint view[4];
	int	split_id, split_x, split_y;
	int mx = mouseX;
	int my = mouseY;

	// Beagle 20120110 added -- Points outside OpenGL window
	//   will result in undefined behavior in glReadPixels().
	if (mx < 0) { mx = 0; }
	if (my < 0) { my = 0; }
	if (mx >= m_glwidth) { mx = m_glwidth-1; }
	if (my >= m_glheight) { my = m_glheight-1; }

	oglMakeCurrent();

	if (m_display_mode == DISPLAY_MODE_NORMAL || m_display_mode == DISPLAY_MODE_DEFECT_STACK) {
		// 分割畫面
		// Beagle 20140210 modified.
		if (m_nRadioButton > 0) {
			split_id = m_nRadioSelected;
		} else {
			split_x = mx*m_wsplit/m_glwidth;
			split_y = my*m_hsplit/m_glheight;
			split_id = split_y*m_wsplit+split_x;
		}

		glGetIntegerv(GL_VIEWPORT, view);
		glReadPixels(mx, m_glheight-my, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, depth);

        if ( m_OGLRolling[ split_id ].IsRollingMode() )
        {
            m_OGLRolling[ split_id ].GetWindowToImage( view, depth[ 0 ], mx, my, x, y );
        }
        else if ( m_OGLSegment[ split_id ].IsSegmentation() )
        {
            m_OGLSegment[ split_id ].GetWindowToImage( view, depth[ 0 ], mx, my, x, y );
        }
		else
        {
            gluUnProject(mx, m_glheight-my, depth[0], m_modelview[split_id], m_projection[split_id], view, &x, &y, &z);
        }
		// Beagle 20110711 Change boundary check rule.
		imageX = (int) x;
		imageY = (int) y;
		if (x < 0)	{ imageX = 0; }
		if (y < 0)	{ imageY = 0; }
		if (x >= m_width[split_id])	{ imageX = m_width[split_id]-1; }	// Beagle 20111101 modified.
		if (y >= m_height[split_id])	{ imageY = m_height[split_id]-1; }	// Beagle 20111101 modified.
	} else {	// m_display_mode == DISPLAY_MODE_DEFECT || m_display_mode == DISPLAY_MODE_DEFECT_PIECE
		// 橫排四小圖
		if (m_num_defect == 0) {
			split_id = -1;	// Beagle 20120320 modified -- Don't show defect window if there is no defect.
		} else {
			int glwidth = m_glwidth / 4;
			split_id = mx / glwidth;
			if (split_id >= m_num_defect) { split_id = -1; }	// Beagle 20120320 modified -- Don't show defect window when click on empty area.
		}
		imageX = mx;
		imageY = my;
	}

	if (window != NULL) { *window = split_id; }
	if (winX != NULL) { *winX = split_x; }
	if (winY != NULL) { *winY = split_y; }
}

/* Private function. */
void COpenGLControl::oglSetNormalTranslate(void)
{
	float winW, winH;

	winW = 1.0f*m_glwidth/m_wsplit; winH = 1.0f*m_glheight/m_hsplit;

	// Beagle 20111101 modified.
	for (int c=0; c<MAX_SPLIT_WINDOW; c++) {
		if (m_width[c] == 0 || m_height[c] == 0) {	// Beagle 20120925 added.
			m_norm_scale[c] = 1.0f;
		} else if (1.0f*winW/winH > 1.0f*m_width[c]/m_height[c]) {
			m_norm_scale[c] = 1.0f * winH / m_height[c];
		} else {
			m_norm_scale[c] = 1.0f * winW / m_width[c];
		}
		if (m_norm_scale[c] == 0) m_norm_scale[c] = 1.0f;
	}
}

// Beagle 20120504 changed to using FTGL font.
void COpenGLControl::oglDrawLargeNumber(void)
{
	wchar_t str[32];
	int	len;
	GLfloat	w, h, scale;
	FTBBox box;

	if (m_largenumber < 0) return;

	swprintf_s(str, 32, L"%7I64d", m_largenumber);
	len = (int) wcslen(str);
	box = m_fontFixed->BBox(str, len);
	w = box.Upper().Xf() - box.Lower().Xf();
	h = box.Upper().Yf() - box.Lower().Yf();
	scale = m_glwidth / w;
	if (h*scale > m_glheight) { scale = m_glheight / h; }

	glDisable(GL_COLOR_LOGIC_OP);
	glLoadIdentity();
	glTranslatef(-0.5f*m_glwidth, -0.5f*m_glheight, -10.0f);
	glScalef(scale, -scale, 1.0f);
	glTranslatef(-box.Lower().Xf(), -box.Upper().Yf(), 0.0f);
	switch (len)
	{
	case 7:		/* <= 7, printf will do the padding. */
		glColor3f(0.3f, 1.0f, 0.3f);
		break;
	case 8:
		glColor3f(1.0f, 1.0f, 0.3f);
		break;
	default:	/* >= 9 */
		glColor3f(1.0f, 0.3f, 0.3f);
		break;
	}
	m_fontFixed->Render(str, len);
}

void COpenGLControl::oglLoadBMP(TCHAR *Filename)
{
	CImage img;
	unsigned char *ptr1, *ptr2, *data;
	int c, d, sourcepitch, targetpitch;

	oglMakeCurrent();

	if (FAILED(img.Load(Filename))) {
		return;
	}

	data = new unsigned char [img.GetWidth()*img.GetHeight()*3];

	ptr1 = (unsigned char*) img.GetBits();
	ptr2 = data;
	sourcepitch = img.GetPitch();
	targetpitch = img.GetWidth()*3;

	for (c=0; c<img.GetHeight(); c++) {
		for (d=0; d<targetpitch; d+=3) {
			ptr2[d]   = ptr1[d+2];
			ptr2[d+1] = ptr1[d+1];
			ptr2[d+2] = ptr1[d];
		}

		ptr1 += sourcepitch;
		ptr2 += targetpitch;
	}

	oglSetTexture(img.GetWidth(), img.GetHeight(), data);

	img.Destroy();
}

void COpenGLControl::oglGetAnchor(int num, int &x, int &y)
{
	if (num < 0 || num > m_anchors) {
		x = -1;
		y = -1;
		return;
	}

	x = m_anchor_x[num];
	y = m_anchor_y[num];
	return;
}

void COpenGLControl::oglDrawMinimap(void)
{
	if (m_minimap == FALSE) { return; }

    if ( m_OGLSegment[ 0 ].IsSegmentation() )
    {
        m_OGLSegment[ 0 ].DrawSegmentMinimap();

        return;
    }
	glPushMatrix();

	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);

	// Background
	glDisable(GL_TEXTURE_2D);
	glLoadIdentity();
	glTranslatef(-0.5f*m_glwidth, -0.5f*m_glheight, -42.0f);
	glScalef(MINIMAP_SHRINK, MINIMAP_SHRINK, 1.0f);
	glBegin(GL_QUADS);
		glColor3f((float)GetRValue(m_glBackColor)/255.0f,
			(float)GetGValue(m_glBackColor)/255.0f,
			(float)GetBValue(m_glBackColor)/255.0f);
		glVertex3i(0, 0, 0);
		glVertex3i(0, m_glheight, 0);
		glVertex3i(m_glwidth, m_glheight, 0);
		glVertex3i(m_glwidth, 0, 0);
	glEnd();

	// Sample image
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_image_texture[0]);
	glLoadIdentity();
	glTranslatef(-0.5f*m_glwidth, -0.5f*m_glheight, -41.0f);
	glScalef(MINIMAP_SHRINK, MINIMAP_SHRINK, 1.0f);
	glTranslatef(0.5f*m_glwidth, 0.5f*m_glheight, 0.0f);
	glScalef(m_norm_scale[0], m_norm_scale[0], 1.0f);
	glTranslatef(-0.5f*m_width[0], -0.5f*m_height[0], 0.0f);	// Beagle 20111101 modified.
	glBegin(GL_QUADS);
		glColor3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3i(0, 0, 0);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3i(0, m_height[0], 0);	// Beagle 20111101 modified.
		glTexCoord2f(1.0f, 1.0f);
		glVertex3i(m_width[0], m_height[0], 0);	// Beagle 20111101 modified.
		glTexCoord2f(1.0f, 0.0f);
		glVertex3i(m_width[0], 0, 0);	// Beagle 20111101 modified.
	glEnd();

	// Thin line frame
	// FIXME! Position bug.
	glDisable(GL_TEXTURE_2D);
	if (m_glScale > 1.0f) {
		glLoadIdentity();
		glTranslatef(-0.5f*m_glwidth, -0.5f*m_glheight, -40.0f);
		glScalef(MINIMAP_SHRINK, MINIMAP_SHRINK, 1.0f);
		glTranslatef(0.5f*m_glwidth, 0.5f*m_glheight, 0.0f);
		glTranslatef(-m_xTranslate*1.0f, -m_yTranslate*1.0f, 0.0f);
		glScalef(1.0f/m_glScale, 1.0f/m_glScale, 1.0f);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINE_LOOP);
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3f(-0.5f*m_glwidth, -0.5f*m_glheight, 0.0f);
			glVertex3f(-0.5f*m_glwidth,  0.5f*m_glheight, 0.0f);
			glVertex3f( 0.5f*m_glwidth,  0.5f*m_glheight, 0.0f);
			glVertex3f( 0.5f*m_glwidth, -0.5f*m_glheight, 0.0f);
		glEnd();
	}

	glPopMatrix();
}

// Beagle 20120508 modified.
void COpenGLControl::oglSetDefect(DEFECT_REGION *pDefect)
{
	int i;

	if (pDefect == NULL) { return; }
	if (pDefect->index < 0 || pDefect->index >= MAX_DEFECTS) return;
	i = pDefect->index;

	memcpy(m_defectRegion+i, pDefect, sizeof(DEFECT_REGION));

	if (m_defectRegion[i].region.left	< 0) { m_defectRegion[i].region.left   = 0; }
	if (m_defectRegion[i].region.top	< 0) { m_defectRegion[i].region.top    = 0; }
	if (m_defectRegion[i].region.right	< 0) { m_defectRegion[i].region.right  = 0; }
	if (m_defectRegion[i].region.bottom	< 0) { m_defectRegion[i].region.bottom = 0; }
}

void COpenGLControl::transByXYRatio(GLfloat *trans_x, GLfloat *trans_y, GLfloat angle, GLfloat cx, GLfloat cy, GLfloat sx, GLfloat sy, GLfloat orig_x, GLfloat orig_y)
{
	float cam2ori = m_XYRatio;
	float ori2cam = 1.0f/m_XYRatio;

	float u = orig_x*cam2ori - cx - sx;
	float v = orig_y - cy - sy;
	float sinval = sin(angle);
	float cosval = cos(angle);

	*trans_x = (u*cosval - v*sinval + cx)*ori2cam;
	*trans_y = u*sinval + v*cosval + cy;

	return;
}

// Beagle 20140711 modified.
void COpenGLControl::DrawDefect(void)
{
	int		c, glwidth, glheight, defectwidth, defectheight;
	float	scale, TextureW, TextureH, TextureX, TextureY;
	float	TextureX1, TextureY1, TextureX2, TextureY2;
	float	TextureX3, TextureY3, TextureX4, TextureY4;

	glDisable(GL_BLEND);	// Beagle 20130403 added.
#ifdef CUDA_OPENGL_INTEROP
	if (m_isOnline) {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_texBuffer[1]);
		glBindTexture(GL_TEXTURE_2D, m_texture[12]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	} 
	else 
	{
		glBindTexture(GL_TEXTURE_2D, m_image_texture[0]);
	}
#else
	glBindTexture(GL_TEXTURE_2D, m_image_texture[0]);
#endif
	glwidth = m_glwidth/4;
	glheight = m_glheight;

	for (c=0; c<m_num_defect; c++) {
		defectwidth = m_defectRegion[c].region.right - m_defectRegion[c].region.left+1;
		defectheight = m_defectRegion[c].region.bottom - m_defectRegion[c].region.top+1;
		if (1.0f*glwidth/glheight > 1.0f*defectwidth/defectheight) {
			scale = 1.0f*(glheight-20)/defectheight;
		} else {
			scale = 1.0f*(glwidth-20)/defectwidth;
		}
		if (scale > 3.0f) { scale=3.0f; }	// Maximum zoom-in 3.0X
		// Individual defect window size.
		TextureW = glwidth/scale;
		TextureH = glheight/scale;
		// The center of the defect.
		TextureX = (m_defectRegion[c].region.right-1+m_defectRegion[c].region.left)/2.0f;
		TextureY = (m_defectRegion[c].region.bottom-1+m_defectRegion[c].region.top)/2.0f;

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		if (m_showDefectShift){
			if(m_defectRegion[c].alignType!=2){
				// Use rotation center as origin.
				TextureX -= m_defectRegion[c].center.x;
				TextureY -= m_defectRegion[c].center.y;

				// Rotate.
				TextureX1 = (TextureX-TextureW/2.0f)*cos(m_defectRegion[c].rot)-(TextureY-TextureH/2.0f)*sin(m_defectRegion[c].rot);
				TextureY1 = (TextureX-TextureW/2.0f)*sin(m_defectRegion[c].rot)+(TextureY-TextureH/2.0f)*cos(m_defectRegion[c].rot);
				TextureX2 = (TextureX-TextureW/2.0f)*cos(m_defectRegion[c].rot)-(TextureY+TextureH/2.0f)*sin(m_defectRegion[c].rot);
				TextureY2 = (TextureX-TextureW/2.0f)*sin(m_defectRegion[c].rot)+(TextureY+TextureH/2.0f)*cos(m_defectRegion[c].rot);
				TextureX3 = (TextureX+TextureW/2.0f)*cos(m_defectRegion[c].rot)-(TextureY+TextureH/2.0f)*sin(m_defectRegion[c].rot);
				TextureY3 = (TextureX+TextureW/2.0f)*sin(m_defectRegion[c].rot)+(TextureY+TextureH/2.0f)*cos(m_defectRegion[c].rot);
				TextureX4 = (TextureX+TextureW/2.0f)*cos(m_defectRegion[c].rot)-(TextureY-TextureH/2.0f)*sin(m_defectRegion[c].rot);
				TextureY4 = (TextureX+TextureW/2.0f)*sin(m_defectRegion[c].rot)+(TextureY-TextureH/2.0f)*cos(m_defectRegion[c].rot);

				// Use original origin.
				TextureX  += m_defectRegion[c].center.x;	// Move back because we need this point later.
				TextureY  += m_defectRegion[c].center.y;
				TextureX1 += m_defectRegion[c].center.x;
				TextureY1 += m_defectRegion[c].center.y;
				TextureX2 += m_defectRegion[c].center.x;
				TextureY2 += m_defectRegion[c].center.y;
				TextureX3 += m_defectRegion[c].center.x;
				TextureY3 += m_defectRegion[c].center.y;
				TextureX4 += m_defectRegion[c].center.x;
				TextureY4 += m_defectRegion[c].center.y;

				// Shift.
				TextureX1 -= m_defectRegion[c].shift.x;
				TextureY1 -= m_defectRegion[c].shift.y;
				TextureX2 -= m_defectRegion[c].shift.x;
				TextureY2 -= m_defectRegion[c].shift.y;
				TextureX3 -= m_defectRegion[c].shift.x;
				TextureY3 -= m_defectRegion[c].shift.y;
				TextureX4 -= m_defectRegion[c].shift.x;
				TextureY4 -= m_defectRegion[c].shift.y;

				glLoadIdentity();
				glTranslatef(-0.5f*m_glwidth, -0.5f*m_glheight, -50.0f);

				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, m_image_texture[0]);
				glBegin(GL_QUADS);
					glTexCoord2f(TextureX1/m_width[0], TextureY1/m_height[0]);
					glVertex3f(glwidth*c*1.0f, 0.0f, 0.0f);
					glTexCoord2f(TextureX2/m_width[0], TextureY2/m_height[0]);
					glVertex3f(glwidth*c*1.0f, glheight*1.0f, 0.0f);
					glTexCoord2f(TextureX3/m_width[0], TextureY3/m_height[0]);
					glVertex3f(glwidth*(c+1)*1.0f, glheight*1.0f, 0.0f);
					glTexCoord2f(TextureX4/m_width[0], TextureY4/m_height[0]);
					glVertex3f(glwidth*(c+1)*1.0f, 0.0f, 0.0f);
				glEnd();

				if (m_showInspectionRgn == TRUE) {	// Beagle 20130403 added.
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
					glBindTexture(GL_TEXTURE_2D, m_region_texture[0]);
					glBegin(GL_QUADS);
						glTexCoord2f(TextureX1/m_width[0], TextureY1/m_height[0]);
						glVertex3f(glwidth*c*1.0f, 0.0f, 0.0f);
						glTexCoord2f(TextureX2/m_width[0], TextureY2/m_height[0]);
						glVertex3f(glwidth*c*1.0f, glheight*1.0f, 0.0f);
						glTexCoord2f(TextureX3/m_width[0], TextureY3/m_height[0]);
						glVertex3f(glwidth*(c+1)*1.0f, glheight*1.0f, 0.0f);
						glTexCoord2f(TextureX4/m_width[0], TextureY4/m_height[0]);
						glVertex3f(glwidth*(c+1)*1.0f, 0.0f, 0.0f);
					glEnd();
					glDisable(GL_BLEND);
				}
			}
			else{
				TextureX1 = TextureX-TextureW/2.0f;
				TextureY1 = TextureY-TextureH/2.0f;
				TextureX2 = TextureX-TextureW/2.0f;
				TextureY2 = TextureY+TextureH/2.0f;
				TextureX3 = TextureX+TextureW/2.0f;
				TextureY3 = TextureY+TextureH/2.0f;
				TextureX4 = TextureX+TextureW/2.0f;
				TextureY4 = TextureY-TextureH/2.0f;

				GLfloat p1_x = TextureX1;
				GLfloat p1_y = TextureY1;
				GLfloat p2_x = TextureX2;
				GLfloat p2_y = TextureY2;
				GLfloat p3_x = TextureX3;
				GLfloat p3_y = TextureY3;
				GLfloat p4_x = TextureX4;
				GLfloat p4_y = TextureY4;

				transByXYRatio(&p1_x, &p1_y, m_defectRegion[c].rot, 
					m_defectRegion[c].center.x, m_defectRegion[c].center.y, 
					m_defectRegion[c].shift.x, m_defectRegion[c].shift.y, 
					p1_x, p1_y);

				transByXYRatio(&p2_x, &p2_y, m_defectRegion[c].rot, 
					m_defectRegion[c].center.x, m_defectRegion[c].center.y, 
					m_defectRegion[c].shift.x, m_defectRegion[c].shift.y, 
					p2_x, p2_y);

				transByXYRatio(&p3_x, &p3_y, m_defectRegion[c].rot, 
					m_defectRegion[c].center.x, m_defectRegion[c].center.y, 
					m_defectRegion[c].shift.x, m_defectRegion[c].shift.y, 
					p3_x, p3_y);

				transByXYRatio(&p4_x, &p4_y, m_defectRegion[c].rot, 
					m_defectRegion[c].center.x, m_defectRegion[c].center.y, 
					m_defectRegion[c].shift.x, m_defectRegion[c].shift.y, 
					p4_x, p4_y);

				glLoadIdentity();
				glTranslatef(-0.5f*m_glwidth, -0.5f*m_glheight, -50.0f);

				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, m_image_texture[0]);
				glBegin(GL_QUADS);
					glTexCoord2f(p1_x/m_width[0], p1_y/m_height[0]);
					glVertex3f(glwidth*c*1.0f, 0.0f, 0.0f);
					glTexCoord2f(p2_x/m_width[0], p2_y/m_height[0]);
					glVertex3f(glwidth*c*1.0f, glheight*1.0f, 0.0f);
					glTexCoord2f(p3_x/m_width[0], p3_y/m_height[0]);
					glVertex3f(glwidth*(c+1)*1.0f, glheight*1.0f, 0.0f);
					glTexCoord2f(p4_x/m_width[0], p4_y/m_height[0]);
					glVertex3f(glwidth*(c+1)*1.0f, 0.0f, 0.0f);
				glEnd();

				if (m_showInspectionRgn == TRUE) {	// Beagle 20130403 added.
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
					glBindTexture(GL_TEXTURE_2D, m_region_texture[0]);
					glBegin(GL_QUADS);
						glTexCoord2f(p1_x/m_width[0], p1_y/m_height[0]);
						glVertex3f(glwidth*c*1.0f, 0.0f, 0.0f);
						glTexCoord2f(p2_x/m_width[0], p2_y/m_height[0]);
						glVertex3f(glwidth*c*1.0f, glheight*1.0f, 0.0f);
						glTexCoord2f(p3_x/m_width[0], p3_y/m_height[0]);
						glVertex3f(glwidth*(c+1)*1.0f, glheight*1.0f, 0.0f);
						glTexCoord2f(p4_x/m_width[0], p4_y/m_height[0]);
						glVertex3f(glwidth*(c+1)*1.0f, 0.0f, 0.0f);
					glEnd();
					glDisable(GL_BLEND);
				}
			}
		} else {
			// Just assign the four corners.
			TextureX1 = TextureX-TextureW/2.0f;
			TextureY1 = TextureY-TextureH/2.0f;
			TextureX2 = TextureX-TextureW/2.0f;
			TextureY2 = TextureY+TextureH/2.0f;
			TextureX3 = TextureX+TextureW/2.0f;
			TextureY3 = TextureY+TextureH/2.0f;
			TextureX4 = TextureX+TextureW/2.0f;
			TextureY4 = TextureY-TextureH/2.0f;

			glLoadIdentity();
			glTranslatef(-0.5f*m_glwidth, -0.5f*m_glheight, -50.0f);

			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, m_image_texture[0]);
			glBegin(GL_QUADS);
				glTexCoord2f(TextureX1/m_width[0], TextureY1/m_height[0]);
				glVertex3f(glwidth*c*1.0f, 0.0f, 0.0f);
				glTexCoord2f(TextureX2/m_width[0], TextureY2/m_height[0]);
				glVertex3f(glwidth*c*1.0f, glheight*1.0f, 0.0f);
				glTexCoord2f(TextureX3/m_width[0], TextureY3/m_height[0]);
				glVertex3f(glwidth*(c+1)*1.0f, glheight*1.0f, 0.0f);
				glTexCoord2f(TextureX4/m_width[0], TextureY4/m_height[0]);
				glVertex3f(glwidth*(c+1)*1.0f, 0.0f, 0.0f);
			glEnd();

			if (m_showInspectionRgn == TRUE) {	// Beagle 20130403 added.
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
				glBindTexture(GL_TEXTURE_2D, m_region_texture[0]);
				glBegin(GL_QUADS);
					glTexCoord2f(TextureX1/m_width[0], TextureY1/m_height[0]);
					glVertex3f(glwidth*c*1.0f, 0.0f, 0.0f);
					glTexCoord2f(TextureX2/m_width[0], TextureY2/m_height[0]);
					glVertex3f(glwidth*c*1.0f, glheight*1.0f, 0.0f);
					glTexCoord2f(TextureX3/m_width[0], TextureY3/m_height[0]);
					glVertex3f(glwidth*(c+1)*1.0f, glheight*1.0f, 0.0f);
					glTexCoord2f(TextureX4/m_width[0], TextureY4/m_height[0]);
					glVertex3f(glwidth*(c+1)*1.0f, 0.0f, 0.0f);
				glEnd();
				glDisable(GL_BLEND);
			}
		}

		// added at 20091209 ----
		glDisable(GL_TEXTURE_2D);
		glColor3f(1.0f, 1.0f, 1.0f);
		glLineWidth(2.0f);
		glBegin(GL_LINE_LOOP);
		glVertex3f(glwidth*(GLfloat)c, 0.0f, 0.0f);
		glVertex3f(glwidth*(GLfloat)c, (GLfloat)glheight, 0.0f);
		glVertex3f(glwidth*(c+1.0f), (GLfloat)glheight, 0.0f);
		glVertex3f(glwidth*(c+1.0f), 0.0f, 0.0f);
		glEnd();
		glLineWidth(1.0f);
		// ----------------------

		glLoadIdentity();
		glTranslatef(-0.5f*m_glwidth, -0.5f*m_glheight, -1.0f);
		glScalef(scale, scale, 1.0f);
		glTranslatef(-TextureX, -TextureY, -45.0f);
		glTranslatef(TextureW/2.0f, TextureH/2.0f, 0.0f);
		if (defectwidth < 10 && defectheight < 10) {
			oglDrawCorona(TextureW*c+TextureX, TextureY, 10.0f*scale);
		}else {
			glColor3f(1.0f, 1.0f, 1.0f);
			glBegin(GL_LINE_LOOP);
				glVertex3f(TextureW*c+m_defectRegion[c].region.left*1.0f,	m_defectRegion[c].region.top*1.0f,		0.0f);
				glVertex3f(TextureW*c+m_defectRegion[c].region.left*1.0f,	m_defectRegion[c].region.bottom*1.0f,	0.0f);
				glVertex3f(TextureW*c+m_defectRegion[c].region.right*1.0f,	m_defectRegion[c].region.bottom*1.0f,	0.0f);
				glVertex3f(TextureW*c+m_defectRegion[c].region.right*1.0f,	m_defectRegion[c].region.top*1.0f,		0.0f);
			glEnd();
		}

		// 方形撞球及文字敘述
		glLoadIdentity();
		glTranslatef((c-2)*0.25f*m_glwidth, -0.5f*m_glheight, -10.0f);
		DrawDefectLabel(m_defectRegion[c].client_no, m_defectRegion[c].color,
			m_defectRegion[c].description);
	}
}

// Beagle 20140711
void COpenGLControl::DrawDefectPiece(void)
{
	glDisable(GL_BLEND);	// Beagle 20130403 added.
	for (int c=0; c<m_num_defect; c++) {
		int w = m_defectPiece[c].image.orig_w;
		int h = m_defectPiece[c].image.orig_h;
		float scale = m_defectPiece[c].scale;

		// 影像
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_piece_texture[c]);
		glEnable(GL_SCISSOR_TEST);
		glScissor(m_glwidth*c/4, 0, m_glwidth/4, m_glheight);
		glLoadIdentity();
		glTranslatef(-0.5f*m_glwidth, -0.5f*m_glheight, -40.0f);
		glTranslatef((c*2+1)*m_glwidth/8.0f, m_glheight/2.0f, 0.0f);
		if (m_showDefectShift) {	// Beagle 20120803 added.
			glRotatef(m_defectRegion[c].rot, 0.0f, 0.0f, 1.0f);
		}
		glScalef(scale, scale, 1.0f);// amike 20130625
		glBegin(GL_QUADS);
			glColor3f(1.0f, 1.0f, 1.0f);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-w/2.0f, -h/2.0f, 0.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(-w/2.0f,  h/2.0f, 0.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f( w/2.0f,  h/2.0f, 0.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f( w/2.0f, -h/2.0f, 0.0f);
		glEnd();

		// 陰影
		if (m_defectPiece[c].shade.type != IMAGE_TYPE_NOTYPE) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
			glBindTexture(GL_TEXTURE_2D, m_piece_tex_shade[c]);
			glColor3f(1.0f, 1.0f, 1.0f);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-w/2.0f, -h/2.0f, 0.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(-w/2.0f,  h/2.0f, 0.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f( w/2.0f,  h/2.0f, 0.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f( w/2.0f, -h/2.0f, 0.0f);
			glEnd();
			glDisable(GL_BLEND);
		}

		// 圓圈或線框
		glDisable(GL_TEXTURE_2D);
		if (m_defectPiece[c].rectW<10.0f && m_defectPiece[c].rectH<10.0f) {
			oglDrawCorona(0, 0, 30.0f);
		} else {
			glBegin(GL_LINE_LOOP);
				glColor3f(1.0f, 1.0f, 1.0f);
				glVertex3f(-m_defectPiece[c].rectW/2.0f, -m_defectPiece[c].rectH/2.0f, 0.0f);
				glVertex3f(-m_defectPiece[c].rectW/2.0f,  m_defectPiece[c].rectH/2.0f, 0.0f);
				glVertex3f( m_defectPiece[c].rectW/2.0f,  m_defectPiece[c].rectH/2.0f, 0.0f);
				glVertex3f( m_defectPiece[c].rectW/2.0f, -m_defectPiece[c].rectH/2.0f, 0.0f);
			glEnd();
		}
		glDisable(GL_SCISSOR_TEST);

		// 方形撞球及文字敘述
		glLoadIdentity();
		glTranslatef((c-2)*0.25f*m_glwidth, -0.5f*m_glheight, -10.0f);
		DrawDefectLabel(m_defectPiece[c].client_no, m_defectPiece[c].color,
			m_defectPiece[c].description);
	}
}

// Normally this display mode is combined with split window to show multi defects.
// Beagle 20140711
void COpenGLControl::DrawDefectStack(void)
{
	int defect_no;

	glDisable(GL_BLEND);
	glEnable(GL_SCISSOR_TEST);
	for (int y=0; y<m_hsplit; y++) for (int x=0; x<m_wsplit; x++) {
		defect_no = y*m_wsplit+x;
		glLoadIdentity();
		glScissor(m_glwidth*x/m_wsplit, m_glheight*(m_hsplit-y-1)/m_hsplit,
			m_glwidth/m_wsplit, m_glheight/m_hsplit);
		glTranslatef(m_glwidth*((0.5f+x)/m_wsplit-0.5f),
			m_glheight*((0.5f+y)/m_hsplit-0.5f), 0.0f);
		DrawSingleDefectPiece(x, y, defect_no);
	}
	glDisable(GL_SCISSOR_TEST);
}

// Beagle 20140711
void COpenGLControl::DrawSingleDefectPiece(int split_x, int split_y, int defect_no)
{
	int w = m_defectPiece[defect_no].image.orig_w;
	int h = m_defectPiece[defect_no].image.orig_h;
	float scale = m_defectPiece[defect_no].scale;
	int split_id = split_y*m_wsplit+split_x;

	glPushMatrix();

	// 影像
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_piece_texture[defect_no]);
	if (m_showDefectShift) {
		glRotatef(m_defectRegion[defect_no].rot, 0.0f, 0.0f, 1.0f);
	}
	glScalef(scale, scale, 1.0f);
	glBegin(GL_QUADS);
		glColor3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-w/2.0f, -h/2.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-w/2.0f,  h/2.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f( w/2.0f,  h/2.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f( w/2.0f, -h/2.0f, 0.0f);
	glEnd();

	// 陰影
	if (m_defectPiece[defect_no].shade.type != IMAGE_TYPE_NOTYPE) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
		glBindTexture(GL_TEXTURE_2D, m_piece_tex_shade[defect_no]);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-w/2.0f, -h/2.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-w/2.0f,  h/2.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f( w/2.0f,  h/2.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f( w/2.0f, -h/2.0f, 0.0f);
		glEnd();
		glDisable(GL_BLEND);
	}

	// 圓圈或線框
	glDisable(GL_TEXTURE_2D);
	if (m_defectPiece[defect_no].rectW<10.0f && m_defectPiece[defect_no].rectH<10.0f) {
		oglDrawCorona(0, 0, 30.0f);
	} else {
		glBegin(GL_LINE_LOOP);
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3f(-m_defectPiece[defect_no].rectW/2.0f,
				-m_defectPiece[defect_no].rectH/2.0f, 0.0f);
			glVertex3f(-m_defectPiece[defect_no].rectW/2.0f,
				 m_defectPiece[defect_no].rectH/2.0f, 0.0f);
			glVertex3f( m_defectPiece[defect_no].rectW/2.0f,
				 m_defectPiece[defect_no].rectH/2.0f, 0.0f);
			glVertex3f( m_defectPiece[defect_no].rectW/2.0f,
				-m_defectPiece[defect_no].rectH/2.0f, 0.0f);
		glEnd();
	}

	glPopMatrix();

	// 方形撞球及文字敘述
	glTranslatef(-0.5*m_glwidth/m_wsplit, -0.5*m_glheight/m_hsplit, 0.0f);
	DrawDefectLabel(m_defectPiece[defect_no].client_no, m_defectPiece[defect_no].color,
		m_defectPiece[defect_no].description);

	int nSelIdx = split_y*m_wsplit+split_x;
	if(nSelIdx>=0&&nSelIdx<MAX_DEFECTS){
		oglDrawStackInspIndex(m_stackInspIndex[nSelIdx],split_x,split_y); //seanchen 20150421-01
	}
}

// Beagle 20140711
void COpenGLControl::DrawDefectLabel(int client_no, COLORREF color, wchar_t *description)
{
	int	len;
	FTBBox box;
	float fontscale;

	if (color == 0) { return; }

	// 方形撞球
	glPushMatrix();
	glTranslatef(16.0f, 16.0f, 0.0f);
	glScalef(15.0f, 15.0f, 1.0f);
	oglDrawBilliardBall(color, client_no);
	glPopMatrix();

	// 文字標籤
	if (m_bShowDefectLabel) {
		glPushMatrix();
		len = (int) wcslen(description);
		glDisable(GL_COLOR_LOGIC_OP);
		glTranslatef(33.0f, 0.0f, 0.0f);
		box = m_font->BBox(description, len);
		fontscale = (0.25f*m_glwidth-36.0f)/(box.Upper().Xf()-box.Lower().Xf()+8.0f);
		if (fontscale > 0.8f) { fontscale = 0.8f; }
		glScalef(fontscale, -fontscale, 1.0f);
		glTranslatef(4.0f-box.Lower().Xf(),
			-box.Upper().Yf()-4.0f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
			glVertex3f(box.Lower().Xf()-4.0f, box.Lower().Yf()-4.0f, 0);
			glVertex3f(box.Lower().Xf()-4.0f, box.Upper().Yf()+4.0f, 0);
			glVertex3f(box.Upper().Xf()+4.0f, box.Upper().Yf()+4.0f, 0);
			glVertex3f(box.Upper().Xf()+4.0f, box.Lower().Yf()-4.0f, 0);
		glEnd();
		glColor3f(0.0f, 0.0f, 0.0f);
		glLineWidth(2.0f);
		glBegin(GL_LINE_STRIP);
			glVertex3f(box.Lower().Xf()-4.0f, box.Lower().Yf()-4.0f, 0);
			glVertex3f(box.Upper().Xf()+4.0f, box.Lower().Yf()-4.0f, 0);
			glVertex3f(box.Upper().Xf()+4.0f, box.Upper().Yf()+4.0f, 0);
		glEnd();
		m_font->Render(description, len);
		glPopMatrix();
	}
}

void COpenGLControl::oglClearAllAnchor()
{
	m_anchors = 0;
	for (int c=0; c<MAX_ANCHORS; c++) {
		m_anchor_x[c] = -1;
		m_anchor_y[c] = -1;
	}
	m_selAnchor = -1;
	m_selAnchorRange = -100; //seanchen 20130819
}

void COpenGLControl::oglDeleteAnchor(int index)
{
	if (index>=m_anchors || index<0) return;

	m_anchors--;
	for (int c=index; c<m_anchors; c++) {
		m_anchor_x[c] = m_anchor_x[c+1];
		m_anchor_y[c] = m_anchor_y[c+1];
	}
	InvalidateRect(NULL, TRUE);
}

void COpenGLControl::oglAddAnchor(int x, int y)
{
	if (m_anchors >= MAX_ANCHORS) return;

	m_anchor_x[m_anchors] = x;
	m_anchor_y[m_anchors] = y;
	m_anchors++;
}

// added by danny at 20110218
void COpenGLControl::oglClearReposAllAnchor()
{
	m_reposAnchorPoint.clear();
	m_selReposAnchor = -1;
	m_selReposAnchorRange = -100; //seanchen 20130819
}

void COpenGLControl::oglDeleteReposAnchor(int x, int y)
{
	int idx = -1;
	for (size_t t=0; t<m_reposAnchorPoint.size(); t++)
	{
		if ((m_reposAnchorPoint[t].x==x) && (m_reposAnchorPoint[t].y==y))
		{
			idx = (int)t;
			break;
		}
	}

	if (idx > -1)
	{
		if (m_selReposAnchor == idx)
			m_selReposAnchor = -1;
		m_reposAnchorPoint.erase(m_reposAnchorPoint.begin() + idx);
		InvalidateRect(NULL, TRUE);
	}
}

void COpenGLControl::oglAddReposAnchor(int x, int y)
{
	POINT pt;
	pt.x = x;
	pt.y = y;
	m_reposAnchorPoint.push_back(pt);
}

void COpenGLControl::oglGetReposAnchor(int num, int &x, int &y)
{
	if (num < 0 || num >= (int)m_reposAnchorPoint.size()) 
	{
		x = 0;
		y = 0;
		return;
	}

	x = m_reposAnchorPoint[num].x;
	y = m_reposAnchorPoint[num].y;
	return;
}

void COpenGLControl::oglSetReposAnchorMark(int x, int y)
{
	m_selReposAnchor = -1;
	for (size_t t=0; t<m_reposAnchorPoint.size(); t++)
	{
		if ((m_reposAnchorPoint[t].x==x) && (m_reposAnchorPoint[t].y==y))
		{
			m_selReposAnchor = (int)t;
			break;
		}
	}
	oglDrawScene();
}

void COpenGLControl::oglSetReposAnchorRangeMark(int nRectIdx) //seanchen 20130819
{
	if (nRectIdx < 0 || nRectIdx > m_mapRectInfo[ NULL ].rects) 
		m_selReposAnchorRange = -100;
	else
		m_selReposAnchorRange = nRectIdx;
	oglDrawScene();
}
//---------------------------------------------------


void COpenGLControl::oglSetRectShape(int index, oglShape shape, int left, int top, int right, int bottom, int TexNum, BOOL redraw)
{
    if ( index >= m_mapRectInfo[ TexNum ].rects ) { m_mapRectInfo[ TexNum ].rects = index + 1; }

    m_mapRectInfo[ TexNum ].rect_type[ index ]   = shape;
    m_mapRectInfo[ TexNum ].rect[ index ].left   = left;
    m_mapRectInfo[ TexNum ].rect[ index ].top    = top;
    m_mapRectInfo[ TexNum ].rect[ index ].right  = right;
    m_mapRectInfo[ TexNum ].rect[ index ].bottom = bottom;
 //   if (index >= m_rects) { m_rects = index+1; };
	//m_rect_type[index] = shape;
	//m_rect[index].left = left;
	//m_rect[index].top = top;
	//m_rect[index].right = right;
	//m_rect[index].bottom = bottom;

	if (redraw == TRUE)
		oglDrawScene();
}

// added by amike at 20120907
void COpenGLControl::oglSetRectInfo(int index, oglShape shape, int left, int top, int right, int bottom, COLORREF color, int TexNum)
{
    if ( m_OGLRolling[ TexNum ].IsRollingMode() )
    {
        m_OGLRolling[ TexNum ].SetRectInfo( index, CRect( left, top, right, bottom ), color );
    }
	oglSetRectShape( index, shape, left, top, right, bottom, TexNum, FALSE ); 
	//if (index >= m_rects) { m_rects = index+1; };
	m_mapRectInfo[ TexNum ].rect_color[index] = color;

	// removed by amike at 20130118
//	if (redraw == TRUE)
//		oglDrawScene();
}
void COpenGLControl::oglClearShapeBySize(RECT rt, int TexNum)
{
    if ( m_mapRectInfo.find( TexNum ) == m_mapRectInfo.end() ) return;

    for ( int i = NULL; i < m_mapRectInfo[ TexNum ].rects; i++ )
    {
        if ( NULL == memcmp( &m_mapRectInfo[ TexNum ].rect[ i ], &rt, sizeof( RECT ) ) )
        {
            memset( &m_mapRectInfo[ TexNum ].rect[ i ], NULL, sizeof( RECT ) );
        }
    }
	//RECT rect[MAX_RECTS];
	//for (int i=0; i<MAX_RECTS; i++) {
	//	rect[i].left=rect[i].right=rect[i].top=rect[i].bottom=0;
	//}
	//BOOL bHit=FALSE;
	//int count=0;
	//for (int i=0; i<m_rects; i++) {
	//	if (m_rect[i].left==rt.left && m_rect[i].right==rt.right && m_rect[i].top==rt.top && m_rect[i].bottom==rt.bottom) {
	//		bHit=TRUE;
	//	} else {
	//		rect[count].left=m_rect[i].left;
	//		rect[count].right=m_rect[i].right;
	//		rect[count].top=m_rect[i].top;
	//		rect[count].bottom=m_rect[i].bottom;
	//		count++;
	//	}
	//}
	//if (bHit==TRUE) {
	//	for (int i=0; i<MAX_RECTS; i++) {
	//		m_rect[i].left=m_rect[i].right=m_rect[i].top=m_rect[i].bottom=0;
	//	}
	//	m_rects--;
	//	for (int i=0; i<m_rects; i++) {
	//		m_rect[i].left=rect[i].left;
	//		m_rect[i].right=rect[i].right;
	//		m_rect[i].top=rect[i].top;
	//		m_rect[i].bottom=rect[i].bottom;
	//	}
	//}
}

void COpenGLControl::oglClearShape(bool bRedraw)
{
	for (std::map< int, RECTINFO >::iterator itr = m_mapRectInfo.begin(); itr != m_mapRectInfo.end(); itr++)
	{
		memset(&itr->second, NULL, sizeof(RECTINFO));
	}
	// added by eric at 20130124
	for(int c=0; c<m_polygons; c++) {
		m_poly_points[c] = 0;
	}
	m_polygons = 0;
	if (bRedraw){
		oglDrawScene();
	}
}
// added by eric at 20110801
void COpenGLControl::oglClearBarcodeShape(void)
{
	m_mapRectInfo[ NULL ].rects = 1;	// Barcode area select item
	m_polygons = 0;
	oglDrawScene();
}
// added by eric at 20121122
void COpenGLControl::oglRemoveLine(int index)
{
	m_lineRect[index].left = m_lineRect[index].top = m_lineRect[index].right = m_lineRect[index].bottom = 0;
	m_lineRects--;
	oglDrawScene();
}
// added by eric at 20121121
void COpenGLControl::oglClearLineShape(void)
{
	BOOL redraw = FALSE;
	if (m_lineRects > 0)
		redraw = TRUE;

	m_lineRects = 0;
	for (int c=0; c<MAX_RECTS; c++) {
		memset(&m_lineRect[c],0,sizeof(RECT));
	}

	m_AngleX = 0.0f;
	m_AngleY = 0.0f;

	oglClearText();	// added by eric at 20161217
	if (redraw == TRUE)
		oglDrawScene();
}
// added by eric at 20121121
void COpenGLControl::oglSetLineShape(int index, int left, int top, int right, int bottom, BOOL redraw)
{
	if (index >= m_lineRects) {
		m_lineRects = index+1; 
	};
	m_lineRect[index].left = left;
	m_lineRect[index].top = top;
	m_lineRect[index].right = right;
	m_lineRect[index].bottom = bottom;

	if (redraw == TRUE)
		oglDrawScene();
}
// added by eric at 20121121
void COpenGLControl::oglGetLineShape(int index, int &left, int &top, int &right, int &bottom)
{
	if (index <= m_lineRects) {
		left = m_lineRect[index].left;
		top = m_lineRect[index].top;
		right = m_lineRect[index].right;
		bottom = m_lineRect[index].bottom;
	}
}
// added by eric at 20130107
void COpenGLControl::oglClearLayoutRect(void)
{
	m_layoutRects = 0;
	for (int c=0; c<MAX_RECTS; c++) {
		m_layoutRect[c].left = m_layoutRect[c].top = m_layoutRect[c].right = m_layoutRect[c].bottom = 0;
	}

	m_vSelectionList.clear();
	m_vRectTrackerList.clear();
}
// added by eric at 20130107
void COpenGLControl::oglGetSelLayoutIndex(int x, int y, int &nIndex)
{
	for (int c=0; c<m_layoutRects; c++) {
		if (m_layoutRect[c].left <= x && m_layoutRect[c].right > x && m_layoutRect[c].top <= y && m_layoutRect[c].bottom > y) {
			nIndex = c;
			break;
		}
	}
}
// added by eric at 20130107
void COpenGLControl::oglAddLayoutShape(int index, int left, int top, int right, int bottom, BOOL redraw)
{
	if (index >= m_layoutRects) { m_layoutRects = index+1; };
	m_layoutRect[index].left = left;
	m_layoutRect[index].top = top;
	m_layoutRect[index].right = right;
	m_layoutRect[index].bottom = bottom;

	if (redraw == TRUE)
		oglDrawScene();
}
// added by eric at 20130107
void COpenGLControl::oglGetLayoutShape(int index, int &left, int &top, int &right, int &bottom)
{
	if (index <= m_layoutRects)
	{
		left = m_layoutRect[index].left;
		top = m_layoutRect[index].top;
		right = m_layoutRect[index].right;
		bottom = m_layoutRect[index].bottom;
	}
}
// added by eric at 20130107
void COpenGLControl::oglSetLayoutShape(int index, BOOL bCtrl)
{
	int left, top, right, bottom;
	oglProjectPoint(left, top, m_layoutRect[index].left, m_layoutRect[index].top);
	oglProjectPoint(right, bottom, m_layoutRect[index].right,m_layoutRect[index].bottom);
	CRect rtTracker = CRect(left, top, right, bottom);

	if (bCtrl == TRUE) {
		BOOL bRemoved = FALSE;
		for (size_t c=0; c<m_vRectTrackerList.size(); c++) {
			if (m_vRectTrackerList[c].m_rect.EqualRect(rtTracker)) {
				m_vRectTrackerList.erase(m_vRectTrackerList.begin()+c);
				m_vSelectionList.erase(m_vSelectionList.begin()+c);
				bRemoved = TRUE;
				break;
			}
		}

		if (bRemoved == FALSE) {
			// modified by eric at 20130118
			CMyTracker tracker;
			//CRectTracker tracker;
			tracker.m_rect = rtTracker;
			tracker.m_nStyle = CRectTracker::dottedLine|CRectTracker::hatchedBorder|CRectTracker::resizeInside;
			m_vRectTrackerList.push_back(tracker);
			m_vSelectionList.push_back(index);
		}
	} else {
		m_vRectTrackerList.clear();
		m_vSelectionList.clear();
		// modified by eric at 20130118
		CMyTracker tracker;
		//CRectTracker tracker;
		tracker.m_rect = rtTracker;
		tracker.m_nStyle = CRectTracker::dottedLine|CRectTracker::hatchedBorder|CRectTracker::resizeInside;
		m_vRectTrackerList.push_back(tracker);
		m_vSelectionList.push_back(index);


		m_selLayoutRect = index;
		m_rtSelected.left = m_layoutRect[m_selLayoutRect].left;
		m_rtSelected.top = m_layoutRect[m_selLayoutRect].top;
		m_rtSelected.right = m_layoutRect[m_selLayoutRect].right;
		m_rtSelected.bottom = m_layoutRect[m_selLayoutRect].bottom;

	}

	oglDrawScene();
}
// added by eric at 20130107
void COpenGLControl::oglMergeLayoutShape(std::vector<RECT> &vRectList, int &nBaseIndex)
{
	// Get Base Index Id
	for (size_t c=0; c<m_vSelectionList.size(); c++) {
		if (c==0) {
			nBaseIndex = m_vSelectionList[c];
		} else {
			if (nBaseIndex > m_vSelectionList[c]) {
				nBaseIndex = m_vSelectionList[c];
			}
		}
	}

	// Get Merge rectangle size
	int left=0, right=0, top=0, bottom=0;
	for (size_t c=0; c<m_vSelectionList.size(); c++) {
		if (nBaseIndex == m_vSelectionList[c]) {
			continue;
		}

		if (m_layoutRect[m_vSelectionList[c]].left < m_layoutRect[nBaseIndex].left) {
			m_layoutRect[nBaseIndex].left = m_layoutRect[m_vSelectionList[c]].left;
		}
		if (m_layoutRect[m_vSelectionList[c]].right > m_layoutRect[nBaseIndex].right) {
			m_layoutRect[nBaseIndex].right = m_layoutRect[m_vSelectionList[c]].right;
		}
		if (m_layoutRect[m_vSelectionList[c]].top < m_layoutRect[nBaseIndex].top) {
			m_layoutRect[nBaseIndex].top = m_layoutRect[m_vSelectionList[c]].top;
		}
		if (m_layoutRect[m_vSelectionList[c]].bottom > m_layoutRect[nBaseIndex].bottom) {
			m_layoutRect[nBaseIndex].bottom = m_layoutRect[m_vSelectionList[c]].bottom;
		}
		m_layoutRect[m_vSelectionList[c]].left = m_layoutRect[m_vSelectionList[c]].top = m_layoutRect[m_vSelectionList[c]].right = m_layoutRect[m_vSelectionList[c]].bottom = 0;
	}

	for (int c=0; c<m_layoutRects; c++) {
		if (!(m_layoutRect[c].left == 0 && m_layoutRect[c].right == 0 && m_layoutRect[c].top == 0 && m_layoutRect[c].bottom == 0)) {
			vRectList.push_back(m_layoutRect[c]);
		}
	}

	for (int c=0; c<MAX_RECTS; c++) {
		m_layoutRect[c].left = m_layoutRect[c].top = m_layoutRect[c].right = m_layoutRect[c].bottom = 0;
	}

	m_layoutRects = 0;
	for (size_t c=0; c<vRectList.size(); c++) {
		m_layoutRect[c].left = vRectList[c].left;
		m_layoutRect[c].right = vRectList[c].right;
		m_layoutRect[c].top = vRectList[c].top;
		m_layoutRect[c].bottom = vRectList[c].bottom;
		m_layoutRects++;
	}

	m_vRectTrackerList.clear();
	m_vSelectionList.clear();
}

void COpenGLControl::oglGetAllLayoutShape(std::vector<RECT> &vRectList)
{
	for (int c=0; c<m_layoutRects; c++) {
		RECT rt;
		rt.left = m_layoutRect[c].left;
		rt.right = m_layoutRect[c].right;
		rt.top = m_layoutRect[c].top;
		rt.bottom = m_layoutRect[c].bottom;
		vRectList.push_back(rt);
	}
}

void COpenGLControl::oglGetRectShape(int index, oglShape &shape, int &left, int &top, int &right, int &bottom, int TexNum)
{
	if (index <= m_mapRectInfo[ TexNum ].rects)	// modified at 20091214, when index == m_rects, means the currently drawed rectangle
	{
		shape = m_mapRectInfo[ TexNum ].rect_type[index];
		left = m_mapRectInfo[ TexNum ].rect[index].left;
		top = m_mapRectInfo[ TexNum ].rect[index].top;
		right = m_mapRectInfo[ TexNum ].rect[index].right;
		bottom = m_mapRectInfo[ TexNum ].rect[index].bottom;
	}
}


void COpenGLControl::oglGetPolygonVertex(int index, std::vector<CPoint> *vertexList)		// added by danny
{
	if (index <= m_polygons)	// modified at 20091214, when index == m_polygons, means the currently drawed polygon
	{
		vertexList->clear();
		for (int i=0; i<m_poly_points[index]; i++)
			vertexList->push_back(m_polygon[index][i]);
	}
}

void COpenGLControl::oglSetPolygonVertex(int index, std::vector<CPoint> *vertexList)		// added by danny
{
	if (index >= MAX_RECTS)
		return;

	int i;
	if (index >= m_polygons) 
	{
		i = m_polygons;
		m_polygons++;
	}
	else
		i = index;

	m_poly_points[i] = ((int)vertexList->size() > MAX_POLY_POINTS)? MAX_POLY_POINTS:(int)vertexList->size();
	for (int j=0; j<m_poly_points[i]; j++)
		m_polygon[i][j] = (*vertexList)[j];

	oglDrawScene();
}
// added by eric at 20130208
void COpenGLControl::oglShowShadowRectShape(BOOL bShow)
{
	m_bShowShadowRectShape = bShow; 
	if (bShow)  {
		m_pLineTracker = new CLineTracker();
	} else {
		m_LineList.RemoveAll();
		if (m_pLineTracker) {
			m_pLineTracker->RemoveAll();
			delete m_pLineTracker;
			m_pLineTracker = NULL;
		}
	}
}
// added by eric at 20130208
void COpenGLControl::oglAddShadowShape(RECT shadowShape, RECT lineShape, RECT lineShape2, int direction)
{
	SHADOWINFO shadowInfo;
	shadowInfo.direction = direction;
	shadowInfo.shadowShape = shadowShape;
	shadowInfo.lineShape = lineShape;
	shadowInfo.lineShape2 = lineShape2;

	int left, top, right, bottom;
	if ((lineShape.right-lineShape.left) != 0 || (lineShape.bottom-lineShape.top) != 0) {
		CLineItem* pLine = new CLineItem();
		oglProjectPoint(left, top, lineShape.left, lineShape.top);
		oglProjectPoint(right, bottom, lineShape.right, lineShape.bottom);
		pLine->m_rcPoints.SetRect(left, top, right, bottom);
		m_LineList.Add(pLine);
	}

	if (m_shadowType == 0) {	// One Way has two lines
		if ((lineShape2.right-lineShape2.left) != 0 || (lineShape2.bottom-lineShape2.top) != 0) {
			CLineItem* pLine = new CLineItem();
			oglProjectPoint(left, top, lineShape2.left, lineShape2.top);
			oglProjectPoint(right, bottom, lineShape2.right, lineShape2.bottom);
			pLine->m_rcPoints.SetRect(left, top, right, bottom);
			m_LineList.Add(pLine);
		}
	}

	m_vShadowInfo.push_back(shadowInfo);
}
// added by eric at 20130208
void COpenGLControl::oglClearShadowShape()
{
	m_vShadowInfo.clear();
	// added by eric at 20130319
	if (m_pLineTracker) {
		m_LineList.RemoveAll();
		m_pLineTracker->RemoveAll();
	}
}
// added by eric at 20130208
void COpenGLControl::oglUndoShadowShape()
{
	m_vShadowInfo.erase(m_vShadowInfo.begin()+m_vShadowInfo.size()-1);
	oglDrawScene();
}
// added by eric at 20130208
void COpenGLControl::OnUpdateShadowLine()
{
	if (m_pLineTracker) {
		// added by eric at 20130503
		int iSelection=-1;
		CLineItem* pSelLine=NULL;
		m_pLineTracker->GetSelection(pSelLine, iSelection);
		//////////////////////////////////////////////////////////////////////////

		m_LineList.RemoveAll();
		m_pLineTracker->RemoveAll();
		int left, top, right, bottom;
		for (int i=0; i<(int)m_vShadowInfo.size(); i++) {
			// modified by eric at 20130226
			if (m_vShadowInfo[i].direction == EMBOSS_ALL_DIRECTION) {	// modified by eric at 20130503
				CLineItem* pLine = new CLineItem();
				pLine->m_rcPoints.SetRect(0, 0, 0, 0);
				m_LineList.Add(pLine);
				if (m_shadowType == 0) {
					CLineItem* pLine = new CLineItem();
					pLine->m_rcPoints.SetRect(0, 0, 0, 0);
					m_LineList.Add(pLine);
				}
			} else {
				if (m_vShadowInfo[i].lineShape.left != 0 && m_vShadowInfo[i].lineShape.top != 0 &&
					m_vShadowInfo[i].lineShape.right != 0 && m_vShadowInfo[i].lineShape.bottom != 0) {
					CLineItem* pLine = new CLineItem();
					oglProjectPoint(left, top, m_vShadowInfo[i].lineShape.left, m_vShadowInfo[i].lineShape.top);
					oglProjectPoint(right, bottom, m_vShadowInfo[i].lineShape.right, m_vShadowInfo[i].lineShape.bottom);
					pLine->m_rcPoints.SetRect(left, top, right, bottom);
					m_LineList.Add(pLine);
				}

				if (m_shadowType == 0) {	// One Way has two lines
					if (m_vShadowInfo[i].lineShape2.left != 0 && m_vShadowInfo[i].lineShape2.top != 0 &&
						m_vShadowInfo[i].lineShape2.right != 0 && m_vShadowInfo[i].lineShape2.bottom != 0) {
						CLineItem* pLine = new CLineItem();
						oglProjectPoint(left, top, m_vShadowInfo[i].lineShape2.left, m_vShadowInfo[i].lineShape2.top);
						oglProjectPoint(right, bottom, m_vShadowInfo[i].lineShape2.right, m_vShadowInfo[i].lineShape2.bottom);
						pLine->m_rcPoints.SetRect(left, top, right, bottom);
						m_LineList.Add(pLine);
					}
				}
			}
		}

		// added by eric at 20130503
		if (iSelection != -1) {
			pSelLine = m_LineList.GetLineItem(iSelection);
			m_pLineTracker->SetSelection(pSelLine, iSelection);
		}
	}
}
// added by eric at 20130208
void COpenGLControl::oglGetShadowShape(int index, CRect &shadowShape, CRect &lineShape, CRect &lineShape2)
{
	if (index <= (int)m_vShadowInfo.size()) {
		shadowShape = m_vShadowInfo[index].shadowShape;
		lineShape = m_vShadowInfo[index].lineShape;
		lineShape2 = m_vShadowInfo[index].lineShape2;
	}
}
// added by eric at 20130318
void COpenGLControl::oglSetShadowShape(int index, CRect shadowShape, CRect lineShape, CRect lineShape2)
{
	if (index <= (int)m_vShadowInfo.size()) {
		m_vShadowInfo[index].shadowShape = shadowShape;
		m_vShadowInfo[index].lineShape = lineShape;
		m_vShadowInfo[index].lineShape2 = lineShape2;
		m_nSelectedIndex = index;
		CRect rt = m_vShadowInfo[index].shadowShape;
		m_nHitHandle = 8;
		::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
		CMyTracker tracker;
		// Beagle 20130424 modified.
		int left, top, right, bottom;
		oglProjectPoint(left, top, rt.left, rt.top);
		oglProjectPoint(right, bottom, rt.right, rt.bottom);

		m_vRectTrackerList.clear();
		CRect rtTracker = CRect(left, top, right, bottom);
		tracker.m_rect = rtTracker;
		tracker.m_nStyle = CRectTracker::dottedLine|CRectTracker::hatchedBorder|CRectTracker::resizeInside;
		m_vRectTrackerList.push_back(tracker);
		m_rtSelected = rt;

		OnUpdateShadowLine();
		oglDrawScene();
	}
}
// added by eric at 20130208
void COpenGLControl::oglClearSelection()
{
	m_nSelectedIndex = -1;
	m_vRectTrackerList.clear();
	if (m_pLineTracker) {
		m_pLineTracker->ReleaseSelected();
	}
}
// added by eric at 20130208
void COpenGLControl::oglDeleteSelection()
{
	if (m_pLineTracker) {
		if (m_nSelectedIndex == -1)
			return;

		m_vShadowInfo.erase(m_vShadowInfo.begin()+m_nSelectedIndex);

		m_nSelectedIndex = -1;
		m_vRectTrackerList.clear();
		m_pLineTracker->RemoveAll();

		OnUpdateShadowLine();
		oglDrawScene();
	}
}
// Beagle 20120504 changed to using FTGL fonts.
void COpenGLControl::oglDrawNumbers(float x, float y, int number, float wmax, float hmax)
{
	GLfloat scale, w, h, oldColor[4];
	GLboolean isColorLogicOp;
	wchar_t str[32];
	int len;
	FTBBox box;

	glPushMatrix();
	glGetFloatv(GL_CURRENT_COLOR, oldColor);
	isColorLogicOp = glIsEnabled(GL_COLOR_LOGIC_OP);
	glDisable(GL_COLOR_LOGIC_OP);
	glDisable(GL_TEXTURE_2D);

	swprintf_s(str, 32, L"%d", number);
	len = (int) wcslen(str);
	box = m_fontFixed->BBox(str, len);
	w = box.Upper().Xf() - box.Lower().Xf() + 4.0f;
	h = box.Upper().Yf() - box.Lower().Yf() + 4.0f;
	scale = hmax / h;
	if (wmax > 0.0f && w*scale > wmax) {
		scale = wmax / w;
	}

	glTranslatef(x, y, 0.0f);
	glScalef(scale, -scale, 1.0f);
	glTranslatef(2.0f-box.Lower().Xf(), -2.0f-box.Upper().Yf(), 0.0f);

	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
		glVertex3f(box.Lower().Xf()-2.0f, box.Lower().Yf()-2.0f, 0);
		glVertex3f(box.Lower().Xf()-2.0f, box.Upper().Yf()+2.0f, 0);
		glVertex3f(box.Upper().Xf()+2.0f, box.Upper().Yf()+2.0f, 0);
		glVertex3f(box.Upper().Xf()+2.0f, box.Lower().Yf()-2.0f, 0);
	glEnd();

	glColor3f(0.3f, 1.0f, 0.3f);
	m_fontFixed->Render(str, len);

	// Restore settings.
	if (isColorLogicOp) glEnable(GL_COLOR_LOGIC_OP);
	glColor4f(oldColor[0], oldColor[1], oldColor[2], oldColor[3]);
	glPopMatrix();

	return;
}
// added by eric at 20120821
void COpenGLControl::oglDrawOKNGText(void)
{
	if (m_bShowOKNGText == FALSE)
		return;

	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glLoadIdentity();
	glTranslatef(-0.5f*m_glwidth, -0.5f*m_glheight, -4.0f);

	GLfloat x = (GLfloat)m_glwidth-(GLfloat)150;
	GLfloat y = 0.0;
	glBlendFunc(GL_DST_COLOR,GL_ZERO);  // mask
	if (m_bShowOKText == TRUE) 
		glBindTexture(GL_TEXTURE_2D, m_texture[8]);		// ok text mask
	else
		glBindTexture(GL_TEXTURE_2D, m_texture[10]);	// ng text mask

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(x, y, 0.0f);
	glTexCoord2f(1, 0);
	glVertex3f(x+100, y, 0.0f);
	glTexCoord2f(1, 1);
	glVertex3f(x+100, y+64, 0.0f);
	glTexCoord2f(0, 1);
	glVertex3f(x, y+64, 0.0f);
	glEnd();

	glBlendFunc(GL_ONE, GL_ONE);
	if (m_bShowOKText == TRUE) 
		glBindTexture(GL_TEXTURE_2D, m_texture[7]);		// ok text
	else
		glBindTexture(GL_TEXTURE_2D, m_texture[9]);	// ng text
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(x, y, 0.0f);
	glTexCoord2f(1, 0);
	glVertex3f(x+100, y, 0.0f);
	glTexCoord2f(1, 1);
	glVertex3f(x+100, y+64, 0.0f);
	glTexCoord2f(0, 1);
	glVertex3f(x, y+64, 0.0f);
	glEnd();
}

void COpenGLControl::oglDrawInspectionText(void)
{
	if (m_show_inspection_text == FALSE)
		return;

	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glLoadIdentity();
	glTranslatef(-0.5f*m_glwidth, -0.5f*m_glheight, -4.0f);

	GLfloat x = (GLfloat)m_glwidth-(GLfloat)150;
	GLfloat y = 0.0;
	glBlendFunc(GL_DST_COLOR,GL_ZERO);  // mask
	if (m_curr_sample_result == TRUE) 
		glBindTexture(GL_TEXTURE_2D, m_texture[8]);		// ok text mask
	else
		glBindTexture(GL_TEXTURE_2D, m_texture[10]);	// ng text mask
	
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(x, y, 0.0f);
	glTexCoord2f(1, 0);
	glVertex3f(x+100, y, 0.0f);
	glTexCoord2f(1, 1);
	glVertex3f(x+100, y+64, 0.0f);
	glTexCoord2f(0, 1);
	glVertex3f(x, y+64, 0.0f);
	glEnd();

	glBlendFunc(GL_ONE, GL_ONE);
	if (m_curr_sample_result == TRUE) 
		glBindTexture(GL_TEXTURE_2D, m_texture[7]);		// ok text
	else
		glBindTexture(GL_TEXTURE_2D, m_texture[9]);	// ng text
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(x, y, 0.0f);
	glTexCoord2f(1, 0);
	glVertex3f(x+100, y, 0.0f);
	glTexCoord2f(1, 1);
	glVertex3f(x+100, y+64, 0.0f);
	glTexCoord2f(0, 1);
	glVertex3f(x, y+64, 0.0f);
	glEnd();
}

void COpenGLControl::oglDrawInspectionMark(void) {
#ifdef	SHOW_INSPECTION_MARK
	GLfloat width, height, x, y, bright;
	int error;

	if (!m_show_inspection_mark) return;

	width = 32;
	height = 32;

	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glBindTexture(GL_TEXTURE_2D, m_texture[6]);
	error = glGetError();
	if (error != GL_NO_ERROR) {
		return;
	}

	glLoadIdentity();
	glTranslatef(-0.5f*m_glwidth, -0.5f*m_glheight, -1.0f);
	for (int c=0; c<MAX_INSPECTION_MARK; c++) {
		switch (((m_curr_mark+MAX_INSPECTION_MARK)-c)%MAX_INSPECTION_MARK) {
		case 0:
			bright = 1.0f;
			break;
		case 1:
			bright = 0.9f;
			break;
		case 2:
			bright = 0.8f;
			break;
		case 3:
			bright = 0.7f;
			break;
		case 4:
			bright = 0.6f;
			break;
		default:
			bright = 0.5f;
			break;
		}

		x = m_glwidth/2-(width*MAX_INSPECTION_MARK)/2+(width) * c;
		y = 0;

		glBlendFunc(GL_DST_COLOR,GL_ZERO);  // mask
		if (m_inspection[c]) {
			glBindTexture(GL_TEXTURE_2D, m_texture[4]);	// ok mask
		} else {
			glBindTexture(GL_TEXTURE_2D, m_texture[6]);	// fail mask
		}
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex3f(x, y, 0.0f);
			glTexCoord2f(1, 0);
			glVertex3f(x+width, y, 0.0f);
			glTexCoord2f(1, 1);
			glVertex3f(x+width, y+height, 0.0f);
			glTexCoord2f(0, 1);
			glVertex3f(x, y+height, 0.0f);
		glEnd();

		glBlendFunc(GL_ONE, GL_ONE);
		if (m_inspection[c]) {
			glBindTexture(GL_TEXTURE_2D, m_texture[3]);	// ok sign
		} else {
			glBindTexture(GL_TEXTURE_2D, m_texture[5]);	// fail sign
		}
		glColor3f(bright, bright, bright);
		glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex3f(x, y, 0.0f);
			glTexCoord2f(1, 0);
			glVertex3f(x+width, y, 0.0f);
			glTexCoord2f(1, 1);
			glVertex3f(x+width, y+height, 0.0f);
			glTexCoord2f(0, 1);
			glVertex3f(x, y+height, 0.0f);
		glEnd();
	}
#endif /* SHOW_INSPECTION_MARK */
	return;
}

void COpenGLControl::oglSetInspectionMark(BOOL mark, BOOL *split_mark, int n) {
	m_curr_mark++;
	if (m_curr_mark >= MAX_INSPECTION_MARK) {
		m_curr_mark = 0;
	}
	m_inspection[m_curr_mark] = mark;

	if (split_mark != NULL && n > 0) {
		for (int c=0; c<n; c++) { m_split_inspection_mark[c] = split_mark[c]; }
	}
	return;
}

void COpenGLControl::oglResetInspectionMark()
{
	for (int c=0; c<MAX_INSPECTION_MARK; c++) 
	{
		m_inspection[c] = TRUE;
	}
	m_curr_mark = MAX_INSPECTION_MARK-1;

	for (int i=0; i<9; i++)
		m_split_inspection_mark[i] = TRUE;

	oglDrawScene();
}

// Beagle 20120703 modified.
void COpenGLControl::oglPrintf(wchar_t *fmt, ...)
{
	va_list	argptr;

	// Beagle 20120713 added -- Overflow protection.
	if (m_num_textstring >= 64) {
		m_num_textstring = 0;
		swprintf_s(m_textstring[m_num_textstring].str, 128, L"Text string overflow.");
		m_textstring[m_num_textstring].align = ALIGN_WINDOW|ALIGN_LEFT|ALIGN_TOP;
		m_textstring[m_num_textstring].x = 0;
		m_textstring[m_num_textstring].y = 0;
		m_num_textstring++;
		return;
	}

	va_start(argptr, fmt);

	m_textstring[m_num_textstring].align = ALIGN_WINDOW|ALIGN_LEFT|ALIGN_BOTTOM;
	m_textstring[m_num_textstring].x = 0;
	m_textstring[m_num_textstring].y = 0;
	vswprintf_s(m_textstring[m_num_textstring].str, 128, fmt, argptr);

	m_num_textstring++;
}

// Beagle 20120703 added.
void COpenGLControl::oglPrintf(int align, float x, float y, wchar_t *fmt, ...)
{
	va_list	argptr;

	// Beagle 20120713 added -- Overflow protection.
	if (m_num_textstring >= 64) {
		m_num_textstring = 0;
		swprintf_s(m_textstring[m_num_textstring].str, 128, L"Text string overflow.");
		m_textstring[m_num_textstring].align = ALIGN_WINDOW|ALIGN_LEFT|ALIGN_TOP;
		m_textstring[m_num_textstring].x = 0;
		m_textstring[m_num_textstring].y = 0;
		m_num_textstring++;
		return;
	}

	va_start(argptr, fmt);
	// modified by eric at 20150203
	BOOL bHit=FALSE;
	for (int i=0; i<m_num_textstring; i++) {
		if (m_textstring[i].align == align &&
			m_textstring[i].x == x &&
			m_textstring[i].y == y) {
			bHit=TRUE;
			vswprintf_s(m_textstring[i].str, 128, fmt, argptr);
			break;
		}
	}
	if (bHit==FALSE) {
		m_textstring[m_num_textstring].align = align;
		m_textstring[m_num_textstring].x = x;
		m_textstring[m_num_textstring].y = y;
		vswprintf_s(m_textstring[m_num_textstring].str, 128, fmt, argptr);
		m_num_textstring++;
	}

}

// Beagle 20120703 modified.
void COpenGLControl::oglDrawTextString(void)
{
	FTBBox box;

	if (m_num_textstring <= 0) { return; }

	glDisable(GL_COLOR_LOGIC_OP);
	for (int c=0; c<m_num_textstring; c++) {
		int split_id = 0; //eric chao 20130628
		if (m_textstring[c].align & ALIGN_WINDOW){
			if (m_textstring[c].align & ALIGN_TOP){
				if (m_textstring[c].align & ALIGN_LEFT) {
					split_id =0 ;
				}else if (m_textstring[c].align & ALIGN_RIGHT) {
					split_id = m_wsplit-1;
				}
			}else if (m_textstring[c].align & ALIGN_BOTTOM){
				if (m_textstring[c].align & ALIGN_LEFT) {
					split_id = (m_hsplit-1)*m_wsplit ;
				}else if (m_textstring[c].align & ALIGN_RIGHT) {
					split_id = m_hsplit*m_wsplit-1;
				}
			}
		}
		if (m_textstring[c].align & ALIGN_SPLIT_MASK){ //eric chao 20131122
			split_id = (m_textstring[c].align&ALIGN_SPLIT_MASK)>>12;
		}
		DrawSingleString(split_id, m_textstring[c].align, m_textstring[c].x, m_textstring[c].y, m_textstring[c].str);
	}
}
// added by eric at 20121121
void COpenGLControl::oglSetLineMark(int idx)
{
	if (idx < 0 || idx > m_lineRects) 
		m_selLine = -1;
	else
		m_selLine = idx;
	oglDrawScene();
}
// added by eric at 20130107
void COpenGLControl::oglSetLineMark(CRect rt)
{
	for (int c=0; c<m_lineRects; c++) {
		if (m_lineRect[c].left == rt.left && m_lineRect[c].top == rt.top && m_lineRect[c].right == rt.right && m_lineRect[c].bottom == rt.bottom) {
			m_selLine = c;
			break;
		}
	}
	oglDrawScene();
}
void COpenGLControl::oglSetAnchorMark(int idx)
{
	if (idx < 0 || idx > m_anchors) 
		m_selAnchor = -1;
	else
		m_selAnchor = idx;
	oglDrawScene();
}

void COpenGLControl::oglSetAnchorRangeMark(int nRectIdx)
{
	if (nRectIdx < 0 || nRectIdx > m_mapRectInfo[ NULL ].rects) 
		m_selAnchorRange = -100;
	else
		m_selAnchorRange = nRectIdx;
	oglDrawScene();
}


void COpenGLControl::oglSetRectangleMark(std::vector<int> *idxList, BOOL bDrawTracker)
{
	m_selRectangle.clear();
	if (idxList != NULL)
		m_selRectangle.assign(idxList->begin(), idxList->end());
	/*
	m_selPolygon = -1;
	if (idx < 0 || idx > m_rects) 
		m_selRectangle = -1;
	else
		m_selRectangle = idx;
	*/

	// added by eric at 20120330
	if (bDrawTracker)
	{
		BOOL found = FALSE;
		for (int c=0; c<m_mapRectInfo[ NULL ].rects; c++) 
		{
			for (int r=0; r<(int)m_selRectangle.size(); r++)
			{
				if (m_selRectangle[r] == c)
				{
					found = TRUE;
					break;
				}
			}

			if (found)
			{
				m_nBoundedType = TYPE_NORMAL;
				m_rtSelected.left = m_mapRectInfo[ NULL ].rect[c].left;
				m_rtSelected.top = m_mapRectInfo[ NULL ].rect[c].top;
				m_rtSelected.right = m_mapRectInfo[ NULL ].rect[c].right;
				m_rtSelected.bottom = m_mapRectInfo[ NULL ].rect[c].bottom;
				m_nSelectedIndex = c;	// modified by eric at 20120730
				SetBoundedTracker();
				break;
			}
		}
		//oglDrawScene();	// modified by eric at 20120928
	}
}

void COpenGLControl::oglSetPolygonMark(std::vector<int> *idxList, BOOL bDrawTracker)
{
	m_selPolygon.clear();
	if (idxList != NULL)
		m_selPolygon.assign(idxList->begin(), idxList->end());
	/*
	m_selRectangle = -1;
	if (idx < 0 || idx > m_polygons) 
		m_selPolygon = -1;
	else
		m_selPolygon = idx;
	*/

	// added by eric at 20120330
	if (bDrawTracker)
	{
		BOOL found = FALSE;
		for (int c=0; c<m_polygons; c++) 
		{
			for (int r=0; r<(int)m_selPolygon.size(); r++)
			{
				if (m_selPolygon[r] == c)
				{
					found = TRUE;
					break;
				}
			}

			if (found)
			{
				m_nBoundedType = TYPE_POLYGON;
				CRgn *rgn = new CRgn();
				int no = m_poly_points[c];
				POINT *vertexList = new POINT[no];
				for (int i=0; i<no; i++)
				{
					vertexList[i].x = m_polygon[c][i].x;
					vertexList[i].y = m_polygon[c][i].y;
				}

				rgn->CreatePolygonRgn(vertexList, no, ALTERNATE);
				rgn->GetRgnBox(&m_rtSelected);

				rgn->DeleteObject();
				delete rgn;
				delete [] vertexList;

				m_nSelectedIndex = c;	// modified by eric at 20120730
				SetBoundedTracker();
				break;
			}
		}
		//oglDrawScene();	// modified by eric at 20120928
	}
}
// added by eric at 20120330
void COpenGLControl::oglSetRegionMark(std::vector<int> *idxList, BOOL bDrawTracker)
{
	m_selRegion.clear();

	if (idxList != NULL)
		m_selRegion.assign(idxList->begin(), idxList->end());

	if (bDrawTracker)
	{
		BOOL found = FALSE;
		for (int c=0; c<m_regions; c++) 
		{
			for (int r=0; r<(int)m_selRegion.size(); r++)
			{
				if (m_selRegion[r] == c)
				{
					found = TRUE;
					break;
				}
			}

			if (found)
			{
				m_nBoundedType = TYPE_REGION;
				m_region[c]->GetRgnBox(&m_rtSelected);
				m_nSelectedIndex = c;	// modified by eric at 20120730
				SetBoundedTracker();
				break;
			}
		}
		//oglDrawScene();	// modified by eric at 20120928
	}
}


void COpenGLControl::oglSetChessboardMark(std::vector<CPoint> *pointList, std::vector<CPoint> *distortionptr,  std::vector<CPoint> *avgdistortionptr)
{
      m_pointList.clear();
	  m_dispointList.clear();
	  m_avgdisptrList.clear();

	if (pointList != NULL)
		m_pointList.assign(pointList->begin(), pointList->end());
	if (distortionptr != NULL)
		m_dispointList.assign(distortionptr->begin(), distortionptr->end());

	if (avgdistortionptr!= NULL)
		m_avgdisptrList.assign(avgdistortionptr->begin(), avgdistortionptr->end());

}

void COpenGLControl::oglMoveMarkedSegment(int type, int idx, POINT offset)
{
	if (type == 0)
	{
		for (int i=0; i<m_poly_points[idx]; i++)
		{
			m_polygon[idx][i].x += offset.x;
			m_polygon[idx][i].y += offset.y;
		}
	}
	else if (type == 1)
	{
		m_mapRectInfo[ NULL ].rect[idx].right += offset.x;
		m_mapRectInfo[ NULL ].rect[idx].left += offset.x;
		m_mapRectInfo[ NULL ].rect[idx].bottom += offset.y;
		m_mapRectInfo[ NULL ].rect[idx].top += offset.y;
	}
}

void COpenGLControl::oglDrawCorona(float cx, float cy, float scale)
{
	glPushMatrix();
	glDisable(GL_TEXTURE_2D);
	glTranslated(cx, cy, 0.0f);
	glScalef(100.0f/scale, 100.0f/scale, 1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINE_LOOP);
	DrawCircle(1.0f);
	glEnd();
	glPopMatrix();
}

void COpenGLControl::oglSetInspectionSignImg(CImage *signList)
{
	int w = signList[0].GetWidth();
	int h = signList[0].GetHeight();
	int pitch = signList[0].GetPitch();
	BOOL isUpSide = pitch<0? TRUE:FALSE;
	int size = w*h*3;
	int i, j;

	oglMakeCurrent();

	unsigned char *m_okSign = new unsigned char[size];
	unsigned char *m_okMask = new unsigned char[size];
	unsigned char *m_failSign = new unsigned char[size];
	unsigned char *m_failMask = new unsigned char[size];

	unsigned char *ptr1 = (unsigned char*)signList[0].GetBits();
	unsigned char *ptr2 = (unsigned char*)signList[1].GetBits();
	unsigned char *ptr3 = (unsigned char*)signList[2].GetBits();
	unsigned char *ptr4 = (unsigned char*)signList[3].GetBits();


	unsigned char	*dtr1 = m_okSign, 
					*dtr2 = m_okMask,
					*dtr3 = m_failSign,
					*dtr4 = m_failMask;
	for (i=0; i<h; i++)
	{
		for (j=0; j<w; j++)
		{
			dtr1[0] = ptr1[2];
			dtr1[1] = ptr1[1];
			dtr1[2] = ptr1[0];

			dtr2[0] = ptr2[2];
			dtr2[1] = ptr2[1];
			dtr2[2] = ptr2[0];

			dtr3[0] = ptr3[2];
			dtr3[1] = ptr3[1];
			dtr3[2] = ptr3[0];

			dtr4[0] = ptr4[2];
			dtr4[1] = ptr4[1];
			dtr4[2] = ptr4[0];

			dtr1+=3;
			dtr2+=3;
			dtr3+=3;
			dtr4+=3;

			ptr1+=3;
			ptr2+=3;
			ptr3+=3;
			ptr4+=3;
		}

		if (isUpSide)
		{
			ptr1 += 2*pitch;
			ptr2 += 2*pitch;
			ptr3 += 2*pitch;
			ptr4 += 2*pitch;
		}
	}

	//
	//	Mask 前景黑色, 背景白色,
	//	Sign 本身背景要是黑色。
	//
	// set inspection sign textures
	glBindTexture(GL_TEXTURE_2D, m_texture[3]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, m_okSign);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, m_texture[4]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, m_okMask);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, m_texture[5]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, m_failSign);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, m_texture[6]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, m_failMask);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	delete [] m_okSign;
	delete [] m_okMask;
	delete [] m_failSign;
	delete [] m_failMask;

	// added by danny at 20091214
	w = signList[4].GetWidth();
	h = signList[4].GetHeight();
	pitch = signList[4].GetPitch();
	isUpSide = pitch<0? TRUE:FALSE;
	size = w*h*3;

	unsigned char *m_okText = new unsigned char[size];
	unsigned char *m_okTextMask = new unsigned char[size];
	unsigned char *m_ngText = new unsigned char[size];
	unsigned char *m_ngTextMask = new unsigned char[size];

	ptr1 = (unsigned char*)signList[4].GetBits();
	ptr2 = (unsigned char*)signList[5].GetBits();
	ptr3 = (unsigned char*)signList[6].GetBits();
	ptr4 = (unsigned char*)signList[7].GetBits();

	dtr1 = m_okText; 
	dtr2 = m_okTextMask;
	dtr3 = m_ngText;
	dtr4 = m_ngTextMask;
	for (i=0; i<h; i++)
	{
		for (j=0; j<w; j++)
		{
			dtr1[0] = ptr1[2];
			dtr1[1] = ptr1[1];
			dtr1[2] = ptr1[0];

			dtr2[0] = ptr2[2];
			dtr2[1] = ptr2[1];
			dtr2[2] = ptr2[0];

			dtr3[0] = ptr3[2];
			dtr3[1] = ptr3[1];
			dtr3[2] = ptr3[0];

			dtr4[0] = ptr4[2];
			dtr4[1] = ptr4[1];
			dtr4[2] = ptr4[0];

			dtr1+=3;
			dtr2+=3;
			dtr3+=3;
			dtr4+=3;

			ptr1+=3;
			ptr2+=3;
			ptr3+=3;
			ptr4+=3;
		}

		if (isUpSide)
		{
			ptr1 += 2*pitch;
			ptr2 += 2*pitch;
			ptr3 += 2*pitch;
			ptr4 += 2*pitch;
		}
	}

	//
	//	Mask 前景黑色, 背景白色,
	//	Sign 本身背景要是黑色。
	//
	// set inspection text textures
	glBindTexture(GL_TEXTURE_2D, m_texture[7]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, m_okText);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, m_texture[8]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, m_okTextMask);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, m_texture[9]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, m_ngText);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, m_texture[10]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, m_ngTextMask);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	delete [] m_okText;
	delete [] m_okTextMask;
	delete [] m_ngText;
	delete [] m_ngTextMask;
}

void COpenGLControl::OnKeyDown(UINT nChar)
{
	switch (nChar)
	{
	case VK_LEFT:
		m_xTranslate -= 5.0f/(m_norm_scale[0]*m_glScale);
		break;
	case VK_RIGHT:
		m_xTranslate += 5.0f/(m_norm_scale[0]*m_glScale);
		break;
	case VK_UP:
		m_yTranslate -= 5.0f/(m_norm_scale[0]*m_glScale);
		break;
	case VK_DOWN:
		m_yTranslate += 5.0f/(m_norm_scale[0]*m_glScale);
		break;
	case VK_PRIOR:
		m_glScale *= (GLfloat)1.02;
		break;
	case VK_NEXT:
		m_glScale *= (GLfloat)0.98;
		break;
	case VK_ESCAPE:
		{
			if (m_LeftButtonDown == TRUE)
			{
				m_LeftButtonDown = FALSE;
				ReleaseCapture();

				switch (m_lmMode)
				{
				case LM_DRAW_RECTANGLE:
				case LM_DRAW_CIRCLE:
				case LM_DRAW_ELLIPSE:
					m_mapRectInfo[ NULL ].rects--;
					break;
				}
			}
			else if (m_poly_drawing == TRUE)
				m_poly_drawing = FALSE;
		}
		break;
	default:
		break;
	}

	InvalidateRect(NULL, FALSE);
}


#ifdef CUDA_OPENGL_INTEROP

void COpenGLControl::oglCreateDynamicBuffer(int width, int height, HDC *pDC, HGLRC *pGLRC, int *pBufNum)
{
	int size = width*height*3*sizeof(unsigned char);

	m_width[0] = width;
	m_height[0] = height;

	oglMakeCurrent();

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_texBuffer[0]);
	GL_CHECK_ERROR();
	glBufferData(GL_PIXEL_UNPACK_BUFFER, size, 0, GL_DYNAMIC_DRAW);
	GL_CHECK_ERROR();
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	GL_CHECK_ERROR();

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_texBuffer[1]);
	GL_CHECK_ERROR();
	glBufferData(GL_PIXEL_UNPACK_BUFFER, size, 0, GL_DYNAMIC_DRAW);
	GL_CHECK_ERROR();
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	GL_CHECK_ERROR();

	if (m_showdefect) {
		*pBufNum = m_texBuffer[1];
	} else {
		*pBufNum = m_texBuffer[0];
	}

	*pDC = hdc;
	*pGLRC = hrc_loader;
}
#endif

// Beagle 20130618 modified.
int COpenGLControl::oglSetDefectPiece(DEFECT_PIECE *pDefect)
{
	int i;

	if (pDefect == NULL) { return -1; }
	if (pDefect->index < 0 || pDefect->index >= MAX_DEFECTS) { return -1; }

	if (m_display_mode == DISPLAY_MODE_DEFECT_STACK) {
		i = m_next_defect;
		m_next_defect++;
		m_next_defect %= (m_wsplit*m_hsplit);
	} else {
		i = pDefect->index;
	}

	wglMakeCurrent(hdc, hrc_loader);

	// Make a shallow copy to keep defect information.
	memcpy(m_defectPiece+i, pDefect, sizeof(DEFECT_PIECE));
	m_defectPiece[i].image.ptr = NULL;
	m_defectPiece[i].shade.ptr = NULL;

	if (pDefect->image.ptr != NULL) {
		SetTexture(&pDefect->image, m_piece_texture[i], 0);
	}

	if (pDefect->shade.ptr != NULL) {
		SetTexture(&pDefect->shade, m_piece_tex_shade[i], 0);
	}

	return i; //draw pos
}

// 以目前滑鼠游標位置為中心點, 計算 nWidth X nHeight 大小的長方形外框位置。
// Beagle 20130425 rewrite oglGetAnchorSize()
void COpenGLControl::oglGetCenteredRect(RECT &rect, int iCenterX, int iCenterY,
										int iWidth, int iHeight)
{
	int w, h, shift;

	// 有效區域包含上下左右邊界
	w = iWidth-1;
	h = iHeight-1;

	if (w < 0 || w > 100000) { w=31; }
	if (h < 0 || h > 100000) { h=31; }

	rect.left   = (int) floorf(iCenterX-w/2.0f);
	rect.right  = (int) floorf(iCenterX+w/2.0f);
	rect.top    = (int) floorf(iCenterY-h/2.0f);
	rect.bottom = (int) floorf(iCenterY+h/2.0f);

	// 出界處理
	if (rect.left < 0) {
		shift = -rect.left;
		rect.left += shift;
		rect.right += shift;
	} else if (rect.right >= m_width[0]) {
		shift = rect.right - m_width[0] + 1;
		rect.left -= shift;
		rect.right -= shift;
	}

	if (rect.top < 0) {
		shift = -rect.top;
		rect.top += shift;
		rect.bottom += shift;
	} else if (rect.bottom >= m_height[0]) {
		shift = rect.bottom - m_height[0] + 1;
		rect.top -= shift;
		rect.bottom -= shift;
	}
}

void COpenGLControl::oglClearUserLayoutRect()
{
	for (int i=0; i<MAX_SPLIT_WINDOW; i++){
		m_rcUserLayout[i].clear();		
	}

}

void COpenGLControl::DrawUserLayoutRectangle(int split_id)
{
	if(split_id<0 || split_id>MAX_SPLIT_WINDOW-1)
		return;

	if(m_rcUserLayout[split_id].size()<=0)
		return;



	float x1 = 0;
	float x2 = 0;
	float y1 = 0;
	float y2 = 0;	
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_LOGIC_OP);
	glDisable(GL_BLEND);
	glLineWidth(3.0f);

	int nExtW = 224;
	int nExtH = 256;
	int nCtrX = 0;
	int nCtrY = 0;

	for (size_t j=0; j<m_rcUserLayout[split_id].size(); j++){

		x1 = m_rcUserLayout[split_id][j].left;
		x2 = m_rcUserLayout[split_id][j].right;
		y1 = m_rcUserLayout[split_id][j].top;
		y2 = m_rcUserLayout[split_id][j].bottom;

		// modified by eric at 20161007
		BOOL bHit=FALSE;
		if (m_nFocusSplitId==split_id && m_rtFocusUserLayoutRectangle.left==x1 && m_rtFocusUserLayoutRectangle.right==x2 && m_rtFocusUserLayoutRectangle.top==y1 && m_rtFocusUserLayoutRectangle.bottom==y2) {
			bHit=TRUE;
			glColor3f(1.0, 0.0, 0.0);
		} else {
			glColor3f(0.0, 1.0, 0.0);
		}
		glBegin(GL_LINE_LOOP);
			glVertex3f(x1, y1, 0.0f);
			glVertex3f(x2, y1, 0.0f);
			glVertex3f(x2, y2, 0.0f);
			glVertex3f(x1, y2, 0.0f);
		glEnd();

		if(m_bDrawUserExtLY){
			nCtrX = (x1 + x2)/2;
			nCtrY = (y1 + y2)/2;

			x1 = nCtrX - nExtW/2;
			x2 = nCtrX + nExtW/2;
			y1 = nCtrY - nExtH/2;
			y2 = nCtrY + nExtH/2;

			if(x1<0)
				x1 = 0;

			if(y1<0)
				y1 = 0;

			if(x2>m_width[split_id]-1)
				x2=m_width[split_id]-1;

			if(y2>m_height[split_id]-1)
				y2=m_height[split_id]-1;
			

 			if (bHit==TRUE) {
				glColor3f((float)GetRValue(m_glExtColor)/255.0f,
				(float)GetGValue(m_glExtColor)/255.0f,
				(float)GetBValue(m_glExtColor)/255.0f);
 			}
			glEnable(GL_COLOR_LOGIC_OP); 
			glDisable(GL_TEXTURE_2D);
 			glLogicOp(GL_OR_REVERSE);

			glBegin(GL_LINE_LOOP);
				glVertex3f(x1, y1, 0.0f);
				glVertex3f(x2, y1, 0.0f);
				glVertex3f(x2, y2, 0.0f);
				glVertex3f(x1, y2, 0.0f);
			glEnd();
		}
	}
	

#if 0  //try advance function

	//int w = m_width[split_id] / m_FrameGrid[0];
	//int h = m_height[split_id] / m_FrameGrid[1];
	//int w = m_width[split_id];
	//int h = m_height[split_id];

	//int c = 0;
	//int xg = c % m_FrameGrid[0];
	//int yg = c / m_FrameGrid[0];
	//float x1 = (float) (xg*w+10.0f);
	//float x2 = (float) ((xg+1)*w-10.0f);
	//float y1 = (float) (yg*h+10.0f);
	//float y2 = (float) ((yg+1)*h-10.0f);

	float x1 = 0;
	float x2 = 0;
	float y1 = 0;
	float y2 = 0;


	x1 = 100.0;
	x2 = 980.0;
	y1 = 100.0;
	y2 = 640.0;


	_D(_T("\n testtest aaa [%d-%d-%d-%d]\n"),(int)x1,(int)x2,(int)y1,(int)y2);

	int c = 0;
	//glColor3f(m_FrameColor[c][1], m_FrameColor[c][2], m_FrameColor[c][3]);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINE_LOOP);
		glVertex3f(x1, y1, 0.0f);
		glVertex3f(x2, y1, 0.0f);
		glVertex3f(x2, y2, 0.0f);
		glVertex3f(x1, y2, 0.0f);
	glEnd();


#endif

}
// added by eric at 20161007
void COpenGLControl::oglClearLayoutRectangle(int split_id, BOOL redraw)
{
	if(split_id<0||split_id>MAX_SPLIT_WINDOW-1)
		return;

	m_rcUserLayout[split_id].clear();
	m_nFocusSplitId=0;
	memset(&m_rtFocusUserLayoutRectangle,0,sizeof(RECT));
	
	if (redraw) {
		oglDrawScene();
	}
}
// added by eric at 20161007
void COpenGLControl::oglSetUserLayoutRectangle(int split_id, RECT RcUserLayout, BOOL redraw, BOOL bFocus)
{
	if(split_id<0||split_id>MAX_SPLIT_WINDOW-1)
		return;

	m_rcUserLayout[split_id].push_back(RcUserLayout);

	if (bFocus) {
		m_nFocusSplitId=split_id;
		m_rtFocusUserLayoutRectangle=RcUserLayout;
	}

	if (redraw) {
		oglDrawScene();
	}
}

// Beagle 20120224 added.
//		barcode_result == NULL: Don't show any numbers.
//		barcode_result != NULL, ocr_result == NULL: Show green numbers of barcode result.
//		barcode_result != NULL, ocr_result != NULL: Compare two results and show green/red color.
//		position = 0: Upper left, 1: Upper right
void COpenGLControl::oglSetOCRResult(char *barcode_result, char *ocr_result, int position)
{
	memset(m_barcode_result, 0, MAX_GL_BARCODE_DIGITS+1);
	memset(m_ocr_result, 0, MAX_GL_BARCODE_DIGITS+1);

	if (barcode_result != NULL) {
		strncpy_s(m_barcode_result, MAX_GL_BARCODE_DIGITS+1, barcode_result, MAX_GL_BARCODE_DIGITS);
	}

	if (ocr_result != NULL) {
		strncpy_s(m_ocr_result, MAX_GL_BARCODE_DIGITS+1, ocr_result, MAX_GL_BARCODE_DIGITS);
	}

	m_barcode_position = position;	// Beagle 20120326 added.
}

void COpenGLControl::oglDrawStackInspIndex(int nHistogram,int nXIndex,int nYIndex)
{

	if(nHistogram < 0)
		return;

	GLfloat w, h;
	FTBBox	Text_box;

	BOOL bBlackGround2Line = FALSE;
	BOOL bBlackGroundLowLine = TRUE;
	BOOL bEndLinePos = TRUE; 
	CString strHistorgram = _T("123456789");
	strHistorgram.Format(_T("%07d"),nHistogram);



	int nStrLen = (int) strHistorgram.GetLength(); //eric chao 20140415

	Text_box = m_fontFixed->BBox(strHistorgram, nStrLen);

	w = Text_box.Upper().Xf() - Text_box.Lower().Xf() + 4.0f;
	h = Text_box.Upper().Yf() - Text_box.Lower().Yf() + 4.0f;

	glDisable(GL_COLOR_LOGIC_OP);
	glLoadIdentity();

	//_D(_T("\n nXIndex = %d , nYIndex = %d , [%f ] [%f ] [%f] [%f ] <-> [%f] [%f ] [%f] [%f ]"),nXIndex,nYIndex
	//													,m_glwidth*((0.5f+nXIndex)/m_wsplit-0.5f),m_glheight*((0.5f+nYIndex)/m_hsplit-0.5f)
	//													,((0.5f+nXIndex)/m_wsplit-0.5f),((0.5f+nYIndex)/m_hsplit-0.5f)
	//													,m_glwidth*((nXIndex+1.0f)/m_wsplit-0.5f),m_glheight*((nYIndex+1.0f)/m_hsplit-0.5f)
	//													,((nXIndex+1.0f)/m_wsplit-0.5f),((nYIndex+1.0f)/m_hsplit-0.5f));
	// === draw  position === //
	int nTextPosMode = 2;
	if(m_wsplit>0&&m_hsplit>0)
	{
		switch(nTextPosMode)
		{
			case 1: //LB
			{
				glTranslatef(m_glwidth*((nXIndex+0.0f)/m_wsplit-0.5f),
				m_glheight*((nYIndex+1.0f)/m_hsplit-0.5f)-h, 0.0f);
			}
			break;
			case 2: //RB
			{
				glTranslatef(m_glwidth*((nXIndex+1.0f)/m_wsplit-0.5f)-w,
				m_glheight*((nYIndex+1.0f)/m_hsplit-0.5f)-h, 0.0f);
			}
			break;

			case 0: //center
			default:
				{
					glTranslatef(m_glwidth*((0.5f+nXIndex)/m_wsplit-0.5f),
					m_glheight*((0.5f+nYIndex)/m_hsplit-0.5f), 0.0f);
				}
				break;
		}
	}
	else
	{

		if (bEndLinePos) { //global 位置
			glTranslatef( 0.5f*m_glwidth-w, -0.5f*m_glheight+h, -40.0f);
		} else {
			glTranslatef(-0.5f*m_glwidth, -0.5f*m_glheight+h, -40.0f);
		}
	}


	glScalef(1.0f, -1.0f, 1.0f);
	glTranslatef(2.0f-Text_box.Lower().Xf(), 2.0f-Text_box.Lower().Yf(), 0.0f);

	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
	if (bBlackGround2Line) { //黑底
		glVertex3f(Text_box.Lower().Xf()-2.0f, Text_box.Lower().Yf()-2.0f-h, 0);
		glVertex3f(Text_box.Lower().Xf()-2.0f, Text_box.Upper().Yf()+2.0f, 0);
		glVertex3f(Text_box.Upper().Xf()+2.0f, Text_box.Upper().Yf()+2.0f, 0);
		glVertex3f(Text_box.Upper().Xf()+2.0f, Text_box.Lower().Yf()-2.0f-h, 0);
	} else {

		if(bBlackGroundLowLine){
			glVertex3f(Text_box.Lower().Xf()-2.0f, Text_box.Lower().Yf()-2.0f-h, 0);
			glVertex3f(Text_box.Lower().Xf()-2.0f, Text_box.Upper().Yf()+2.0f-h, 0);
			glVertex3f(Text_box.Upper().Xf()+2.0f, Text_box.Upper().Yf()+2.0f-h, 0);
			glVertex3f(Text_box.Upper().Xf()+2.0f, Text_box.Lower().Yf()-2.0f-h, 0);
		}
		else{
			glVertex3f(Text_box.Lower().Xf()-2.0f, Text_box.Lower().Yf()-2.0f, 0);
			glVertex3f(Text_box.Lower().Xf()-2.0f, Text_box.Upper().Yf()+2.0f, 0);
			glVertex3f(Text_box.Upper().Xf()+2.0f, Text_box.Upper().Yf()+2.0f, 0);
			glVertex3f(Text_box.Upper().Xf()+2.0f, Text_box.Lower().Yf()-2.0f, 0);
		}
	}
	glEnd();



	if (0) {
		glColor3f(0.3f, 1.0f, 0.3f);	//show green.
	} else {
		glColor3f(1.0f, 0.3f, 0.3f);	//show red.
	}


	//m_fontFixed->Render(strHistorgram.GetBuffer(), strHistorgram.GetLength());
	m_fontFixed->Render(strHistorgram.GetBuffer(), strHistorgram.GetLength(),FTPoint(0, -h, 0));
	
	//m_fontFixed->Render(strHistorgram.GetBuffer(), strHistorgram.GetLength(), FTPoint(0, 0, 0)); //additional line

	//glTranslatef(m_fontFixed->Advance(strHistorgram.GetBuffer(), 1), 0.0f, 0.0f);
	glTranslatef(m_fontFixed->Advance(strHistorgram.GetBuffer(), strHistorgram.GetLength()), 0.0f, 0.0f); //0.0f, 0.0f //不旋轉


	strHistorgram.ReleaseBuffer();
	//strHistorgram.
}



// Beagle 20120224 added. // Beagle 20120504 changed to using FTGL font.
void COpenGLControl::oglDrawBarcodeResult(void)
{
	GLfloat w, h;
	BOOL	compare_result;
	FTBBox	bar_box;

	if (m_barcode_result[0] == '\0') { return; }
	if (m_ocr_result[0] != '\0') { compare_result = TRUE; }
	else { compare_result = FALSE; }

	int nStrLen = (int) strlen(m_barcode_result); //eric chao 20140415
	if (nStrLen > MAX_GL_BARCODE_DIGITS){
		nStrLen = MAX_GL_BARCODE_DIGITS;
	}
	bar_box = m_fontFixed->BBox(m_barcode_result, nStrLen);

	w = bar_box.Upper().Xf() - bar_box.Lower().Xf() + 4.0f;
	h = bar_box.Upper().Yf() - bar_box.Lower().Yf() + 4.0f;

	glDisable(GL_COLOR_LOGIC_OP);
	glLoadIdentity();
	// Beagle 20120326 modified.
	if (m_barcode_position == 1) {
		glTranslatef( 0.5f*m_glwidth-w, -0.5f*m_glheight+h, -40.0f);
	} else {
		glTranslatef(-0.5f*m_glwidth, -0.5f*m_glheight+h, -40.0f);
	}
	glScalef(1.0f, -1.0f, 1.0f);
	glTranslatef(2.0f-bar_box.Lower().Xf(), 2.0f-bar_box.Lower().Yf(), 0.0f);

	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
	if (compare_result) {
		glVertex3f(bar_box.Lower().Xf()-2.0f, bar_box.Lower().Yf()-2.0f-h, 0);
		glVertex3f(bar_box.Lower().Xf()-2.0f, bar_box.Upper().Yf()+2.0f, 0);
		glVertex3f(bar_box.Upper().Xf()+2.0f, bar_box.Upper().Yf()+2.0f, 0);
		glVertex3f(bar_box.Upper().Xf()+2.0f, bar_box.Lower().Yf()-2.0f-h, 0);
	} else {
		glVertex3f(bar_box.Lower().Xf()-2.0f, bar_box.Lower().Yf()-2.0f, 0);
		glVertex3f(bar_box.Lower().Xf()-2.0f, bar_box.Upper().Yf()+2.0f, 0);
		glVertex3f(bar_box.Upper().Xf()+2.0f, bar_box.Upper().Yf()+2.0f, 0);
		glVertex3f(bar_box.Upper().Xf()+2.0f, bar_box.Lower().Yf()-2.0f, 0);
	}
	glEnd();

	glColor3f(0.3f, 1.0f, 0.3f);	// Default color green.
	for (int c=0; c<MAX_GL_BARCODE_DIGITS; c++) {
		char barcode_char = m_barcode_result[c];
		char ocr_char = m_ocr_result[c];

		if (barcode_char < '0' || barcode_char > '9') { barcode_char = ' '; }
		if (ocr_char < '0' || ocr_char > '9') { ocr_char = ' '; }
		if (compare_result) {
			if (barcode_char == ocr_char) {
				glColor3f(0.3f, 1.0f, 0.3f);	// Two digits match, show green.
			} else {
				glColor3f(1.0f, 0.3f, 0.3f);	// Digits mismatch, show red.
			}
		}

		m_fontFixed->Render(&barcode_char, 1);
		if (compare_result) m_fontFixed->Render(&ocr_char, 1, FTPoint(0, -h, 0));

		glTranslatef(m_fontFixed->Advance(&barcode_char, 1), 0.0f, 0.0f);
	}
}

// Beagle 20120301 added.
void COpenGLControl::oglSetBarcodeDigitImage(int n, int x, int y, unsigned char *ptr, int depth)
{
	unsigned char *buf;
	int rx = (x+3)&(~0x03);
	int	size = rx*y*depth;
	int	srcPitch = x*depth;
	int dstPitch = rx*depth;

	if (ptr == NULL) return;
	if (depth != 3 && depth != 1) return;	// Only accept 8 bits and 24 bits.

	buf = (unsigned char *) malloc(size);
	memset(buf, 0, size);
	for (int h=0; h<y; h++) {
		memcpy(buf+h*dstPitch, ptr+h*srcPitch, srcPitch);
	}

	wglMakeCurrent(hdc, hrc_loader);
	glBindTexture(GL_TEXTURE_2D, m_texBarcodeDigits[n]);
	if (depth == 3) {
#ifdef USE_BGR_IMGDATA
		glTexImage2D(GL_TEXTURE_2D, 0, 3, rx, y, 0, GL_BGR, GL_UNSIGNED_BYTE, buf);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, 3, rx, y, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
#endif
	} else {	// depth == 1
		glTexImage2D(GL_TEXTURE_2D, 0, 1, rx, y, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buf);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	m_width_BarcodeDigits = x;
	m_height_BarcodeDigits = y;

	free(buf);
}

// Beagle 20120301 added.
void COpenGLControl::oglBarcodeDigitPosition(int n, int *nArray, float *xArray, float *yArray)
{
	if (xArray == NULL || yArray == NULL) return;

	memset(m_pos_BarcodeDigits, 0, sizeof(m_pos_BarcodeDigits));
	for (int c=0; c<n; c++) {
		m_pos_BarcodeDigits[c].n = nArray[c];
		m_pos_BarcodeDigits[c].x = xArray[c];
		m_pos_BarcodeDigits[c].y = yArray[c];
	}
}

// Beagle 20120301 added.
void COpenGLControl::oglDrawBarcodeDigits(void)
{
	int		w = m_width_BarcodeDigits;
	int		h = m_height_BarcodeDigits;
	GLfloat	tw = w * 1.0f / ((w+3)&(~0x03));

	if (m_showBarcodeDigitsImage == FALSE) return;

	glDisable(GL_BLEND);
	glDisable(GL_COLOR_LOGIC_OP);
	glEnable(GL_TEXTURE_2D);

	for (int c=0; c<20; c++) {
		int		n = m_pos_BarcodeDigits[c].n;
		float	x = m_pos_BarcodeDigits[c].x;
		float	y = m_pos_BarcodeDigits[c].y;

		if (n < 0 || n > 9) { continue; }
		glBindTexture(GL_TEXTURE_2D, m_texBarcodeDigits[n]);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(x, y, 0.0f);
			glTexCoord2f(tw, 0.0f);
			glVertex3f(x+w, y, 0.0f);
			glTexCoord2f(tw, 1.0f);
			glVertex3f(x+w, y+h, 0.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(x, y+h, 0.0f);
		glEnd();
	}
}

// added by eric at 20120330
void COpenGLControl::OnMouseDownShape(CPoint point)
{
	m_nHitHandle = -1;
	int x, y;
	oglTranslatePoint(point.x, point.y, x, y);
	m_ptDown.x = x;
	m_ptDown.y = y;
	m_ptMoveOffset.x = 0;
	m_ptMoveOffset.y = 0;

	m_bLMouseDown = TRUE;
	m_bTracking = FALSE;

	if (m_tracker)
	{
		m_bPolygonPointHit = FALSE;
		m_nPolygonHitIndex = -1;

		m_nHitHandle = m_tracker->HitTest(point);
		if (m_nHitHandle != -1)
		{
			if (m_nBoundedType == TYPE_NORMAL)
			{
				if (m_nHitHandle == 0 || m_nHitHandle == 2)
					::SetCursor(::LoadCursor(NULL, IDC_SIZENWSE));
				else if (m_nHitHandle == 1 || m_nHitHandle == 3)
					::SetCursor(::LoadCursor(NULL, IDC_SIZENESW));
				else if (m_nHitHandle == 4 || m_nHitHandle == 6)
					::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
				else if (m_nHitHandle == 5 || m_nHitHandle == 7)
					::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
				else if (m_nHitHandle == 8)
					::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
				else
					::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			}
			else if (m_nBoundedType == TYPE_POLYGON)
			{
				if (m_nHitHandle == 8)
				{
					CRect rt;
					rt.left = m_ptDown.x-8;
					rt.top = m_ptDown.y-8;
					rt.right = m_ptDown.x+8;
					rt.bottom = m_ptDown.y+8;

					m_bPolygonPointHit = FALSE;
					int no = m_poly_points[m_nSelectedIndex];
					for (int i=0; i<no; i++)
					{
						if ((m_polygon[m_nSelectedIndex][i].x > rt.left && m_polygon[m_nSelectedIndex][i].x < rt.right) && 
							(m_polygon[m_nSelectedIndex][i].y > rt.top && m_polygon[m_nSelectedIndex][i].y < rt.bottom)) 
						{
							m_bPolygonPointHit = TRUE;
							m_nPolygonHitIndex = i;
							break;
						}
					}

					if (m_bPolygonPointHit)
						::SetCursor(::LoadCursor(NULL, IDC_ARROW));
					else
						::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
				}
				else
					::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			}
			else if (m_nBoundedType == TYPE_REGION || m_nBoundedType == TYPE_INSPREGION)
			{
				if (m_nHitHandle == 8)
					::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
				else
					::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			}
		}
	}
	else
	{
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}

	if (m_nHitHandle >= 0)
	{
		int nSelectedShape = -1;
		// added by eric at 20120730
		// m_nHandleSize = 4
		int x=0, y=0;
		if (m_nHitHandle == 0) {
			x=y=4;
		} else if (m_nHitHandle == 1) {
			x=-4; y=4;
		} else if (m_nHitHandle == 2) {
			x=y=-4;
		} else if (m_nHitHandle == 3) {
			x=4; y=-4;
		} else if (m_nHitHandle == 4) {
			y=4;
		} else if (m_nHitHandle == 5) {
			x=-4;
		} else if (m_nHitHandle == 6) {
			y=-4;
		} else if (m_nHitHandle == 7) {
			x=4;
		}
		point.Offset(x, y);		
		
		if (m_bDrawPatternShape == TRUE) {	// modified by eric at 20140613
			nSelectedShape = GetSelectedShapeIndex(point);
		} else {
			if (m_isShowSegment == FALSE) {
				if (m_inspectRegions > 0) {
					nSelectedShape = 0;
					m_nBoundedType = TYPE_INSPREGION;
				}
			} else {
				nSelectedShape = GetSelectedShapeIndex(point);
			}
		}

		if (nSelectedShape >= 0)
		{
			m_nSelectedIndex = nSelectedShape;
			if (m_nBoundedType == TYPE_NORMAL)
			{
				m_rtSelected = m_mapRectInfo[ NULL ].rect[nSelectedShape];

				// Send to AOI
				int position[4];
				position[0] = m_rtSelected.left;
				position[1] = m_rtSelected.top;
				position[2] = m_rtSelected.right;
				position[3] = m_rtSelected.bottom;
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_DOWN_SEL, (LPARAM)position);
			}
			else if (m_nBoundedType == TYPE_POLYGON)
			{
				CRgn *rgn = new CRgn();
				int no = m_poly_points[m_nSelectedIndex];
				POINT *vertexList = new POINT[no];
				for (int i=0; i<no; i++)
				{
					vertexList[i].x = m_polygon[m_nSelectedIndex][i].x;
					vertexList[i].y = m_polygon[m_nSelectedIndex][i].y;
				}
				rgn->CreatePolygonRgn(vertexList, no, ALTERNATE);
				rgn->GetRgnBox(&m_rtSelected);

				delete [] vertexList;		
				rgn->DeleteObject();
				rgn = NULL;

				int position[1];
				position[0] = m_nSelectedIndex;
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_DOWN_SEL, (LPARAM)position);
			}
			else if (m_nBoundedType == TYPE_REGION)
			{
				m_region[m_nSelectedIndex]->GetRgnBox(&m_rtSelected);
				int position[1];
				position[0] = m_nSelectedIndex;
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_DOWN_SEL, (LPARAM)position);
			}
			else if (m_nBoundedType == TYPE_INSPREGION)
			{
				m_inspectRegion[m_nSelectedIndex]->GetRgnBox(&m_rtSelected);
				SetBoundedTracker();
				int position[1];
				position[0] = m_nSelectedIndex;
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_DOWN_SEL, (LPARAM)position);
			}
		}
		else
		{
			if (m_tracker != NULL)
			{
				delete m_tracker;
				m_tracker = NULL;
				m_rtSelected.SetRectEmpty();
			}

			m_nSelectedIndex = -1;
		}
	}
	else
	{
		int nSelectedShape = -1;
		// modified by eric at 20130116
		if (m_showInspectionRgn == TRUE)
		{
			if (m_inspectRegions > 0)
			{
				nSelectedShape = GetSelectedShapeIndex(point);
				m_nBoundedType = TYPE_INSPREGION;
			}
		}
		else
		{
			nSelectedShape = GetSelectedShapeIndex(point);
		}

		if (nSelectedShape >= 0)
		{
			m_nSelectedIndex = nSelectedShape;
			if (m_nBoundedType == TYPE_NORMAL)
			{
				m_rtSelected = m_mapRectInfo[ NULL ].rect[m_nSelectedIndex];
				// Send to AOI
				int position[4];
				position[0] = m_rtSelected.left;
				position[1] = m_rtSelected.top;
				position[2] = m_rtSelected.right;
				position[3] = m_rtSelected.bottom;
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_DOWN_SEL, (LPARAM)position);
			}
			else if (m_nBoundedType == TYPE_POLYGON)
			{
				CRgn *rgn = new CRgn();
				int no = m_poly_points[m_nSelectedIndex];
				POINT *vertexList = new POINT[no];
				for (int i=0; i<no; i++)
				{
					vertexList[i].x = m_polygon[m_nSelectedIndex][i].x;
					vertexList[i].y = m_polygon[m_nSelectedIndex][i].y;
				}
				rgn->CreatePolygonRgn(vertexList, no, ALTERNATE);
				rgn->GetRgnBox(&m_rtSelected);

				delete [] vertexList;		
				rgn->DeleteObject();
				delete rgn;		//seanchen 20140724-09  find memory leak
				rgn = NULL;

				int position[1];
				position[0] = m_nSelectedIndex;
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_DOWN_SEL, (LPARAM)position);
			}
			else if (m_nBoundedType == TYPE_REGION)
			{
				m_region[m_nSelectedIndex]->GetRgnBox(&m_rtSelected);
				int position[1];
				position[0] = m_nSelectedIndex;
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_DOWN_SEL, (LPARAM)position);
			}
			else if (m_nBoundedType == TYPE_INSPREGION)
			{
				m_inspectRegion[m_nSelectedIndex]->GetRgnBox(&m_rtSelected);
				SetBoundedTracker();
				int position[1];
				position[0] = m_nSelectedIndex;
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_DOWN_SEL, (LPARAM)position);
			}
			// added by eric at 20120521
			InvalidateRect(NULL, FALSE);
			SetBoundedTracker();
		}
		else
		{
			if (m_tracker != NULL)
			{
				delete m_tracker;
				m_tracker = NULL;
				m_rtSelected.SetRectEmpty();
			}

			m_nSelectedIndex = -1;
			// modified by eric at 20130116
			int position[1];
			position[0] = m_nSelectedIndex;

			hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_DOWN_SEL, (LPARAM)position);	// added by eric at 20120502
		}
	}

	oglDrawScene();
}
// added by eric at 20120330
void COpenGLControl::OnMouseMoveShape(CPoint point)
{
	// set cursor
	if (m_tracker != NULL)
	{
		int nHitHandle =  -1;

		int x, y;
		oglTranslatePoint(point.x, point.y, x, y);

		CPoint ptMouse;
		ptMouse.x = x;
		ptMouse.y = y;

		if (m_bLMouseDown && (m_nHitHandle >= 0 &&  m_nHitHandle <= 8)) 
		{
			m_bTracking = TRUE;
			nHitHandle = m_nHitHandle;

			CPoint ptOffset;
			ptOffset.x = x - m_ptDown.x;
			ptOffset.y = y - m_ptDown.y;
			// added by eric at 20120730
			if (ptOffset.x == 0 && ptOffset.y == 0)
				return;

			m_ptDown.x = x;
			m_ptDown.y = y;

			if (m_nBoundedType == TYPE_NORMAL)
			{
				if (m_isPatternMode == TRUE) {	// modified by eric at 20121008
					if (m_nHitHandle == 8) {
						OnMoveResizing(m_nHitHandle, ptMouse, ptOffset);
					}
				} else {
					OnMoveResizing(m_nHitHandle, ptMouse, ptOffset);	// modified by eric at 20120521
					// added by eric at 20130116
					m_ptMoveOffset.x += ptOffset.x;
					m_ptMoveOffset.y += ptOffset.y;
				}
			}
			else if (m_nBoundedType == TYPE_POLYGON)
			{
				if (m_nHitHandle == 8)
				{
					OnPolygonResizing(ptMouse, ptOffset);
					// added by eric at 20130116
					m_ptMoveOffset.x += ptOffset.x;
					m_ptMoveOffset.y += ptOffset.y;
				}
			}
			else if (m_nBoundedType == TYPE_REGION)
			{
				if (m_nHitHandle == 8)
				{
					if (m_bHideSelectedRegion == FALSE)
					{
						m_bHideSelectedRegion = TRUE;
						int position[1];
						position[0] = m_nSelectedIndex;
						hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_HIDE_REGION, (LPARAM)position);
					}

					BOOL bOK = OnRegionMoving(ptOffset);
					if (bOK)
					{
						m_ptMoveOffset.x += ptOffset.x;
						m_ptMoveOffset.y += ptOffset.y;
					}
				}
			}
			else if (m_nBoundedType == TYPE_INSPREGION)
			{
				if (m_nHitHandle == 8)
				{
					m_showInspectionRgn = FALSE;
					BOOL bOK = OnRegionMoving(ptOffset);
					if (bOK)
					{
						m_ptMoveOffset.x += ptOffset.x;
						m_ptMoveOffset.y += ptOffset.y;
					}
				}
			}
		}
		else 
		{
			nHitHandle =  m_tracker->HitTest(point);
			//TRACE("HitHandle = %d\n", nHitHandle);
		}

		if (nHitHandle == 8) 
		{
			if (m_nBoundedType == TYPE_POLYGON) 
			{
				CRect rt;
				rt.left = ptMouse.x-8;
				rt.top = ptMouse.y-8;
				rt.right = ptMouse.x+8;
				rt.bottom = ptMouse.y+8;

				BOOL bfind = FALSE;
				int no = m_poly_points[m_nSelectedIndex];
				for (int i=0; i<no; i++)
				{
					if ((m_polygon[m_nSelectedIndex][i].x > rt.left && m_polygon[m_nSelectedIndex][i].x < rt.right) && 
						(m_polygon[m_nSelectedIndex][i].y > rt.top && m_polygon[m_nSelectedIndex][i].y < rt.bottom)) 
					{
						bfind = TRUE;

						break;
					}
				}
				if (bfind)
					::SetCursor(::LoadCursor(NULL, IDC_ARROW));
				else
					::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
			}
			else 
			{
				::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
			}
		}
		else
		{
			if (m_nBoundedType == TYPE_POLYGON || m_nBoundedType == TYPE_REGION)
			{
				if (nHitHandle == 8)
					::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
				else
					::SetCursor(::LoadCursor(NULL, IDC_ARROW));	
			}
			else
			{
				if (m_isPatternMode == TRUE) {	// modified by eric at 20121008
					if (nHitHandle == 8)
						::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
					else
						::SetCursor(::LoadCursor(NULL, IDC_ARROW));	
				} else {
					if (nHitHandle == 0 || nHitHandle == 2)
						::SetCursor(::LoadCursor(NULL, IDC_SIZENWSE));
					else if (nHitHandle == 1 || nHitHandle == 3)
						::SetCursor(::LoadCursor(NULL, IDC_SIZENESW));
					else if (nHitHandle == 4 || nHitHandle == 6)
						::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
					else if (nHitHandle == 5 || nHitHandle == 7)
						::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
					else if (nHitHandle == 8)
						::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
					else
						::SetCursor(::LoadCursor(NULL, IDC_ARROW));	
				}
			}
		}
	}
}
// added by eric at 20120330
void COpenGLControl::OnMouseUpShape(CPoint point)
{
	if (m_bLMouseDown)
	{
		if (m_nBoundedType == TYPE_NORMAL)
		{
			int nTemp = 0;
			if (m_rtSelected.left > m_rtSelected.right)
			{
				nTemp = m_rtSelected.left;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].left = m_rtSelected.left = m_rtSelected.right;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].right = m_rtSelected.right = nTemp;
			}
			if (m_rtSelected.top >m_rtSelected.bottom)
			{
				nTemp = m_rtSelected.top;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].top = m_rtSelected.top = m_rtSelected.bottom;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].bottom = m_rtSelected.bottom = nTemp;
			}

			// added by eric at 20120730
			int w = abs(m_rtSelected.right - m_rtSelected.left);
			int h = abs(m_rtSelected.bottom - m_rtSelected.top);
			if (m_mapRectInfo[ NULL ].rect_type[m_nSelectedIndex] == SHAPE_RECT)
			{
				if (w == 0 && h == 0){
					w += 1; h += 1;
				} else if (w == 0){
					w += 1;
				} else if (h == 0){
					h += 1;
				}
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].right = m_rtSelected.right = m_rtSelected.left + w;	// modified by eric at 20120803
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].bottom = m_rtSelected.bottom = m_rtSelected.top + h;	// modified by eric at 20120803

				InvalidateRect(NULL, FALSE);
				SetBoundedTracker();
			}
			else if (m_mapRectInfo[ NULL ].rect_type[m_nSelectedIndex] == SHAPE_CIRCLE)
			{
				int radius;
				if (w > h)
					radius = h/2;
				else
					radius = w/2;

				w = (int)((m_rtSelected.left + m_rtSelected.right) / 2);
				h = (int)((m_rtSelected.top + m_rtSelected.bottom) / 2);

				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].left = m_rtSelected.left = w-radius;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].top = m_rtSelected.top = h-radius;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].right = m_rtSelected.right = w+radius;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].bottom = m_rtSelected.bottom = h+radius;
			}

			if (m_bTracking)
			{
				int position[6];
				position[0] = m_rtSelected.left;
				position[1] = m_rtSelected.top;
				position[2] = m_rtSelected.right;
				position[3] = m_rtSelected.bottom;
				// added by eric at 20130116
				position[4] = m_ptMoveOffset.x;
				position[5] = m_ptMoveOffset.y;
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_UP_SEL, (LPARAM)position);
			}
		}
		else if (m_nBoundedType == TYPE_POLYGON)
		{
			if (m_bTracking)
			{
				int position[3];
				position[0] = m_nSelectedIndex;
				// added by eric at 20130116
				position[1] = m_ptMoveOffset.x;
				position[2] = m_ptMoveOffset.y;
				hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_UP_SEL, (LPARAM)position);
			}
		}
		else if (m_nBoundedType == TYPE_REGION)
		{
			if (m_bTracking)
			{
				if (m_ptMoveOffset.x != 0 || m_ptMoveOffset.y != 0)
				{
					int position[3];
					position[0] = m_nSelectedIndex;
					position[1] = m_ptMoveOffset.x;
					position[2] = m_ptMoveOffset.y;
					hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_UP_SEL, (LPARAM)position);
				}
				m_bHideSelectedRegion = FALSE;
			}
		}
		else if (m_nBoundedType == TYPE_INSPREGION)
		{
			if (m_bTracking)
			{
				if (m_ptMoveOffset.x != 0 || m_ptMoveOffset.y != 0)
				{
					m_inspectRegion[m_nSelectedIndex]->OffsetRgn(m_ptMoveOffset.x, m_ptMoveOffset.y);
					int position[3];
					position[0] = m_nSelectedIndex;
					position[1] = m_ptMoveOffset.x;
					position[2] = m_ptMoveOffset.y;
					hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_UP_SEL, (LPARAM)position);
				}
				m_showInspectionRgn = TRUE;
				oglDrawScene();
			}
		}
		m_bLMouseDown = FALSE;
		m_bTracking = FALSE;
	}
}
// added by eric at 20120330
bool SortPredicateByRgnSize(CRgn *p1, CRgn *p2)
{
	CRect rect1, rect2;
	p1->GetRgnBox(&rect1);
	p2->GetRgnBox(&rect2);

	int w1 = rect1.Width();
	int h1 = rect1.Height();
	int size1 = w1*h1;

	int w2 = rect2.Width();
	int h2 = rect2.Height();
	int size2 = w2*h2;

	if (rect1.Width()*rect1.Height() >= rect2.Width()*rect2.Height())
		return false;
	else
		return true;
}
// added by eric at 20120330, 20130116
int COpenGLControl::GetSelectedShapeIndex(CPoint point, int TexNum)
{
    if ( m_mapRectInfo.find( TexNum ) == m_mapRectInfo.end() ) return EOF;

	int nSelected = -1;
	CPoint ptMouse;
	int x, y;
	oglTranslatePoint(point.x, point.y, x, y);

	ptMouse.x = x;
	ptMouse.y = y;

	m_nBoundedType = TYPE_NONE;
	BOOL bfound = FALSE;

	std::vector<CRgn*> sortRgnList;
	if (m_showInspectionRgn == FALSE) {
		for (int c=0; c<m_mapRectInfo[ TexNum ].rects; c++) 
		{
			CRgn *rgn = NULL;
			if( (m_mapRectInfo[ TexNum ].rect_type[c] == SHAPE_RECT)
				|| (m_mapRectInfo[ TexNum ].rect_type[c] == SHAPE_ANCHOR_RECT) ) //seanchen 20130820
			{
				rgn = new CRgn();
				rgn->CreateRectRgn(m_mapRectInfo[ TexNum ].rect[c].left, m_mapRectInfo[ TexNum ].rect[c].top, m_mapRectInfo[ TexNum ].rect[c].right, m_mapRectInfo[ TexNum ].rect[c].bottom);
			}
			else if (m_mapRectInfo[ TexNum ].rect_type[c] == SHAPE_CIRCLE)
			{
				rgn = new CRgn();
				int w = abs(m_mapRectInfo[ TexNum ].rect[c].left - m_mapRectInfo[ TexNum ].rect[c].right);
				int h = abs(m_mapRectInfo[ TexNum ].rect[c].top - m_mapRectInfo[ TexNum ].rect[c].bottom);
				int radius;
				if (w > h)
					radius = h/2;
				else
					radius = w/2;
				w = (int)((m_mapRectInfo[ TexNum ].rect[c].left + m_mapRectInfo[ TexNum ].rect[c].right) / 2);
				h = (int)((m_mapRectInfo[ TexNum ].rect[c].top + m_mapRectInfo[ TexNum ].rect[c].bottom) / 2);
				rgn->CreateEllipticRgn(w-radius, h-radius, w+radius, h+radius);
			}
			else if (m_mapRectInfo[ TexNum ].rect_type[c] == SHAPE_ELLIPSE)
			{
				rgn = new CRgn();
				int left, right, top, bottom;
				if (m_mapRectInfo[ TexNum ].rect[c].left > m_mapRectInfo[ TexNum ].rect[c].right)
				{
					left = m_mapRectInfo[ TexNum ].rect[c].right;
					right = m_mapRectInfo[ TexNum ].rect[c].left;
				}
				else
				{
					left = m_mapRectInfo[ TexNum ].rect[c].left;
					right = m_mapRectInfo[ TexNum ].rect[c].right;
				}
				if (m_mapRectInfo[ TexNum ].rect[c].top > m_mapRectInfo[ TexNum ].rect[c].bottom)
				{
					top = m_mapRectInfo[ TexNum ].rect[c].bottom;
					bottom = m_mapRectInfo[ TexNum ].rect[c].top;
				}
				else
				{
					top = m_mapRectInfo[ TexNum ].rect[c].top;
					bottom = m_mapRectInfo[ TexNum ].rect[c].bottom;
				}
				rgn->CreateEllipticRgn(left, top, right, bottom);
			}
			if (rgn) {
				CRect rt;
				rgn->GetRgnBox(&rt);
				if (rt.IsRectEmpty() == FALSE) {
					sortRgnList.push_back(rgn);
				}		
			}
		}

		for (int c=0; c<m_polygons; c++)
		{
			CRgn *rgn = new CRgn();
			int no = m_poly_points[c];
			POINT *vertexList = new POINT[no];
			for (int i=0; i<no; i++)
			{
				vertexList[i].x = m_polygon[c][i].x;
				vertexList[i].y = m_polygon[c][i].y;
			}
			rgn->CreatePolygonRgn(vertexList, no, ALTERNATE);
			sortRgnList.push_back(rgn);

			delete [] vertexList;		
		}

		for (int c=0; c<m_regions; c++)
		{
			CRgn *rgn = new CRgn();
			rgn->CreateRectRgn(0, 0, 0, 0);
			rgn->CopyRgn(m_region[c]);
			sortRgnList.push_back(rgn);
		}
	} else {
		for (int c=0; c<m_inspectRegions; c++)
		{
			CRgn *rgn = new CRgn();
			rgn->CreateRectRgn(0, 0, 0, 0);
			rgn->CopyRgn(m_inspectRegion[c]);
			sortRgnList.push_back(rgn);
		}
	}

	std::sort(sortRgnList.begin(), sortRgnList.end(), SortPredicateByRgnSize);

	for (size_t t=0; t<sortRgnList.size(); t++)
	{
		CRect rt, rt1;
		sortRgnList[t]->GetRgnBox(&rt);

		// modified by eric at 20120521, 20120730
		rt1.left = rt.left;
		rt1.top = rt.top;
		rt1.right = rt.right+1;
		rt1.bottom = rt.bottom+1;

		if (rt1.PtInRect(ptMouse))
		{
			if (m_showInspectionRgn == FALSE) {
				for (int c=0; c<m_mapRectInfo[ TexNum ].rects; c++) 
				{
					CRgn *rgn = new CRgn();
					if( (m_mapRectInfo[ TexNum ].rect_type[c] == SHAPE_RECT)
						|| (m_mapRectInfo[ TexNum ].rect_type[c] == SHAPE_ANCHOR_RECT) ) //seanchen 20130820
					{
						rgn->CreateRectRgn(m_mapRectInfo[ TexNum ].rect[c].left, m_mapRectInfo[ TexNum ].rect[c].top, m_mapRectInfo[ TexNum ].rect[c].right, m_mapRectInfo[ TexNum ].rect[c].bottom);
					}
					else if (m_mapRectInfo[ TexNum ].rect_type[c] == SHAPE_CIRCLE)
					{
						int w = abs(m_mapRectInfo[ TexNum ].rect[c].left - m_mapRectInfo[ TexNum ].rect[c].right);
						int h = abs(m_mapRectInfo[ TexNum ].rect[c].top - m_mapRectInfo[ TexNum ].rect[c].bottom);
						int radius;
						if (w > h)
							radius = h/2;
						else
							radius = w/2;
						w = (int)((m_mapRectInfo[ TexNum ].rect[c].left + m_mapRectInfo[ TexNum ].rect[c].right) / 2);
						h = (int)((m_mapRectInfo[ TexNum ].rect[c].top + m_mapRectInfo[ TexNum ].rect[c].bottom) / 2);
						rgn->CreateEllipticRgn(w-radius, h-radius, w+radius, h+radius);
					}
					else if (m_mapRectInfo[ TexNum ].rect_type[c] == SHAPE_ELLIPSE)
					{
						int left, right, top, bottom;
						if (m_mapRectInfo[ TexNum ].rect[c].left > m_mapRectInfo[ TexNum ].rect[c].right)
						{
							left = m_mapRectInfo[ TexNum ].rect[c].right;
							right = m_mapRectInfo[ TexNum ].rect[c].left;
						}
						else
						{
							left = m_mapRectInfo[ TexNum ].rect[c].left;
							right = m_mapRectInfo[ TexNum ].rect[c].right;
						}
						if (m_mapRectInfo[ TexNum ].rect[c].top > m_mapRectInfo[ TexNum ].rect[c].bottom)
						{
							top = m_mapRectInfo[ TexNum ].rect[c].bottom;
							bottom = m_mapRectInfo[ TexNum ].rect[c].top;
						}
						else
						{
							top = m_mapRectInfo[ TexNum ].rect[c].top;
							bottom = m_mapRectInfo[ TexNum ].rect[c].bottom;
						}
						rgn->CreateEllipticRgn(left, top, right, bottom);
					}

					CRect rect1;
					rgn->GetRgnBox(&rect1);

					// modified by eric at 20120417
					if (rt.Width()*rt.Height() == rect1.Width()*rect1.Height() &&
						rt.left == rect1.left && rt.right == rect1.right && rt.top == rect1.top && rt.bottom == rect1.bottom)
					{
						CRect rect2;
						rgn->GetRgnBox(&rect2);
						// modified by eric at 20120521
						rect2.left = rt.left;
						rect2.top = rt.top;
						rect2.right = rt.right+1;
						rect2.bottom = rt.bottom+1;

						if (rect2.PtInRect(ptMouse))
						{

							if(m_mapRectInfo[ TexNum ].rect_type[c] == SHAPE_ANCHOR_RECT)//seanchen 20130820
							{
								m_nBoundedType = TYPE_NONE;
							}
							else
							{	
								nSelected = c;
								bfound = TRUE;
								m_nBoundedType = TYPE_NORMAL;
							}
							
						}
					}

					rgn->DeleteObject();
					delete rgn;

					if (bfound)
						break;
				}

				if (bfound == FALSE)
				{
					for (int c=0; c<m_polygons; c++) 
					{
						CRgn *rgn = new CRgn();
						int no = m_poly_points[c];
						POINT *vertexList = new POINT[no];
						for (int i=0; i<no; i++)
						{
							vertexList[i].x = m_polygon[c][i].x;
							vertexList[i].y = m_polygon[c][i].y;
						}
						rgn->CreatePolygonRgn(vertexList, no, ALTERNATE);

						CRect rect1;
						rgn->GetRgnBox(&rect1);

						// modified by eric at 20120417
						if (rt.Width()*rt.Height() == rect1.Width()*rect1.Height() &&
							rt.left == rect1.left && rt.right == rect1.right && rt.top == rect1.top && rt.bottom == rect1.bottom)
						{
							nSelected = c;
							bfound = TRUE;
							m_nBoundedType = TYPE_POLYGON;
						}

						rgn->DeleteObject();
						delete rgn;
						delete [] vertexList; //seanchen 20140724-09 find memory leak

						if (bfound)
							break;
					}
				}

				if (bfound == FALSE)
				{
					for (int c=0; c<m_regions; c++)
					{
						CRect rect1;
						m_region[c]->GetRgnBox(&rect1);

						// modified by eric at 20120417
						if (rt.Width()*rt.Height() == rect1.Width()*rect1.Height() &&
							rt.left == rect1.left && rt.right == rect1.right && rt.top == rect1.top && rt.bottom == rect1.bottom)
						{
							nSelected = c;
							bfound = TRUE;
							m_nBoundedType = TYPE_REGION;
						}

						if (bfound)
							break;
					}
				}
			} else {
				for (int c=0; c<m_inspectRegions; c++)
				{
					CRect rect1;
					m_inspectRegion[c]->GetRgnBox(&rect1);

					// modified by eric at 20120417
					if (rt.Width()*rt.Height() == rect1.Width()*rect1.Height() &&
						rt.left == rect1.left && rt.right == rect1.right && rt.top == rect1.top && rt.bottom == rect1.bottom)
					{
						nSelected = c;
						bfound = TRUE;
						m_nBoundedType = TYPE_INSPREGION;
					}

					if (bfound)
						break;
				}
			}
		}

		if (bfound)
			break;
	}

	for (unsigned int i=0; i<sortRgnList.size(); i++)
	{
		sortRgnList[i]->DeleteObject();
		delete sortRgnList[i];
	}
	sortRgnList.clear();

	return  nSelected;
}
// added by eric at 20120330
void COpenGLControl::SetBoundedTracker()
{	
	if (m_tracker)
	{
		delete m_tracker;
		m_tracker = NULL;
	}

	// modified by eric at 20120702
	if (m_bEnableShapModify)
	{
		// modified by eric at 20120730
		int left, top, right, bottom;
		if (m_mapRectInfo[ NULL ].rect_type[m_nSelectedIndex] == SHAPE_RECT)
		{
			if (m_rtSelected.left > m_rtSelected.right && m_rtSelected.top > m_rtSelected.bottom)
			{
				oglProjectPoint(left, top, m_rtSelected.right, m_rtSelected.bottom);
				oglProjectPoint(right, bottom, m_rtSelected.left,m_rtSelected.top);
			}
			else if (m_rtSelected.left > m_rtSelected.right)
			{
				oglProjectPoint(left, top, m_rtSelected.right, m_rtSelected.top);
				oglProjectPoint(right, bottom, m_rtSelected.left,m_rtSelected.bottom);
			}
			else if (m_rtSelected.top > m_rtSelected.bottom)
			{
				oglProjectPoint(left, top, m_rtSelected.left, m_rtSelected.bottom);
				oglProjectPoint(right, bottom, m_rtSelected.right,m_rtSelected.top);
			}
			else
			{
				oglProjectPoint(left, top, m_rtSelected.left, m_rtSelected.top);
				oglProjectPoint(right, bottom, m_rtSelected.right,m_rtSelected.bottom);
			}
		}
		else
		{
			oglProjectPoint(left, top, m_rtSelected.left, m_rtSelected.top);
			oglProjectPoint(right, bottom, m_rtSelected.right,m_rtSelected.bottom);
		}

		CRect rtTracker = CRect(left, top, right, bottom);

		// modified by eric at 20130118
		if (m_nBoundedType == TYPE_NORMAL) {
			//m_tracker = new CRectTracker(rtTracker, CRectTracker::solidLine|CRectTracker::hatchedBorder|CRectTracker::resizeOutside);
			m_tracker = new CMyTracker();
			m_tracker->m_rect = rtTracker;
			m_tracker->m_nStyle = CRectTracker::dottedLine|CRectTracker::hatchedBorder|CRectTracker::resizeOutside;

		}
		else if (m_nBoundedType == TYPE_POLYGON) {
			//m_tracker = new CRectTracker(rtTracker, CRectTracker::solidLine|CRectTracker::hatchedBorder);
			m_tracker = new CMyTracker();
			m_tracker->m_rect = rtTracker;
			m_tracker->m_nStyle = CRectTracker::solidLine|CRectTracker::hatchedBorder;
		}
		else if(m_nBoundedType == TYPE_REGION || m_nBoundedType == TYPE_INSPREGION) {
			//m_tracker = new CRectTracker(rtTracker, CRectTracker::solidLine|CRectTracker::hatchedBorder);
			m_tracker = new CMyTracker();
			m_tracker->m_rect = rtTracker;
			m_tracker->m_nStyle = CRectTracker::solidLine|CRectTracker::hatchedBorder;
		}
	}
}
// added by eric at 20120330
void COpenGLControl::oglClearBoundedTracker()
{
	if (m_tracker)
	{
		delete m_tracker;
		m_tracker = NULL;
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}

	m_rtSelected.SetRectEmpty();
	m_nSelectedIndex = -1;
	m_bLMouseDown = FALSE;
	m_nHitHandle = -1;
	m_nBoundedType = TYPE_NONE;
	m_bTracking = FALSE;

	// added by eric at 20130107
	m_vSelectionList.clear();
	m_vRectTrackerList.clear();

	oglDrawScene();
}
// added by eric at 20120330
void COpenGLControl::OnMoveResizing(int nHandle, CPoint ptMouse, CPoint ptOffset)	// modified by eric at 20120521
{
	if (nHandle == 8)	// move position
	{
		// modified by eric at 20120521
		if (m_rtSelected.left + ptOffset.x >= 0 && m_rtSelected.top + ptOffset.y >= 0 &&
			m_rtSelected.right + ptOffset.x < m_width[0] && m_rtSelected.bottom + ptOffset.y < m_height[0])
		{
			m_rtSelected.left += ptOffset.x;
			m_rtSelected.right += ptOffset.x;
			m_rtSelected.top += ptOffset.y;
			m_rtSelected.bottom += ptOffset.y;
			m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].left += ptOffset.x;
			m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].right += ptOffset.x;
			m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].top += ptOffset.y;
			m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].bottom += ptOffset.y;
		}

		// added by eric at 20120521
		int width = m_rtSelected.Width();
		int height = m_rtSelected.Height();
		if (ptMouse.x >= m_width[0]-1)
		{
			m_rtSelected.right = m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].right = m_width[0]-1;
			m_rtSelected.left = m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].left = m_width[0]-1-width;
		}
		else if (ptMouse.x == 0)
		{
			m_rtSelected.left = m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].left = 0;
			m_rtSelected.right = m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].right = width;
		}
		if (ptMouse.y >= m_height[0]-1)
		{
			m_rtSelected.bottom = m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].bottom = m_height[0]-1;
			m_rtSelected.top = m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].top = m_height[0]-1-height;
		}
		else if (ptMouse.y == 0)
		{
			m_rtSelected.top = m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].top = 0;
			m_rtSelected.bottom = m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].bottom = height;
		}

		InvalidateRect(NULL, FALSE);
		SetBoundedTracker();
	}
	else
	{
		if (nHandle == 0)	// NorthWest
		{
			// modified by eric at 20120521
			if (m_ptDown.x == 0 && ptOffset.x == 0)
			{
				m_rtSelected.left = 0;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].left = 0;
			}
			else
			{
				m_rtSelected.left += ptOffset.x;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].left += ptOffset.x;
			}

			if (m_ptDown.y == 0 && ptOffset.y == 0)
			{
				m_rtSelected.top = 0;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].top = 0;
			}
			else
			{
				m_rtSelected.top += ptOffset.y;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].top += ptOffset.y;
			}
		}
		else if (nHandle == 1)	// NorthEast
		{
			// modified by eric at 20120521
			if (m_ptDown.x == m_width[0]-1 && ptOffset.x == 0)
			{
				m_rtSelected.right = m_width[0]-1;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].right = m_width[0]-1;
			}
			else
			{
				m_rtSelected.right += ptOffset.x;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].right += ptOffset.x;
			}

			if (m_ptDown.y == 0 && ptOffset.y == 0)
			{
				m_rtSelected.top = 0;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].top = 0;
			}
			else
			{
				m_rtSelected.top += ptOffset.y;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].top += ptOffset.y;
			}
		}
		else if (nHandle == 2) // SouthEast
		{
			// modified by eric at 20120521
			if (m_ptDown.x == m_width[0]-1 && ptOffset.x == 0)
			{
				m_rtSelected.right = m_width[0]-1;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].right = m_width[0]-1;
			}
			else
			{
				m_rtSelected.right += ptOffset.x;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].right += ptOffset.x;
			}

			if (m_ptDown.y == m_height[0]-1 && ptOffset.y == 0)
			{
				m_rtSelected.bottom = m_height[0]-1;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].bottom = m_height[0]-1;
			}
			else
			{
				m_rtSelected.bottom += ptOffset.y;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].bottom += ptOffset.y;
			}
		}
		else if (nHandle == 3)	// SouthWest
		{
			// modified by eric at 20120521
			if (m_ptDown.x == 0 && ptOffset.x == 0)
			{
				m_rtSelected.left = 0;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].left = 0;
			}
			else
			{
				m_rtSelected.left += ptOffset.x;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].left += ptOffset.x;
			}

			if (m_ptDown.y == m_height[0]-1 && ptOffset.y == 0)
			{
				m_rtSelected.bottom = m_height[0]-1;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].bottom = m_height[0]-1;
			}
			else
			{
				m_rtSelected.bottom += ptOffset.y;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].bottom += ptOffset.y;
			}
		}
		else if (nHandle == 4)	// North
		{
			// modified by eric at 20120521
			if (m_ptDown.y == 0 && ptOffset.y == 0)
			{
				m_rtSelected.top = 0;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].top = 0;
			}
			else
			{
				m_rtSelected.top += ptOffset.y;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].top += ptOffset.y;
			}
		}
		else if (nHandle == 5)	// East
		{
			// modified by eric at 20120521
			if (m_ptDown.x == m_width[0]-1 && ptOffset.x == 0)
			{
				m_rtSelected.right = m_width[0]-1;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].right = m_width[0]-1;
			}
			else
			{
				m_rtSelected.right += ptOffset.x;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].right += ptOffset.x;
			}
		}
		else if (nHandle == 6)	// South
		{
			// modified by eric at 20120521
			if (m_ptDown.y == m_height[0]-1 && ptOffset.y == 0)
			{
				m_rtSelected.bottom = m_height[0]-1;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].bottom = m_height[0]-1;
			}
			else
			{
				m_rtSelected.bottom += ptOffset.y;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].bottom += ptOffset.y;
			}
		}
		else if (nHandle == 7)	// West
		{
			// modified by eric at 20120521
			if (m_ptDown.x == 0 && ptOffset.x == 0)
			{
				m_rtSelected.left = 0;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].left = 0;
			}
			else
			{
				m_rtSelected.left += ptOffset.x;
				m_mapRectInfo[ NULL ].rect[m_nSelectedIndex].left += ptOffset.x;
			}
		}

		InvalidateRect(NULL, FALSE);
		SetBoundedTracker();
	}
}
// added by eric at 20120330
void COpenGLControl::OnPolygonResizing(CPoint ptMouse, CPoint ptOffset)
{
	if (m_bPolygonPointHit)
	{
		m_polygon[m_nSelectedIndex][m_nPolygonHitIndex].x += ptOffset.x;
		m_polygon[m_nSelectedIndex][m_nPolygonHitIndex].y += ptOffset.y;

		CRgn *rgn = new CRgn();
		int no = m_poly_points[m_nSelectedIndex];
		POINT *vertexList = new POINT[no];
		for (int i=0; i<no; i++)
		{
			vertexList[i].x = m_polygon[m_nSelectedIndex][i].x;
			vertexList[i].y = m_polygon[m_nSelectedIndex][i].y;
		}

		rgn->CreatePolygonRgn(vertexList, no, ALTERNATE);
		rgn->GetRgnBox(&m_rtSelected);

		rgn->DeleteObject();
		delete rgn;
		delete [] vertexList;

		InvalidateRect(NULL, FALSE);
		SetBoundedTracker();
	}
	else
	{
		if (m_rtSelected.left + ptOffset.x > 0 && m_rtSelected.top + ptOffset.y > 0 &&
			m_rtSelected.right + ptOffset.x < m_width[0] && m_rtSelected.bottom + ptOffset.y < m_height[0])
		{
			m_rtSelected.left += ptOffset.x;
			m_rtSelected.right += ptOffset.x;
			m_rtSelected.top += ptOffset.y;
			m_rtSelected.bottom += ptOffset.y;

			int no = m_poly_points[m_nSelectedIndex];
			for (int i=0; i<no; i++)
			{
				m_polygon[m_nSelectedIndex][i].x += ptOffset.x;
				m_polygon[m_nSelectedIndex][i].y += ptOffset.y;
			}
		}

		InvalidateRect(NULL, FALSE);
		SetBoundedTracker();
	}
}
// added by eric at 20120330
BOOL COpenGLControl::OnRegionMoving(CPoint ptOffset)
{
	BOOL bOK = FALSE;
	if (m_rtSelected.left + ptOffset.x > 0 && m_rtSelected.top + ptOffset.y > 0 &&
		m_rtSelected.right + ptOffset.x < m_width[0] && m_rtSelected.bottom + ptOffset.y < m_height[0])
	{
		m_rtSelected.left += ptOffset.x;
		m_rtSelected.right += ptOffset.x;
		m_rtSelected.top += ptOffset.y;
		m_rtSelected.bottom += ptOffset.y;

		InvalidateRect(NULL, FALSE);
		SetBoundedTracker();

		bOK = TRUE;
	}

	return bOK;
}
// added by eric at 20120330
void COpenGLControl::oglSetRegionSize(int index, CRgn* rgn)
{
	if (index >= MAX_RECTS)
		return;

	m_regions++;
	CRgn *region = new CRgn();
	region->CreateRectRgn(0, 0, 0, 0);
	region->CopyRgn(rgn);
	m_region.push_back(region);
}
// added by eric at 20120330
void COpenGLControl::oglGetRegionSize(int index, CRgn* rgn)
{
	if (index <= m_regions)
		rgn->CopyRgn(m_region[index]);
}
// added by eric at 20130111
void COpenGLControl::oglAddInspectionRegion(std::vector<CRgn*> vList) 
{
	for (int i=0; i<(int)m_inspectRegion.size(); i++) {
		m_inspectRegion[i]->DeleteObject();
		delete [] m_inspectRegion[i];
		m_inspectRegion[i] = NULL;
	}
	m_inspectRegion.clear();

	if (m_tracker != NULL) {
		delete m_tracker;
		m_tracker = NULL;
		m_rtSelected.SetRectEmpty();
	}

	m_inspectRegion.assign(vList.begin(), vList.end());
	m_inspectRegions = (int)m_inspectRegion.size();
}
// added by eric at 20120330
void COpenGLControl::oglSetInspectionRegionCount(int count) 
{
	for (unsigned int i=0; i<m_inspectRegion.size(); i++)
	{
		m_inspectRegion[i]->DeleteObject();
		delete m_inspectRegion[i];
		m_inspectRegion[i] = NULL;
	}
	m_inspectRegion.clear();

	if (m_tracker != NULL)
	{
		delete m_tracker;
		m_tracker = NULL;
		m_rtSelected.SetRectEmpty();
	}

	m_inspectRegions = count;
}
// added by eric at 20120330, 20130111
void COpenGLControl::oglSetInspectionSelection(int index)
{
	if (m_tracker) {
		delete m_tracker;
		m_tracker = NULL;
	}

	if (index != -1) {
		if (index < m_inspectRegions) {
			m_inspectRegion[index]->GetRgnBox(m_rtSelected);
			SetBoundedTracker();
		}
	}
}

// Draw a ball of specified color with a digit on it. -- Beagle 20120425
// Beagle 20120504 changed to using FTGL font.
void COpenGLControl::oglDrawBilliardBall(COLORREF color, int number)
{
	GLfloat r, g, b;
	r = GetRValue(color)/255.0f;
	g = GetGValue(color)/255.0f;
	b = GetBValue(color)/255.0f;

	glLineWidth(1.2f);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_LOGIC_OP);

	// Inner color circle
	glColor3f(r, g, b);
	glBegin(GL_QUADS);
//	DrawCircle(1.0f);
		glVertex3f(-1.0f, -1.0f, 0.0f);
		glVertex3f( 1.0f, -1.0f, 0.0f);
		glVertex3f( 1.0f,  1.0f, 0.0f);
		glVertex3f(-1.0f,  1.0f, 0.0f);
	glEnd();

	// Outline
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINE_LOOP);
//	DrawCircle(1.0f);
		glVertex3f(-1.0f, -1.0f, 0.0f);
		glVertex3f( 1.0f, -1.0f, 0.0f);
		glVertex3f( 1.0f,  1.0f, 0.0f);
		glVertex3f(-1.0f,  1.0f, 0.0f);
	glEnd();
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);

	// Specular reflection
	glBegin(GL_TRIANGLE_FAN);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.0f,  0.0f, 0.0f);
		glColor3f(r, g, b);
		glVertex3f( 0.0f, -0.90f, 0.0f);
		glVertex3f( 0.7f, -0.45f, 0.0f);
		glVertex3f( 0.7f,  0.45f, 0.0f);
		glVertex3f( 0.0f,  0.90f, 0.0f);
		glVertex3f(-0.7f,  0.45f, 0.0f);
		glVertex3f(-0.7f, -0.45f, 0.0f);
		glVertex3f( 0.0f, -0.90f, 0.0f);
	glEnd();

	if (number > 0) {
		FTBBox box;
		GLfloat xoffset, yoffset;
		wchar_t str[32];

		glPushMatrix();
		swprintf_s(str, L"%d", number);
		box = m_font->BBox(str, 1);
		xoffset = (box.Upper().Xf() + box.Lower().Xf())/2.0f;
		yoffset = (box.Upper().Yf() + box.Lower().Yf())/2.0f;

		glScalef(0.066667f, -0.066667f, 1.0f);

		glTranslatef(-xoffset, -yoffset, 0);
		glColor3f(0.0f, 0.0f, 0.0f);
		m_font->Render(str, (int) wcslen(str), FTPoint(0, 0, 0));
		glPopMatrix();
	}
}

// Draw 32 vertices to form a circular shape. -- Beagle 20130807 modified.
void COpenGLControl::DrawCircle(float radius)
{
	float r = radius;

	glVertex3f( 1.000000f*r,  0.000000f*r, 0.0f);
	glVertex3f( 0.980785f*r,  0.195090f*r, 0.0f);
	glVertex3f( 0.923880f*r,  0.382683f*r, 0.0f);
	glVertex3f( 0.831470f*r,  0.555570f*r, 0.0f);
	glVertex3f( 0.707107f*r,  0.707107f*r, 0.0f);
	glVertex3f( 0.555570f*r,  0.831470f*r, 0.0f);
	glVertex3f( 0.382683f*r,  0.923880f*r, 0.0f);
	glVertex3f( 0.195090f*r,  0.980785f*r, 0.0f);
	glVertex3f(-0.000000f*r,  1.000000f*r, 0.0f);
	glVertex3f(-0.195090f*r,  0.980785f*r, 0.0f);
	glVertex3f(-0.382683f*r,  0.923880f*r, 0.0f);
	glVertex3f(-0.555570f*r,  0.831470f*r, 0.0f);
	glVertex3f(-0.707107f*r,  0.707107f*r, 0.0f);
	glVertex3f(-0.831470f*r,  0.555570f*r, 0.0f);
	glVertex3f(-0.923880f*r,  0.382683f*r, 0.0f);
	glVertex3f(-0.980785f*r,  0.195090f*r, 0.0f);
	glVertex3f(-1.000000f*r, -0.000000f*r, 0.0f);
	glVertex3f(-0.980785f*r, -0.195090f*r, 0.0f);
	glVertex3f(-0.923880f*r, -0.382683f*r, 0.0f);
	glVertex3f(-0.831470f*r, -0.555570f*r, 0.0f);
	glVertex3f(-0.707107f*r, -0.707107f*r, 0.0f);
	glVertex3f(-0.555570f*r, -0.831469f*r, 0.0f);
	glVertex3f(-0.382684f*r, -0.923879f*r, 0.0f);
	glVertex3f(-0.195090f*r, -0.980785f*r, 0.0f);
	glVertex3f( 0.000000f*r, -1.000000f*r, 0.0f);
	glVertex3f( 0.195090f*r, -0.980785f*r, 0.0f);
	glVertex3f( 0.382684f*r, -0.923879f*r, 0.0f);
	glVertex3f( 0.555570f*r, -0.831470f*r, 0.0f);
	glVertex3f( 0.707107f*r, -0.707107f*r, 0.0f);
	glVertex3f( 0.831470f*r, -0.555570f*r, 0.0f);
	glVertex3f( 0.923880f*r, -0.382683f*r, 0.0f);
	glVertex3f( 0.980785f*r, -0.195090f*r, 0.0f);
}

void COpenGLControl::DrawMaxtrixLine()
{
    const std::vector< CString >& vecFlipMode = ( FLIP_NONE     == m_eMatrixFlipMode ) ? m_vecMatrixNormalSequence   :
                                                ( FLIP_HORIZON  == m_eMatrixFlipMode ) ? m_vecMatrixHorizonSequence  :
                                                ( FLIP_VERTICAL == m_eMatrixFlipMode ) ? m_vecMatrixVerticalSequence : m_vecMatrixH_and_VSequence ;
    FTBBox  box;
    int     iIndex    = NULL;
	int     nSplit    = 3;
    int     nHLine    = (m_rcMatrix.bottom - m_rcMatrix.top) / nSplit;
	int     nWLine    = (m_rcMatrix.right - m_rcMatrix.left) / nSplit;
    int     iFontLocX = nWLine / 2;
    int     iFontLocY = nHLine / 2;

	glColor3f(1.0f,0.0f,0.0f);
	glBegin(GL_LINES);

	for ( int i = NULL; i <= nSplit; i++ )
    {
		glVertex3f( ( GLfloat )m_rcMatrix.left,                  ( GLfloat )( m_rcMatrix.top + nHLine * i ), 0.0f );
		glVertex3f( ( GLfloat )m_rcMatrix.right,                 ( GLfloat )( m_rcMatrix.top + nHLine * i ), 0.0f );
		glVertex3f( ( GLfloat )( m_rcMatrix.left + nWLine * i ), ( GLfloat )m_rcMatrix.top,                  0.0f );
		glVertex3f( ( GLfloat )( m_rcMatrix.left + nWLine * i ), ( GLfloat )m_rcMatrix.bottom,               0.0f );
	}
	glEnd();

    m_font->FaceSize( 72 );

    float billiard_size = ( 1.0f / m_glScale / m_norm_scale[ 0 ] ) / 2;

    if ( billiard_size < 1.0f ) billiard_size = 1.0f;

	glColor4f( 0.0f, 0.0f, 0.0f, 0.25f );

    for ( int i = NULL; i < nSplit; i++ )
    {
        for ( int j = NULL; j < nSplit; j++ )
        {
            glPushMatrix();

            iIndex = i * nSplit + j;
            box    = m_font->BBox( vecFlipMode[ iIndex ] );

            glScalef( 1.0, -1.0, 1.0f );

		    glTranslatef( ( m_rcMatrix.left + iFontLocX + nWLine * j ) - ( box.Upper().Xf() - box.Lower().Xf() ) / 2,
                            -( box.Upper().Yf() - box.Lower().Yf() ) / 2 * billiard_size - ( m_rcMatrix.top + iFontLocY + nHLine * i ),
                            0.0f );
            glScalef( billiard_size, billiard_size, 1.0f );

            m_font->Render( vecFlipMode[ iIndex ] );

            glPopMatrix();
        }
    }
    m_font->FaceSize( 18 );
}

// Beagle 20120710 modified.
// style == GRID_STYLE_RECTANGLE : line loop rectangle
// style == GRID_STYLE_BILLIARD  : Square billiard ball
void COpenGLControl::oglSetFrameGrid(int gridX, int gridY, int style)
{
	if (gridX == 0 || gridY == 0) {
		// No any frame. Disable frame display.
		m_FrameGrid[0] = m_FrameGrid[1] = m_FrameNumber = 0;
		return;
	}

	m_FrameGrid[0] = gridX;
	m_FrameGrid[1] = gridY;
	m_FrameNumber = gridX*gridY;
	if (style != GRID_STYLE_NOCHANGE) {
		m_FrameGridStyle = style;
	}
}

// Beagle 20120704 added.
void COpenGLControl::oglSetFrameColor(int num, BOOL show, COLORREF color)
{
	if (num < 0 || num >= m_FrameNumber) { return; }

	m_FrameColor[num][0] = (show ? 1.0f : 0.0f);
	m_FrameColor[num][1] = GetRValue(color)/255.0f;
	m_FrameColor[num][2] = GetGValue(color)/255.0f;
	m_FrameColor[num][3] = GetBValue(color)/255.0f;
}
void COpenGLControl::oglShowFrameColor(bool bShow)
{
	for (int i=0; i<m_FrameNumber; i++) {
		m_FrameColor[i][0] = (bShow ? 1.0f : 0.0f);
	}
}
// added by eric at 20120713
void COpenGLControl::oglClearFrameColor()
{
	for (int i=0; i<m_FrameNumber; i++) {
		m_FrameColor[i][0] = 0.0f;
	}
}
// Beagle 20120710 modified.
void COpenGLControl::DrawFrame(int split_id)
{
	if (m_FrameNumber == 0) { return; }

	int w = m_width[split_id] / m_FrameGrid[0];
	int h = m_height[split_id] / m_FrameGrid[1];

	switch (m_FrameGridStyle) {
	case GRID_STYLE_RECTANGLE:
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_COLOR_LOGIC_OP);
		glDisable(GL_BLEND);
		glLineWidth(3.0f);
		for (int c=0; c<m_FrameNumber; c++) {
			if (m_FrameColor[c][0] < 1.0f) { continue; }

			int xg = c % m_FrameGrid[0];
			int yg = c / m_FrameGrid[0];
			float x1 = (float) (xg*w+10.0f);
			float x2 = (float) ((xg+1)*w-10.0f);
			float y1 = (float) (yg*h+10.0f);
			float y2 = (float) ((yg+1)*h-10.0f);

			glColor3f(m_FrameColor[c][1], m_FrameColor[c][2], m_FrameColor[c][3]);
			glBegin(GL_LINE_LOOP);
				glVertex3f(x1, y1, 0.0f);
				glVertex3f(x2, y1, 0.0f);
				glVertex3f(x2, y2, 0.0f);
				glVertex3f(x1, y2, 0.0f);
			glEnd();
		}
		break;
	case GRID_STYLE_BILLIARD:
	default:
		for (int c=0; c<m_FrameNumber; c++) {
			if (m_FrameColor[c][0] < 1.0f) { continue; }

			float billiard_size = 15.0f/m_glScale/m_norm_scale[0];
			int xg = c % m_FrameGrid[0];
			int yg = c / m_FrameGrid[0];
			float x1 = (float) (xg*w+billiard_size);
			float y1 = (float) (yg*h+billiard_size);

			glPushMatrix();
			glTranslatef(x1, y1, 0.0f);
			glScalef(billiard_size, billiard_size, 1.0f);
			oglDrawBilliardBall(RGB((int)floorf(m_FrameColor[c][1]*255),
				(int)floorf(m_FrameColor[c][2]*255),
				(int)floorf(m_FrameColor[c][3]*255)), c+1);
			glPopMatrix();
		}
		break;
	}
}

// Beagle 20120705 added.
void COpenGLControl::oglClearText(void)
{
	// modified by eric at 20150203
	for (int i=0; i<m_num_textstring; i++) {
		m_textstring[i].align=0;
		m_textstring[i].x=0;
		m_textstring[i].y=0;
		swprintf_s(m_textstring[i].str, 128, L"");
	}
	m_num_textstring = 0;
	AddIndexText();
}

// Beagle 20120710 modified.
void COpenGLControl::AddIndexText(void)
{
	if (m_showFrameIndex == FALSE) { return; }	// Beagle 20120716 added.

	float w = ((float) m_width[0])/m_FrameGrid[0];
	float h = ((float) m_height[0])/m_FrameGrid[1];

	for (int x=0; x<m_FrameGrid[0]; x++) for (int y=0; y<m_FrameGrid[1]; y++) {
		oglPrintf(ALIGN_SHEET|ALIGN_HCENTER|ALIGN_TOP,
			(x+0.5f)*w-m_width[0]/2.0f, y*h, L"%d-%d", x+1, y+1);
	}
}

// Beagle 20120817 added.
// orientation == TRUE:  horizontal line.
// orientation == FALSE: vertical line.
// start value can be negative.
// period <= 0: Don't display.
void COpenGLControl::oglSetCutHereLine(BOOL orientation, int start, int period)
{
	m_cutHereLine.orientation = orientation;
	m_cutHereLine.start = start;
	if (period > 0) {
		m_cutHereLine.period = period;
	} else {
		m_cutHereLine.period = 0;
	}
}

// Beagle 20120817 added.
void COpenGLControl::oglDrawCutHereLine(int split_id)
{
	if (m_cutHereLine.period <= 0) { return; }

	glEnable(GL_LINE_STIPPLE);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glLineWidth (1.5);
	glLineStipple(2, 0x00FF);
	if (m_cutHereLine.orientation == TRUE) {
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_LINES);
		for (float y=m_cutHereLine.start*1.0f; y<m_height[split_id]; y+=m_cutHereLine.period) {
			if (y < 0) { continue; }
			glVertex3f(0.0f, y, 0.0f);
			glVertex3f(m_width[split_id]*1.0f, y, 0.0f);
		}
		glEnd();
	} else {	// m_cutHereLine.orientation == FALSE
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_LINES);
		for (float x=m_cutHereLine.start*1.0f; x<m_width[split_id]; x+=m_cutHereLine.period) {
			if (x < 0) { continue; }
			glVertex3f(x, 0.0f, 0.0f);
			glVertex3f(x, m_height[split_id]*1.0f, 0.0f);
		}
		glEnd();
	}
	glDisable(GL_LINE_STIPPLE);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);
}

// orientation == TRUE:  horizontal line.
// orientation == FALSE: vertical line.
void COpenGLControl::oglSetUnitLine(BOOL orientation,std::vector<int> vUnit) //eric chao 20160527
{
	m_xUnitDivide.orientation = orientation;
	m_xUnitDivide.vUnitline = vUnit;
}

void COpenGLControl::oglDrawUnitLine(int split_id) //eric chao 20160527
{
	int nSize = m_xUnitDivide.vUnitline.size();
	if (nSize == 0) { return; }

	glEnable(GL_LINE_STIPPLE);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glLineWidth (1.5);
	glLineStipple(2, 0x00FF);
	if (m_xUnitDivide.orientation == TRUE) {
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_LINES);
		for (int i=0;i<nSize;i++){
			float f_line = 0;
			f_line = m_xUnitDivide.vUnitline[i];
			if (f_line < m_height[split_id]){ //y
				glVertex3f(0.0f, f_line, 0.0f);
				glVertex3f(m_width[split_id]*1.0f, f_line, 0.0f);
			}
		}
		glEnd();
	} else {	// m_xUnitDivide.orientation == FALSE
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_LINES);
		for (int i=0;i<nSize;i++){
			float f_line = 0;
			f_line = m_xUnitDivide.vUnitline[i];
			if (f_line < m_width[split_id]){ //x
				glVertex3f(f_line, 0.0f, 0.0f);
				glVertex3f(f_line, m_height[split_id]*1.0f, 0.0f);
			}
		}
		glEnd();
	}
	glDisable(GL_LINE_STIPPLE);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);
}
// Beagle 20120921 added.
void COpenGLControl::oglGetWindowSize(int &w, int &h) {
	w = m_glwidth;
	h = m_glheight;
}

// Beagle 20120921 added.
void COpenGLControl::oglGetSplitWinSize(int &w, int &h) {
	w = m_glwidth / m_wsplit;
	h = m_glheight / m_hsplit;
}
// added by eric at 20121008
void COpenGLControl::oglClearPatternShape()
{
	for (int i=0; i<m_rectsPattern; i++) {
		m_rectPattern[i].left = m_rectPattern[i].top = m_rectPattern[i].right = m_rectPattern[i].bottom = 0;
	}
	m_rectsPattern = 0;
}
// added by eric at 20121008
void COpenGLControl::oglSetPatternRectShape(int index, int left, int top, int right, int bottom)
{
	// added by amike at 20121129
	if(index >= MAX_RECTS)
		return;

	if (index >= m_rectsPattern) { 
		m_rectsPattern = index+1; 
	};
	m_rectPattern[index].left = left;
	m_rectPattern[index].top = top;
	m_rectPattern[index].right = right;
	m_rectPattern[index].bottom = bottom;
}

// Center (x,y) is a client window coordinate. -- Beagle 20130103 added.
void COpenGLControl::oglScaleChange(float rate, float center_x, float center_y)
{
	GLfloat xoffset=center_x-m_glwidth/2.0f;
	GLfloat yoffset=center_y-m_glheight/2.0f;

	if (m_wsplit > 1 || m_hsplit > 1){ //eric chao 20160825 fix Split Window Scale Change.... Need Check~
		int nUnitW = m_glwidth / m_wsplit;
		int nUnitH = m_glheight / m_hsplit;
		while (center_x > nUnitW){
			center_x -= nUnitW;
		}
		while (center_y > nUnitH){
			center_y -= nUnitH;
		}
		xoffset = center_x - nUnitW/2.0f;
		yoffset = center_y - nUnitH/2.0f;
	}
    m_glScale *= rate;

    if ( m_OGLRolling[ NULL ].IsRollingMode() && m_glScale < 1.0f )
    {
        m_xTranslate = 0.0f;
        m_glScale    = 1.0f;
    }
    else
    {
	    m_xTranslate += xoffset * (1.0f/rate-1.0f) / m_glScale;
	    m_yTranslate += yoffset * (1.0f/rate-1.0f) / m_glScale;
    }
	//oglClearBoundedTracker();
}

// Beagle 20130103 added.
void COpenGLControl::oglScaleSet(float scale)
{
	if (scale > 100.0f) { m_glScale = 100.0f; }
	else if (scale < 0.01f) { m_glScale = 0.01f; }
	else { m_glScale = scale; }

    if ( m_OGLRolling[ NULL ].IsRollingMode() && m_glScale < 1.0f ) m_glScale = 1.0f;

	// Return to center.
	m_xTranslate=0.0f;
	m_yTranslate=0.0f;

	oglClearBoundedTracker();
	OnUpdateShadowLine();	// added by eric at 20130219
}

// (pos_x, pos_y) is a texture coordinate of the new center.
// Beagle 20140605.
void COpenGLControl::oglSetPosition(float pos_x, float pos_y, int TexNum)
{
	m_xTranslate=(m_width[TexNum]/2-pos_x)*m_norm_scale[TexNum];
	m_yTranslate=(m_height[TexNum]/2-pos_y)*m_norm_scale[TexNum];
}

// Beagle 20130610 support custom color.
void COpenGLControl::DrawCrosshair(int s)
{
	float scale;

	if (!m_showCrossHair) return;
	if (s < 0 || s >= m_hsplit*m_wsplit) return;

	scale = 15.0f/m_glScale/m_norm_scale[s];

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_LOGIC_OP);
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	// Draw circles and crosshairs.
	for (int c=0; c<m_crosshair[s].num; c++) {
		GLfloat r, g, b;
		r = GetRValue(m_crosshair[s].p[c].color)/255.0f;
		g = GetGValue(m_crosshair[s].p[c].color)/255.0f;
		b = GetBValue(m_crosshair[s].p[c].color)/255.0f;

		glPushMatrix();
		glTranslatef(m_crosshair[s].p[c].x, m_crosshair[s].p[c].y, 0.0f);
		glScalef(scale, scale, 1.0f);

		// Outline
		glLineWidth(3.0f);
		if (r>0.3f || g>0.3f || b>0.6f) {
			glColor3f(0.0f, 0.0f, 0.0f);	// Light color with dark outline.
		} else {
			glColor3f(1.0f, 1.0f, 1.0f);	// Dark color with bright outline.
		}
		glBegin(GL_LINES);
			glVertex3f(-1.0f,  0.0f, 0.0f);
			glVertex3f( 1.0f,  0.0f, 0.0f);
			glVertex3f( 0.0f, -1.0f, 0.0f);
			glVertex3f( 0.0f,  1.0f, 0.0f);
		glEnd();
		glBegin(GL_LINE_LOOP);
		DrawCircle(1.0f);
		glEnd();

		glLineWidth(1.0f);
		glColor3f(r, g, b);
		glBegin(GL_LINES);
			glVertex3f(-1.0f,  0.0f, 0.0f);
			glVertex3f( 1.0f,  0.0f, 0.0f);
			glVertex3f( 0.0f, -1.0f, 0.0f);
			glVertex3f( 0.0f,  1.0f, 0.0f);
		glEnd();
		glBegin(GL_LINE_LOOP);
		DrawCircle(1.0f);
		glEnd();
		glPopMatrix();
	}
	// Draw measurement gauges for paired points.
	for (int c=0; c<m_crosshair[s].pair_num; c++) {
		float x1, x2, rx, hx, cx, y1, y2, ry, cy, di, dx, dy;
		float fXDis , fYDis, fLDis;
		x1 = m_crosshair[s].p[m_crosshair[s].pair[c].p1].x;
		y1 = m_crosshair[s].p[m_crosshair[s].pair[c].p1].y;
		x2 = m_crosshair[s].p[m_crosshair[s].pair[c].p2].x;
		y2 = m_crosshair[s].p[m_crosshair[s].pair[c].p2].y;
		cx = (x1+x2)/2.0f;
		cy = (y1+y2)/2.0f;
		if (y1 > y2) {
			rx = x2;
			ry = y1;
			hx = x1;
		} else {
			rx = x1;
			ry = y2;
			hx = x2;
		}
		glBegin(GL_LINES);
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f(x1, y1, 0.0f);
			glVertex3f(x2, y2, 0.0f);
			glVertex3f(x1, y1, 0.0f);
			glVertex3f(rx, ry, 0.0f);
			glVertex3f(x2, y2, 0.0f);
			glVertex3f(rx, ry, 0.0f);
		glEnd();
		dx = abs(x1-x2);
		dy = abs(y1-y2);
		di = sqrtf(dx*dx+dy*dy);

		fXDis = dx;
		fYDis = dy;
		fLDis = di;
		if(m_crosshair[s].pDistance)
		{
			fXDis = m_crosshair[s].pDistance[c].fXDis;
			fYDis = m_crosshair[s].pDistance[c].fYDis;
			fLDis = m_crosshair[s].pDistance[c].fLDis;
		}

		if (di*m_norm_scale[s]*m_glScale >= 150.0f) {


			//if((m_fDistance_XRatio == 1.0)&&(m_fDistance_YRatio==1.0))
			if(m_strDistanceUnitString==_T(""))
			{
				DrawPrintf(s, ALIGN_SHEET|(rx<hx?ALIGN_LEFT:ALIGN_RIGHT)|ALIGN_BOTTOM,
					rx<hx?cx:-m_width[s]+cx, -m_height[s]+cy,
					L"%s:%.1f", m_vDistanceStrings[2], di);
				DrawPrintf(s, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
					cx, ry,
					L"%s:%.1f", m_vDistanceStrings[0], dx);
				DrawPrintf(s, ALIGN_SHEET|(rx<hx?ALIGN_RIGHT:ALIGN_LEFT)|ALIGN_TOP,
					rx<hx?-m_width[s]+rx:rx, cy,
					L"%s:%.1f", m_vDistanceStrings[1], dy);
			}
			else	// seanchen 20130826-2
			{
				CString strT1;
				strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[2], fLDis,m_strDistanceUnitString);
				DrawPrintf(s, ALIGN_SHEET|(rx<hx?ALIGN_LEFT:ALIGN_RIGHT)|ALIGN_BOTTOM,
					rx<hx?cx:-m_width[s]+cx, -m_height[s]+cy,
					strT1);
				
				strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[0], fXDis,m_strDistanceUnitString);				
				DrawPrintf(s, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
					cx, ry,
					strT1);

				strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[1], fYDis,m_strDistanceUnitString);
				DrawPrintf(s, ALIGN_SHEET|(rx<hx?ALIGN_RIGHT:ALIGN_LEFT)|ALIGN_TOP,
					rx<hx?-m_width[s]+rx:rx, cy,
					strT1);

				//double fDistance_LRatio = sqrtf((float)m_fDistance_XRatio*(float)m_fDistance_XRatio+(float)m_fDistance_YRatio*(float)m_fDistance_YRatio);
				//CString strT1;
				//strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[2], di*fDistance_LRatio,m_strDistanceUnitString);
				//DrawPrintf(s, ALIGN_SHEET|(rx<hx?ALIGN_LEFT:ALIGN_RIGHT)|ALIGN_BOTTOM,
				//	rx<hx?cx:-m_width[s]+cx, -m_height[s]+cy,
				//	strT1);
				//
				//strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[0], dx*m_fDistance_XRatio,m_strDistanceUnitString);				
				//DrawPrintf(s, ALIGN_SHEET|ALIGN_LEFT|ALIGN_TOP,
				//	cx, ry,
				//	strT1);

				//strT1.Format(_T("%s:%.1f %s"),m_vDistanceStrings[1], dy*m_fDistance_YRatio,m_strDistanceUnitString);
				//DrawPrintf(s, ALIGN_SHEET|(rx<hx?ALIGN_RIGHT:ALIGN_LEFT)|ALIGN_TOP,
				//	rx<hx?-m_width[s]+rx:rx, cy,
				//	strT1);
			}
		}
	}
}

// Beagle 20130117 added, 20130610 modified.
void COpenGLControl::oglAddCrosshair(int split, float x, float y, COLORREF color)
{
	int n;

	if (split < 0 || split >= MAX_SPLIT_WINDOW) return;

	ExtendCrosshairPoints(split);

	n = m_crosshair[split].num;
	m_crosshair[split].p[n].x = x;
	m_crosshair[split].p[n].y = y;
	m_crosshair[split].p[n].color = color;
	m_crosshair[split].num++;
}

// Beagle 20130118 added, 20130611 modified.
void COpenGLControl::oglAddCrosshairPair(int split, float x1, float y1, float x2, float y2, COLORREF color)
{
	int n1, n2, pn;

	if (split < 0 || split >= MAX_SPLIT_WINDOW) return;

	ExtendCrosshairPoints(split, 2);
	ExtendCrosshairPairs(split, 1);

	n1 = m_crosshair[split].num;
	n2 = m_crosshair[split].num+1;
	m_crosshair[split].num += 2;

	m_crosshair[split].p[n1].x = x1;
	m_crosshair[split].p[n1].y = y1;
	m_crosshair[split].p[n1].color = color;
	m_crosshair[split].p[n2].x = x2;
	m_crosshair[split].p[n2].y = y2;
	m_crosshair[split].p[n2].color = color;

	pn = m_crosshair[split].pair_num;
	m_crosshair[split].pair_num++;
	m_crosshair[split].pair[pn].p1 = n1;
	m_crosshair[split].pair[pn].p2 = n2;
}


void COpenGLControl::oglAddCrosshairPair_AndDis(int split, float x1, float y1, float x2, float y2, COLORREF color,float fXDis,float fYDis,float fLDis)
{
	int n1, n2, pn;

	if (split < 0 || split >= MAX_SPLIT_WINDOW) return;

	ExtendCrosshairPoints(split, 2);
	ExtendCrosshairPairs(split, 1);

	n1 = m_crosshair[split].num;
	n2 = m_crosshair[split].num+1;
	m_crosshair[split].num += 2;

	m_crosshair[split].p[n1].x = x1;
	m_crosshair[split].p[n1].y = y1;
	m_crosshair[split].p[n1].color = color;
	m_crosshair[split].p[n2].x = x2;
	m_crosshair[split].p[n2].y = y2;
	m_crosshair[split].p[n2].color = color;

	pn = m_crosshair[split].pair_num;
	m_crosshair[split].pair_num++;
	m_crosshair[split].pair[pn].p1 = n1;
	m_crosshair[split].pair[pn].p2 = n2;

	if(m_crosshair[split].pDistance)
	{
		m_crosshair[split].pDistance[pn].fXDis = fXDis;
		m_crosshair[split].pDistance[pn].fYDis = fYDis;
		m_crosshair[split].pDistance[pn].fLDis = fLDis;
	}
}


// Beagle 20130117 added.
// split == -1: Clear all crosshair mark of all window.
void COpenGLControl::oglClearCrosshair(int split)
{
	if (split >= 0 && split < MAX_SPLIT_WINDOW) {
		m_crosshair[split].num = 0;
		m_crosshair[split].pair_num = 0;
	} else if (split == -1) {
		for (int c=0; c<MAX_SPLIT_WINDOW; c++) {
			m_crosshair[c].num = 0;
			m_crosshair[c].pair_num = 0;
		}
	}
}

// Beagle 20130118 added.
void COpenGLControl::ExtendCrosshairPoints(int split, int space)
{
	int sp = max(space, 1);

	if (m_crosshair[split].num >= m_crosshair[split].size-sp) {	// Re-allocate.
		int s;
		struct _CROSSHAIR_POINT *p;

		s = max(m_crosshair[split].size, 0) + 16;
		p = (struct _CROSSHAIR_POINT *) malloc(s*sizeof(struct _CROSSHAIR_POINT));
		if (p == NULL) return;

		memset(p, 0, s*sizeof(struct _CROSSHAIR_POINT));
		if (m_crosshair[split].p != NULL) {
			if (m_crosshair[split].size > 0) {
				memcpy(p, m_crosshair[split].p,
					m_crosshair[split].size*sizeof(struct _CROSSHAIR_POINT));
			}
			free(m_crosshair[split].p);
		}
		m_crosshair[split].p = p;
		m_crosshair[split].size = s;
	}
}

// Beagle 20130118 added.
void COpenGLControl::ExtendCrosshairPairs(int split, int space)
{
	int sp = max(space, 1);

	if (m_crosshair[split].pair_num >= m_crosshair[split].pair_size-sp) {
		int s;
		struct _CROSSHAIR_PAIR *pair;
		struct _CROSSHAIR_LENGTH *pDistance; //seanchen 20130826-3

		s = max(m_crosshair[split].pair_size, 0) + 16;
		pair = (struct _CROSSHAIR_PAIR *) malloc(s*sizeof(_CROSSHAIR_PAIR));
		pDistance = (struct _CROSSHAIR_LENGTH *) malloc(s*sizeof(_CROSSHAIR_LENGTH));
		if ((pair == NULL) || (pDistance == NULL))
		{
			if(pair)
			{
				free(pair);
			}
			
			if(pDistance)
			{
				free(pDistance);
			}
			return;
		}

		memset(pair, 0, s*sizeof(struct _CROSSHAIR_PAIR));
		memset(pDistance, 0, s*sizeof(struct _CROSSHAIR_LENGTH));

		if (m_crosshair[split].pair != NULL) {
			if (m_crosshair[split].pair_size > 0) {
				memcpy(pair, m_crosshair[split].pair,
					m_crosshair[split].pair_size*sizeof(_CROSSHAIR_PAIR));
			}
			free(m_crosshair[split].pair);
		}
		if (m_crosshair[split].pDistance != NULL) {
			if (m_crosshair[split].pair_size > 0) {
				memcpy(pDistance, m_crosshair[split].pDistance,
					m_crosshair[split].pair_size*sizeof(_CROSSHAIR_LENGTH));
			}
			free(m_crosshair[split].pDistance);
		}

		m_crosshair[split].pair = pair;
		m_crosshair[split].pDistance = pDistance;
		m_crosshair[split].pair_size = s;
	}
}

// Beagle 20130121 added.
void COpenGLControl::DrawSingleString(int split_id, int align, float x, float y, const wchar_t *str)
{
	FTBBox box;
	int len;
	int split_x = split_id % m_wsplit;
	int split_y = split_id / m_wsplit;

	len = (int)wcslen(str);
	if (len <= 0) return;

	box = m_fontFixed->BBox(str, len, FTPoint(0, 0, 0), FTPoint(0, 0, 0));

	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_LOGIC_OP); 
	glDisable(GL_BLEND);
	glLoadIdentity();
	glTranslatef(m_glwidth*((0.5f+split_x)/m_wsplit-0.5f),
		m_glheight*((0.5f+split_y)/m_hsplit-0.5f), 0.0f);
	if (align & ALIGN_SHEET) {
		float scale = m_glScale*m_norm_scale[split_id];
		glTranslatef(m_xTranslate*m_glScale, m_yTranslate*m_glScale, 0.0f);
		glTranslatef(x*scale, y*scale, 0.0f);

		if ((align&0x0F) == ALIGN_LEFT) {
			glTranslatef(-0.5f*m_width[split_id]*scale, 0.0f, 0.0f);
		} else if ((align&0x0F) == ALIGN_RIGHT) {
			glTranslatef( 0.5f*m_width[split_id]*scale-box.Upper().Xf(), 0.0f, 0.0f);
		} else {	// ALIGN_HCENTER
			glTranslatef((box.Lower().Xf()-box.Upper().Xf())/2.0f, 0.0f, 0.0f);
		}

		if ((align&0xF0) == ALIGN_TOP) {
			glTranslatef(0.0f, -0.5f*m_height[split_id]*scale+m_fontFixed->Ascender(), 0.0f);
		} else if ((align&0xF0) == ALIGN_BOTTOM) {
			glTranslatef(0.0f,  0.5f*m_height[split_id]*scale+m_fontFixed->Descender(), 0.0f);
		} else {	// ALIGN_VCENTER
			glTranslatef(0.0f, (box.Lower().Yf()-box.Upper().Yf())/2.0f, 0.0f);
		}
	} else {	// ALIGN_WINDOW
		glTranslatef(x, y, 0.0f);

		if ((align&0x0F) == ALIGN_LEFT) {
			glTranslatef((-0.5f*m_glwidth)/m_wsplit, 0.0f, 0.0f);
		} else if ((align&0x0F) == ALIGN_RIGHT) {
			glTranslatef( (0.5f*m_glwidth)/m_wsplit-box.Upper().Xf(), 0.0f, 0.0f);
		} else {	// ALIGN_HCENTER
			glTranslatef((box.Lower().Xf()-box.Upper().Xf())/2.0f, 0.0f, 0.0f);
		}
 
		if ((align&0xF0) == ALIGN_TOP) {
			glTranslatef( 0.0f, (-0.5f*m_glheight)/m_hsplit+m_fontFixed->Ascender(), 0.0f);
		} else if ((align&0xF0) == ALIGN_BOTTOM) {
			glTranslatef( 0.0f,  (0.5f*m_glheight)/m_hsplit+m_fontFixed->Descender(), 0.0f);
		} else {
			glTranslatef(0.0f, (box.Lower().Yf()-box.Upper().Yf())/2.0f, 0.0f);
		}
	}
	glTranslatef(0.0f, 0.0f, -40.0f);
	glScalef(1.0f, -1.0f, 1.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
		glVertex3f(box.Lower().Xf()-2.0f, box.Lower().Yf()-2.0f, 0);
		glVertex3f(box.Lower().Xf()-2.0f, box.Upper().Yf()+2.0f, 0);
		glVertex3f(box.Upper().Xf()+2.0f, box.Upper().Yf()+2.0f, 0);
		glVertex3f(box.Upper().Xf()+2.0f, box.Lower().Yf()-2.0f, 0);
	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);
	m_fontFixed->Render(str, len, FTPoint(0, 0, 0), FTPoint(0, 0, 0), FTGL::RENDER_FRONT);
	glPopMatrix();
}

// Beagle 20130121 added.
void COpenGLControl::DrawPrintf(int split_id, int align, float x, float y, const wchar_t *fmt, ...)
{
	va_list argptr;
	wchar_t	str[128];

	va_start(argptr, fmt);
	vswprintf(str, 128, fmt, argptr);
	DrawSingleString(split_id, align, x, y, str);
}

// Private Function
// Beagle 20130305 added.
void COpenGLControl::SetTexture(IMAGE *TexImg, unsigned int texture, int flag)
{
	int gl_inter, gl_format, gl_datatype;
	BOOL do_set = FALSE;

	if (TexImg == NULL || TexImg->ptr == NULL) { return; }

	switch (TexImg->type) {
	case IMAGE_TYPE_MONO8:
		gl_inter	= 1;
		gl_format	= GL_LUMINANCE;
		gl_datatype	= GL_UNSIGNED_BYTE;
		break;
	case IMAGE_TYPE_RGB24:
		gl_inter	= 3;
		gl_format	= GL_RGB;
		gl_datatype	= GL_UNSIGNED_BYTE;
		break;
	case IMAGE_TYPE_BGR24:
		gl_inter	= 3;
		gl_format	= GL_BGR;
		gl_datatype	= GL_UNSIGNED_BYTE;
		break;
	case IMAGE_TYPE_RGB8_332:
		gl_inter	= GL_R3_G3_B2;
		gl_format	= GL_RGB;
		gl_datatype	= GL_UNSIGNED_BYTE_3_3_2;
		break;
	default:	// Unsupported type.
		return;
	}

	glBindTexture(GL_TEXTURE_2D, texture);
	if (glGetError() != GL_NO_ERROR) { 
        // m_texture[11]: region segment 因為第一次BindTexture會出現1281錯誤訊息 造成資料沒有存到Buffer (glTexSubImage2D)
        // 但第二次BindTexture就會成功 目前無法知道原因 所以先暫時做兩次BindTexture
        glBindTexture(GL_TEXTURE_2D, texture);
        if (glGetError() != GL_NO_ERROR) { return; }
    }	// Invalid texture.

	if ((TexImg->data_w & 0x03) == 0) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	} else {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}

	if (flag & STFLAG_SIZE_NO_CHANGE) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
			TexImg->data_w, TexImg->data_h,
			gl_format, gl_datatype, TexImg->ptr);
		if (glGetError() != GL_NO_ERROR) { do_set = TRUE; }
	} else { do_set = TRUE; }

	if (do_set == TRUE) {
		GLfloat color[4] = {
			GetRValue(m_glBackColor)/255.0f,
			GetGValue(m_glBackColor)/255.0f,
			GetBValue(m_glBackColor)/255.0f,
			1.0f
		};

		glTexImage2D(GL_TEXTURE_2D, 0, gl_inter,
			TexImg->data_w, TexImg->data_h, 0,
			gl_format, gl_datatype, TexImg->ptr);
		GL_CHECK_ERROR();

		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}

	if (flag & STFLAG_SMOOTH) {
		// Smooth image
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	} else {
		// Clear image (square pixel)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}

/* This is a private function. */
/* Beagle 20130409 added. */
// 貼圖不可超出 texture 原有尺吋, 否則會產生 glError 而不執行。
// OpenGL 並不會自動幫你 wrap 或裁切。
BOOL COpenGLControl::UpdateTexture(IMAGE *TexImg, unsigned int texture, int xoffset, int yoffset)
{
	int gl_inter, gl_format, gl_datatype;
	GLenum rc;

	if (TexImg == NULL || TexImg->ptr == NULL) { return FALSE; }

	switch (TexImg->type) {
	case IMAGE_TYPE_MONO8:
		gl_inter	= 1;
		gl_format	= GL_LUMINANCE;
		gl_datatype	= GL_UNSIGNED_BYTE;
		break;
	case IMAGE_TYPE_RGB24:
		gl_inter	= 3;
		gl_format	= GL_RGB;
		gl_datatype	= GL_UNSIGNED_BYTE;
		break;
	case IMAGE_TYPE_BGR24:
		gl_inter	= 3;
		gl_format	= GL_BGR;
		gl_datatype	= GL_UNSIGNED_BYTE;
		break;
	case IMAGE_TYPE_RGB8_332:
		gl_inter	= GL_R3_G3_B2;
		gl_format	= GL_RGB;
		gl_datatype	= GL_UNSIGNED_BYTE_3_3_2;
		break;
	default:	// Unsupported type.
		return FALSE;
	}

	glBindTexture(GL_TEXTURE_2D, texture);
	if (glGetError() != GL_NO_ERROR) { return FALSE; }	// Invalid texture.

	if ((TexImg->data_w & 0x03) == 0) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	} else {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}

	glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset,
		TexImg->data_w, TexImg->data_h,
		gl_format, gl_datatype, TexImg->ptr);
	rc = glGetError();
	if (rc != GL_NO_ERROR) {
		_D(L"%S(%d): Error %d: %S\n", __FILE__, __LINE__, rc, gluErrorString(rc));
		return FALSE;
	}

	return TRUE;
}

// Beagle 20130409 added.
BOOL COpenGLControl::oglUpdateTexture(IMAGE *TexImg, int xoffset, int yoffset, int TexNum)
{
	if (TexNum<0 || TexNum>MAX_SPLIT_WINDOW) return FALSE;

	wglMakeCurrent(hdc, hrc_loader);

    if ( m_OGLSegment[ TexNum ].IsSegmentation() )
    {
        return m_OGLSegment[ TexNum ].UpdateSegmentTexture( TexImg, xoffset, yoffset );
    }
	return UpdateTexture(TexImg, m_image_texture[TexNum], xoffset, yoffset);
}

// Beagle 20140212 added.
// 需檢查傳回值, 超界會發生 error
BOOL COpenGLControl::oglUpdateTextureBySplit(IMAGE *TexImg,
	int index, int total, int TexNum)
{
	int	xoffset, yoffset, xtotal;

	if (TexNum<0 || TexNum>MAX_SPLIT_WINDOW) return FALSE;

	for (xtotal=1; xtotal<=6; xtotal++) {
		if (xtotal*xtotal >= total) break;
	}
	if (xtotal > 6 || index >= total) return FALSE;

	xoffset = (index % xtotal) * m_width[TexNum] / xtotal;
	yoffset = (index / xtotal) * m_height[TexNum] / xtotal;

	wglMakeCurrent(hdc, hrc_loader);

	return UpdateTexture(TexImg, m_image_texture[TexNum], xoffset, yoffset);
}

// Beagle 20140212 added.
// 
void COpenGLControl::oglClearTexture(int data_w, int data_h, COLORREF color, int TexNum, int orig_w, int orig_h)
{
	IMAGE TexImg;
	unsigned char *data;
	DWORD data_size = data_w*data_h*3;

	if (data_w <= 0 || data_h <= 0) return;

    int iMaxTextureSize = 0;

    glGetIntegerv( GL_MAX_TEXTURE_SIZE, &iMaxTextureSize );

    if ( data_w > iMaxTextureSize || 
         data_h > iMaxTextureSize )
    {
        data = NULL; // initial by OpenGLSegmentation class
    }
	else
    {
        data = (unsigned char *) malloc(data_size);

	    for (DWORD c=0; c<data_size; c+=3) {
		    data[c]   = GetRValue(color);
		    data[c+1] = GetGValue(color);
		    data[c+2] = GetBValue(color);
	    }
    }
	ImageInit(&TexImg, IMAGE_TYPE_RGB24, data, data_w, data_h, orig_w, orig_h);
	oglSetTexture(&TexImg, TexNum);
	free(data);
}

// 把需要在 DrawScene() 做完之後才做的 GDI function 放在這裡。
// 注意: GDI function 速度非常慢，繪圖部份還是儘量使用 OpenGL 硬體加速。
// Beagle 20130423 added.
void COpenGLControl::PostDrawScene(BOOL isPaint)
{
	if (!m_tracker && m_vRectTrackerList.size()==0 && !m_pLineTracker) { return; }

	glFinish();
	CDC *pDC = GetDC();

	OnUpdateShadowLine();

	if (m_tracker) { m_tracker->Draw(pDC, &m_redPen); }

	for (size_t c=0; c<m_vRectTrackerList.size(); c++) {
		if (m_lmMode != LM_MODIFY_SHADOWRECT) {
			m_vRectTrackerList[c].Draw(pDC, &m_redPen);
		} else {
			m_vRectTrackerList[c].Draw(pDC, &m_bluePen);
		}
	}

	if (m_pLineTracker) {
		CPoint ptStart;
		CPoint ptEnd;

		pDC->SelectObject(&m_redPen);

		int size = m_LineList.GetSize();
		for (int i=0; i<size; i++) {
			CLineItem* pLine = m_LineList.GetLineItem(i);
			ptStart.x = pLine->m_rcPoints.left;
			ptStart.y = pLine->m_rcPoints.top;

			ptEnd.x = pLine->m_rcPoints.right;
			ptEnd.y = pLine->m_rcPoints.bottom;

			pDC->MoveTo(ptStart);
			pDC->LineTo(ptEnd);
		}

		m_pLineTracker->Draw(pDC);
	}

	ReleaseDC(pDC);
}

// Beagle 20130424 added.
void COpenGLControl::DrawCursorOutline(int split_id)
{
	if (m_show_cursor_outline == FALSE) { return; }
	if (split_id != m_mouse_sp) {return;}
	// 放大到單一像素顯示大小超過 6x6，才標示出游標所在位置的像素。
	if (m_norm_scale[0]*m_glScale < 6.0f) { return; }

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_LOGIC_OP);
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);
	glColor3f(0.6f, 1.0f, 1.0f);
	glBegin(GL_LINE_LOOP);
		glVertex3f(m_mouse_x+0.1f, m_mouse_y+0.1f, 0);
		glVertex3f(m_mouse_x+0.9f, m_mouse_y+0.1f, 0);
		glVertex3f(m_mouse_x+0.9f, m_mouse_y+0.9f, 0);
		glVertex3f(m_mouse_x+0.1f, m_mouse_y+0.9f, 0);
	glEnd();
}

// Beagle 20130621 added.
void COpenGLControl::oglEnableMinimumRedrawInterval(BOOL enable)
{
	if (enable == TRUE) {
		m_enable_redraw_interval = TRUE;
		m_need_redraw = TRUE;
		SetTimer(IDT_GL_TIMER, 100, NULL);
		// The timer will redraw later.
	} else {	// enable == FALSE
		KillTimer(IDT_GL_TIMER);
		m_enable_redraw_interval = FALSE;
		m_need_redraw = FALSE;
		// Redraw now to sync OpenGL screen.
		DrawScene(FALSE);
		PostDrawScene(FALSE);
	}
}

// Beagle 20140214
void COpenGLControl::oglSetRadioButtonNumber(int n)
{
	if (n < 0) return;

	m_nRadioButton=n;
	m_nRadioSelected=0;
	for (int c=0; c<m_nRadioButton; c++) {
		m_splitEnable[c] = true;
	}
}

// Beagle 20130807
void COpenGLControl::DrawRadioButton(void)
{
	float radius = 10.0f;

	if (m_nRadioButton < 1 || m_nRadioButton > MAX_SPLIT_WINDOW) { return; }

	glPushMatrix();
	glPushName(0);
	for (int c=0; c<m_nRadioButton; c++) {
		glLoadName(c);
		glLoadIdentity();
		glTranslatef(-0.5f*m_glwidth+(radius*2+15.0f)*c+radius+10.0f,
			0.5f*m_glheight-(radius+10.0f), -10.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		DrawRing(radius, radius+5.0f);

		if (c == m_nRadioSelected) {
			glColor3f(0.6f, 0.6f, 0.6f);
		} else {
			glColor3f(0.8f, 0.8f, 0.8f);
		}
		glBegin(GL_POLYGON);
			DrawCircle(radius);
		glEnd();

		glScalef(radius*1.2f, radius*1.2f, 1.0f);
		FTBBox box;
		GLfloat xoffset, yoffset;
		wchar_t str[32];
		swprintf_s(str, L"%d", c+1);
		box = m_font->BBox(str, 1);
		xoffset = (box.Upper().Xf() + box.Lower().Xf())/2.0f;
		yoffset = (box.Upper().Yf() + box.Lower().Yf())/2.0f;

		glScalef(0.066667f, -0.066667f, 1.0f);

		glTranslatef(-xoffset, -yoffset, 0);
		if (c == m_nRadioSelected) {
			glColor3f(0.5f, 1.0f, 0.5f);
		} else {
			glColor3f(0.0f, 0.0f, 0.0f);
		}
		m_font->Render(str, (int) wcslen(str), FTPoint(0, 0, 0));
	}
	glPopName();
	glPopMatrix();
}

// Beagle 20130807
void COpenGLControl::DrawRing(float radius1, float radius2)
{
	float iR, oR;

	if (radius1 > radius2) {
		iR = radius2;
		oR = radius1;
	} else {
		iR = radius1;
		oR = radius2;
	}

	glBegin(GL_TRIANGLE_STRIP);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 1.000000f*oR,  0.000000f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 1.000000f*iR,  0.000000f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 0.980785f*oR,  0.195090f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.980785f*iR,  0.195090f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 0.923880f*oR,  0.382683f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.923880f*iR,  0.382683f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 0.831470f*oR,  0.555570f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.831470f*iR,  0.555570f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 0.707107f*oR,  0.707107f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.707107f*iR,  0.707107f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 0.555570f*oR,  0.831470f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.555570f*iR,  0.831470f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 0.382683f*oR,  0.923880f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.382683f*iR,  0.923880f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 0.195090f*oR,  0.980785f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.195090f*iR,  0.980785f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f(-0.000000f*oR,  1.000000f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(-0.000000f*iR,  1.000000f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f(-0.195090f*oR,  0.980785f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(-0.195090f*iR,  0.980785f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f(-0.382683f*oR,  0.923880f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(-0.382683f*iR,  0.923880f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f(-0.555570f*oR,  0.831470f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(-0.555570f*iR,  0.831470f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f(-0.707107f*oR,  0.707107f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(-0.707107f*iR,  0.707107f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f(-0.831470f*oR,  0.555570f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(-0.831470f*iR,  0.555570f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f(-0.923880f*oR,  0.382683f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(-0.923880f*iR,  0.382683f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f(-0.980785f*oR,  0.195090f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(-0.980785f*iR,  0.195090f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f(-1.000000f*oR, -0.000000f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(-1.000000f*iR, -0.000000f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f(-0.980785f*oR, -0.195090f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(-0.980785f*iR, -0.195090f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f(-0.923880f*oR, -0.382683f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(-0.923880f*iR, -0.382683f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f(-0.831470f*oR, -0.555570f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(-0.831470f*iR, -0.555570f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f(-0.707107f*oR, -0.707107f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(-0.707107f*iR, -0.707107f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f(-0.555570f*oR, -0.831469f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(-0.555570f*iR, -0.831469f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f(-0.382684f*oR, -0.923879f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(-0.382684f*iR, -0.923879f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f(-0.195090f*oR, -0.980785f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(-0.195090f*iR, -0.980785f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 0.000000f*oR, -1.000000f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.000000f*iR, -1.000000f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 0.195090f*oR, -0.980785f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.195090f*iR, -0.980785f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 0.382684f*oR, -0.923879f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.382684f*iR, -0.923879f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 0.555570f*oR, -0.831470f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.555570f*iR, -0.831470f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 0.707107f*oR, -0.707107f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.707107f*iR, -0.707107f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 0.831470f*oR, -0.555570f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.831470f*iR, -0.555570f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 0.923880f*oR, -0.382683f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.923880f*iR, -0.382683f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 0.980785f*oR, -0.195090f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.980785f*iR, -0.195090f*iR, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		glVertex3f( 1.000000f*oR,  0.000000f*oR, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 1.000000f*iR,  0.000000f*iR, 0.0f);
	glEnd();
}

// Beagle 20130807
void COpenGLControl::MousePick(int x, int y)
{
	GLuint	buffer[PICKBUFFER];
	GLint	hits;
	GLint	viewport[4];

	glGetIntegerv(GL_VIEWPORT, viewport);
	glSelectBuffer(PICKBUFFER, buffer);
	glRenderMode(GL_SELECT);
	glInitNames();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	glLoadIdentity();
	gluPickMatrix((GLdouble)x, (GLdouble)(viewport[3]-y),
		1.0f, 1.0f, viewport);
	glOrtho(-0.5f*m_glwidth, 0.5f*m_glwidth,
		0.5f*m_glheight, -0.5f*m_glheight, 0.0f, 100.0f);

	glMatrixMode(GL_MODELVIEW);

	// Draw
	DrawRadioButton();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	hits = glRenderMode(GL_RENDER);

	// Pick the nearest object hit.
	if (hits > 0) {
		int choose=-1, nearest=-1;
		for (int c=0,obj=0; c<hits; c++) {
			int size = buffer[obj]+3;
			int	depth = buffer[obj+1];
			int name = -1;
			if (size > 3) { name = buffer[obj+size-1]; }
			if (choose==-1 || depth<nearest) {
				choose = name;
				nearest = depth;
			}
			obj += size;
		}
		m_nRadioSelected = choose;
		hWnd->SendMessage(WM_OPENGL_MSG, IDM_OPENGL_RADIO_INDEX, m_nRadioSelected);	// added by eric at 20170411
	}
}

// Beagle 20130814
void COpenGLControl::DrawLuminanceGraph(int split_id)
{
	bool isHorizontal;
	float x, y;

	if ((m_xLuminance.type&LUMINANCE_MODE_MASK)==LUMINANCE_NONE||m_xLuminance.len<=0) {
		return;
	}

	if ((m_xLuminance.type&LUMINANCE_MODE_MASK) == LUMINANCE_HORIZONTAL) {
		isHorizontal = true;
	} else {
		isHorizontal = false;
	}

	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glLoadIdentity();
	glTranslatef(-0.5f*m_glwidth, -0.5f*m_glheight, -10.0f);

	glLineWidth(1.5f);

	glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
	DrawCurve(split_id, isHorizontal, m_xLuminance.dataLuminance, m_xLuminance.len);

	glColor4f(1.0f, 0.0f, 0.0f, 0.3f);
	DrawCurve(split_id, isHorizontal, m_xLuminance.dataRed, m_xLuminance.len);

	glColor4f(0.0f, 1.0f, 0.0f, 0.3f);
	DrawCurve(split_id, isHorizontal, m_xLuminance.dataGreen, m_xLuminance.len);

	glColor4f(0.0f, 0.0f, 1.0f, 0.3f);
	DrawCurve(split_id, isHorizontal, m_xLuminance.dataBlue, m_xLuminance.len);

	if (m_xLuminance.type & LUMINANCE_GRIDLINE) { DrawGrid(isHorizontal); }

	TransformImage2Window(split_id, (float)m_xLuminance.x,
		(float)m_xLuminance.y, x, y);
	if (isHorizontal) {
		DrawDashedLine(isHorizontal, y);
	} else {
		DrawDashedLine(isHorizontal, x);
	}

	glLineWidth(1.0f);
}

// Assign color before call this function. -- Beagle 20130815
void COpenGLControl::DrawCurve(int split_id, bool isHorizontal, const unsigned char *data, int len)
{
	GLfloat x, y, x1, y1, x2, y2, dx, dy;

	TransformImage2Window(split_id, 0, 0, x1, y1);
	TransformImage2Window(split_id, 1, 1, x2, y2);
	dx = x2 - x1;
	dy = y2 - y1;

	glBegin(GL_LINE_STRIP);
	if (isHorizontal) {
		y1 = (float)m_glheight;
		dy = (-m_glheight/255.0f);
		for (int i=0; i<len; i++) {
			x = i*dx+x1;
			y = data[i]*dy+y1;
			glVertex3f(x, y, 0.0f);
		}
	} else {
		x1 = 0.0f;
		dx = m_glwidth/255.0f;
		for (int i=0; i<len; i++) {
			x = data[i]*dx+x1;
			y = i*dy+y1;
			glVertex3f(x, y, 0.0f);
		}
	}
	glEnd();
}

// Beagle 20130816
void COpenGLControl::DrawGrid(bool isHorizontal)
{
	GLfloat a;

	glBegin(GL_LINES);
	for (int i=0; i<255; i+=10) {
		if (i % 50 == 0) {
			glColor4f(1.0f, 0.3f, 0.3f, 0.5f);
		} else {
			glColor4f(0.3f, 1.0f, 0.3f, 0.5f);
		}
		if (isHorizontal) {
			a = m_glheight*(1.0f-i*1.0f/255.0f);
			glVertex3f(0.0f, a, 0.0f);
			glVertex3f(1.0f*m_glwidth, a, 0.0f);
		} else {
			a = m_glwidth*(i*1.0f/255.0f);
			glVertex3f(a, 0.0f, 0.0f);
			glVertex3f(a, 1.0f*m_glheight, 0.0f);
		}
	}
	glEnd();
}

// Beagle 20130816
void COpenGLControl::DrawDashedLine(bool isHorizontal, float position)
{
	if (m_xLuminance.x == -1 || m_xLuminance.y == -1) { return; }

	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0x0F0F);
	glLineWidth(1.2f);
	glLogicOp(GL_INVERT);

	glColor3f(1.0f, 1.0f, 0.0f);
	if (isHorizontal) {
		glBegin(GL_LINES);
			glVertex3f(0.0f, position, 0.0f);
			glVertex3f((GLfloat)m_glwidth, position, 0.0f);
		glEnd();
	} else {
		glBegin(GL_LINES);
			glVertex3f(position, 0.0f, 0.0f);
			glVertex3f(position, (GLfloat)m_glheight, 0.0f);
		glEnd();
	}

	glDisable(GL_LINE_STIPPLE);
	glLineWidth(1.0f);
}

BOOL COpenGLControl::IsHitLineRect(CPoint pt, int &idx)
{
	for (int i=0; i<m_lineRects; i++) {
		CRect rt;
		rt=m_lineRect[i];
		rt.InflateRect(16,16);
		if (rt.PtInRect(pt)==TRUE) {
			if (m_nEnableMap & (1<<i)){ //if Enable InkJet
				idx=i;
				
                return ( m_lineRect[i].left == m_lineRect[i].right ? 1 : 2 );
			}
		}
	}
	return FALSE;
}
// Beagle 20130816
void COpenGLControl::TransformImage2Window(int split_id,
	float ix, float iy, float &wx, float &wy)
{
	wx = m_width[split_id]/2.0f;
	wx -= ix+0.5f;
	wx *= m_norm_scale[split_id];
	wx -= m_xTranslate;
	wx *= m_glScale;
	wx = m_glwidth/2 - wx;

	wy = m_height[split_id]/2.0f;
	wy -= iy+0.5f;
	wy *= m_norm_scale[split_id];
	wy -= m_yTranslate;
	wy *= m_glScale;
	wy = m_glheight/2 - wy;
}

void COpenGLControl::oglEnableRollingMode( const BOOL bEnable, const int iRollingMilliSeconds, const int iTextureNum )
{
    if ( iTextureNum < MAX_SPLIT_WINDOW )
    {
        m_OGLRolling[ iTextureNum ].AttachData( m_glwidth / m_wsplit, m_glheight / m_hsplit, &m_glBackColor, &m_glScale, &m_xTranslate, &m_yTranslate );
        m_OGLRolling[ iTextureNum ].SetMilliSeconds( bEnable ? iRollingMilliSeconds : NULL, this );
    }
}

void COpenGLControl::oglRunRollingMode( const BOOL bRun, const int iTextureNum )
{
    if ( iTextureNum < MAX_SPLIT_WINDOW )
    {
        m_OGLRolling[ iTextureNum ].RunRollingMode( bRun );
    }
}

void COpenGLControl::oglNoColorCubeBackground( const BOOL bNoColorCubeBackground )
{
    m_bNoColorCubeBackground = bNoColorCubeBackground;
}

/* End of OpenGLControl.cpp */
