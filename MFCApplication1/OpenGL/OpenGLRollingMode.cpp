#include "stdafx.h"
#include "OpenGLRollingMode.h"
#include "OpenGLControl.h"

OpenGLRollingMode::OpenGLRollingMode() : m_pOGLControl( NULL ),
                                         m_pCurrentRollingInfo( NULL ),
                                         m_dwMilliSeconds( NULL ),
                                         m_iUnVisiblePos( NULL ),
                                         m_iLastPos( NULL ),
                                         m_iSrcWidth( NULL ),
                                         m_iSrcHeiight( NULL ),
                                         m_iTextureWidth( NULL ),
                                         m_iTextureHeight( NULL ),
                                         m_pBackColor( NULL ),
                                         m_pfScale( NULL ),
                                         m_pfTranslateX( NULL ),
                                         m_pfTranslateY( NULL ),
                                         m_iSplitWidth( NULL ),
                                         m_iSplitHeight( NULL ),
                                         m_fProjectRate( NULL ),
                                         m_hThread( NULL ),
                                         m_hExit( ::CreateEvent( NULL, TRUE, FALSE, NULL ) )
{
    memset( m_iRollingTexture, NULL, sizeof( int ) * MAX_ROLLING_TEXTURE );
}

OpenGLRollingMode::~OpenGLRollingMode()
{
    if ( m_dwMilliSeconds )
    {
        RunRollingMode( FALSE );

        ::CloseHandle( m_hExit );

        glDeleteTextures( MAX_ROLLING_TEXTURE, m_iRollingTexture );
    }
}

void OpenGLRollingMode::RunRollingMode( const BOOL bRun )
{
    if ( bRun && IsRollingMode() )
    {
        if ( m_dwMilliSeconds && !m_hThread ) m_hThread = ::CreateThread( NULL, NULL, Thread_Timer, this, NULL, NULL );
    }
    else
    {
        ::SetEvent( m_hExit );
        ::WaitForSingleObject( m_hThread, INFINITE );
        ::ResetEvent( m_hExit );

        m_hThread = NULL;
    }
}

void OpenGLRollingMode::AttachData( const int iSplitWidth, const int iSplitHeight, COLORREF* pBackColor, const float* pfScale, const float* pfTranslateX, const float* pfTranslateY )
{
    if ( !m_pBackColor   ) m_pBackColor   = pBackColor;
    if ( !m_pfScale      ) m_pfScale      = pfScale;
    if ( !m_pfTranslateX ) m_pfTranslateX = pfTranslateX;
    if ( !m_pfTranslateY ) m_pfTranslateY = pfTranslateY;
    if ( !m_iSplitWidth  ) m_iSplitWidth  = iSplitWidth;
    if ( !m_iSplitHeight ) m_iSplitHeight = iSplitHeight;
}

void OpenGLRollingMode::SetMilliSeconds( const DWORD dwMilliSeconds, COpenGLControl* pOGLControl )
{
    if ( !m_iRollingTexture[ NULL ] )
    {
        glGenTextures( MAX_ROLLING_TEXTURE, m_iRollingTexture );
    }
    m_pOGLControl    = pOGLControl;
    m_dwMilliSeconds = dwMilliSeconds;
}

BOOL OpenGLRollingMode::SetRollingTexture( const _IMAGE *pSrcImg )
{
#if _MSC_VER >= 1800
    std::lock_guard< std::mutex > lock( m_mutex );
#endif
    RollingInfo* pRollingInfo = GetRollingInfo();
    int          gl_inter     = NULL;
    int          gl_format    = NULL;
    int          gl_datatype  = NULL;

    if ( !pRollingInfo || !GetImageInfo( pSrcImg, gl_inter, gl_format, gl_datatype ) )
    {
        return FALSE;
    }
    if ( m_iTextureWidth != pSrcImg->data_w || m_iTextureHeight != pSrcImg->data_h )
    {
        ResetInfo( pSrcImg );
    }
    pRollingInfo->vecDefectRect.clear();

    glBindTexture( GL_TEXTURE_2D, m_iRollingTexture[ pRollingInfo->iTextureIdx ] );

	glPixelStorei( GL_UNPACK_ALIGNMENT, ( m_iTextureWidth & 0x03 ? 1 : 4 ) );

    glTexImage2D( GL_TEXTURE_2D, NULL, gl_inter, m_iTextureWidth, m_iTextureHeight, NULL, gl_format, gl_datatype, pSrcImg->ptr );

	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_BORDER );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_BORDER );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR          );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR          );

    return TRUE;
}

void OpenGLRollingMode::SetRectInfo( const int index, const CRect& rt, const COLORREF color )
{
#if _MSC_VER >= 1800
    std::lock_guard< std::mutex > lock( m_mutex );
#endif
    if ( !m_pCurrentRollingInfo ) return;

    m_pCurrentRollingInfo->dwColor = color;

    if ( index >= m_pCurrentRollingInfo->vecDefectRect.size() )
    {
        m_pCurrentRollingInfo->vecDefectRect.push_back( rt );
    }
    else m_pCurrentRollingInfo->vecDefectRect[ index ] = rt;
}

