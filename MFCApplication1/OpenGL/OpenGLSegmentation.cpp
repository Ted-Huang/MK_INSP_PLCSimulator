#include "stdafx.h"
#include "OpenGLSegmentation.h"
#include "OpenGLControl.h"

#include "aoiconst.h"

#define COLORPALETTE_BYTES 1024
#define COLORPALETTE_SIZE  256

const float COLOR_MARKER_R = 200 / 255.0f;
const float COLOR_MARKER_G = 200 / 255.0f;
const float COLOR_MARKER_B = 0 / 255.0f;

OpenGLSegmentation::OpenGLSegmentation() : m_eMode( MODE_SEGMENTATION ),
                                           m_pWnd( NULL ),
                                           m_pBMPInfo( NULL ),
                                           m_pBMPData( NULL ),
                                           m_dwBMPDataLen( NULL ),
                                           m_bInitial( FALSE ),
                                           m_iSrcWidth( NULL ),
                                           m_iSrcHeight( NULL ),
                                           m_iTextureWidth( NULL ),
                                           m_iTextureHeight( NULL ),
                                           m_pBackColor( NULL ),
                                           m_pfScale( NULL ),
                                           m_pfTranslateX( NULL ),
                                           m_pfTranslateY( NULL ),
                                           m_iSplitWidth( NULL ),
                                           m_iSplitHeight( NULL ),
                                           m_fProjectRate( NULL )
{
    for ( int i = NULL; i < MAX_SEGMENTATION; i++ ) memset( &m_iCenterPt[ i ], NULL, sizeof( POINT ) );
}

OpenGLSegmentation::~OpenGLSegmentation()
{
    if ( m_pBMPInfo ) delete[] m_pBMPInfo;
    if ( m_pBMPData ) delete[] m_pBMPData;
}

void OpenGLSegmentation::AttachData( const int iSplitWidth, const int iSplitHeight, COLORREF* pBackColor, const float* pfScale, const float* pfTranslateX, const float* pfTranslateY )
{
    if ( !m_pBackColor   ) m_pBackColor   = pBackColor;
    if ( !m_pfScale      ) m_pfScale      = pfScale;
    if ( !m_pfTranslateX ) m_pfTranslateX = pfTranslateX;
    if ( !m_pfTranslateY ) m_pfTranslateY = pfTranslateY;
    if ( !m_iSplitWidth  ) m_iSplitWidth  = iSplitWidth;
    if ( !m_iSplitHeight ) m_iSplitHeight = iSplitHeight;
}

void OpenGLSegmentation::SetSegmentTexture( const _IMAGE *pSrcImg, const MODE_TEXTURE eMode, CWnd* pWnd )
{
    int iMaxTextureSize = NULL;

    if ( !m_bInitial )
    {
        m_bInitial = TRUE;

        glGenTextures( MAX_SEGMENTATION, m_iSegmentTexture );
    }
    glEnable( GL_TEXTURE_2D );
    glGetIntegerv( GL_MAX_TEXTURE_SIZE, &iMaxTextureSize );

    m_pWnd           = pWnd;
    m_eMode          = eMode;
    m_iTextureCountX = 1;
    m_iTextureCountY = 1;
    m_iSrcWidth      = pSrcImg->data_w;
    m_iSrcHeight     = pSrcImg->data_h;

    if ( ( float )m_iSplitWidth / m_iSplitHeight > ( float )m_iSrcWidth / m_iSrcHeight )
    {
        m_fProjectRate = ( float )m_iSplitHeight / m_iSrcHeight;
	}
    else
    {
        m_fProjectRate = ( float )m_iSplitWidth / m_iSrcWidth;
    }
    if ( MODE_SEGMENTATION == eMode ) SegmentatModeTexture( pSrcImg, iMaxTextureSize );
    else                              ScaleModeTexture( pSrcImg, iMaxTextureSize );
}

void OpenGLSegmentation::UnsetSegmentTexture()
{
    if ( IsSegmentation() )
    {
        m_bInitial = FALSE;

        glDeleteTextures( MAX_SEGMENTATION, m_iSegmentTexture );
    }
}

