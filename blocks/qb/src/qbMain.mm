//
//  qbMain.h
//
//  Created by Roger Sodre on 01/07/2011
//  Copyright 2011 Studio Avante. All rights reserved.
//
#include "qbMain.h"
#include "qb.h"
#include "cinder/Display.h"

const float winScales[] = { 0.8, 0.75, 0.5, 0.25, 0.2, 0.1, 0.08, 0.075, 0.05, 0.025, 0.01, 0.0 };

using namespace ci;
using namespace ci::app;
using namespace ci::gl;
using namespace sysinfo;

namespace cinder { namespace qb {
	
	// Globals
	qbMain __qb;
	
	//////////////////////////////////////////////////////////////////
	//
	// SETTINGS
	//
	qbMain::qbMain()
	{
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

		bInited					= false;
		bRenderToFbo			= true;
		bAutoResizeFbos			= false;
		bDrawGui				= true;
		bVerbose				= true;
		bSyphonControls			= true;
		bRenderControls			= true;
		bPaletteControls		= false;
		mFarThrowMultiplyer		= 2.1;
		mDefaultCamera			= CAMERA_TYPE_PERSP;
		mCameraNear				= 1.0f;		// Nao baixar muito para eviar Z-Fight
		mCameraScale			= Vec3f::one();
		
		mScreenName		= "QB";

		for (int u = 0 ; u < QB_MAX_UNITS ; u++ )
			mBoundSource[u]	= -1;
		
		[pool drain];
	}
	qbMain::~qbMain()
	{
		delete mConfig;
		std::map<int,gl::Light*>::const_iterator it;
		for ( it=mLights.begin() ; it != mLights.end() ; it++ )
			delete (it->second);
	}
	void qbMain::enableMultiTouch( AppBasic::Settings *settings )
	{
		mTouch.enable( settings );
	}
	
	
	//
	// ALL AUTOMATIC
	// Auto FBO Size, fit to window
	// Keep Metric aspect
	void qbMain::init()
	{
		bRenderToFbo = false;
		bAutoResizeFbos = true;
		Vec2i sz = app::getWindowSize();
		this->init( sz.x, sz.y );
	}
	
	//
	// Fized sizes
	void qbMain::init( int _w, int _h, bool _autoWindowSize )
	{
		// Screen name
		std::stringstream os;
		//os << "__" << SysInfo::getAppName();
		os << SysInfo::getAppName() << " v_" << SysInfo::getAppVersion();
		mScreenName = os.str();
		printf("----- QB app_name [%s] screen_name [%s] app_ver [%s]\n",SysInfo::getAppName().c_str(),mScreenName.c_str(),SysInfo::getAppVersionLong().c_str());

		// Make Config
		mConfig = new qbConfig();
		mConfig->setup();

		//
		// Render Setup
		// Call those same functions from inside your prepareSettings() to setup your project
		this->resizeRender( _w, _h );		// This is the render size for export
		this->resizeMetric( _w, _h );		// This is the app unit
		this->resizePrint( _w, _h );		// For High-res print export

		// Window Setup
		App::get()->setFpsSampleInterval(0.5);
		// Events setup
		getWindow()->connectResize( & qbMain::onResize, this );
		getWindow()->connectKeyDown( & qbMain::onKeyDown, this );
		getWindow()->connectFileDrop( & qbMain::onFileDrop, this );
		mTouch.setupEvents();

		// Scale window to screen
		if ( _autoWindowSize )
		{
			const DisplayRef disp = Display::getMainDisplay();
			if ( _w > disp->getWidth() || _h > disp->getHeight() )
			{
				int ww = _w;
				int wh = _h;
				for (int n = 0 ; (ww > disp->getWidth() || wh > disp->getHeight()) && winScales[n] ; n++)
				{
					ww = _w * winScales[n];
					wh = _h * winScales[n];
					printf(">>>> QB::WINDOW SCALE %.2f .. [%d/%d]\n",winScales[n],ww,wh);
				}
				// Force window resize
				app::setWindowSize( ww, wh );
			}
		}
		
		printf(">>>> QB::RENDER [%d/%d]\n",mRenderWidth,mRenderHeight);
		printf(">>>> QB::METRIC [%d/%d]\n",(int)mMetricWidth,(int)mMetricHeight);
		printf(">>>> QB::WINDOW [%d/%d]\n",mWindowWidth,mWindowHeight);

		// Make FBOs +  Cameras
		this->resizeFbos();
		this->resizeCameras();
		
		// Renderer file names
		os.str("");
		printf(">>>> QB::Base name [%s]\n",os.str().c_str());
		//os << "__" << SysInfo::getAppName();
		os << SysInfo::getAppName();
		printf(">>>> QB::Base name [%s]\n",os.str().c_str());
		mRenderer.setFileNameBase( os.str() );
		mRenderer.setFolder( QB_CAPTURE_FOLDER );
		
		// Misc objects
		mFontHelvetica = Font( "Helvetica", 12 );
		mFont = Font( loadResource("pf_tempesta_seven.ttf"), 8 );
		mCorner = gl::Texture( loadImage( loadResource( "resize_corner.png" ) ) );
		mSyphonIcon =  gl::Texture( loadImage( loadResource( "syphon.png" ) ) );
		mBackgroundColor = Color::gray(0.2f);

		// Modul8 Syphon Client
		mSyphonModul8.setup();
		mSyphonModul8.setApplicationName( "Modul8" );
		mSyphonModul8.setServerName("");
		
		// start Timer!
		mPlayhead.seekToTime( (double) mConfig->get(QBCFG_CURRENT_TIME) );
		if ( mConfig->getBool(QBCFG_PLAYING) )
			mPlayhead.resume();
		
		// Make Shaders
		mShaderStereo = gl::GlslProg( loadResource( "shStereoBlend.vert" ), loadResource( "shStereoBlend.frag" ) );
		mShaderStereo.bind();
		mShaderStereo.uniform( "u_texLeft",		(int) 0 );
		mShaderStereo.uniform( "u_texRight",	(int) 1 );
		mShaderStereo.unbind();
		
		// Default Material
		mMaterial.setSpecular( Color( 1.0f, 1.0f, 1.0f ) );
		mMaterial.setDiffuse( Color( 1.0f, 1.0f, 1.0f ) );
		mMaterial.setAmbient( Color( 0.5f, 0.5f, 0.5f ) );
		mMaterial.setShininess( 50.0f );

		// Setup OpenGL
		glPolygonOffset( 1.0f, 1.0f );
		glEnable( GL_POLYGON_OFFSET_FILL );
		
		// Sources
		this->addPath( getPathDirectory( app::getAppPath().string() ) + "Resources/" );
		
		// Palette manager
		mPalette.setupOsc();

		bInited = true;
	}


	
	//////////////////////////////////////////////////////////////////
	//
	// EVENTS
	//
	// resize event
	void qbMain::onResize()
	{
		this->resizeWindow( app::getWindowWidth(), app::getWindowHeight() );
		if ( bAutoResizeFbos )
		{
			this->resizeRender( app::getWindowWidth(), app::getWindowHeight() );
			this->resizeMetric( mMetricBase.x, mMetricBase.y );
		}
	}
	
