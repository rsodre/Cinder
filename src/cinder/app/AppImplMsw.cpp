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

#include "cinder/app/AppImplMsw.h"
#include "cinder/app/App.h"
#include "cinder/Utilities.h"
#include "cinder/Unicode.h"
#include "cinder/Display.h"
#include "cinder/msw/CinderMsw.h"

#include <Windows.h>
#include <CommDlg.h>
#include <ShellAPI.h>
#include <Shlobj.h>
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define LOSHORT(l)           ((SHORT)(l))
#define HISHORT(l)           ((SHORT)(((DWORD)(l) >> 16) & 0xFFFF))
#include <gdiplus.h>
#undef min
#undef max
#pragma comment(lib, "gdiplus")
// ROGER (Windows port): VERSIONINFO, known folders, registry
#include <KnownFolders.h>
#include <algorithm>
#pragma comment(lib, "version")
#pragma comment(lib, "shell32")
#pragma comment(lib, "ole32")
#pragma comment(lib, "advapi32")
#pragma comment(lib, "uuid")

using std::string;
using std::wstring;
using std::vector;
using std::pair;

namespace cinder { namespace app {

LRESULT CALLBACK WndProc( HWND mWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK BlankingWndProc( HWND mWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
static const wchar_t *WINDOWED_WIN_CLASS_NAME = TEXT("CinderWinClass");
static const wchar_t *BLANKING_WINDOW_CLASS_NAME = TEXT("CinderBlankingWindow");

AppImplMsw::AppImplMsw( App *aApp )
	: mApp( aApp ), mSetupHasBeenCalled( false ), mActive( true ),
	mRememberPosWnd( NULL ), mRememberPosDirty( false ), mInSizeMove( false )
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup( &mGdiplusToken, &gdiplusStartupInput, NULL );
}

AppImplMsw::~AppImplMsw()
{
	// there's no way to ensure all GDI+ objects have been freed, so we don't do this
	// for now this seems fine
//	Gdiplus::GdiplusShutdown( mGdiplusToken );
}

void AppImplMsw::hideCursor()
{
	int counter;

	while( (counter = ::ShowCursor(false)) > -1 );

	// when repeatedly calling hideCursor(), keep counter at -1
	if(counter < -1) while( ::ShowCursor(true) < -1 );
}

void AppImplMsw::showCursor()
{
	int counter;

	while( (counter = ::ShowCursor(true)) < 0 );

	// when repeatedly calling showCursor(), keep counter at 0
	if(counter > 0) while( ::ShowCursor(false) > 0 );
}

Buffer AppImplMsw::loadResource( int id, const std::string &type )
{
	HRSRC resInfoHandle;
	HGLOBAL resHandle;
	void *dataPtr;
	size_t dataSize;

	wchar_t unicodeType[1024]; 
	wsprintfW( unicodeType, L"%S", type.c_str() );
	resInfoHandle = ::FindResource( NULL, MAKEINTRESOURCE(id), unicodeType );
	if( resInfoHandle == NULL ) {
		throw ResourceLoadExc( id, type );
	}
	resHandle = ::LoadResource( NULL, resInfoHandle );
	if( resHandle == NULL ) {
		throw ResourceLoadExc( id, type );
	}
	// it's not necessary to unlock resources because the system automatically deletes them when the process
	// that created them terminates.
	dataPtr = ::LockResource( resHandle );
	if( dataPtr == 0 ) {
		throw ResourceLoadExc( id, type );
	}
	dataSize = ::SizeofResource( NULL, resInfoHandle );
	return Buffer( dataPtr, dataSize );
}

fs::path AppImplMsw::getAppPath()
{
	wchar_t appPath[MAX_PATH] = L"";

	// fetch the path of the executable
	::GetModuleFileName( 0, appPath, sizeof(appPath) - 1);

	// get a pointer to the last occurrence of the windows path separator
	wchar_t *appDir = wcsrchr( appPath, L'\\' );
	if( appDir ) {
		++appDir;

		// this shouldn't be null but one never knows
		if( appDir ) {
			// null terminate the string
			*appDir = 0;
		}
	}

	return fs::path( appPath );
}

fs::path AppImplMsw::getOpenFilePath( const fs::path &initialPath, vector<string> extensions )
{
	OPENFILENAMEW ofn;       // common dialog box structure
	wchar_t szFile[MAX_PATH];       // buffer for file name
	wchar_t extensionStr[10000];
	wchar_t initialPathStr[MAX_PATH];

	// Initialize OPENFILENAME
	::ZeroMemory( &ofn, sizeof(ofn) );
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = App::get()->getRenderer()->getHwnd();
	ofn.lpstrFile = szFile;
	
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	//
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( szFile );
	if( extensions.empty() ) {
		ofn.lpstrFilter = L"All\0*.*\0";
	}
	else {
		size_t offset = 0;
		
		wcscpy( extensionStr, L"Supported Types" );
		offset += wcslen( extensionStr ) + 1;
		for( vector<string>::const_iterator strIt = extensions.begin(); strIt != extensions.end(); ++strIt ) {
			wcscpy( extensionStr + offset, L"*." );
			offset += 2;
			wcscpy( extensionStr + offset, msw::toWideString( *strIt ).c_str() );
			offset += strIt->length();
			// append a semicolon to all but the last extensions
			if( strIt + 1 != extensions.end() ) {
				extensionStr[offset] = L';';
				offset += 1;
			}
			else {
				extensionStr[offset] = L'\0';
				offset += 1;
			}
		}

		extensionStr[offset] = 0;
		ofn.lpstrFilter = extensionStr;
	}
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	if( initialPath.empty() ) {
		ofn.lpstrInitialDir = NULL;
	}
	else if( fs::is_directory( initialPath ) ) {
		wcscpy( initialPathStr, initialPath.wstring().c_str() );
		ofn.lpstrInitialDir = initialPathStr;
	}
	else {
		wcscpy( initialPathStr, initialPath.parent_path().wstring().c_str() );
		ofn.lpstrInitialDir = initialPathStr;
		wcscpy( szFile, initialPath.filename().wstring().c_str() );
	}
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box.
	if( ::GetOpenFileNameW( &ofn ) == TRUE ) {
		return fs::path( ofn.lpstrFile );
	}
	else
		return fs::path();
}

namespace {

// see http://support.microsoft.com/kb/179378 "How To Browse for Folders from the Current Directory"
INT CALLBACK getFolderPathBrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData ) 
{
	switch( uMsg ) {
		case BFFM_INITIALIZED: 
			// WParam is TRUE since you are passing a path.
			// It would be FALSE if you were passing a pidl.
			// pData is a pointer to the wide string containing our initial path back at the original call site
			::SendMessage( hwnd, BFFM_SETSELECTION, TRUE, pData );
		break;
	}
	return 0;
}

} // anonymous namespace

fs::path AppImplMsw::getFolderPath( const fs::path &initialPath )
{
	string result;

	::BROWSEINFO bi = { 0 };
	bi.lParam = reinterpret_cast<LPARAM>( initialPath.wstring().c_str() );
	bi.lpfn = getFolderPathBrowseCallbackProc;
	bi.lpszTitle = L"Pick a Directory";
	::LPITEMIDLIST pidl = ::SHBrowseForFolder( &bi );
	if( pidl ) {
		// get the name of the folder
		TCHAR path[MAX_PATH];
		if( ::SHGetPathFromIDList( pidl, path ) ) {
			result = msw::toUtf8String( path );
		}

		// free memory used
		::IMalloc * imalloc = 0;
		if( SUCCEEDED( ::SHGetMalloc( &imalloc ) ) ) {
			imalloc->Free( pidl );
			imalloc->Release();
		}
	}

	return result;
}

fs::path AppImplMsw::getSaveFilePath( const fs::path &initialPath, vector<string> extensions )
{
	OPENFILENAMEW ofn;       // common dialog box structure
	wchar_t szFile[MAX_PATH];       // buffer for file name
	wchar_t initialPathStr[MAX_PATH];
	wchar_t extensionStr[10000];

	// Initialize OPENFILENAME
	ZeroMemory( &ofn, sizeof(ofn) );
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = App::get()->getRenderer()->getHwnd();
	ofn.lpstrFile = szFile;

	// Set lpstrFile[0] to '\0' so that GetSaveFileName does not
	// use the contents of szFile to initialize itself.
	//
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( szFile );
	if( extensions.empty() ) {
		ofn.lpstrFilter = L"All\0*.*\0";
	}
	else {
		size_t offset = 0;

		wcscpy( extensionStr, L"Supported Types" );
		offset += wcslen( extensionStr ) + 1;
		for( vector<string>::const_iterator strIt = extensions.begin(); strIt != extensions.end(); ++strIt ) {
			wcscpy( extensionStr + offset, L"*." );
			offset += 2;
			wcscpy( extensionStr + offset, msw::toWideString( strIt->c_str() ).c_str() );
			offset += strIt->length();
			// append a semicolon to all but the last extensions
			if( strIt + 1 != extensions.end() ) {
				extensionStr[offset] = L';';
				offset += 1;
			}
			else {
				extensionStr[offset] = 0;
				offset += 1;
			}
		}

		extensionStr[offset] = 0;
		ofn.lpstrFilter = extensionStr;
	}
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	if( initialPath.empty() ) {
		ofn.lpstrInitialDir = NULL;
	}
	else if( fs::is_directory( initialPath ) ) {
		wcscpy( initialPathStr, initialPath.wstring().c_str() );
		ofn.lpstrInitialDir = initialPathStr;
	}
	else {
		wcscpy( initialPathStr, initialPath.parent_path().wstring().c_str() );
		ofn.lpstrInitialDir = initialPathStr;
		wcscpy( szFile, initialPath.filename().wstring().c_str() );
	}
	ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT;

	// Display the Open dialog box.
	if( ::GetSaveFileName( &ofn ) == TRUE )
		return fs::path( ofn.lpstrFile );
	else
		return fs::path();
}


///////////////////////////////////////////////////////////////////////////////
// ROGER (Windows port): DPI, app info, cursor, dropped files, main window position

void AppImplMsw::enableDpiAwareness()
{
	static bool sDone = false;
	if( sDone )
		return;
	sDone = true;

	// Windows 10 1703+: per-monitor V2. Fails with ERROR_ACCESS_DENIED when a manifest (or an earlier call) already set the awareness: keep it.
	typedef BOOL (WINAPI *SetProcessDpiAwarenessContextFn)( HANDLE );
	HMODULE user32 = ::GetModuleHandleW( L"user32.dll" );
	SetProcessDpiAwarenessContextFn setContext = user32 ? (SetProcessDpiAwarenessContextFn)::GetProcAddress( user32, "SetProcessDpiAwarenessContext" ) : NULL;
	if( setContext ) {
		const HANDLE PER_MONITOR_AWARE_V2 = (HANDLE)(LONG_PTR)-4;	// DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
		if( setContext( PER_MONITOR_AWARE_V2 ) || ::GetLastError() == ERROR_ACCESS_DENIED )
			return;
	}
	// Windows 8.1+: per-monitor (V1)
	HMODULE shcore = ::LoadLibraryW( L"shcore.dll" );
	if( shcore ) {
		typedef HRESULT (WINAPI *SetProcessDpiAwarenessFn)( int );
		SetProcessDpiAwarenessFn setAwareness = (SetProcessDpiAwarenessFn)::GetProcAddress( shcore, "SetProcessDpiAwareness" );
		if( setAwareness ) {
			HRESULT hr = setAwareness( 2 );	// PROCESS_PER_MONITOR_DPI_AWARE
			if( SUCCEEDED( hr ) || hr == E_ACCESSDENIED )
				return;
		}
	}
	// Vista+: system aware
	typedef BOOL (WINAPI *SetProcessDPIAwareFn)();
	SetProcessDPIAwareFn setAware = user32 ? (SetProcessDPIAwareFn)::GetProcAddress( user32, "SetProcessDPIAware" ) : NULL;
	if( setAware )
		setAware();
}

std::string AppImplMsw::wideToUtf8( const wchar_t *wide )
{
	if( wide == NULL || wide[0] == 0 )
		return std::string();
	int len = ::WideCharToMultiByte( CP_UTF8, 0, wide, -1, NULL, 0, NULL, NULL );
	if( len <= 1 )
		return std::string();
	std::string result( len - 1, '\0' );
	::WideCharToMultiByte( CP_UTF8, 0, wide, -1, &result[0], len, NULL, NULL );
	return result;
}

std::vector<fs::path> AppImplMsw::getDroppedFiles( HDROP drop )
{
	std::vector<fs::path> files;
	UINT count = ::DragQueryFileW( drop, 0xFFFFFFFF, NULL, 0 );
	for( UINT i = 0; i < count; ++i ) {
		UINT len = ::DragQueryFileW( drop, i, NULL, 0 );
		std::wstring name( len + 1, L'\0' );
		UINT got = ::DragQueryFileW( drop, i, &name[0], len + 1 );
		name.resize( got );
		// built from the wide name: exact whatever boost::filesystem's narrow codecvt is (path.string() converts with it, path.wstring() is exact)
		files.push_back( fs::path( name ) );
	}
	return files;
}

namespace {

std::wstring getExePathW()
{
	std::vector<wchar_t> buf( MAX_PATH );
	for( ;; ) {
		DWORD len = ::GetModuleFileNameW( NULL, buf.data(), (DWORD)buf.size() );
		if( len == 0 )
			return std::wstring();
		if( len < buf.size() )
			return std::wstring( buf.data(), len );
		buf.resize( buf.size() * 2 );
	}
}

// The exe's VERSIONINFO, loaded once (same lookup as BLOCK/SysInfo/SysInfo_win.cpp)
class ExeVersionInfo {
  public:
	ExeVersionInfo() : mFixed( NULL )
	{
		std::wstring path = getExePathW();
		if( path.empty() )
			return;
		DWORD handle = 0;
		DWORD size = ::GetFileVersionInfoSizeW( path.c_str(), &handle );
		if( size == 0 )
			return;
		mData.resize( size );
		if( ! ::GetFileVersionInfoW( path.c_str(), 0, size, mData.data() ) ) {
			mData.clear();
			return;
		}
		UINT len = 0;
		VS_FIXEDFILEINFO *fixed = NULL;
		if( ::VerQueryValueW( mData.data(), L"\\", (LPVOID*)&fixed, &len ) && fixed && len >= sizeof( VS_FIXEDFILEINFO ) )
			mFixed = fixed;
		struct LangCodePage { WORD lang; WORD codepage; } *trans = NULL;
		if( ::VerQueryValueW( mData.data(), L"\\VarFileInfo\\Translation", (LPVOID*)&trans, &len ) && trans ) {
			for( UINT i = 0; i < len / sizeof( LangCodePage ); ++i ) {
				wchar_t block[64];
				swprintf( block, 64, L"\\StringFileInfo\\%04x%04x\\", trans[i].lang, trans[i].codepage );
				mBlocks.push_back( block );
			}
		}
		mBlocks.push_back( L"\\StringFileInfo\\040904b0\\" );
		mBlocks.push_back( L"\\StringFileInfo\\040904e4\\" );
	}