void OpenGLSegmentation::SegmentatModeTexture( const _IMAGE *pSrcImg, const int iMaxTextureSize )
{
    while ( pSrcImg->data_w / m_iTextureCountX > iMaxTextureSize ) m_iTextureCountX <<= 1;
    while ( pSrcImg->data_h / m_iTextureCountY > iMaxTextureSize ) m_iTextureCountY <<= 1;

    BYTE* pClipImage     = NULL;
    int   gl_inter       = NULL;
    int   gl_format      = NULL;
    int   gl_datatype    = NULL;
    int   iStretchWidth  = int( m_iSrcWidth  * m_fProjectRate ) / m_iTextureCountX;
    int   iStretchHeight = int( m_iSrcHeight * m_fProjectRate ) / m_iTextureCountY;

    if ( m_iTextureCountX * m_iTextureCountY > MAX_SEGMENTATION )
    {
        ::OutputDebugString( _T( "Out of MAX_SEGMENTATION" ) );

        ASSERT( FALSE );
    }
    if ( ( m_iTextureCountX > 1 && pSrcImg->data_w % ( m_iTextureCountX * 4 ) ) ||
         ( m_iTextureCountY > 1 && pSrcImg->data_h % m_iTextureCountY ) )
    {
        ::OutputDebugString( _T( "Segment length are not 4x" ) );

        ASSERT( FALSE );
    }
    m_iTextureWidth  = pSrcImg->data_w / m_iTextureCountX;
    m_iTextureHeight = pSrcImg->data_h / m_iTextureCountY;

    if ( !GetImageInfo( pSrcImg, gl_inter, gl_format, gl_datatype ) ) return;

    if ( pSrcImg->ptr ) pClipImage = GetBuffData( ImageTypeToPixelBytes( pSrcImg->type ) );

    const int iXpos = ( iStretchWidth - ( iStretchWidth * m_iTextureCountX ) ) / 2;

    int iYPos = ( iStretchHeight - ( iStretchHeight * m_iTextureCountY ) ) / 2;

    for ( int i = NULL; i < m_iTextureCountY; i ++ )
    {
        int iXPosTemp = iXpos;

        for ( int j = NULL; j < m_iTextureCountX; j ++ )
        {
            const int iIdx = i * m_iTextureCountX + j;

            m_iCenterPt[ iIdx ].x = iXPosTemp;
            m_iCenterPt[ iIdx ].y = iYPos - 1;

            if ( pClipImage ) ClipImage( pClipImage, m_iTextureWidth, m_iTextureHeight, j * m_iTextureWidth, i * m_iTextureHeight, pSrcImg );

            glBindTexture( GL_TEXTURE_2D, m_iSegmentTexture[ iIdx ] );

		    glPixelStorei( GL_UNPACK_ALIGNMENT, ( pSrcImg->data_w & 0x03 ? 1 : 4 ) );

            glTexImage2D( GL_TEXTURE_2D, NULL, gl_inter, m_iTextureWidth, m_iTextureHeight, NULL, gl_format, gl_datatype, pClipImage );

		    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_BORDER );
		    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_BORDER );
            glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST         );
            glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST         );

            iXPosTemp += iStretchWidth;
        }
        iYPos += iStretchHeight;
    }
}

void OpenGLSegmentation::ScaleModeTexture( const _IMAGE *pSrcImg, const int iMaxTextureSize )
{
    int gl_inter    = NULL;
    int gl_format   = NULL;
    int gl_datatype = NULL;

    if ( !GetImageInfo( pSrcImg, gl_inter, gl_format, gl_datatype ) ) return;

    m_iTextureWidth  = ( m_iSrcWidth  > m_iSrcHeight ) ? iMaxTextureSize : iMaxTextureSize * m_iSrcWidth  / m_iSrcHeight;
    m_iTextureHeight = ( m_iSrcHeight > m_iSrcWidth  ) ? iMaxTextureSize : iMaxTextureSize * m_iSrcHeight / m_iSrcWidth;

    glBindTexture( GL_TEXTURE_2D, m_iSegmentTexture[ NULL ] );

	glPixelStorei( GL_UNPACK_ALIGNMENT, ( m_iTextureWidth & 0x03 ? 1 : 4 ) );

    glTexImage2D( GL_TEXTURE_2D, NULL, gl_inter, m_iTextureWidth, m_iTextureHeight, NULL, gl_format, gl_datatype, NULL );

	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_BORDER );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_BORDER );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST         );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST         );

    if ( pSrcImg->ptr ) ScaleModeUpdate( pSrcImg, NULL, NULL );
}