	// keydown event
	void qbMain::onKeyDown( app::KeyEvent & event )
	{
		//printf("EVENT>> qbMain::keyDown [%d] [%c]\n",event.getChar(),event.getChar());
		// Save (COMMAND)
		switch( event.getChar() ) {
			case 27:	// ESC
				setFullScreen( ! isFullScreen() );
				this->hideCursorOrNot();
				break;
			case 'h':
			case 'H':
			case 'g':
			case 'G':
				mConfig->guiShowHide();
				this->hideCursorOrNot();
				break;
			case 'r':
			case 'R':
				if (event.isMetaDown()) // COMMAND
					mConfig->reset();
				break;
			case ' ':
				this->playSwitch();
				break;
			case KeyEvent::KEY_DELETE:
				this->rewind();
				break;
			case '/':
				mConfig->invert( QBCFG_PLAY_BACKWARDS );
				break;
			case 'y':
			case 'Y':
				if (event.isMetaDown() && bSyphonControls) // COMMAND
				{
					mConfig->invert( QBCFG_SYPHON_OUTPUT );
					event.setHandled();
				}
				break;
			case 'x':
			case 'X':
				if (event.isMetaDown() && bRenderControls) // COMMAND
				{
					mRenderer.startstop();
					event.setHandled();
				}
				break;
			case 'z':
			case 'Z':
				if (event.isMetaDown() && bRenderControls) // COMMAND
				{
					mRenderer.finish();
					event.setHandled();
				}
				break;
			case 'c':
			case 'C':
				if (event.isMetaDown()) // COMMAND
				{
					mRenderer.takeScreenshot();
					event.setHandled();
				}
				break;
			case 'p':
				if (event.isMetaDown()) // COMMAND
				{
					mConfig->invert( QBCFG_PREVIEW_DOWNSCALE );
					event.setHandled();
				}
				break;
			case 'P':
				if (event.isMetaDown()) // COMMAND
				{
					mConfig->invert( QBCFG_PREVIEW_UPSCALE );
					event.setHandled();
				}
				break;
		}
	}
	void qbMain::hideCursorOrNot()
	{
		if ( app::isFullScreen() && ! bDrawGui )
			AppBasic::get()->hideCursor();
		else
			AppBasic::get()->showCursor();
	}
	