void OpenGLRollingMode::GetWindowToImage( const int* pView, const float& fDepth, const int iWindowX, const int iWindowY, double& dbImageX, double& dbImageY )
{
    static double z;

    gluUnProject( iWindowX, m_iSplitHeight - iWindowY, fDepth, m_modelview, m_projection, pView, &dbImageX, &dbImageY, &z );
}

void OpenGLRollingMode::GetImageToWindow( const int* pView, const float& fDepth, const int iImageX, const int iImageY, double& iWindowX, double& iWindowY )
{
    static double z;

    gluProject( iImageX + 0.5, iImageY + 0.5, fDepth, m_modelview, m_projection, pView, &iWindowX, &iWindowY, &z );
}

void OpenGLRollingMode::DrawRolling()
{
#if _MSC_VER >= 1800
    std::lock_guard< std::mutex > lock( m_mutex );
#endif
    glDisable( GL_BLEND );

    for ( int i = NULL; i < MAX_ROLLING_TEXTURE; i++ )
    {
        glPushMatrix();

        MoveScaleTexture( m_xRollingInfo[ i ].iPos );

        if ( m_xRollingInfo[ i ].bEmpty )
        {
            DrawEmpty(); 
        }
        else
        {
            glGetDoublev( GL_MODELVIEW_MATRIX,  m_modelview  );
            glGetDoublev( GL_PROJECTION_MATRIX, m_projection );

            DrawTexture( m_iRollingTexture[ i ] );

            DrawDefect( ( RollingInfo* )&m_xRollingInfo[ i ] );
        }
        glPopMatrix();
    }
    MoveScaleTexture( NULL );
}

void OpenGLRollingMode::DrawEmpty()
{
    glDisable( GL_TEXTURE_2D );

    glBegin( GL_QUADS );
        glColor3f( ( float )GetRValue( *m_pBackColor ) / 255.f,
			       ( float )GetGValue( *m_pBackColor ) / 255.f,
			       ( float )GetBValue( *m_pBackColor ) / 255.f );
		glVertex3i( 0, 0, 0 );
		glVertex3i( 0, m_iSrcHeiight, 0 );
		glVertex3i( m_iSrcWidth, m_iSrcHeiight, 0 ) ;
		glVertex3i( m_iSrcWidth, 0, 0 );
	glEnd();
}

void OpenGLRollingMode::DrawTexture( const int iTexture )
{
    glEnable( GL_TEXTURE_2D );

    glBindTexture( GL_TEXTURE_2D, iTexture );

    glBegin( GL_QUADS );
		glColor3f( 1.0f, 1.0f, 1.0f );

		glTexCoord2i( 0, 0 );
		glVertex3i( 0, 0, 0 );

		glTexCoord2i( 0, 1 );
		glVertex3i( 0, m_iSrcHeiight, 0 );

		glTexCoord2i( 1, 1 );
		glVertex3i( m_iSrcWidth, m_iSrcHeiight, 0 ) ;

		glTexCoord2i( 1, 0 );
		glVertex3i( m_iSrcWidth, 0, 0 );
    glEnd();
}

void OpenGLRollingMode::DrawDefect( const RollingInfo* pRollingInfo )
{
    glDisable( GL_TEXTURE_2D );

    for ( UINT i = NULL; i < pRollingInfo->vecDefectRect.size(); i++ )
    {
        glLineWidth( 1.2f );

        glColor3f( 0.2f, 0.2f, 0.8f );

        glBegin( GL_LINE_LOOP );
			glVertex3i( pRollingInfo->vecDefectRect[ i ].left,  pRollingInfo->vecDefectRect[ i ].top,    0 );
			glVertex3i( pRollingInfo->vecDefectRect[ i ].right, pRollingInfo->vecDefectRect[ i ].top,    0 );
			glVertex3i( pRollingInfo->vecDefectRect[ i ].right, pRollingInfo->vecDefectRect[ i ].bottom, 0 );
			glVertex3i( pRollingInfo->vecDefectRect[ i ].left,  pRollingInfo->vecDefectRect[ i ].bottom, 0 );
		glEnd();

        if ( m_pOGLControl->oglIsShowRectColor() )
        {
            glPushMatrix();

            glTranslatef( ( float )pRollingInfo->vecDefectRect[ i ].left, ( float )pRollingInfo->vecDefectRect[ i ].top, 0.0f );

			if( *m_pfScale < 10.0f ) glScalef( 10.0f/ *m_pfScale / m_fProjectRate, 10.0f / *m_pfScale / m_fProjectRate, 1.0f );
            else                     glScalef( 1.0f / m_fProjectRate, 1.0f / m_fProjectRate, 1.0f );

            m_pOGLControl->oglDrawBilliardBall( pRollingInfo->dwColor, i + 1 );

            glPopMatrix();
        }
    }
}

