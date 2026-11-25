/*
 Copyright (c) 2012, The Cinder Project, All rights reserved.

 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "cinder/app/AppImplMswRendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"
#include "cinder/Camera.h"
#include <windowsx.h>
#include <map>

namespace cinder { namespace app {

bool sMultisampleSupported = false;
int sArbMultisampleFormat;

AppImplMswRendererGl::AppImplMswRendererGl( App *aApp, RendererGl *aRenderer )
	: AppImplMswRenderer( aApp ), mRenderer( aRenderer )
{
	mPrevRC = 0;
	mRC = 0;
}

void AppImplMswRendererGl::prepareToggleFullScreen()
{
	mPrevRC = mRC;
	mWasVerticalSynced = gl::isVerticalSyncEnabled();
}

void AppImplMswRendererGl::finishToggleFullScreen()
{
	if( mPrevRC ) {
		::wglDeleteContext( mPrevRC );
	}

	gl::enableVerticalSync( mWasVerticalSynced );
}

void AppImplMswRendererGl::defaultResize() const
{
	::RECT clientRect;
	::GetClientRect( mWnd, &clientRect );
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;

	glViewport( 0, 0, width, height );
	cinder::CameraPersp cam( width, height, 60.0f );

	glMatrixMode( GL_PROJECTION );
	glLoadMatrixf( cam.getProjectionMatrix().m );

	glMatrixMode( GL_MODELVIEW );
	glLoadMatrixf( cam.getModelViewMatrix().m );
	glScalef( 1.0f, -1.0f, 1.0f );           // invert Y axis so increasing Y goes down.
	glTranslatef( 0.0f, (float)-height, 0.0f );       // shift origin up to upper-left corner.
}

void AppImplMswRendererGl::swapBuffers() const
{
	::SwapBuffers( mDC );
}

void AppImplMswRendererGl::makeCurrentContext()
{
	::wglMakeCurrent( mDC, mRC );
}

HWND createDummyWindow( int *width, int *height, bool fullscreen )
{
	DWORD windowExStyle, windowStyle;
	HWND wnd;
	const char *title = "flint";
	int bits = 32;

	if( *width <= 0 ) {
		*width = ::GetSystemMetrics( SM_CXSCREEN );
		*height = ::GetSystemMetrics( SM_CYSCREEN );
	}

	WNDCLASS	wc;						// Windows Class Structure
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = 0L;
	WindowRect.right = (long)*width;
	WindowRect.top = 0L;
	WindowRect.bottom = (long)*height;

	HINSTANCE instance	= ::GetModuleHandle( NULL );				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= DefWindowProc;						// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= instance;
	wc.hIcon			= ::LoadIcon( NULL, IDI_WINLOGO );		// Load The Default Icon
	wc.hCursor			= ::LoadCursor( NULL, IDC_ARROW );		// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= TEXT("FLINTDUMMY");

	if( ! ::RegisterClass( &wc ) ) {								// Attempt To Register The Window Class
		DWORD err = ::GetLastError();
		return 0;											
	}

	if( fullscreen ) {
		windowExStyle = WS_EX_APPWINDOW;
		windowStyle = WS_POPUP;
		::ShowCursor( TRUE );
	}
	else {
		windowExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		windowStyle = ( WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME );
	}

	::AdjustWindowRectEx( &WindowRect, windowStyle, FALSE, windowExStyle );		// Adjust Window To True Requested Size

	wchar_t unicodeTitle[1024]; 
	wsprintfW( unicodeTitle, L"%S", title );

	// Create The Window
	if( ! ( wnd = ::CreateWindowEx( windowExStyle,						// Extended Style For The Window
		TEXT("FLINTDUMMY"),
		unicodeTitle,						// Window Title
		windowStyle,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right-WindowRect.left,	// Calculate Window Width
		WindowRect.bottom-WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		instance,							// Instance
		0 )) )

	{
		return 0;								
	}

	return wnd;
}

// ROGER (Windows port): the pixel format is chosen per window (per renderer AA level). It used to be kept in
// static globals once the first MSAA window was created, so every later window (AA_NONE too) got the MSAA format.
// Depth 24 / alpha 8 / stencil 8 in both paths (was depth 16, no alpha, no stencil). Cocoa asks for depth 24 (alpha and stencil not requested).
namespace {

PIXELFORMATDESCRIPTOR makePixelFormatDescriptor()
{
	PIXELFORMATDESCRIPTOR pfd;
	memset( &pfd, 0, sizeof( pfd ) );
	pfd.nSize = sizeof( PIXELFORMATDESCRIPTOR );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;
	return pfd;
}

// requested AA level -> ARB pixel format (0 if none) and the level obtained
struct MsaaPixelFormat { int pixelFormat; int level; };
std::map<int, MsaaPixelFormat>	sMsaaPixelFormats;

} // anonymous namespace

bool AppImplMswRendererGl::initialize( HWND wnd, HDC dc, RendererRef sharedRenderer )
{
	const int requestedLevel = mRenderer->getAntiAliasing();
	int pixelFormat = 0;

	if( requestedLevel > RendererGl::AA_NONE ) {
		auto found = sMsaaPixelFormats.find( requestedLevel );
		if( found == sMsaaPixelFormats.end() ) {
			// first create a dummy window with a plain context, needed to call wglChoosePixelFormatARB
			MsaaPixelFormat result = { 0, RendererGl::AA_NONE };
			int width = 640;
			int height = 480;
			HGLRC prevRC = ::wglGetCurrentContext();
			HDC prevDC = ::wglGetCurrentDC();
			HWND dummyWnd = createDummyWindow( &width, &height, false );
			if( dummyWnd ) {
				HDC dummyDC = ::GetDC( dummyWnd );
				PIXELFORMATDESCRIPTOR pfd = makePixelFormatDescriptor();
				int dummyFormat = ::ChoosePixelFormat( dummyDC, &pfd );
				if( dummyFormat && ::SetPixelFormat( dummyDC, dummyFormat, &pfd ) ) {
					HGLRC dummyRC = ::wglCreateContext( dummyDC );
					if( dummyRC && ::wglMakeCurrent( dummyDC, dummyRC ) ) {
						result.level = initMultisample( pfd, requestedLevel, dummyDC );
						result.pixelFormat = sArbMultisampleFormat;
						if( result.level <= RendererGl::AA_NONE )
							result.pixelFormat = 0;
					}
					::wglMakeCurrent( NULL, NULL );
					if( dummyRC )
						::wglDeleteContext( dummyRC );
				}
				::ReleaseDC( dummyWnd, dummyDC );
				::DestroyWindow( dummyWnd );
			}
			::UnregisterClass( TEXT("FLINTDUMMY"), ::GetModuleHandle( NULL ) );
			if( prevRC )
				::wglMakeCurrent( prevDC, prevRC );
			found = sMsaaPixelFormats.insert( std::make_pair( requestedLevel, result ) ).first;
		}
		pixelFormat = found->second.pixelFormat;
		mRenderer->setAntiAliasing( found->second.level );
	}

	RendererGl *sharedRendererGl = dynamic_cast<RendererGl*>( sharedRenderer.get() );
	HGLRC sharedRC = ( sharedRendererGl && sharedRendererGl->mImpl ) ? sharedRendererGl->mImpl->mRC : NULL;

	return initializeInternal( wnd, dc, sharedRC, pixelFormat );
}

bool AppImplMswRendererGl::initializeInternal( HWND wnd, HDC dc, HGLRC sharedRC, int pixelFormat )
{
	mWnd = wnd;
	mDC = dc;

	PIXELFORMATDESCRIPTOR pfd = makePixelFormatDescriptor();

	if( pixelFormat == 0 ) {
		pixelFormat = ::ChoosePixelFormat( dc, &pfd );				// Find A Compatible Pixel Format
		if( pixelFormat == 0 ) {									// Did We Find A Compatible Format?
			return false;
		}
	}
	else
		::DescribePixelFormat( dc, pixelFormat, sizeof( pfd ), &pfd );	// ARB (multisample) format

	if( ! ::SetPixelFormat( dc, pixelFormat, &pfd ) ) {		// Are We Able To Set The Pixel Format?
		return false;
	}

	if( ! ( mRC = ::wglCreateContext( dc ) ) )	{			// Are We Able To Get A Rendering Context?
		return false;
	}

	// share before the context is made current or used (wglShareLists requires a context without objects)
	if( mPrevRC )
		::wglShareLists( mPrevRC, mRC );

	if( sharedRC )
		::wglShareLists( sharedRC, mRC );

	if( ! ::wglMakeCurrent( dc, mRC ) ){					// Try To Activate The Rendering Context
		return false;
	}

	if( mPrevRC )
		BOOL success = ::wglCopyContext( mPrevRC, mRC, GL_ALL_ATTRIB_BITS );

	return true;									// Success
}

int AppImplMswRendererGl::initMultisample( PIXELFORMATDESCRIPTOR pfd, int requestedLevelIdx, HDC dc )
{
	// GLee: wglChoosePixelFormatARB is resolved on first call (needs a current context)
	int	pixelFormat;
	int	valid;
	UINT numFormats;
	float fAttributes[] = {0,0};

	int iAttributes[] =
	{
		WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
		WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB,24,
		WGL_ALPHA_BITS_ARB,8,
		WGL_DEPTH_BITS_ARB,24,		// ROGER: was 16
		WGL_STENCIL_BITS_ARB,8,		// ROGER: was 0
		WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
		WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
		WGL_SAMPLES_ARB, 0,
		0,0
	};

	// iterate down the levels until one sticks
	for( int levelIdx = requestedLevelIdx; levelIdx > RendererGl::AA_NONE; --levelIdx ) {
		iAttributes[19] = RendererGl::sAntiAliasingSamples[levelIdx];

		numFormats = 0;
		valid = ::wglChoosePixelFormatARB( dc, iAttributes, fAttributes, 1, &pixelFormat, &numFormats );
		if( valid && ( numFormats >= 1 ) ) {
			sMultisampleSupported = true;
			sArbMultisampleFormat = pixelFormat;
			return levelIdx;
		}
	}

	// Return failure if we hit level 0
	return 0;
}

void AppImplMswRendererGl::kill()
{
	if( mRC ) {											// Do We Have A Rendering Context?
		// ROGER: only unbind it if it is the current one (closing a window used to leave no context current at all)
		if( ::wglGetCurrentContext() == mRC )
			::wglMakeCurrent( NULL, NULL );				// release The DC And RC Contexts
		::wglDeleteContext( mRC );						// delete The RC
	}

	mRC = 0;
}

} } // namespace cinder::app