	std::string getString( const wchar_t *key ) const
	{
		if( mData.empty() )
			return std::string();
		for( auto &block : mBlocks ) {
			std::wstring query = block + key;
			wchar_t *value = NULL;
			UINT len = 0;
			if( ::VerQueryValueW( (LPVOID)mData.data(), query.c_str(), (LPVOID*)&value, &len ) && value && len > 0 ) {
				std::string s = AppImplMsw::wideToUtf8( value );
				size_t b = s.find_first_not_of( " \t\r\n" );
				if( b == std::string::npos )
					continue;
				size_t e = s.find_last_not_of( " \t\r\n" );
				return s.substr( b, e - b + 1 );
			}
		}
		return std::string();
	}

	const VS_FIXEDFILEINFO*	getFixed() const { return mFixed; }

	static const ExeVersionInfo& get() { static ExeVersionInfo sInfo; return sInfo; }

  private:
	std::vector<BYTE>			mData;
	std::vector<std::wstring>	mBlocks;
	VS_FIXEDFILEINFO			*mFixed;
};

} // anonymous namespace

std::string AppImplMsw::getVersionInfoAppName()
{
	const ExeVersionInfo &vi = ExeVersionInfo::get();
	std::string result = vi.getString( L"BundleName" );		// mirrors CFBundleName
	if( result.empty() )
		result = vi.getString( L"ProductName" );
	if( result.empty() ) {
		std::wstring path = getExePathW();
		size_t slash = path.find_last_of( L"\\/" );
		if( slash != std::wstring::npos )
			path = path.substr( slash + 1 );
		size_t dot = path.find_last_of( L'.' );
		if( dot != std::wstring::npos && dot > 0 )
			path = path.substr( 0, dot );
		result = wideToUtf8( path.c_str() );
	}
	if( result.empty() )
		result = "CinderApp";
	return result;
}

std::string AppImplMsw::getVersionInfoAppVersion()
{
	// Cocoa returns CFBundleVersion (the build), so the custom "BuildVersion" key comes first
	const ExeVersionInfo &vi = ExeVersionInfo::get();
	std::string result = vi.getString( L"BuildVersion" );
	if( result.empty() )
		result = vi.getString( L"ProductVersion" );
	if( result.empty() )
		result = vi.getString( L"FileVersion" );
	if( result.empty() && vi.getFixed() ) {
		const VS_FIXEDFILEINFO *f = vi.getFixed();
		char buf[64];
		snprintf( buf, sizeof( buf ), "%u.%u.%u.%u", HIWORD( f->dwProductVersionMS ), LOWORD( f->dwProductVersionMS ), HIWORD( f->dwProductVersionLS ), LOWORD( f->dwProductVersionLS ) );
		result = buf;
	}
	if( result.empty() )
		result = "0";
	return result;
}

fs::path AppImplMsw::getApplicationSupportFolder( const std::string &appName, bool create )
{
	PWSTR roaming = NULL;
	fs::path result;
	if( SUCCEEDED( ::SHGetKnownFolderPath( FOLDERID_RoamingAppData, KF_FLAG_CREATE, NULL, &roaming ) ) && roaming )
		result = fs::path( std::wstring( roaming ) ) / fs::path( msw::toWideString( appName ) );
	if( roaming )
		::CoTaskMemFree( roaming );
	if( result.empty() )
		return result;
	boost::system::error_code ec;
	if( create && ! fs::exists( result, ec ) )
		fs::create_directories( result, ec );
	return result;
}

static HCURSOR sAppCursor = NULL;

void AppImplMsw::setCursor( HCURSOR cursor )
{
	sAppCursor = cursor;
	// apply now if the mouse is over one of our windows' client area
	POINT pt;
	if( ::GetCursorPos( &pt ) ) {
		HWND wnd = ::WindowFromPoint( pt );
		wchar_t className[64] = L"";
		if( wnd && ::GetClassNameW( wnd, className, 64 ) && wcscmp( className, WINDOWED_WIN_CLASS_NAME ) == 0 )
			::SetCursor( sAppCursor ? sAppCursor : ::LoadCursor( NULL, IDC_ARROW ) );
	}
}

HCURSOR AppImplMsw::getCursor()
{
	return sAppCursor;
}

namespace {

void computeWindowStyles( bool fullScreen, bool borderless, bool resizable, bool toolWindow, bool noActivate, bool alwaysOnTop, DWORD *style, DWORD *exStyle )
{
	DWORD baseEx = toolWindow ? WS_EX_TOOLWINDOW : WS_EX_APPWINDOW;
	if( fullScreen ) {
		*exStyle = baseEx;
		*style = WS_POPUP;
	}
	else if( borderless ) {
		*exStyle = baseEx;
		*style = WS_POPUP;
	}
	else {
		*exStyle = baseEx | WS_EX_WINDOWEDGE;
		*style = ( resizable ) ? WS_OVERLAPPEDWINDOW
			:	( WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX );
	}
	if( noActivate )
		*exStyle |= WS_EX_NOACTIVATE;
	if( alwaysOnTop )
		*exStyle |= WS_EX_TOPMOST;
}

std::wstring mainWindowRegKey( const std::string &appName )
{
	return std::wstring( L"Software\\Studio Avante\\" ) + msw::toWideString( appName ) + L"\\MainWindow";
}

bool regReadInt( HKEY key, const wchar_t *name, int *value )
{
	DWORD v = 0, size = sizeof( v ), type = 0;
	if( ::RegQueryValueExW( key, name, NULL, &type, (LPBYTE)&v, &size ) != ERROR_SUCCESS || type != REG_DWORD )
		return false;
	*value = (int)v;
	return true;
}

void regWriteInt( HKEY key, const wchar_t *name, int value )
{
	DWORD v = (DWORD)value;
	::RegSetValueExW( key, name, 0, REG_DWORD, (const BYTE*)&v, sizeof( v ) );
}

struct MonitorWorkAreas {
	std::vector<RECT> rects;
	static BOOL CALLBACK proc( HMONITOR mon, HDC, LPRECT, LPARAM lParam ) {
		MONITORINFO mi = { sizeof( MONITORINFO ) };
		if( ::GetMonitorInfoW( mon, &mi ) )
			reinterpret_cast<MonitorWorkAreas*>( lParam )->rects.push_back( mi.rcWork );
		return TRUE;
	}
	MonitorWorkAreas() { ::EnumDisplayMonitors( NULL, NULL, proc, (LPARAM)this ); }
};

} // anonymous namespace

bool AppImplMsw::restoreMainWindowPos( Window::Format &format, const std::string &appName )
{
	HKEY key = NULL;
	if( ::RegOpenKeyExW( HKEY_CURRENT_USER, mainWindowRegKey( appName ).c_str(), 0, KEY_READ, &key ) != ERROR_SUCCESS )
		return false;
	int x = 0, y = 0, w = 0, h = 0, maximized = 0;
	bool ok = regReadInt( key, L"X", &x ) && regReadInt( key, L"Y", &y ) && regReadInt( key, L"Width", &w ) && regReadInt( key, L"Height", &h );
	regReadInt( key, L"Maximized", &maximized );
	::RegCloseKey( key );
	if( ! ok || w < 64 || h < 64 || w > 32000 || h > 32000 )
		return false;

	// the frame rect this client rect would get
	DWORD style, exStyle;
	computeWindowStyles( false, format.isBorderless(), format.isResizable(), format.isToolWindow(), format.isNoActivate(), format.isAlwaysOnTop(), &style, &exStyle );
	RECT wr = { x, y, x + w, y + h };
	::AdjustWindowRectEx( &wr, style, FALSE, exStyle );

	// valid only if the top of the window (where the title bar is) is visibly inside the work area of a current monitor
	// (displays may have changed since the position was saved)
	MonitorWorkAreas monitors;
	const RECT *home = NULL;
	for( auto &work : monitors.rects ) {
		RECT band = { wr.left, wr.top, wr.right, wr.top + 24 };
		RECT inter;
		if( ::IntersectRect( &inter, &band, &work ) && ( inter.right - inter.left ) >= std::min<LONG>( 100, w ) && ( inter.bottom - inter.top ) >= 8 ) {
			home = &work;
			break;
		}
	}
	if( ! home )
		return false;

	// keep the whole window inside that monitor's work area if it does not fit
	int frameW = ( wr.right - wr.left ) - w, frameH = ( wr.bottom - wr.top ) - h;
	int workW = home->right - home->left, workH = home->bottom - home->top;
	if( w + frameW > workW ) w = std::max( 64, workW - frameW );
	if( h + frameH > workH ) h = std::max( 64, workH - frameH );
	int left = x + ( wr.left - x ), top = y + ( wr.top - y );	// frame origin
	int fw = w + frameW, fh = h + frameH;
	if( left < home->left ) left = home->left;
	if( top < home->top ) top = home->top;
	if( left + fw > home->right ) left = home->right - fw;
	if( top + fh > home->bottom ) top = home->bottom - fh;
	x = left + ( x - wr.left );
	y = top + ( y - wr.top );

	format.setPos( x, y );
	format.setSize( w, h );
	return maximized != 0;
}

void AppImplMsw::trackMainWindowPos( HWND wnd, const std::string &appName )
{
	mRememberPosWnd = wnd;
	mRememberPosAppName = appName;
	mRememberPosDirty = false;
}

void AppImplMsw::saveMainWindowPos( bool force )
{
	if( ! mRememberPosWnd || ! ::IsWindow( mRememberPosWnd ) )
		return;
	if( ! force && ( ! mRememberPosDirty || mInSizeMove ) )
		return;
	mRememberPosDirty = false;

	if( ::IsIconic( mRememberPosWnd ) )
		return;
	WindowImplMsw *impl = reinterpret_cast<WindowImplMsw*>( ::GetWindowLongPtr( mRememberPosWnd, GWLP_USERDATA ) );
	if( impl && impl->isFullScreen() )
		return;

	HKEY key = NULL;
	if( ::RegCreateKeyExW( HKEY_CURRENT_USER, mainWindowRegKey( mRememberPosAppName ).c_str(), 0, NULL, 0, KEY_WRITE, NULL, &key, NULL ) != ERROR_SUCCESS )
		return;
	bool zoomed = ::IsZoomed( mRememberPosWnd ) != FALSE;
	if( ! zoomed ) {
		RECT client;
		::GetClientRect( mRememberPosWnd, &client );
		POINT origin = { 0, 0 };
		::ClientToScreen( mRememberPosWnd, &origin );
		regWriteInt( key, L"X", origin.x );
		regWriteInt( key, L"Y", origin.y );
		regWriteInt( key, L"Width", client.right - client.left );
		regWriteInt( key, L"Height", client.bottom - client.top );
	}
	regWriteInt( key, L"Maximized", zoomed ? 1 : 0 );
	::RegCloseKey( key );
}

///////////////////////////////////////////////////////////////////////////////
// WindowImplMsw
WindowImplMsw::WindowImplMsw( const Window::Format &format, RendererRef sharedRenderer, AppImplMsw *appImpl )
	: mWindowOffset( 0, 0 ), mAppImpl( appImpl ), mIsDragging( false ), mHidden( false ), mDisplayChangeCount( Display::getChangeCount() )
{
	mFullScreen = format.isFullScreen();
	mDisplay = format.getDisplay();
	mRenderer = format.getRenderer();
	mResizable = format.isResizable();
	mAlwaysOnTop = format.isAlwaysOnTop();
	mBorderless = format.isBorderless();
	mToolWindow = format.isToolWindow();	// ROGER
	mNoActivate = format.isNoActivate();	// ROGER
	if( ! mDisplay )
		mDisplay = Display::getMainDisplay();

	mWindowedSize = format.getSize();
	mWindowWidth = mWindowedSize.x;
	mWindowHeight = mWindowedSize.y;
	if( format.isPosSpecified() )
		mWindowOffset = mWindowedPos = format.getPos();
	else {
		// ROGER: centred on the target display (was centred on its size only, ignoring its origin)
		Vec2i displaySize = mDisplay->getSize();
		mWindowOffset = mWindowedPos = mDisplay->getBounds().getUL() + ( displaySize - mWindowedSize ) / 2;
	}

	createWindow( Vec2i( mWindowWidth, mWindowHeight ), format.getTitle(), mDisplay, sharedRenderer );
	// set WindowRef and its impl pointer to this
	mWindowRef = Window::privateCreate__( this, mAppImpl->getApp() );
	
	completeCreation();
}

WindowImplMsw::WindowImplMsw( HWND hwnd, RendererRef renderer, RendererRef sharedRenderer, AppImplMsw *appImpl )
	: mWnd( hwnd ), mRenderer( renderer ), mAppImpl( appImpl ), mIsDragging( false ),
	mHidden( false ), mFullScreen( false ), mBorderless( false ), mAlwaysOnTop( false ), mResizable( true ),
	mToolWindow( false ), mNoActivate( false ), mWindowStyle( 0 ), mWindowExStyle( 0 ), mDisplayChangeCount( Display::getChangeCount() )
{
	RECT rect;
	::GetWindowRect( mWnd, &rect );
	
	mDC = ::GetDC( hwnd );
	mWindowOffset = Vec2i( rect.left, rect.top );
	mWindowWidth = rect.right - rect.left;
	mWindowHeight = rect.bottom - rect.top;

	mDisplay = Display::findFromHmonitor( ::MonitorFromWindow( mWnd, MONITOR_DEFAULTTONEAREST ) );

	mRenderer->setup( mAppImpl->getApp(), mWnd, mDC, sharedRenderer );

	mWindowRef = Window::privateCreate__( this, mAppImpl->getApp() );
}

void WindowImplMsw::createWindow( const Vec2i &windowSize, const std::string &title, const DisplayRef display, RendererRef sharedRenderer )
{
	RECT windowRect;
	Area displayArea = display->getBounds();

	if( mFullScreen ) {
		windowRect.left = displayArea.getX1();
		windowRect.right = displayArea.getX2();
		windowRect.top = displayArea.getY1();
		windowRect.bottom = displayArea.getY2();
	}
	else {
		windowRect.left = mWindowedPos.x; 
		windowRect.right = mWindowedPos.x + windowSize.x;
		windowRect.top = mWindowedPos.y;
		windowRect.bottom = mWindowedPos.y + windowSize.y;
	}

	registerWindowClass();

	// ROGER: styles also carry toolWindow / noActivate / alwaysOnTop (WS_EX_TOPMOST set at creation)
	updateWindowStyles();

	::AdjustWindowRectEx( &windowRect, mWindowStyle, FALSE, mWindowExStyle );		// Adjust Window To True Requested Size

	std::wstring wideTitle = msw::toWideString( title ); 

	// Create The Window
	if( ! ( mWnd = ::CreateWindowEx( mWindowExStyle,						// Extended Style For The Window
		WINDOWED_WIN_CLASS_NAME,
		wideTitle.c_str(),						// Window Title
		mWindowStyle,					// Required Window Style
		windowRect.left, windowRect.top,								// Window Position
		windowRect.right - windowRect.left,	// Calculate Window Width
		windowRect.bottom - windowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		::GetModuleHandle( NULL ),
		reinterpret_cast<LPVOID>( this ) )) )

	{
		//killWindow();							// Reset The Display
		return;		
	}

	mDC = ::GetDC( mWnd );
	if( ! mDC ) {
		return;
	}

	if( mAlwaysOnTop ) {
		::SetWindowLongA( mWnd, GWL_EXSTYLE, ::GetWindowLongA( mWnd, GWL_EXSTYLE ) | WS_EX_TOPMOST );
		::SetWindowPos( mWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE );	// ROGER: NOACTIVATE, the window is shown later
	}

	// update display
	mDisplay = Display::findFromHmonitor( ::MonitorFromWindow( mWnd, MONITOR_DEFAULTTONEAREST ) );
	mDisplayChangeCount = Display::getChangeCount();

	mRenderer->setup( mAppImpl->getApp(), mWnd, mDC, sharedRenderer );
}

// ROGER
void WindowImplMsw::updateWindowStyles()
{
	computeWindowStyles( mFullScreen, mBorderless, mResizable, mToolWindow, mNoActivate, mAlwaysOnTop, &mWindowStyle, &mWindowExStyle );
}

// ROGER: the Display is looked up again after a display configuration change
DisplayRef WindowImplMsw::getDisplay() const
{
	if( mDisplayChangeCount != Display::getChangeCount() && mWnd ) {
		mDisplay = Display::findFromHmonitor( ::MonitorFromWindow( mWnd, MONITOR_DEFAULTTONEAREST ) );
		mDisplayChangeCount = Display::getChangeCount();
	}
	return mDisplay;
}

void WindowImplMsw::completeCreation()
{
	::DragAcceptFiles( mWnd, TRUE );
	if( mAppImpl->mApp->getSettings().isMultiTouchEnabled() )
		enableMultiTouch();

	if( mNoActivate ) {
		// ROGER: never steals the focus / foreground from the current window
		::ShowWindow( mWnd, SW_SHOWNOACTIVATE );
	}
	else {
		::ShowWindow( mWnd, SW_SHOW );
		::SetForegroundWindow( mWnd );
		::SetFocus( mWnd );
	}
}

void WindowImplMsw::registerWindowClass()
{
	static bool sRegistered = false;

	if( sRegistered )
		return;

	WNDCLASSEX	wc;
	HMODULE instance	= ::GetModuleHandle( NULL );				// Grab An Instance For Our Window
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= WndProc;						// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= instance;							// Set The Instance
	wc.hIcon = (HICON)::LoadImage( instance, MAKEINTRESOURCE(1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE ); // Load The Default Cinder Icon
	wc.hIconSm = NULL;
	wc.hCursor			= ::LoadCursor( NULL, IDC_ARROW );		// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= WINDOWED_WIN_CLASS_NAME;

	if( ! ::RegisterClassEx( &wc ) ) {								// Attempt To Register The Window Class
		DWORD err = ::GetLastError();
		return;							
	}

	sRegistered = true;
}

void WindowImplMsw::setFullScreen( bool fullScreen, const app::FullScreenOptions &options )
{
	if( mFullScreen != fullScreen )
		toggleFullScreen( options );
}

void WindowImplMsw::toggleFullScreen( const app::FullScreenOptions &options )
{
	Vec2i newWindowSize;
	bool prevFullScreen = mFullScreen;
	HDC oldDC = mDC;
	HWND oldWnd = mWnd;
	
	mFullScreen = ! mFullScreen;
	
	if( prevFullScreen ) {
		newWindowSize = mWindowedSize;
	}
	else {
		mWindowedPos = mWindowOffset;
		mWindowedSize = Vec2i( mWindowWidth, mWindowHeight );
		newWindowSize = mDisplay->getSize();
	}

	getRenderer()->prepareToggleFullScreen();
	DisplayRef display = options.getDisplay();
	if( ! display ) // use the default, which is this Window's display
		display = mDisplay;
	// we don't need to share a renderer because we're not really creating a window per se
	createWindow( newWindowSize, getTitle(), display, RendererRef() );
	getRenderer()->finishToggleFullScreen();

	::ReleaseDC( oldWnd, oldDC );
	::DestroyWindow( oldWnd ); 

	completeCreation();
	mWindowRef->emitResize();
}

void WindowImplMsw::getScreenSize( int clientWidth, int clientHeight, int *resultWidth, int *resultHeight )
{
	RECT windowRect;
	windowRect.left = windowRect.top = 0;
	windowRect.right = clientWidth;
	windowRect.bottom = clientHeight;
	::AdjustWindowRectEx( &windowRect, mWindowStyle, FALSE, mWindowExStyle );
	*resultWidth = windowRect.right - windowRect.left;
	*resultHeight = windowRect.bottom - windowRect.top;
}

void WindowImplMsw::setPos( const Vec2i &windowPos )
{
	RECT clientArea;
	clientArea.left = windowPos.x; clientArea.top = windowPos.y;
	clientArea.right = windowPos.x + 1; clientArea.bottom = windowPos.y + 1; // we don't actually care about the lower-right
	::AdjustWindowRectEx( &clientArea, mWindowStyle, FALSE, mWindowExStyle );
	::SetWindowPos( mWnd, HWND_TOP, clientArea.left, clientArea.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | ( mNoActivate ? SWP_NOACTIVATE : 0 ) );	// ROGER: NOACTIVATE
	
	POINT upperLeft;
	upperLeft.x = upperLeft.y = 0;
	::ClientToScreen( mWnd, &upperLeft );
	mWindowOffset.x = upperLeft.x;
	mWindowOffset.y = upperLeft.y;
}

void WindowImplMsw::hide()
{
	::ShowWindow( mWnd, SW_HIDE );
	mHidden = true;
}

void WindowImplMsw::show()
{
	::ShowWindow( mWnd, mNoActivate ? SW_SHOWNOACTIVATE : SW_SHOW );	// ROGER
	mHidden = false;
}

bool WindowImplMsw::isHidden() const
{
	return mHidden;
}

std::string	WindowImplMsw::getTitle() const
{
	int numChars = ::GetWindowTextLengthW( mWnd );
	wchar_t *wideChars = (wchar_t*)malloc( sizeof(wchar_t) * (numChars + 1) );
	::GetWindowTextW( mWnd, &wideChars[0], numChars + 1 );
	wideChars[numChars] = 0;
	std::string result = msw::toUtf8String( wideChars );
	free( (void*)wideChars );
	return result;
}

void WindowImplMsw::setTitle( const std::string &title )
{
	std::wstring titleWide = msw::toWideString( title );
	if( titleWide.empty() )
		::SetWindowText( mWnd, L"" );
	else
		::SetWindowText( mWnd, titleWide.c_str() );
}

void WindowImplMsw::setSize( const Vec2i &windowSize )
{
	int screenWidth, screenHeight;
	getScreenSize( windowSize.x, windowSize.y, &screenWidth, &screenHeight );
	::SetWindowPos( mWnd, HWND_TOP, 0, 0, screenWidth, screenHeight, SWP_NOMOVE | SWP_NOZORDER | ( mNoActivate ? SWP_NOACTIVATE : 0 ) );	// ROGER: NOACTIVATE
}

void WindowImplMsw::close()
{
	getAppImpl()->closeWindow( this );
	// at this point 'this' is invalid so do nothing after this line
}

void WindowImplMsw::enableMultiTouch()
{
	// we need to make sure this version of User32 even has MultiTouch symbols, so we'll do that with GetProcAddress
	BOOL (WINAPI *RegisterTouchWindow)( HWND, ULONG);
	*(size_t *)&RegisterTouchWindow = (size_t)::GetProcAddress( ::GetModuleHandle(TEXT("user32.dll")), "RegisterTouchWindow" );
	if( RegisterTouchWindow ) {
		(*RegisterTouchWindow)( mWnd, 0 );
	}
}

void WindowImplMsw::onTouch( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	// pull these symbols dynamically out of the user32.dll
	static BOOL (WINAPI *GetTouchInputInfo)( HTOUCHINPUT, UINT, PTOUCHINPUT, int ) = NULL;
	if( ! GetTouchInputInfo )
		*(size_t *)&GetTouchInputInfo = (size_t)::GetProcAddress( ::GetModuleHandle(TEXT("user32.dll")), "GetTouchInputInfo" );
	static BOOL (WINAPI *CloseTouchInputHandle)( HTOUCHINPUT ) = NULL;
	if( ! CloseTouchInputHandle )
		*(size_t *)&CloseTouchInputHandle = (size_t)::GetProcAddress( ::GetModuleHandle(TEXT("user32.dll")), "CloseTouchInputHandle" );

	bool handled = false;
	double currentTime = app::getElapsedSeconds(); // we don't trust the device's sense of time
	unsigned int numInputs = LOWORD( wParam );
	std::shared_ptr<TOUCHINPUT> pInputs = std::shared_ptr<TOUCHINPUT>( new TOUCHINPUT[numInputs], checked_array_deleter<TOUCHINPUT>() );
	if( pInputs ) {
		vector<TouchEvent::Touch> beganTouches, movedTouches, endTouches, activeTouches;
		if( GetTouchInputInfo((HTOUCHINPUT)lParam, numInputs, pInputs.get(), sizeof(TOUCHINPUT) ) ) {
			for( unsigned int i = 0; i < numInputs; i++ ) {
				const TOUCHINPUT &ti = pInputs.get()[i];
				if( ti.dwID != 0 ) {
					POINT pt;
					// this has a small problem, which is that we lose the subpixel precision of the touch points.
					// However ScreenToClient doesn't support floating or fixed point either, so we're stuck 
					// unless we write our own ScreenToClient, which actually should be doable
					pt.x = TOUCH_COORD_TO_PIXEL( ti.x );
					pt.y = TOUCH_COORD_TO_PIXEL( ti.y );
					::ScreenToClient( hWnd, &pt );
					if( ti.dwFlags & 0x0004/*TOUCHEVENTF_UP*/ ) {
						Vec2f prevPos = mMultiTouchPrev[ti.dwID];
						endTouches.push_back( TouchEvent::Touch( Vec2f( (float)pt.x, (float)pt.y ), prevPos, ti.dwID, currentTime, &pInputs.get()[i] ) );
						mMultiTouchPrev.erase( ti.dwID );
					}
					else if( ti.dwFlags & 0x0002/*TOUCHEVENTF_DOWN*/ ) {
						beganTouches.push_back( TouchEvent::Touch( Vec2f( (float)pt.x, (float)pt.y ), Vec2f( (float)pt.x, (float)pt.y ), ti.dwID, currentTime, &pInputs.get()[i] ) );
						mMultiTouchPrev[ti.dwID] = Vec2f( (float)pt.x, (float)pt.y );
						activeTouches.push_back( beganTouches.back() );
					}
					else if( ti.dwFlags & 0x0001/*TOUCHEVENTF_MOVE*/ ) {
						movedTouches.push_back( TouchEvent::Touch( Vec2f( (float)pt.x, (float)pt.y ), mMultiTouchPrev[ti.dwID], ti.dwID, currentTime, &pInputs.get()[i] ) );
						activeTouches.push_back( movedTouches.back() );
						mMultiTouchPrev[ti.dwID] = Vec2f( (float)pt.x, (float)pt.y );
					}
				}
            }
            
			mActiveTouches = activeTouches;
            
            // we need to post the event here so that our pInputs array is still valid since we've passed addresses into it as the native pointers
            if( ! beganTouches.empty() ) {
				TouchEvent event( getWindow(), beganTouches );
				getWindow()->emitTouchesBegan( &event );
			}
			if( ! movedTouches.empty() ) {
				TouchEvent event( getWindow(), movedTouches );
				getWindow()->emitTouchesMoved( &event );
			}
			if( ! endTouches.empty() ) {
				TouchEvent event( getWindow(), endTouches );
				getWindow()->emitTouchesEnded( &event );
			}
			
            handled = ( ! beganTouches.empty() ) || ( ! movedTouches.empty() ) || ( ! endTouches.empty() );
            CloseTouchInputHandle( (HTOUCHINPUT)lParam ); // this is exception-unsafe; we need some RAII goin' on there*/
        }
        else {
			// for now we'll just ignore an error
        }
	}

    if( ! handled ) {
        // if we didn't handle the message, let DefWindowProc handle it
        ::DefWindowProc( hWnd, WM_TOUCH, wParam, lParam );
    }
}

unsigned int prepMouseEventModifiers( WPARAM wParam )
{
	unsigned int result = 0;
	if( wParam & MK_CONTROL ) result |= MouseEvent::CTRL_DOWN;
	if( wParam & MK_LBUTTON ) result |= MouseEvent::LEFT_DOWN;
	if( wParam & MK_MBUTTON ) result |= MouseEvent::MIDDLE_DOWN;
	if( wParam & MK_RBUTTON ) result |= MouseEvent::RIGHT_DOWN;
	if( wParam & MK_SHIFT ) result |= MouseEvent::SHIFT_DOWN;
	if( ::GetKeyState( VK_MENU ) < 0 ) result |= MouseEvent::ALT_DOWN;	
	if( (::GetKeyState( VK_LWIN ) < 0) || (::GetKeyState( VK_RWIN ) < 0) ) result |= MouseEvent::META_DOWN;
	return result;
}

// Certain key codes need to be refined, for example VK_MENU needs to be
// converted into VK_LALT or VK_RALT
int prepNativeKeyCode( WPARAM wParam )
{
	unsigned int result = (int)wParam;
	if( wParam == VK_MENU ) {
		result = ( ::GetKeyState( VK_RMENU ) ) ? VK_RMENU : VK_LMENU;
	}
	else if( wParam == VK_SHIFT ) {
		result = ( ::GetKeyState( VK_RSHIFT ) ) ? VK_RSHIFT : VK_LSHIFT;	
	}
	else if( wParam == VK_CONTROL ) {
		result = ( ::GetKeyState( VK_RCONTROL ) ) ? VK_RCONTROL : VK_LCONTROL;		
	}
	return result;
}

WCHAR mapVirtualKey( WPARAM wParam )
{
	BYTE keyboardState[256];
	::GetKeyboardState( keyboardState );
	WCHAR result[4] = { 0, 0, 0, 0 };

	// the control key messes up the ToAscii result, so we zero it out
	keyboardState[VK_CONTROL] = 0;
	
	int resultLength = ::ToUnicode( wParam, ::MapVirtualKey( wParam, 0 ), keyboardState, result, 4, 0 );
	if( resultLength >= 1 )
		return result[0];
	else
		return 0;
}

unsigned int prepKeyEventModifiers()
{
	unsigned int result = 0;
	if( ::GetKeyState( VK_CONTROL ) & 0x8000 ) result |= KeyEvent::CTRL_DOWN;
	if( ::GetKeyState( VK_SHIFT ) & 0x8000 ) result |= KeyEvent::SHIFT_DOWN;
	if( ( ::GetKeyState( VK_LMENU ) & 0x8000 ) || ( ::GetKeyState( VK_RMENU ) & 0x8000 ) ) result |= KeyEvent::ALT_DOWN;	
	if( ( ::GetKeyState( VK_LWIN ) < 0 ) || ( ::GetKeyState( VK_RWIN ) < 0 ) ) result |= KeyEvent::META_DOWN;
	return result;
}

void WindowImplMsw::setBorderless( bool borderless )
{
	if( mBorderless != borderless ) {
		mBorderless = borderless;
		// ROGER: keeps toolWindow / noActivate / topmost (was resetting the ex style to WS_EX_APPWINDOW)
		updateWindowStyles();
		if( ! mBorderless && ! mResizable )
			mWindowStyle = ( WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME );	// as before for this case

		POINT upperLeft;
		upperLeft.x = upperLeft.y = 0;
		::ClientToScreen( mWnd, &upperLeft );

		RECT windowRect;
		::GetClientRect( mWnd, &windowRect );
		windowRect.left += upperLeft.x; windowRect.right += upperLeft.x;
		windowRect.top += upperLeft.y; windowRect.bottom += upperLeft.y;
		::AdjustWindowRectEx( &windowRect, mWindowStyle, FALSE, mWindowExStyle );		// Adjust Window To True Requested Size

		::SetWindowLongA( mWnd, GWL_STYLE, mWindowStyle );
		::SetWindowLongA( mWnd, GWL_EXSTYLE, mWindowExStyle );
		::SetWindowPos( mWnd, HWND_TOP, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
				SWP_FRAMECHANGED|SWP_SHOWWINDOW|SWP_NOZORDER | ( mNoActivate ? SWP_NOACTIVATE : 0 ) );
		if( mBorderless )
			::InvalidateRect( 0, NULL, TRUE );
	}
}

void WindowImplMsw::setAlwaysOnTop( bool alwaysOnTop )
{
	if( mAlwaysOnTop != alwaysOnTop ) {
		mAlwaysOnTop = alwaysOnTop;
		LONG oldExStyle = ::GetWindowLongA( mWnd, GWL_EXSTYLE );
		if( mAlwaysOnTop ) {
			::SetWindowLongA( mWnd, GWL_EXSTYLE, oldExStyle | WS_EX_TOPMOST );
			::SetWindowPos( mWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | ( mNoActivate ? SWP_NOACTIVATE : 0 ) );	// ROGER: NOACTIVATE
		}
		else {
			::SetWindowLongA( mWnd, GWL_EXSTYLE, oldExStyle &= (~WS_EX_TOPMOST) );
			::SetWindowPos( mWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | ( mNoActivate ? SWP_NOACTIVATE : 0 ) );	// ROGER: NOACTIVATE
		}
	}
}

LRESULT CALLBACK WndProc(	HWND	mWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	WindowImplMsw* impl;

	// if the message is WM_NCCREATE we need to hide 'this' in the window long
	if( uMsg == WM_NCCREATE ) {
		impl = reinterpret_cast<WindowImplMsw*>(((LPCREATESTRUCT)lParam)->lpCreateParams);
		::SetWindowLongPtr( mWnd, GWLP_USERDATA, (__int3264)(LONG_PTR)impl ); 
	}
	else // the warning on this line is harmless
		impl = reinterpret_cast<WindowImplMsw*>( ::GetWindowLongPtr( mWnd, GWLP_USERDATA ) );

	// ROGER (Windows port): a borderless (WS_POPUP, no frame) window may be larger than the desktop, like NSWindowUnlimited on the Mac.
	// Handled before the impl test: the first WM_GETMINMAXINFO arrives before WM_NCCREATE, during CreateWindowEx.
	if( uMsg == WM_GETMINMAXINFO ) {
		LONG style = ::GetWindowLongW( mWnd, GWL_STYLE );
		if( ( style & WS_POPUP ) && ! ( style & ( WS_THICKFRAME | WS_CAPTION ) ) ) {
			MINMAXINFO *mmi = reinterpret_cast<MINMAXINFO*>( lParam );
			DefWindowProc( mWnd, uMsg, wParam, lParam );
			mmi->ptMaxTrackSize.x = 32767;	// ptMaxSize (maximized size) is left alone
			mmi->ptMaxTrackSize.y = 32767;
			return 0;
		}
	}

	if( ! impl )
		return DefWindowProc( mWnd, uMsg, wParam, lParam );
	impl->mAppImpl->setWindow( impl->mWindowRef );

	switch( uMsg ) {
		case WM_SYSCOMMAND:
			switch( wParam ) {
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
					if( ! impl->getAppImpl()->getApp()->isPowerManagementEnabled() )
						return false;
					else
						return DefWindowProc( mWnd, uMsg, wParam, lParam );
			}
		break;
		case WM_ACTIVATEAPP:
			if( wParam ) {
				if( ! impl->getAppImpl()->mActive ) {
					impl->getAppImpl()->mActive = true;
					impl->getAppImpl()->getApp()->emitDidBecomeActive();
				}
			}
			else {
				if( impl->getAppImpl()->mActive ) {
					impl->getAppImpl()->mActive = false;
					impl->getAppImpl()->getApp()->emitWillResignActive();
				}
			}
		break;
		case WM_ACTIVATE:
			if( ( wParam == WA_ACTIVE ) || ( wParam == WA_CLICKACTIVE ) )
				impl->getAppImpl()->setForegroundWindow( impl->getWindow() );
		break;
		case WM_CLOSE:								// Did We Receive A Close Message?
			impl->getAppImpl()->closeWindow( impl );
			// be careful not to do anything with 'impl' after here
			return 0;
		break;
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN: {
			WCHAR c = mapVirtualKey( wParam );
			KeyEvent event( impl->getWindow(), KeyEvent::translateNativeKeyCode( prepNativeKeyCode( (int)wParam ) ), 
							c, c, prepKeyEventModifiers(), (int)wParam );
			impl->getWindow()->emitKeyDown( &event );
			if ( event.isHandled() )
				return 0;
		}
		break;
		case WM_SYSKEYUP:
		case WM_KEYUP: {
			WCHAR c = mapVirtualKey( wParam );
			KeyEvent event( impl->getWindow(), KeyEvent::translateNativeKeyCode( prepNativeKeyCode( (int)wParam ) ), 
							c, c, prepKeyEventModifiers(), (int)wParam );
			impl->getWindow()->emitKeyUp( &event );
			if ( event.isHandled() )
				return 0;
		}
		break;
		// mouse events
		case WM_LBUTTONDOWN: {
			::SetCapture( mWnd );
			impl->mIsDragging = true;
			MouseEvent event( impl->getWindow(), MouseEvent::LEFT_DOWN, LOSHORT(lParam), HISHORT(lParam), prepMouseEventModifiers( wParam ), 0.0f, static_cast<unsigned int>( wParam ) );
			impl->getWindow()->emitMouseDown( &event );
			return 0;
		}
		break;
		case WM_RBUTTONDOWN: {
			::SetCapture( mWnd );
			impl->mIsDragging = true;
			MouseEvent event( impl->getWindow(), MouseEvent::RIGHT_DOWN, LOSHORT(lParam), HISHORT(lParam), prepMouseEventModifiers( wParam ), 0.0f, static_cast<unsigned int>( wParam ) );
			impl->getWindow()->emitMouseDown( &event );
			return 0;
		}
		break;		
		case WM_MBUTTONDOWN: {
			::SetCapture( mWnd );
			impl->mIsDragging = true;		
			MouseEvent event( impl->getWindow(), MouseEvent::MIDDLE_DOWN, LOSHORT(lParam), HISHORT(lParam), prepMouseEventModifiers( wParam ), 0.0f, static_cast<unsigned int>( wParam ) );
			impl->getWindow()->emitMouseDown( &event );
			return 0;
		}
		break;
		case WM_LBUTTONUP: {
			::ReleaseCapture();
			impl->mIsDragging = false;
			MouseEvent event( impl->getWindow(), MouseEvent::LEFT_DOWN, LOSHORT(lParam), HISHORT(lParam), prepMouseEventModifiers( wParam ), 0.0f, static_cast<unsigned int>( wParam ) );
			impl->getWindow()->emitMouseUp( &event );
			return 0;
		}
		break;
		case WM_RBUTTONUP: {
			::ReleaseCapture();
			impl->mIsDragging = false;		
			MouseEvent event( impl->getWindow(), MouseEvent::RIGHT_DOWN, LOSHORT(lParam), HISHORT(lParam), prepMouseEventModifiers( wParam ), 0.0f, static_cast<unsigned int>( wParam ) );
			impl->getWindow()->emitMouseUp( &event );
			return 0;
		}
		break;		
		case WM_MBUTTONUP: {
			::ReleaseCapture();
			impl->mIsDragging = false;
			MouseEvent event( impl->getWindow(), MouseEvent::MIDDLE_DOWN, LOSHORT(lParam), HISHORT(lParam), prepMouseEventModifiers( wParam ), 0.0f, static_cast<unsigned int>( wParam ) );
			impl->getWindow()->emitMouseUp( &event );
			return 0;
		}
		break;
		case WM_MOUSEWHEEL: {
			MouseEvent event( impl->getWindow(), 0, LOSHORT(lParam), HISHORT(lParam), prepMouseEventModifiers( wParam ),
								GET_WHEEL_DELTA_WPARAM( wParam ) / 120.0f, static_cast<unsigned int>( wParam ) );
			impl->getWindow()->emitMouseWheel( &event );
		}
		break;
		case WM_KILLFOCUS:
			// if we lose capture during a drag, post a mouseup event as a notifier
			if( impl->mIsDragging ) {
				MouseEvent event( impl->getWindow(), 0, LOSHORT(lParam), HISHORT(lParam), prepMouseEventModifiers( wParam ), 0.0f, static_cast<unsigned int>( wParam ) );
				impl->getWindow()->emitMouseUp( &event );
			}
			impl->mIsDragging = false;
		break;
		case WM_MOUSEMOVE: {
			if( impl->mIsDragging ) {
				MouseEvent event( impl->getWindow(), 0, LOSHORT(lParam), HISHORT(lParam), prepMouseEventModifiers( wParam ), 0.0f, static_cast<unsigned int>( wParam ) );
				impl->getWindow()->emitMouseDrag( &event );						
			}
			else {
				MouseEvent event( impl->getWindow(), 0, LOSHORT(lParam), HISHORT(lParam), prepMouseEventModifiers( wParam ), 0.0f, static_cast<unsigned int>( wParam ) );
				impl->getWindow()->emitMouseMove( &event );
			}
		}
		break;
		case WM_SIZE:
			impl->mWindowWidth = LOWORD(lParam);
			impl->mWindowHeight = HIWORD(lParam);
			impl->mAppImpl->mainWindowPosChanged( mWnd );	// ROGER
			if( impl->getWindow() && impl->mAppImpl->setupHasBeenCalled() ) {
				impl->getWindow()->emitResize();
			}
			return 0;
		break;
		// ROGER (Windows port)
		case WM_ENTERSIZEMOVE:
			impl->mAppImpl->mInSizeMove = true;
		break;
		case WM_EXITSIZEMOVE:
			impl->mAppImpl->mInSizeMove = false;
			impl->mAppImpl->mainWindowPosChanged( mWnd );
		break;
		case WM_MOUSEACTIVATE:
			if( impl->mNoActivate )
				return MA_NOACTIVATE;
		break;
		case WM_SETCURSOR:
			if( LOWORD( lParam ) == HTCLIENT && AppImplMsw::getCursor() ) {
				::SetCursor( AppImplMsw::getCursor() );
				return TRUE;
			}
		break;
		case WM_DISPLAYCHANGE:
			// the display list is re-enumerated on its next use; Window::getDisplay() looks its Display up again
			Display::privateMarkChanged__();
		break;
		case WM_MOVE: {
			impl->mAppImpl->mainWindowPosChanged( mWnd );	// ROGER
			impl->mWindowOffset = Vec2i( LOSHORT(lParam), HISHORT(lParam) );
			if( impl->getWindow() ) {
				DisplayRef oldDisplay = impl->mDisplay;
				impl->mDisplay = Display::findFromHmonitor( ::MonitorFromWindow( mWnd, MONITOR_DEFAULTTONEAREST ) );
				// signal display change as appropriate
				if( oldDisplay != impl->mDisplay ) {
					impl->getWindow()->emitDisplayChange();
				}
				// signal window move
				impl->getWindow()->emitMove();
			}
			return 0;
		}
		break;
		case WM_DROPFILES: {
			HDROP dropH = (HDROP)wParam;
			POINT dropPoint;
			// ROGER: wide names (DragQueryFileA lost every character outside the ANSI code page)
			vector<fs::path> files = AppImplMsw::getDroppedFiles( dropH );

			::DragQueryPoint( dropH, &dropPoint );
			::DragFinish( dropH );

			FileDropEvent dropEvent( impl->getWindow(), dropPoint.x, dropPoint.y, files );
			impl->getWindow()->emitFileDrop( &dropEvent );
			return 0;
		}
		break;
		case WM_PAINT:
			impl->draw();
		break;
		case WM_TOUCH:
			impl->onTouch( mWnd, wParam, lParam );
		break;
	}

	// unhandled messages To DefWindowProc
	return DefWindowProc( mWnd, uMsg, wParam, lParam );
}

void WindowImplMsw::draw()
{
	mAppImpl->setWindow( mWindowRef );
	mRenderer->startDraw();
	mWindowRef->emitDraw();
	mRenderer->finishDraw();
}

void WindowImplMsw::resize()
{
	mAppImpl->setWindow( mWindowRef );
	mWindowRef->emitResize();
}

void WindowImplMsw::redraw()
{
	::RedrawWindow( mWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW );
}

void WindowImplMsw::privateClose()
{
	mRenderer->kill();

	if( mDC )
		::ReleaseDC( mWnd, mDC );

	if( mWnd )
		::DestroyWindow( mWnd );

	mWnd = 0;
}

void WindowImplMsw::keyDown( const KeyEvent &event )
{
	KeyEvent localEvent( event );
	mAppImpl->setWindow( mWindowRef );
	mWindowRef->emitKeyDown( &localEvent );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BlankingWindow
BlankingWindow::BlankingWindow( DisplayRef display )
{
	registerWindowClass();

	RECT windowRect;
	Area displayArea = display->getBounds();
	windowRect.left = displayArea.getX1();
	windowRect.right = displayArea.getX2();
	windowRect.top = displayArea.getY1();
	windowRect.bottom = displayArea.getY2();
	
	UINT windowExStyle = WS_EX_APPWINDOW;
	UINT windowStyle = WS_POPUP;

	::AdjustWindowRectEx( &windowRect, windowStyle, FALSE, windowExStyle );		// Adjust Window To True Requested Size

	std::wstring unicodeTitle = L""; 

	// Create The Window
	if( ! ( mWnd = ::CreateWindowEx( windowExStyle,						// Extended Style For The Window
		BLANKING_WINDOW_CLASS_NAME,
		unicodeTitle.c_str(),
		windowStyle,					// Required Window Style
		windowRect.left, windowRect.top,								// Window Position
		windowRect.right - windowRect.left,	// Calculate Window Width
		windowRect.bottom - windowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		::GetModuleHandle( NULL ),
		reinterpret_cast<LPVOID>( this ) )) )
	{
		//killWindow();							// Reset The Display
		return;
	}

	::ShowWindow( mWnd, SW_SHOW );
	::SetWindowLongA( mWnd, GWL_EXSTYLE, ::GetWindowLongA( mWnd, GWL_EXSTYLE ) | WS_EX_TOPMOST );
	::SetWindowPos( mWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
	::SetForegroundWindow( mWnd );
	::SetFocus( mWnd );

}

void BlankingWindow::registerWindowClass()
{
	static bool sRegistered = false;

	if( sRegistered )
		return;

	WNDCLASS	wc;
	HMODULE instance	= ::GetModuleHandle( NULL );				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= BlankingWndProc;						// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= instance;							// Set The Instance
	wc.hIcon			= ::LoadIcon( NULL, IDI_WINLOGO );		// Load The Default Icon
	wc.hCursor			= ::LoadCursor( NULL, IDC_ARROW );		// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= BLANKING_WINDOW_CLASS_NAME;

	if( ! ::RegisterClass( &wc ) ) {								// Attempt To Register The Window Class
		DWORD err = ::GetLastError();
		return;							
	}

	sRegistered = true;
}

LRESULT CALLBACK BlankingWndProc( HWND wnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static HBRUSH backgroundBrush = ::CreateSolidBrush( RGB( 0, 0, 0 ) );
	
	switch( uMsg ) {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			RECT clientRect;
			HDC hdc = ::BeginPaint( wnd, &ps );
			::GetClientRect( wnd, &clientRect );
			::SelectObject( hdc, backgroundBrush );
			::Rectangle( hdc, 0, 0, clientRect.right, clientRect.bottom );
			::EndPaint( wnd, &ps );
			return 0;
		}
		break;
		default:
			return ::DefWindowProc( wnd, uMsg, wParam, lParam );
	}
}

void BlankingWindow::destroy()
{
	if( mWnd )
		::DestroyWindow( mWnd );
}

} } // namespace cinder::app
