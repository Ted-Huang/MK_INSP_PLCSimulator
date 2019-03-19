#pragma once

#define INTMAX_MAX          INT64_MAX
#define MAX_ROLLING_TEXTURE 10

#include "aoiconst.h"

#if _MSC_VER >= 1800
#include <mutex>
#endif

#include <vector>

struct _IMAGE;
class COpenGLControl;
class OpenGLRollingMode
{
    struct RollingInfo
    {
        int   iTextureIdx;
        int   iPos;
        BOOL  bEmpty;
        DWORD dwColor;

        std::vector< CRect > vecDefectRect;

        RollingInfo() : iTextureIdx( NULL ), iPos( NULL ), bEmpty( TRUE ), dwColor( NULL ), vecDefectRect( MAX_UI_DEFECT ) {}
    };
private:
#if _MSC_VER >= 1800
    std::mutex           m_mutex;
#endif
    COpenGLControl*      m_pOGLControl;
    DWORD                m_dwMilliSeconds;
    RollingInfo*         m_pCurrentRollingInfo;
    UINT                 m_iRollingTexture[ MAX_ROLLING_TEXTURE ];
    HANDLE               m_hThread;
    HANDLE               m_hExit;
    int                  m_iUnVisiblePos;
    int                  m_iLastPos;
    volatile RollingInfo m_xRollingInfo[ MAX_ROLLING_TEXTURE ];
    int                  m_iSrcWidth;
    int                  m_iSrcHeiight;
    int                  m_iTextureWidth;
    int                  m_iTextureHeight;
    float                m_fProjectRate;
    COLORREF*            m_pBackColor;
    const float*         m_pfScale;
    const float*         m_pfTranslateX;
    const float*         m_pfTranslateY;
    int                  m_iSplitWidth;
    int                  m_iSplitHeight;
    double               m_modelview[ 16 ];
    double               m_projection[ 16 ];

    void         ResetInfo( const _IMAGE *pSrcImg );
    BOOL         GetImageInfo( const _IMAGE* pSrcImg, int& gl_inter, int& gl_format, int& gl_datatype );
    RollingInfo* GetRollingInfo();
    void         MoveScaleTexture( const int iRollPos );
    void         DrawEmpty();
    void         DrawTexture( const int iTexture );
    void         DrawDefect( const RollingInfo* pRollingInfo );

    static DWORD __stdcall Thread_Timer( void* pvoid );

public:

    OpenGLRollingMode();
    ~OpenGLRollingMode();

    inline BOOL IsRollingMode() { return m_dwMilliSeconds != NULL; };

    void RunRollingMode( const BOOL bRun );
    
    void AttachData( const int iSplitWidth, const int iSplitHeight, COLORREF* pBackColor, const float* pfScale, const float* pfTranslateX, const float* pfTranslateY );

    void SetMilliSeconds( const DWORD dwMilliSeconds, COpenGLControl* pOGLControl );
    BOOL SetRollingTexture( const _IMAGE *pSrcImg );

    void SetRectInfo( const int index, const CRect& rt, const COLORREF color );

    void DrawRolling();

    void GetWindowToImage( const int* pView, const float& fDepth, const int iWindowX, const int iWindowY, double& dbImageX, double& dbImageY );
    void GetImageToWindow( const int* pView, const float& fDepth, const int iImageX, const int iImageY, double& iWindowX, double& iWindowY );
};