BOOL OpenGLSegmentation::UpdateSegmentTexture( const _IMAGE *pSrcImg, const int iXoffset, const int iYoffset )
{
    return ( MODE_SEGMENTATION == m_eMode ) ? SegmentatModeUpdate( pSrcImg, iXoffset, iYoffset ) : ScaleModeUpdate( pSrcImg, iXoffset, iYoffset );
}

BOOL OpenGLSegmentation::SegmentatModeUpdate( const _IMAGE *pSrcImg, const int iXoffset, const int iYoffset )
{
    int gl_inter      = NULL;
    int gl_format     = NULL;
    int gl_datatype   = NULL;
    int iClipWidth    = NULL;
    int iClipHeight   = NULL;
    int iClipXStart   = NULL;
    int iClipYStart   = NULL;
    int iUpdateWidth  = NULL;
    int iUpdateHeight = pSrcImg->data_h;
    int iTextureXPos  = iXoffset;
    int iTextureYPos  = iYoffset;

    if ( !GetImageInfo( pSrcImg, gl_inter, gl_format, gl_datatype ) ) return FALSE;

    BYTE* pClipImage = GetBuffData( ImageTypeToPixelBytes( pSrcImg->type ) );

    for ( int i = NULL; i < m_iTextureCountY; i++ )
    {
        if ( NULL == iUpdateHeight )
        {
            break;
        }
        if ( iTextureYPos >= m_iTextureHeight ) // find current Y-texture
        {
            iTextureYPos -= m_iTextureHeight;

            continue;
        }
        iClipHeight  = m_iTextureHeight - iTextureYPos;

        if ( iClipHeight > iUpdateHeight ) iClipHeight = iUpdateHeight;

        iTextureYPos = m_iTextureHeight - iClipHeight - iTextureYPos;
        iTextureXPos = iXoffset;
        iUpdateWidth = pSrcImg->data_w;
        iClipXStart  = NULL;

        for ( int j = NULL; j < m_iTextureCountX; j++ )
        {
            if ( NULL == iUpdateWidth )
            {
                break;
            }
            if ( iTextureXPos >= m_iTextureWidth ) // find current X-texture
            {
                iTextureXPos -= m_iTextureWidth;

                continue;
            }
            iClipWidth = m_iTextureWidth - iTextureXPos;

            if ( iClipWidth > iUpdateWidth ) iClipWidth = iUpdateWidth;

            ClipImage( pClipImage, iClipWidth, iClipHeight, iClipXStart, iClipYStart, pSrcImg ); // partial update

            glBindTexture( GL_TEXTURE_2D, m_iSegmentTexture[ m_iTextureCountX * i + j ] );

            glTexSubImage2D( GL_TEXTURE_2D, NULL, iTextureXPos, iTextureYPos, iClipWidth, iClipHeight, gl_format, gl_datatype, pClipImage );

            iClipXStart  += iClipWidth;
            iUpdateWidth -= iClipWidth;

            iTextureXPos = NULL;
        }
        iClipYStart   += iClipHeight;
        iUpdateHeight -= iClipHeight;

        iTextureYPos = NULL;
    }
	return glGetError() == GL_NO_ERROR;
}

