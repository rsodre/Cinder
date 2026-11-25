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

#include "cinder/app/AppImplMswBasic.h"
#include "cinder/app/AppBasic.h"
#include "cinder/app/AppImplMswRenderer.h"
#include "cinder/app/Renderer.h"
#include "cinder/Utilities.h"

#include <windowsx.h>
#include <winuser.h>
#include <shellapi.h>
#include <algorithm>
#include <vector>

using std::vector;
using std::string;

namespace cinder { namespace app {

AppImplMswBasic::AppImplMswBasic( AppBasic *aApp )
	: AppImplMsw( aApp ), mApp( aApp )
{
	mShouldQuit = false;
}

void AppImplMswBasic::run()
{
	mFrameRate = mApp->getSettings().getFrameRate();
	mFrameRateEnabled = mApp->getSettings().isFrameRateEnabled();

	auto formats = mApp->getSettings().getWindowFormats();
	if( formats.empty() )
		formats.push_back( mApp->getSettings().getDefaultWindowFormat() );

	// ROGER (Windows port): Settings::rememberMainWindowPos(), restored before the main window is created
	const bool rememberPos = mApp->getSettings().getRememberMainWindowPos();
	const std::string appName = rememberPos ? mApp->getAppName() : std::string();
	bool maximizeMainWindow = false;

	for( auto format = formats.begin(); format != formats.end(); ++format ) {
		if( ! format->isTitleSpecified() )
			format->setTitle( mApp->getSettings().getTitle() );
		if( rememberPos && format == formats.begin() && ! format->isFullScreen() )
			maximizeMainWindow = restoreMainWindowPos( *format, appName );
		createWindow( *format );
	}

	if( rememberPos && ! mWindows.empty() ) {
		HWND mainWnd = mWindows.front()->getHwnd();
		if( maximizeMainWindow )
			::ShowWindow( mainWnd, SW_MAXIMIZE );
		trackMainWindowPos( mainWnd, appName );
	}

	mApp->privateSetup__();
	mSetupHasBeenCalled = true;

	// issue initial app activation event
	mApp->emitDidBecomeActive();

	for( auto windowIt = mWindows.begin(); windowIt != mWindows.end(); ++windowIt )
		(*windowIt)->resize();

	// initialize our next frame time
	mNextFrameTime = getElapsedSeconds();

	bool firstFrame = true;

	// inner loop
	while( ! mShouldQuit ) {
		// update and draw
		mApp->privateUpdate__();
		// ROGER: iterate over a copy, a window can be closed (and deleted) while another one draws
		std::vector<WindowImplMswBasic*> windows( mWindows.begin(), mWindows.end() );
		for( auto windowIt = windows.begin(); windowIt != windows.end(); ++windowIt ) {
			if( std::find( mWindows.begin(), mWindows.end(), *windowIt ) != mWindows.end() )
				(*windowIt)->redraw();
		}

		// ROGER: "open document" from the command line (double-click on a document), as Cocoa's application:openFile:,
		// which a running app receives between frames
		if( firstFrame ) {
			firstFrame = false;
			emitCommandLineFileOpen();
		}

		// ROGER: remember the main window position (only writes when it changed and no move/size is in progress)
		saveMainWindowPos();

		// get current time in seconds
		double currentSeconds = mApp->getElapsedSeconds();

		// calculate time per frame in seconds
		double secondsPerFrame = 1.0 / mFrameRate;

		// determine if application was frozen for a while and adjust next frame time
		double elapsedSeconds = currentSeconds - mNextFrameTime;
		if( elapsedSeconds > 1.0 ) {
			int numSkipFrames = (int)(elapsedSeconds / secondsPerFrame);
			mNextFrameTime += (numSkipFrames * secondsPerFrame);
		}

		// determine when next frame should be drawn
		mNextFrameTime += secondsPerFrame;

		// sleep and process messages until next frame
		if( ( mFrameRateEnabled ) && ( mNextFrameTime > currentSeconds ) )
			sleep(mNextFrameTime - currentSeconds);
		else {
			MSG msg;
			while( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {
				::TranslateMessage( &msg );
				::DispatchMessage( &msg );
			}
		}
	}

//	killWindow( mFullScreen );
	saveMainWindowPos( true );	// ROGER
	mApp->emitShutdown();
	delete mApp;
}

void AppImplMswBasic::sleep( double seconds )
{
	// create waitable timer
	static HANDLE timer = ::CreateWaitableTimer( NULL, FALSE, NULL );

	// specify relative wait time in units of 100 nanoseconds
	LARGE_INTEGER waitTime;
	waitTime.QuadPart = (LONGLONG)(seconds * -10000000);
	if(waitTime.QuadPart >= 0) return;

	// activate waitable timer
	if ( !::SetWaitableTimer( timer, &waitTime, 0, NULL, NULL, FALSE ) )
		return;

	// handle events until specified time has elapsed
	DWORD result;
	MSG msg;
	while( ! mShouldQuit ) {
		result = ::MsgWaitForMultipleObjects( 1, &timer, false, INFINITE, QS_ALLINPUT );
		if( result == (WAIT_OBJECT_0 + 1) ) {
			// execute messages as soon as they arrive
			while( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {
				::TranslateMessage( &msg );
				::DispatchMessage( &msg );
			}
			// resume waiting
		}
		else return; // time has elapsed
	}
}

WindowRef AppImplMswBasic::createWindow( Window::Format format )
{
	if( ! format.getRenderer() )
		format.setRenderer( mApp->getDefaultRenderer()->clone() );

	mWindows.push_back( new WindowImplMswBasic( format, mApp->findSharedRenderer( format.getRenderer() ), this ) );

	// emit initial resize if we have fired setup
	if( mSetupHasBeenCalled )
		mWindows.back()->getWindow()->emitResize();

	return mWindows.back()->getWindow();
}

void AppImplMswBasic::closeWindow( WindowImplMsw *windowImpl )
{
	auto winIt = find( mWindows.begin(), mWindows.end(), windowImpl );
	if( winIt != mWindows.end() ) {
		// ROGER: last save of the main window position
		if( windowImpl->getHwnd() == mRememberPosWnd ) {
			saveMainWindowPos( true );
			mRememberPosWnd = NULL;
		}
		WindowRef closingWindow = windowImpl->getWindow();
		closingWindow->emitClose();
		windowImpl->privateClose();
		closingWindow->setInvalid();	// ROGER: as Cocoa; using the closed Window now throws ExcInvalidWindow instead of touching freed memory
		delete windowImpl; // this corresponds to winIt
		mWindows.erase( winIt );

		// ROGER: the active / foreground Window must not stay on the closed one (Cocoa falls back to the first window)
		if( mActiveWindow == closingWindow )
			mActiveWindow = mWindows.empty() ? WindowRef() : mWindows.front()->getWindow();
		if( mForegroundWindow == closingWindow )
			mForegroundWindow = WindowRef();
		// ROGER: keep a GL context current, so GL calls made after closing a window (e.g. in update()) still work
		if( mActiveWindow && ::wglGetCurrentContext() == NULL && mActiveWindow->getRenderer() )
			mActiveWindow->getRenderer()->makeCurrentContext();
	}

	if( mWindows.empty() && mApp->getSettings().isQuitOnLastWindowCloseEnabled() )
		mShouldQuit = true;
}

size_t AppImplMswBasic::getNumWindows() const
{
	return mWindows.size();
}

WindowRef AppImplMswBasic::getWindowIndex( size_t index )
{
	if( index >= mWindows.size() )
		return cinder::app::WindowRef();
	
	std::list<WindowImplMswBasic*>::iterator iter = mWindows.begin();
	std::advance( iter, index );
	return (*iter)->mWindowRef;
}

WindowRef AppImplMswBasic::getForegroundWindow() const
{
	return mForegroundWindow;
}

void AppImplMswBasic::setForegroundWindow( WindowRef window )
{
	mForegroundWindow = window;
}

// This creates a full-screen blanking (all black) Window on each display besides 'fullScreenDisplay'
void AppImplMswBasic::setupBlankingWindows( DisplayRef fullScreenDisplay )
{
	destroyBlankingWindows();

	for( auto displayIt = Display::getDisplays().begin(); displayIt != Display::getDisplays().end(); ++displayIt ) {
		if( *displayIt == fullScreenDisplay )
			continue;
		mBlankingWindows.push_back( BlankingWindowRef( new BlankingWindow( *displayIt ) ) );
	}
}

void AppImplMswBasic::destroyBlankingWindows()
{
	for( auto winIt = mBlankingWindows.begin(); winIt != mBlankingWindows.end(); ++winIt )
		(*winIt)->destroy();

	mBlankingWindows.clear();
}

float AppImplMswBasic::setFrameRate( float aFrameRate )
{
	mFrameRate = aFrameRate;
	mFrameRateEnabled = true;
	mNextFrameTime = mApp->getElapsedSeconds();
	return aFrameRate;
}

void AppImplMswBasic::disableFrameRate()
{
	mFrameRateEnabled = false;
}

bool AppImplMswBasic::isFrameRateEnabled() const
{
	return mFrameRateEnabled;
}

// ROGER (Windows port)
bool AppImplMswBasic::isMinimized() const
{
	if( mWindows.empty() )
		return false;
	return ::IsIconic( mWindows.front()->getHwnd() ) != FALSE;
}

void AppImplMswBasic::setFocusMainWindow()
{
	if( mWindows.empty() )
		return;
	HWND wnd = mWindows.front()->getHwnd();
	// Cocoa makes it the key window without un-minimizing it
	if( ::GetForegroundWindow() != wnd )
		::SetForegroundWindow( wnd );
	::SetFocus( wnd );
}

void AppImplMswBasic::emitCommandLineFileOpen()
{
	int numArgs = 0;
	LPWSTR *args = ::CommandLineToArgvW( ::GetCommandLineW(), &numArgs );
	if( ! args )
		return;
	vector<fs::path> files;
	for( int i = 1; i < numArgs; ++i ) {
		// wide path: exact for any character
		const std::wstring arg( args[i] );
		fs::path p( arg );
		boost::system::error_code ec;
		if( fs::is_regular_file( p, ec ) ) {
			try { p = fs::absolute( p ); }
			catch( ... ) {}
			files.push_back( p );
		}
	}
	::LocalFree( args );

	// one event per file, at (0,0) on the main window, like Cocoa's application:openFile:
	for( auto &file : files ) {
		if( mWindows.empty() )
			break;
		WindowImplMswBasic *mainWindow = mWindows.front();
		WindowRef window = mainWindow->getWindow();
		setWindow( window );
		if( window->getRenderer() )
			window->getRenderer()->makeCurrentContext();
		vector<fs::path> paths( 1, file );
		FileDropEvent event( window, 0, 0, paths );
		window->emitFileDrop( &event );
	}
}

///////////////////////////////////////////////////////////////////////////////
// WindowImplMswBasic
void WindowImplMswBasic::toggleFullScreen( const app::FullScreenOptions &options )
{
	// if we were full-screen, destroy our blanking windows
	if( mFullScreen )
		mAppImplBasic->destroyBlankingWindows();

	WindowImplMsw::toggleFullScreen( options );

	// if we've entered full-screen, setup our blanking windows if necessary
	if( options.isSecondaryDisplayBlankingEnabled() && mFullScreen )
		mAppImplBasic->setupBlankingWindows( getDisplay() );
}

} } // namespace cinder::app