	// Use file dropped on palette
	void qbMain::onFileDrop( ci::app::FileDropEvent & event )
	{
		if ( event.getNumFiles() == 0 )
			return;
		std::string theFile = event.getFile( 0 ).string();
		
		/*
		 stringstream ss;
		 ss << "You dropped files @ " << event.getPos() << " and the files were: " << endl;
		 for( size_t s = 0; s < event.getNumFiles(); ++s )
		 ss << event.getFile( s ) << endl;
		 console() << ss.str() << endl;
		 */
		
		// Reduce palette?
		if ( bPaletteControls )
		{
			mPalette.reduce( theFile );
			mConfig->set( QBCFG_PALETTE_FLAG, true );
			event.setHandled();
			return;
		}
		
		// Send to sources
		/*
		for ( auto it = mSources.begin() ; it != mSources.end(); it++ )
		{
			int i = it->first;
			if (this->source(i))
			{
				if (this->source(i).isCatchingFiles())
				{
					this->source(i).load(theFile);
					return;
				}
			}
		}
		 */
	}


	
	//////////////////////////////////////////////////////////////////
	//
	// SETTERS
	//
	void qbMain::resizeRender( int w, int h )
	{
		mRenderWidth = w;
		mRenderHeight = h;
		mRenderAspect = (mRenderWidth / (float)mRenderHeight);
		mRenderSize = Vec2i( w, h );
		mRenderBounds = Area( 0, 0, mRenderWidth, mRenderHeight );
		if ( mDomeMaster.isEnabled() )
			mDomeMaster.setup( mRenderWidth, mRenderHeight );

		// Resize FBOs
		this->resizeFbos();

		// Update GUI
		mConfig->set(DUMMY_RENDER_WIDTH, mRenderWidth);
		mConfig->set(DUMMY_RENDER_HEIGHT, mRenderHeight);
	}
	
	void qbMain::resizePrint( int w, int h )
	{
		mPrintWidth = w;
		mPrintHeight = h;
	}
	
	void qbMain::resizeMetric( float w, float h )
	{
		mMetricBase = Vec2f(w,h);
		if ( bAutoResizeFbos )
		{
			float a = (w / h);
			float s = (mRenderAspect / a);
			if ( mRenderAspect > a )
				w *= s;
			else
				h /= s;
		}
		
		mMetricWidth = w;
		mMetricHeight = h;
		mMetricAspect = (w / h);
		mMetricScale = (w / (float)mRenderWidth);
		mMetricSize = Vec2f( w, h );
		mMetricCenter = Vec3f( mMetricSize * 0.5, 0.0);
		mMetricBounds = Rectf( 0, 0, mMetricWidth, mMetricHeight );
		
		this->resizeCameras();
		this->resizePreview();

		// Update GUI
		mConfig->set(DUMMY_QB_WIDTH, mMetricWidth);
		mConfig->set(DUMMY_QB_HEIGHT, mMetricHeight);
		mConfig->set(DUMMY_QB_THROW, mMetricThrow);
	}
	