BOOL OpenGLSegmentation::ScaleModeUpdate( const _IMAGE *pSrcImg, const int iXoffset, const int iYoffset )
{
    if ( !pSrcImg->ptr ) return FALSE;

    int         gl_inter           = NULL;
    int         gl_format          = NULL;
    int         gl_datatype        = NULL;
    BYTE*       pScaleImage        = NULL;
    BITMAPINFO* pBMInfo            = GetBMPInfo();
    const int   iColorSpace        = ImageTypeToPixelBytes( pSrcImg->type );
    const int   iStretchWidth      = pSrcImg->data_w * m_iTextureWidth  / m_iSrcWidth;
    const int   iStretchHeight     = pSrcImg->data_h * m_iTextureHeight / m_iSrcHeight;
    const int   iStretchOffsetX    = iXoffset        * m_iTextureWidth  / m_iSrcWidth;
    const int   iStretchOffsetY    = iYoffset        * m_iTextureHeight / m_iSrcHeight;

    pBMInfo->bmiHeader.biBitCount  = iColorSpace == 1 ? 8 : 24;
    pBMInfo->bmiHeader.biWidth     = iStretchWidth;
    pBMInfo->bmiHeader.biHeight    = iStretchHeight;
    pBMInfo->bmiHeader.biSizeImage = pBMInfo->bmiHeader.biWidth * pBMInfo->bmiHeader.biHeight * iColorSpace;

    CDC*    pDC = m_pWnd->GetDC();
    HDC     hdc = ::CreateCompatibleDC( *pDC );
    HBITMAP hBM = ::CreateDIBSection( *pDC, pBMInfo, DIB_RGB_COLORS, ( void** )&pScaleImage, NULL, NULL );

    if ( !GetImageInfo( pSrcImg, gl_inter, gl_format, gl_datatype ) ) return FALSE;

    if ( !pScaleImage ) return FALSE;

    pBMInfo->bmiHeader.biWidth     = pSrcImg->data_w;
    pBMInfo->bmiHeader.biHeight    = pSrcImg->data_h;
    pBMInfo->bmiHeader.biSizeImage = pSrcImg->data_w * pSrcImg->data_h * iColorSpace;

    ::SetStretchBltMode( hdc, STRETCH_DELETESCANS );
    ::SelectObject( hdc, hBM );
    ::StretchDIBits( hdc, NULL, NULL, iStretchWidth, iStretchHeight, NULL, NULL, pSrcImg->data_w, pSrcImg->data_h, pSrcImg->ptr, pBMInfo, DIB_RGB_COLORS, SRCCOPY );

    glBindTexture( GL_TEXTURE_2D, m_iSegmentTexture[ NULL ] );

    glTexSubImage2D( GL_TEXTURE_2D, NULL, iStretchOffsetX, iStretchOffsetY, iStretchWidth, iStretchHeight, gl_format, gl_datatype, pScaleImage );

    ::DeleteObject( hBM );
    ::DeleteDC( hdc );

    m_pWnd->ReleaseDC( pDC );

    return glGetError() == GL_NO_ERROR;
}

void OpenGLSegmentation::DrawSegmentMarker()
{
    glDisable( GL_TEXTURE_2D );
	
    glPushMatrix();

	glTranslatef( float( m_iSplitWidth - 15 ), float( m_iSplitHeight - 15 ), 0.0f );
    glTranslatef( -0.5f * m_iSplitWidth, -0.5f * m_iSplitHeight, 0.0f );
	glScalef(1.0f, 1.0f, 1.0f);

	glBegin( GL_QUADS );
        glColor3f( COLOR_MARKER_R, COLOR_MARKER_G, COLOR_MARKER_B );
		glVertex3i( 0, 0, 0 );
		glVertex3i( 0, 8, 0 );
		glVertex3i( 8, 8, 0 );
		glVertex3i( 8, 0, 0 );
	glEnd();

    glPopMatrix();
}

void OpenGLSegmentation::DrawSegment()
{
    const int iTotalCount    = m_iTextureCountX * m_iTextureCountY;
    const int iProjectWidth  = ( MODE_SEGMENTATION == m_eMode ) ? m_iTextureWidth  : m_iSrcWidth;
    const int iProjectHeight = ( MODE_SEGMENTATION == m_eMode ) ? m_iTextureHeight : m_iSrcHeight;

    DrawSegmentMarker();

    glDisable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );

    for ( int i = iTotalCount - 1; i > EOF; i-- )
    {
        glPushMatrix();

        glScalef( *m_pfScale, *m_pfScale, 1.0f );
        glScalef( m_fProjectRate, m_fProjectRate, 1.0f );

        glTranslatef( ( *m_pfTranslateX + m_iCenterPt[ i ].x ) / m_fProjectRate, ( *m_pfTranslateY + m_iCenterPt[ i ].y ) / m_fProjectRate, 0.0f );
        glTranslatef( -0.5f * iProjectWidth, -0.5f * iProjectHeight, 0.0f );
    
        glGetDoublev( GL_MODELVIEW_MATRIX,  m_modelview  );
        glGetDoublev( GL_PROJECTION_MATRIX, m_projection );

        glBindTexture( GL_TEXTURE_2D, m_iSegmentTexture[ i ] );

        glBegin( GL_QUADS );
		    glColor3f( 1, 1, 1 );
		    glTexCoord2f( 0.0f, 0.0f );

		    glVertex3f( 0.0f, 0.0f, 0.0f );
		    glTexCoord2f( 0.0f, 1.0f );

		    glVertex3f( 0.0f, iProjectHeight * 1.0f, 0.0f );
		    glTexCoord2f( 1.0f, 1.0f );

		    glVertex3f( iProjectWidth * 1.0f, iProjectHeight * 1.0f, 0.0f ) ;
		    glTexCoord2f( 1.0f, 0.0f );

		    glVertex3f( iProjectWidth * 1.0f, 0.0f, 0.0f );
	    glEnd();

        glPopMatrix();
    }
    glScalef( *m_pfScale, *m_pfScale, 1.0f );
    glScalef( m_fProjectRate, m_fProjectRate, 1.0f );

    glTranslatef( ( *m_pfTranslateX + m_iCenterPt[ NULL ].x ) / m_fProjectRate, ( *m_pfTranslateY + m_iCenterPt[ NULL ].y ) / m_fProjectRate, 0.0f );
    glTranslatef( -0.5f * iProjectWidth, -0.5f * iProjectHeight, 0.0f );
}