void OpenGLRollingMode::MoveScaleTexture( const int iRollPos )
{
    glScalef( *m_pfScale, *m_pfScale, 1.0f );
    glScalef( m_fProjectRate, m_fProjectRate, 1.0f );

    glTranslatef( *m_pfTranslateX / m_fProjectRate, ( *m_pfTranslateY + iRollPos ) / m_fProjectRate, 0.0f );
    glTranslatef( -0.5f * m_iSrcWidth, -0.5f * m_iSrcHeiight, 0.0f );
}

void OpenGLRollingMode::ResetInfo( const _IMAGE *pSrcImg )
{
    const BOOL bWidthPriority = ( float )pSrcImg->orig_w / pSrcImg->orig_h > ( float )m_iSplitWidth / m_iSplitHeight;

    m_fProjectRate = bWidthPriority ? ( float )m_iSplitWidth / pSrcImg->orig_w : ( float )m_iSplitHeight / pSrcImg->orig_h;

    const int iHeightStretch = int( pSrcImg->orig_h * m_fProjectRate );

    m_iSrcWidth      = pSrcImg->orig_w;
    m_iSrcHeiight    = pSrcImg->orig_h;
    m_iTextureWidth  = pSrcImg->data_w;
    m_iTextureHeight = pSrcImg->data_h;
    m_iUnVisiblePos  = -( m_iSplitHeight + iHeightStretch ) / 2;
    m_iLastPos       = iHeightStretch * ( MAX_ROLLING_TEXTURE - 1 ) + ( iHeightStretch - m_iSplitHeight ) / 2;

    for ( int i = NULL; i < MAX_ROLLING_TEXTURE; i++ )
    {
        m_xRollingInfo[ i ].iTextureIdx = i;
        m_xRollingInfo[ i ].iPos        = i * iHeightStretch;
    }
}

OpenGLRollingMode::RollingInfo* OpenGLRollingMode::GetRollingInfo()
{
    m_pCurrentRollingInfo = NULL;

    for ( int i = NULL; i < MAX_ROLLING_TEXTURE; i++ )
    {
        if ( m_xRollingInfo[ i ].bEmpty ) // find top-empty
        {
            if ( !m_pCurrentRollingInfo )
            {
                m_pCurrentRollingInfo = ( RollingInfo* )&m_xRollingInfo[ i ];

                m_pCurrentRollingInfo->bEmpty = FALSE;
            }
            else if ( m_pCurrentRollingInfo->iPos > m_xRollingInfo[ i ].iPos )
            {
                m_pCurrentRollingInfo->bEmpty = TRUE;

                m_pCurrentRollingInfo = ( RollingInfo* )&m_xRollingInfo[ i ];

                m_pCurrentRollingInfo->bEmpty = FALSE;
            }
        }
    }
    return m_pCurrentRollingInfo;
}

BOOL OpenGLRollingMode::GetImageInfo( const _IMAGE *pSrcImg, int& gl_inter, int& gl_format, int& gl_datatype )
{
    if ( pSrcImg == NULL ) return FALSE;

	switch ( pSrcImg->type )
    {
	case IMAGE_TYPE_MONO8:
        {
		    gl_inter	= 1;
		    gl_format	= GL_LUMINANCE;
		    gl_datatype	= GL_UNSIGNED_BYTE;
        }
		break;
	case IMAGE_TYPE_RGB24:
        {
		    gl_inter	= 3;
		    gl_format	= GL_RGB;
		    gl_datatype	= GL_UNSIGNED_BYTE;
        }
		break;
	case IMAGE_TYPE_BGR24:
        {
		    gl_inter	= 3;
		    gl_format	= GL_BGR;
		    gl_datatype	= GL_UNSIGNED_BYTE;
        }
		break;
	case IMAGE_TYPE_RGB8_332:
        {
		    gl_inter	= GL_R3_G3_B2;
		    gl_format	= GL_RGB;
		    gl_datatype	= GL_UNSIGNED_BYTE_3_3_2;
        }
		break;
	default:
		return FALSE;
	}
    return TRUE;
}

DWORD OpenGLRollingMode::Thread_Timer( void* pvoid )
{
    OpenGLRollingMode* pThis = ( OpenGLRollingMode* )pvoid;

    BOOL bRun = TRUE;

    while ( bRun )
    {
        switch ( ::WaitForSingleObject( pThis->m_hExit, pThis->m_dwMilliSeconds ) )
        {
        case WAIT_TIMEOUT:
            {
#if _MSC_VER >= 1800
                std::lock_guard< std::mutex > lock( pThis->m_mutex );
#endif
                for ( int i = NULL; i < MAX_ROLLING_TEXTURE; i++ )
                {
                    if ( pThis->m_xRollingInfo[ i ].iPos == pThis->m_iUnVisiblePos )
                    {
                        pThis->m_xRollingInfo[ i ].bEmpty = TRUE;
                        pThis->m_xRollingInfo[ i ].iPos   = pThis->m_iLastPos;
                    }
                    pThis->m_xRollingInfo[ i ].iPos--;
                }
                pThis->m_pOGLControl->PostMessage( WM_PAINT );
            }
            break;
        case WAIT_OBJECT_0:
            {
                bRun = FALSE;
            }
            break;
        }
    }
    return NULL;
}