	// From App
	void qbMain::resizeWindow( int w, int h)
	{
		mWindowWidth = w;
		mWindowHeight = h;
		mAspectWindow = (mWindowWidth / (float)mWindowHeight);
		this->resizePreview();
		printf(">>>> QB::RESIZE WINDOW qb[%d/%d] win[%d/%d] %s\n",
			   mRenderWidth,mRenderHeight,
			   (int)getWindowWidth(),(int)getWindowHeight(),
			   (app::isFullScreen()?"FULLSCREEN":"WINDOWED"));
	}
	void qbMain::resizePreview()
	{
		// Fit render inside new window
		if (mAspectWindow > mRenderAspect)
		{
			mPreviewScale = (mWindowHeight / (float)mRenderHeight);
			float gap = (mWindowWidth - (mRenderWidth*mPreviewScale)) / 2.0f;
			mFittingBounds = Rectf( gap, 0, mWindowWidth-gap, mWindowHeight);
		}
		else
		{
			mPreviewScale = (mWindowWidth / (float)mRenderWidth);
			float gap = (mWindowHeight - (mRenderHeight*mPreviewScale)) / 2.0f;
			mFittingBounds = Rectf( 0, gap, mWindowWidth, mWindowHeight-gap );
		}
		// center render?
		if ( ! mConfig->getBool(QBCFG_PREVIEW_UPSCALE) && mPreviewScale > 1.0 )
		{
			mFittingBounds *= (1.0 / mPreviewScale);
			mFittingBounds += mRenderSize * (mPreviewScale-1.0) * 0.5f;
			mPreviewScale = 1.0;
		}
		mFittingArea = mRenderBounds;
		
		// Zoomed preview?
		if ( ! mConfig->getBool(QBCFG_PREVIEW_DOWNSCALE) && mPreviewScale < 1.0 )
		{
			Rectf b = mFittingBounds;
			if ( b.getWidth() == mWindowWidth )
			{
				int h = math<int>::min( mWindowHeight, mRenderHeight );
				b.y1 = (mWindowHeight - h) * 0.5f;
				b.y2 = b.y1 + h;
			}
			else
			{
				// TODO:: Testar isso (renders verticais)
				int w = math<int>::min( mWindowWidth, mRenderWidth );
				b.x1 = (mWindowWidth - w) * 0.5f;
				b.x2 = b.x1 + w;
			}
			mFittingBounds = b;
			mFittingArea = Area( 0, 0, b.getWidth(), b.getHeight() );
			mPreviewScale = 1.0;
		}
		mFittingAreaPan = Vec2f( 0.5f, 0.5f );
		
		// Update GUI
		mConfig->set(DUMMY_PREVIEW_SCALE, mPreviewScale);
	}
	
	
	//////////////////////////////////////////////////////////////////
	//
	// GETTERS
	//
	// Check if file exist and return it's absolute path
	// If it does not exist, return path to BUNDLED version
	// If it does not exist, return empty string
	std::string qbMain::getFilePath( const std::string & _f )
	{
		// File exist!
		fs::path p( _f );
		if( exists( p ) )
			return _f;
		
		// File in Bundle?
		std::string strippedFile = getPathFileName( _f );
		std::string bundleFile = App::get()->getResourcePath(strippedFile).string();
		if ( bundleFile.length() > 0 )
			return bundleFile;
		
		// Try path
		for (int n = 0 ; n < mPathList.size() ; n++ )
		{
			std::stringstream os;
			os.str("");
			os << mPathList[n] << "/" << _f;
			fs::path p( os.str() );
			//printf("GET FILE [%s] exist %d\n",os.str().c_str(),(int)exists(p));
			if( exists( p ) )
				return os.str();
		}
		
		// Not found!
		return std::string();
	}
	
	
	//////////////////////////////////////////////////////////////////
	//
	// CAMERA SETUP
	//
	void qbMain::resizeCameras()
	{
		mMetricThrow = math<float>::max(mMetricWidth,mMetricHeight) * mConfig->get( QBCFG_METRIC_THROW );
		mCamTarget = mMetricCenter;
		mCamEye = Vec3f( mCamTarget.x, mCamTarget.y, mCamTarget.z+mMetricThrow );
		mCameraFar = (mMetricThrow * mFarThrowMultiplyer);
		float fovV = atanf((mMetricHeight/2.0f)/mMetricThrow) * 2.0f;
		float fovH = atanf((mMetricWidth/2.0f)/mMetricThrow) * 2.0f;
		float fovGl = toDegrees(fovV);		// SE A ESCALA ESTIVER ESQUISITA, TROCAR W POR H
		//printf(">>>> QB::CAMERA throw [%.1f] near [%.1f] far [%.1f] fovV [%.1f] fovH [%.1f] aspect [%.2f]\n",mMetricThrow,mCameraNear,mCameraFar,toDegrees(fovV),toDegrees(fovH),mMetricAspect);
		//
		// Orthographic (parallel)
		// BIZARRO!! Nao entendi porque precisa voltar QB_CENTER.xy()
		//Rectf o = mMetricBounds + mCameraOffset.xy();
		Rectf o = mMetricBounds - QB_CENTER.xy();
		mCameraOrtho = CameraOrtho( o.x1, o.x2, o.y1, o.y2, mCameraNear, mCameraFar );
		//printf(">>>> QB::CAMERA left [%.1f] right [%.1f] top [%.1f] bottom [%.1f]\n",o.x1,o.x2,o.y1,o.y2);
		//
		// Fisheye Camera
		//mCameraDome = CameraOrtho( -1, +1, -1, +1, 0.0, 1000.0 );
		//mCameraDome.lookAt( CAMERA_EYE_DOME, CAMERA_TARGET_DOME, CAMERA_UP_DOME );
		//
		// Perspective
		mCameraPersp = CameraPersp( mMetricBounds.getWidth(), mMetricBounds.getHeight(), fovGl );
		mCameraPersp.setPerspective( fovGl, mMetricAspect, mCameraNear, mCameraFar );
		float p = mConfig->get( QBCFG_PERSPECTIVE_PROG );
		if ( p != 1.0 )
		{
			mCameraPersp.lerpProjection( p, mCameraOrtho );
			mCameraPersp.lerpModelView( p, mCameraOrtho );
		}
		//
		// Stereo Cameras
		// From: http://paulbourke.net/miscellaneous/stereographics/stereorender/
		float focalLength = mMetricThrow;
		mStereoSep = (mMetricThrow / 30.0f);
		mStereoDelta = (mStereoSep * mMetricWidth) / (2.0f * focalLength *  tan(fovH/2.0f));
		float stereoFovH = 2.0f * atanf( ( (mMetricWidth + mStereoDelta) * tan(fovH/2.0f) ) / mMetricWidth );
		float stereoW = 2.0f * mMetricThrow * tan(stereoFovH / 2.0f);
		float stereoAspect = (stereoW / mMetricHeight);
		mCameraLeft = CameraPersp( stereoW, mMetricBounds.getHeight(), fovGl );
		mCameraLeft.setPerspective( fovGl, stereoAspect, mCameraNear, mCameraFar );
		mCameraRight = CameraPersp( stereoW, mMetricBounds.getHeight(), fovGl );
		mCameraRight.setPerspective( fovGl, stereoAspect, mCameraNear, mCameraFar );
		//
		// Ground perspective
		if ( this->isCameraGroundActive() )
		{
			mCameraPersp.setLensShift( 0, -this->getCameraGroundShift() );
			mCameraLeft.setLensShift(  0, -this->getCameraGroundShift() );
			mCameraRight.setLensShift( 0, -this->getCameraGroundShift() );
		}
		//
		// Select active camera
		this->setActiveCamera();
		//printf(">>>> QB::CAMERA STEREO fovH [%.1f > %.1f] aspect [%.2f > %.2f]\n",toDegrees(fovH),toDegrees(stereoFovH),mMetricAspect,stereoAspect);
	}
	void qbMain::setActiveCamera( int type )
	{
		int t = ( type >= 0 ? type : this->getCameraType() );
		if ( t == CAMERA_TYPE_ORTHO )
			mCameraActive = &mCameraOrtho;
		else
			mCameraActive = &mCameraPersp;
	}
	//
	// CAMERA SETTERS
	void qbMain::setCameraOffset( const Vec3f & off )
	{
		mCameraOffset = off;
		if ( bInited ) this->resizeCameras();
	}
	void qbMain::setCameraScale( const Vec3f & sc )
	{
		mCameraScale = sc;
		if ( bInited ) this->resizeCameras();
	}
	void qbMain::setCameraNear( const float n )
	{
		mCameraNear = n;
		if ( bInited ) this->resizeCameras();
	}
	void qbMain::setCameraOnTheGround( const bool b )
	{
		mConfig->set( QBCFG_CAMERA_GROUND, b );
		if ( bInited ) this->resizeCameras();
	}
	void qbMain::setFarThrowMultiplyer( const float f )
	{
		mFarThrowMultiplyer = f;
		if ( bInited ) this->resizeCameras();
	}
	// Camera Type
	void qbMain::setCameraType( const int t )
	{
		mConfig->set( QBCFG_CAMERA_TYPE, t );
		if ( this->isCameraStereo() && ( !mFboLeft || !mFboLeft ) )
			this->resizeFbos();
		if ( bInited )
			this->resizeCameras();
	}
	void qbMain::setCameraStereo( bool b )					
	{
		this->setCameraType( b ? CAMERA_TYPE_STEREO : CAMERA_TYPE_PERSP );
	}
	void qbMain::stereoSwitch()
	{
		this->setCameraStereo( !this->isCameraStereo() );
	}
	//
	// CAMERA UPDATE/DRAW
	void qbMain::placeCamera( int type )
	{
		this->setActiveCamera( type );		// Select a camera
		this->updateCamera();
	}
	void qbMain::placeCameraLeft()
	{
		mCameraActive = &mCameraLeft;	// Select a camera
		Vec3f off = Vec3f( mStereoDelta * -0.5f, 0, 0 );
		this->updateCamera( off );
	}
	void qbMain::placeCameraRight()
	{
		mCameraActive = &mCameraRight;	// Select a camera
		Vec3f off = Vec3f( mStereoDelta * 0.5f, 0, 0 );
		this->updateCamera( off );
	}
	// private
	void qbMain::updateCamera( Vec3f off )
	{
		if ( this->isCameraGroundActive() )
			off += Vec3f( 0.0f, mMetricHeight * this->getCameraGroundShift() * 0.5, 0.0f );
		Vec3f e = mCamEye + mCameraOffset + off;
		Vec3f t = mCamTarget + mCameraOffset + off;
		mCameraActive->lookAt( e, t );
		//printf("__LOOKAT  %.1f %.1f %.1f  >  %.1f %.1f %.1f\n",e.x,e.y,e.z,t.x,t.y,t.z);
		gl::setMatrices( *mCameraActive );
		gl::setViewport( mRenderBounds );
		gl::scale(mCameraScale);
		// need to flip Y to UL when not using SetMatricesWindow()
		if ( ! bRenderToFbo )
		{
			glScalef( 1.0f, -1.0f, 1.0f );
			glTranslatef( 0.0f, -mMetricHeight, 0.0f );
		}
	}
	// place WINDOW camera to draw final FBO preview
	void qbMain::placeCameraWindow()
	{
		gl::setMatricesWindow( getWindowSize() );
		gl::setViewport( getWindowBounds() );
	}