void OpenGLSegmentation::DrawSegmentMinimap()
{
    const int iTotalCount    = m_iTextureCountX * m_iTextureCountY;
    const int iProjectWidth  = ( MODE_SEGMENTATION == m_eMode ) ? m_iTextureWidth  : m_iSrcWidth;
    const int iProjectHeight = ( MODE_SEGMENTATION == m_eMode ) ? m_iTextureHeight : m_iSrcHeight;

	glDisable( GL_BLEND );
	glDisable( GL_SCISSOR_TEST );
	glDisable( GL_TEXTURE_2D );

	glLoadIdentity();

	glTranslatef( -0.5f * m_iSplitWidth, -0.5f * m_iSplitHeight, -42.0f );

	glScalef(MINIMAP_SHRINK, MINIMAP_SHRINK, 1.0f);

	glBegin( GL_QUADS );
        glColor3f( ( float )GetRValue( *m_pBackColor ) / 255.0f,
			       ( float )GetGValue( *m_pBackColor ) / 255.0f,
			       ( float )GetBValue( *m_pBackColor ) / 255.0f);
		glVertex3i( 0, 0, 0 );
		glVertex3i( 0, m_iSplitHeight, 0 );
		glVertex3i( m_iSplitWidth, m_iSplitHeight, 0 );
		glVertex3i( m_iSplitWidth, 0, 0 );
	glEnd();

	glEnable( GL_TEXTURE_2D );

    for ( int i = iTotalCount - 1; i > EOF; i-- )
    {
	    glLoadIdentity();

        glPushMatrix();

        glTranslatef( -0.5f * m_iSplitWidth, -0.5f * m_iSplitHeight, -41.0f );

        glScalef( MINIMAP_SHRINK, MINIMAP_SHRINK, 1.0f );

	    glTranslatef( 0.5f * m_iSplitWidth, 0.5f * m_iSplitHeight, 0.0f);

        glScalef( m_fProjectRate, m_fProjectRate, 1.0f );

        glTranslatef( m_iCenterPt[ i ].x / m_fProjectRate, m_iCenterPt[ i ].y / m_fProjectRate, 0.0f );       
	    glTranslatef( -0.5f * iProjectWidth, -0.5f * iProjectHeight, 0.0f );

        glBindTexture( GL_TEXTURE_2D, m_iSegmentTexture[ i ] );

	    glBegin( GL_QUADS );
		    glColor3f( 1, 1, 1 );
		    glTexCoord2f( 0.0f, 0.0f );

		    glVertex3f( 0.0f, 0.0f, 0.0f );
		    glTexCoord2f( 0.0f, 1.0f );

		    glVertex3f( 0.0f, iProjectHeight * 1.0f, 0.0f );
		    glTexCoord2f( 1.0f, 1.0f );

		    glVertex3f( iProjectWidth * 1.0f, iProjectHeight * 1.0f, 0.0f ) ;
		    glTexCoord2f( 1.0f, 0.0f );

		    glVertex3f( iProjectWidth * 1.0f, 0.0f, 0.0f );
	    glEnd();

        glPopMatrix();
    }
	if ( *m_pfScale > 1.0f ) // Thin line frame
    {
		glLoadIdentity();

		glTranslatef( -0.5f * m_iSplitWidth, -0.5f * m_iSplitHeight, -40.0f );

		glScalef(MINIMAP_SHRINK, MINIMAP_SHRINK, 1.0f);

		glTranslatef( 0.5f * m_iSplitWidth, 0.5f * m_iSplitHeight, 0.0f);

        glTranslatef( -*m_pfTranslateX * 1.0f, -*m_pfTranslateY * 1.0f, 0.0f );

		glScalef( 1.0f / *m_pfScale, 1.0f / *m_pfScale, 1.0f );

		glDisable( GL_TEXTURE_2D );
		glBegin( GL_LINE_LOOP );
			glColor3f( 1.0f, 1.0f, 1.0f );
			glVertex3f( -0.5f * m_iSplitWidth, -0.5f * m_iSplitHeight, 0.0f );
			glVertex3f( -0.5f * m_iSplitWidth,  0.5f * m_iSplitHeight, 0.0f );
			glVertex3f(  0.5f * m_iSplitWidth,  0.5f * m_iSplitHeight, 0.0f );
			glVertex3f(  0.5f * m_iSplitWidth, -0.5f * m_iSplitHeight, 0.0f );
		glEnd();
	}
}

