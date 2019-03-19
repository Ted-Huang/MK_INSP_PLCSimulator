#pragma once

#define MAX_SEGMENTATION 16

enum MODE_TEXTURE
{
    MODE_SEGMENTATION,
    MODE_SCALE
};

struct _IMAGE;
class OpenGLSegmentation
{
    CWnd*           m_pWnd;
    MODE_TEXTURE    m_eMode;
    BOOL            m_bInitial;
    UINT            m_iSegmentTexture[ MAX_SEGMENTATION ];
    POINT           m_iCenterPt[ MAX_SEGMENTATION ];
    COLORREF*       m_pBackColor;
    BYTE*           m_pBMPInfo;
    BYTE*           m_pBMPData;
    DWORD           m_dwBMPDataLen;
    int             m_iSrcWidth;
    int             m_iSrcHeight;
    int             m_iTextureWidth;
    int             m_iTextureHeight;
    int             m_iTextureCountX;
    int             m_iTextureCountY;
    double          m_modelview[ 16 ];
    double          m_projection[ 16 ];
    float           m_fProjectRate;
    const float*    m_pfScale;
    const float*    m_pfTranslateX;
    const float*    m_pfTranslateY;
    int             m_iSplitWidth;
    int             m_iSplitHeight;

    BYTE*       GetBuffData( const int iColorSpace );
    BITMAPINFO* GetBMPInfo();

    BOOL        GetImageInfo( const _IMAGE* pSrcImg, int& gl_inter, int& gl_format, int& gl_datatype );
    void        ClipImage( BYTE* pClipImage, const int iClipImageWidth, const int iClipImageHeight, const int iXPos, const int iYPos, const _IMAGE* pSrcImg );

    void        SegmentatModeTexture( const _IMAGE *pSrcImg, const int iMaxTextureSize );
    void        ScaleModeTexture( const _IMAGE *pSrcImg, const int iMaxTextureSize );

    BOOL        SegmentatModeUpdate( const _IMAGE *pSrcImg, const int iXoffset, const int iYoffset );
    BOOL        ScaleModeUpdate( const _IMAGE *pSrcImg, const int iXoffset, const int iYoffset );

    void        DrawSegmentMarker();

public:

    OpenGLSegmentation();
    ~OpenGLSegmentation();

    inline BOOL IsSegmentation() { return m_bInitial; }

    void SetSegmentTexture( const _IMAGE *pSrcImg, const MODE_TEXTURE eMode, CWnd* pWnd );
    void UnsetSegmentTexture();

    void AttachData( const int iSplitWidth, const int iSplitHeight, COLORREF* pBackColor, const float* pfScale, const float* pfTranslateX, const float* pfTranslateY );
    void DrawSegment();
    void DrawSegmentMinimap();
    
    BOOL UpdateSegmentTexture( const _IMAGE *pSrcImg, const int iXoffset, const int iYoffset );

    void GetWindowToImage( const int* pView, const float& fDepth, const int iWindowX, const int iWindowY, double& dbImageX, double& dbImageY );
    void GetImageToWindow( const int* pView, const float& fDepth, const int iImageX, const int iImageY, double& iWindowX, double& iWindowY );
};