	//////////////////////////////////////////////////////////////////
	//
	// FBOs
	//
	// Resize ALL FBOs
	void qbMain::resizeFbos()
	{
		// TODO: Explorar os samples
		gl::Fbo::Format		fmt;
		fmt.setTarget( GL_TEXTURE_RECTANGLE_ARB );
		//fmt.setSamples( 4 );		// antialiasing
		//fmt.setMinFilter(GL_LINEAR);
		//fmt.setMagFilter(GL_LINEAR);
		//fmt.enableMipmapping();

		// Make Render FBO
		if ( bRenderToFbo )
		{
			mFboRender = gl::Fbo( mRenderWidth, mRenderHeight, fmt );
			//mFboRender.getTexture().setFlipped();
			mFboRender.bindFramebuffer();
			gl::clear( ColorA(0,0,0,this->getAlpha()) );
			mFboRender.unbindFramebuffer();
		}
		else
			gl::clear( ColorA(0,0,0,this->getAlpha()) );
		
		// Stereo FBOs
		if ( this->isCameraStereo() || mFboLeft || mFboRight )
		{
			mFboLeft = gl::Fbo( mRenderWidth, mRenderHeight, fmt );
			mFboLeft.bindFramebuffer();
			gl::clear( ColorA(0,0,0,this->getAlpha()) );
			mFboLeft.unbindFramebuffer();
			mFboRight = gl::Fbo( mRenderWidth, mRenderHeight, fmt );
			mFboRight.bindFramebuffer();
			gl::clear( ColorA(0,0,0,this->getAlpha()) );
			mFboRight.unbindFramebuffer();
		}
		
		// Resize Additional FBOs
		std::map<int,gl::Fbo>::const_iterator it;
		for ( it=mFbos.begin() ; it != mFbos.end() ; it++ )
		{
			int i = (int)(it->first);
			//gl::Fbo fbo = (gl::Fbo) (it->second);
			// make new
			//mFbos[i] = gl::Fbo( mRenderWidth, mRenderHeight, true, true, true );
			mFbos[i] = gl::Fbo( mRenderWidth, mRenderHeight, fmt );
			mFbos[i].getTexture().setFlipped();
			// clear FBO
			mFbos[i].bindFramebuffer();
			gl::clear( ColorA(0,0,0,this->getAlpha()) );
			mFbos[i].unbindFramebuffer();
		}
	}
	//
	// TODO: Deal with FBOs with size different than RENDER
	void qbMain::makeFbo( int i )
	{
		this->makeFbo( i, mRenderWidth, mRenderHeight );
	}
	void qbMain::makeFbo( int i, float scale )
	{
		this->makeFbo( i, mRenderWidth * scale, mRenderHeight * scale );
	}
	void qbMain::makeFbo( int i, int w, int h )
	{
		// Make FBO ..., alpha, color, depth);
		mFbos[i] = gl::Fbo( mRenderWidth, mRenderHeight, true, true, true );
	}
	//
	// Bind FBO for drawing
	// PRIVATE
	void qbMain::bindFramebuffer( gl::Fbo & fbo )
	{
		if ( fbo )
			fbo.bindFramebuffer();
		if ( _cfg.getInt(QBCFG_MODUL8_INPUT) == MODUL8_BELOW )
			this->drawModul8( QB_BOUNDS );
	}
	void qbMain::bindFramebuffer( gl::Fbo & fbo, Color8u c )
	{
		this->bindFramebuffer( fbo, ColorA( c, this->getAlpha() ) );
	}
	void qbMain::bindFramebuffer( gl::Fbo & fbo, Color c )
	{
		this->bindFramebuffer( fbo, ColorA( c, this->getAlpha() ) );
	}
	void qbMain::bindFramebuffer( gl::Fbo & fbo, ColorA c )
	{
		if ( fbo )
			fbo.bindFramebuffer();
		gl::clear(c);
		if ( _cfg.getInt(QBCFG_MODUL8_INPUT) == MODUL8_BELOW )
			this->drawModul8( QB_BOUNDS );
	}