void OpenGLSegmentation::GetWindowToImage( const int* pView, const float& fDepth, const int iWindowX, const int iWindowY, double& dbImageX, double& dbImageY )
{
    static double z;

    gluUnProject( iWindowX, m_iSplitHeight - iWindowY, fDepth, m_modelview, m_projection, pView, &dbImageX, &dbImageY, &z );

    if ( MODE_SCALE == m_eMode )
    {
        dbImageX = dbImageX * m_iSrcWidth  / m_iTextureWidth;
        dbImageY = dbImageY * m_iSrcHeight / m_iTextureHeight;
    }
}

void OpenGLSegmentation::GetImageToWindow( const int* pView, const float& fDepth, const int iImageX, const int iImageY, double& iWindowX, double& iWindowY )
{
    static double z;

    int iImageXp = iImageX;
    int iImageYp = iImageY;

    if ( MODE_SCALE == m_eMode )
    {
        iImageXp = iImageX * m_iTextureWidth  / m_iSrcWidth;
        iImageYp = iImageY * m_iTextureHeight / m_iSrcHeight;
    }
    gluProject( ( iImageXp * m_iTextureWidth / m_iSrcWidth ) + 0.5, ( iImageYp * m_iTextureHeight / m_iSrcHeight ) + 0.5, fDepth, m_modelview, m_projection, pView, &iWindowX, &iWindowY, &z );
}

BYTE* OpenGLSegmentation::GetBuffData( const int iColorSpace )
{
    const DWORD dwImageSize = m_iTextureWidth * m_iTextureHeight * iColorSpace;

    if ( m_dwBMPDataLen < dwImageSize )
    {
        if ( m_pBMPData ) delete[] m_pBMPData;

        m_dwBMPDataLen = dwImageSize;
        m_pBMPData     = new BYTE[ dwImageSize ];
    }
    return m_pBMPData;
}

BITMAPINFO* OpenGLSegmentation::GetBMPInfo()
{
    if ( !m_pBMPInfo )
    {
        m_pBMPInfo = new BYTE[ COLORPALETTE_BYTES + sizeof( BITMAPINFO ) ];

        BITMAPINFO* pBMInfo = ( BITMAPINFO* )m_pBMPInfo;

        memset( pBMInfo, NULL, sizeof( BITMAPINFO ) );

        pBMInfo->bmiHeader.biSize    = sizeof( BITMAPINFOHEADER );
        pBMInfo->bmiHeader.biPlanes  = 1;
        pBMInfo->bmiHeader.biClrUsed = COLORPALETTE_SIZE;

        memcpy( &pBMInfo->bmiColors, ctMonoColorTable, COLORPALETTE_BYTES );
    }
    return ( BITMAPINFO* )m_pBMPInfo;
}

BOOL OpenGLSegmentation::GetImageInfo( const _IMAGE *pSrcImg, int& gl_inter, int& gl_format, int& gl_datatype )
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

void OpenGLSegmentation::ClipImage( BYTE* pClipImage, const int iClipImageWidth, const int iClipImageHeight, const int iXPos, const int iYPos, const _IMAGE* pSrcImg )
{
    const int iColorSpace  = ImageTypeToPixelBytes( pSrcImg->type );
    const int iSrcPitch    = pSrcImg->data_w * iColorSpace;
    const int iClipPitch   = iClipImageWidth * iColorSpace;

    const BYTE* pSrc = &pSrcImg->ptr[ iYPos * iSrcPitch + ( iXPos * iColorSpace ) ];

    for ( int i = NULL; i < iClipImageHeight; i++ )
    {
        memcpy( pClipImage, pSrc, iClipPitch );

        pClipImage += iClipPitch;

        pSrc += iSrcPitch;
    }
}