	////////////////////////
	//
	// QB Sources
	void qbMain::makeSource( int i, std::string _name, char _flags )
	{
		//mSources[i] = qb::qbSourceSelector( _name );
		mSources[i] = qb::qbSourceSelector();
		mSources[i].setFlags( _flags );
		if ( _name.length() > 0 )
			mSources[i].load( _name );
	}
	void qbMain::makeSourceFromList( int i, const char *name0, ... )
	{
		// make source
		mSources[i] = qb::qbSourceSelector();
		// populate list
		va_list args;
		va_start (args, name0);
		char *p = (char*) name0;			// get first
		for (int key = 0 ; p ; key++)
		{
			mSources[i].setList( key, std::string(p));
			p = va_arg (args, char *);	// next
		}
		va_end (args);
		// load first
		mSources[i].load( name0 );
	}
	void qbMain::makeSourceFromList( int i, const std::vector<std::string> & names, char _flags )
	{
		// make source
		mSources[i] = qb::qbSourceSelector();
		mSources[i].setFlags( _flags );
		// populate list
		for (int key = 0 ; key < names.size() ; key++)
			mSources[i].setList( key, names[key]);
		// load first
		if (names.size() > 0)
			mSources[i].load( names[0] );
	}
	void qbMain::bindSource( int i, int unit )
	{
		if ( mBoundSource[unit] != i )
		{
			this->unbindSource( unit );
			this->source( i ).bind( unit );
			mBoundSource[unit] = i;
		}
	}
	void qbMain::unbindSource(int unit)
	{
		if ( mBoundSource[unit] >= 0 )
		{
			this->source( mBoundSource[unit] ).unbind();
			mBoundSource[unit] = -1;
		}
	}
	void qbMain::unbindAllSources()
	{
		for (int u = 0 ; u < QB_MAX_UNITS ; u++)
			this->unbindSource(u);
	}

	
	////////////////////////
	//
	// Lights
#define LIGHT_AMBIENT_MIN		Color( 0.0f, 0.0f, 0.0f )
#define LIGHT_AMBIENT_MAX		Color( 0.9f, 0.9f, 0.9f )
#define LIGHT_DIFFUSE_MIN		Color( 0.0f, 0.0f, 0.0f )
#define LIGHT_DIFFUSE_MAX		Color( 0.9f, 0.9f, 0.9f )
#define LIGHT_SPECULAR_MIN		Color( 0.1f, 0.1f, 0.1f )
#define LIGHT_SPECULAR_MAX		Color( 1.0f, 1.0f, 1.0f )
	void qbMain::makeLight( int i, int type, const Vec3f &eye, const Vec3f &target )
	{
		gl::Light * l = new gl::Light( type, 0 );
		l->lookAt( eye, target );
		mLights[i] = l;
	}
	void qbMain::enableLight( int i, float intensity )
	{
		glEnable( GL_LIGHTING );
		gl::Light *l = mLights[i];
		l->setAmbient( LIGHT_AMBIENT_MIN + ((LIGHT_AMBIENT_MAX-LIGHT_AMBIENT_MIN) * intensity) );
		l->setDiffuse( LIGHT_DIFFUSE_MIN + ((LIGHT_DIFFUSE_MAX-LIGHT_DIFFUSE_MIN) * intensity) );
		l->setSpecular( LIGHT_SPECULAR_MIN + ((LIGHT_SPECULAR_MAX-LIGHT_SPECULAR_MIN) * intensity) );
		l->update( *mCameraActive );
		l->enable();
		mMaterial.apply();
	}
	void qbMain::disableLights()
	{
		// se desabilitar as luzes individuais, fode tudo
		//std::map<int,gl::Light*>::const_iterator it;
		//for ( it=mLights.begin() ; it != mLights.end() ; it++ )
		//	(it->second)->disable();
		glDisable( GL_LIGHTING );
	}

	
	//////////////////////////////////////////////////////////////////
	//
	// MAIN LOOP - UPDATE
	//
	void qbMain::update()
	{
		// Update from CONFIG
		mConfig->update();
		mConfig->set(DUMMY_CURRENT_FPS, App::get()->getAverageFps());
		
		// Changed tab from file
		if ( _cfg.isFresh(QBCFG_CURRENT_TAB) )
			_cfg.guiSetTab( _cfg.getInt( QBCFG_CURRENT_TAB ) );
		// changed on the GUI
		else if ( _cfg.getInt( QBCFG_CURRENT_TAB ) != _cfg.guiGetTabId() )
			_cfg.set( QBCFG_CURRENT_TAB, _cfg.guiGetTabId() );
		// avoid QB tab when QB tab is disabled
		if ( _cfg.getInt( QBCFG_CURRENT_TAB ) == 0 && ! mConfig->tabQB->enabled && mConfig->mGui->shouldDrawTabs() )
			_cfg.guiSetTab( 1 );


		// Get mouse preview pan
		Vec2f m = AppBasic::get()->getMousePosMainWindow();
		Vec2f mm = m / Vec2f( mWindowWidth, mWindowHeight);
		float pmin = 0.1;
		float pmax = 0.3;
		if ( mm.x < pmin )
			mMousePan.x = 0.0f;
		else if ( mm.x < pmax )
			mMousePan.x = lmap( mm.x, pmin, pmax, 0.0f, 0.5f);
		else if ( mm.x < (1.0f - pmax) )
			mMousePan.x = 0.5f;
		else if ( mm.x < (1.0f - pmin) )
			mMousePan.x = lmap( mm.x, (1.0f-pmax), (1.0f-pmin), 0.5f, 1.0f);
		else
			mMousePan.x = 1.0f;
		if ( mm.y < pmin )
			mMousePan.y = 0.0f;
		else if ( mm.y < pmax )
			mMousePan.y = lmap( mm.y, pmin, pmax, 0.0f, 0.5f);
		else if ( mm.y < (1.0f - pmax) )
			mMousePan.y = 0.5f;
		else if ( mm.y < (1.0f - pmin) )
			mMousePan.y = lmap( mm.y, (1.0f-pmax), (1.0f-pmin), 0.5f, 1.0f);
		else
			mMousePan.y = 1.0f;
		if ( ! mConfig->getBool(QBCFG_PREVIEW_DOWNSCALE) && ! mConfig->guiIsVisible() )
		mFittingAreaPan = Vec2f( ( mFittingBounds.getWidth() < mRenderWidth ? (mRenderWidth - mFittingBounds.getWidth()) * mMousePan.x : 0.0), 
								( mFittingBounds.getHeight() < mRenderHeight ? (mRenderHeight - mFittingBounds.getHeight()) * mMousePan.y : 0.0) );
		//printf("pan  %.2f  %.2f\n",mMousePan.x,mMousePan.y);
		// Metric mouse Pos
		mMousePos = ( Vec2f(m - mFittingBounds.getUpperLeft() + this->getFittingArea().getUL()) / mPreviewScale ) * mMetricScale;
		mMousePos += mCameraOffset.xy();
		mMousePos *= mCameraScale.xy();
		mConfig->set(DUMMY_MOUSE_X, mMousePos.x);
		mConfig->set(DUMMY_MOUSE_Y, mMousePos.y);
		
		// Palette networking
		mPalette.receiveOsc();
		
		// Update timer
		mPlayhead.update();
		
		// Update Sources
		for ( auto it = mSources.begin() ; it != mSources.end(); it++ )
		{
			int i = it->first;
			if (this->source(i))
				this->source(i).update();
		}

		// Update what needs updating
		mUpdatePool.update();
	}
	
	
	
	//////////////////////////////////////////////////////////////////
	//
	// MAIN LOOP - DRAW
	//
	// Compose STEREO to final FBO
	// Then draw Final FBO to Window
	void qbMain::blendStereo()
	{
		glEnable( GL_TEXTURE_2D );
		glDisable( GL_DEPTH_TEST );
		glDisable( GL_LIGHTING );
		
		this->bindFbo();
		
		this->placeCamera( CAMERA_TYPE_ORTHO );
		//gl::setMatricesWindow( mRenderSize );
		//gl::setViewport( mRenderBounds );
		
		gl::clear( Color::black() );
		gl::color( Color::white() );
		
		mFboLeft.getTexture().bind( 0 );
		mFboRight.getTexture().bind( 1 );
		
		mShaderStereo.bind();
		mShaderStereo.uniform( "u_delta", (float) (mStereoDelta / mMetricWidth) );
		glPushMatrix();
		gl::translate(mCameraOffset);		// BIZARRO!! Nao entendi porque esta deslocado
		//glTranslatef(0,mRenderHeight,0);
		//glScalef(1,-1,1);
		gl::drawSolidRect( mMetricBounds );	// render quad
		glPopMatrix();
		mShaderStereo.unbind();
		
		this->unbindFbo();
	}
	
	//
	// Draw Final FBO to Window
	void qbMain::drawModul8( Rectf b )
	{
		if ( bSyphonControls )
		{
			glDisable( GL_LIGHTING );
			SaveDepthState d;
			gl::disableDepthRead();
			gl::disableDepthWrite();
			SaveBlendingState bl;
			gl::enableAlphaBlending();
			gl::color( ColorA::white() );
			mSyphonModul8.draw( b );
		}
	}
	
	//
	// Draw Final FBO to Window
	void qbMain::finishAndDraw()
	{
		this->unbindAllSources();
		this->disableLights();
		
		glNormal3f(0,0,1);
		gl::color( Color::white() );

		// Blend Stereo Channels?
		if ( this->isCameraStereo() )
			this->blendStereo();
		
		// Draw Modul8
		if ( _cfg.getInt(QBCFG_MODUL8_INPUT) == MODUL8_ABOVE )
		{
			this->bindFbo();
			gl::setMatricesWindow( mRenderSize );
			gl::setViewport( mRenderBounds );
			glPushMatrix();
			glTranslatef(0,mRenderHeight,0);
			glScalef(1.0, -1.0, 1.0);
			this->drawModul8( mRenderBounds );
			glPopMatrix();
			this->unbindFbo();
		}
		
		// Publish Syphon
		gl::Texture tex;
		if ( bSyphonControls && mConfig->getBool(QBCFG_SYPHON_OUTPUT) )
		{
			if ( ! mSyphonServer.isRunning() )
				mSyphonServer.setName("Render");
			if ( bRenderToFbo )
			{
				if ( ! tex )
					tex = mFboRender.getTexture();
				mSyphonServer.publishTexture( tex );
			}
			else
				mSyphonServer.publishScreen();
		}
		else if ( mSyphonServer.isRunning() )
			mSyphonServer.shutdown();
		
		// Add to render
		// TODO:: Render sem FBO
		if ( mRenderer.isRendering() && bRenderToFbo )
		{
			if ( ! tex )
				tex = mFboRender.getTexture();
			mRenderer.add( tex );
			mConfig->setRenderTexture( &tex );
		}

		//
		// Draw FBO preview to Window
		if ( bRenderToFbo )
		{
			glEnable( GL_TEXTURE_2D );
			glDisable( GL_DEPTH_TEST );
			glDisable( GL_LIGHTING );
			gl::clear( mBackgroundColor );
			gl::color( Color::white() );
			this->placeCameraWindow();
			this->drawFbo( mFittingBounds, this->getFittingArea() );
		}
		
		// GUI Elements
		if ( ! AppBasic::get()->isMinimized() )
		{
			// GUI
			mConfig->draw();	// if needed
			bDrawGui = mConfig->guiIsVisible();
			if ( bDrawGui )
			{
				// preview stereo
				if ( this->isCameraStereo() )
				{
					float h = 200;
					float w = h * mRenderAspect;
					Rectf b(0,0,w,h);
					glPushMatrix();
					glTranslatef(0,getWindowHeight()-h,0);
					gl::enableAlphaBlending();
					gl::color(Color::red());
					gl::draw( mFboLeft.getTexture(), b );
					glTranslatef( w,0,0);
					gl::color(Color::cyan());
					gl::draw( mFboRight.getTexture(), b );
					gl::disableAlphaBlending();
					glPopMatrix();
				}

				// Resize corner
				gl::color( Color::white() );
				gl::enableAlphaBlending();
				if ( App::get()->getSettings().isResizable() && !App::get()->isFullScreen() )
					gl::draw( mCorner, Vec2f(getWindowWidth()-mCorner.getWidth(), getWindowHeight()-mCorner.getHeight()) );
				// Syphon Icon
				if ( mConfig->getBool(QBCFG_SYPHON_OUTPUT) )
					gl::draw( mSyphonIcon, Rectf( getWindowWidth()-mSyphonIcon.getWidth(), 0, getWindowWidth(), mSyphonIcon.getHeight()) );
				gl::disableAlphaBlending();
			}
		}
	}
	
	
	
	/////////////////////////////////////////////////
	//
	// MISC Objective-C functions
	//
	void qbMain::createFolder( const std::string & name )
	{
		NSURL *url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:name.c_str()]];
		[[NSFileManager defaultManager] createDirectoryAtURL:url withIntermediateDirectories:YES attributes: nil error:nil];
		printf("Create Folder [%s]\n",name.c_str());
	}

} } // cinder::qb







