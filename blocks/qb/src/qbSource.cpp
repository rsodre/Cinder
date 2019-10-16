//
//  qbSource.h
//
//  Created by Roger Sodre on 01/07/2011
//  Copyright 2011 Studio Avante. All rights reserved.
//

#include "cinder/Filesystem.h"

#include "qbSource.h"
#include "qbPalette.h"
#include "ciConfig.h"
#include "qb.h"

#include "CinderNDIReceiver.h"
 #include "CinderNDIFinder.h"

using namespace ci;
using namespace ci::gl;
#ifdef QTXX
using namespace ci::qtime;
#endif

namespace cinder { namespace qb {
	
	std::vector<std::string> _qbSourceExt;

	//////////////////////////////////////////////////////////////////
	//
	// WRAPPER Source
	//
	qbSourceSelector::qbSourceSelector() : mSrc( std::shared_ptr<qbSourceBase>( new qbSourceBase() ) )
	{
		bNewFrame		= false;
		bCatchingFiles	= false;
		mFlags			= 0;
		cfgSelectorId	= -1;
		cfgTriggerId	= -1;
		cfgNameId		= -1;
		
		mCatchConn = app::getWindow()->connectFileDrop( & qbSourceSelector::onFileDrop, this );
		
		// possible extensions
		// if add any, chage isMovieFile() and isImageFile() as well
		if ( _qbSourceExt.empty() )
		{
#ifdef QTXX
			_qbSourceExt.push_back( "mov" );
//			_qbSourceExt.push_back( "mp4" );
#endif
			_qbSourceExt.push_back( "jpg" );
			_qbSourceExt.push_back( "jpeg" );
			_qbSourceExt.push_back( "png" );
		}
	}
	qbSourceSelector::~qbSourceSelector()
	{
		mCatchConn.disconnect();
	}
	
	bool qbSourceSelector::isMovieFile( const std::string & f )
	{
		std::string ext = getPathExtension( f );
		return ( ! toLower(ext).compare(0,3,"mov") || ! toLower(ext).compare(0,3,"mp4") );
	}
	bool qbSourceSelector::isImageFile( const std::string & f )
	{
		std::string ext = getPathExtension( f );
		return ( ! toLower(ext).compare(0,3,"jpg") || ! toLower(ext).compare(0,3,"jpeg") || ! toLower(ext).compare(0,3,"png") );
	}

	/*qbSourceSelector::qbSourceSelector(const std::string & _f) : mSrc( std::shared_ptr<qbSourceBase>( new qbSourceBase() ) )
	{
		this->load(_f);
	}*/
	bool qbSourceSelector::load( const std::string & _f )
	{
		printf("SOURCE [%s]\n",_f.c_str());
		// Syphn prefix
		if ( _f.compare(0,8,"syphon::") == 0 )
		{
			this->loadSyphon( _f.substr(8), "" );
		}
		else if ( _f.compare(0,5,"ndi::") == 0 )
		{
			this->loadNDI( _f.substr(5), "" );
		}
#ifdef QB_PALETTE
		else if ( _f.compare("qbSourcePalette") == 0 )
		{
			qbSourcePalette *newSrc = new qbSourcePalette();
			this->setSource(newSrc);
		}
#endif
		else
		{
			// Load by extension
			if ( this->isMovieFile(_f) )
			{
#ifdef QTXX
				qbSourceMovie *newSrc = new qbSourceMovie();
				if ( newSrc->load(_f,mFlags) == false )
				{
					delete newSrc;
					return false;
				}
				this->setSource(newSrc);
#else
				return false;
#endif
			}
			else if ( this->isImageFile(_f) )
			{
				qbSourceImage *newSrc = new qbSourceImage();
				if ( newSrc->load(_f,mFlags) == false )
				{
					delete newSrc;
					return false;
				}
				this->setSource(newSrc);
			}
			else
				return false;
		}
		// Update ciConfig
		if ( cfgNameId >= 0 )
			cfgNamePtr->set( cfgNameId, _f );
		return true;
	}
	bool qbSourceSelector::loadSyphon( const std::string & _app, const std::string & _tex )
	{
		qbSourceSyphon *newSrc = new qbSourceSyphon();
		newSrc->load( _app, _tex, mFlags );
		this->setSource(newSrc);
		return true;
	}
	bool qbSourceSelector::loadNDI( const std::string & _app, const std::string & _tex )
	{
		qbSourceNDI *newSrc = nullptr;
		try {
			newSrc = new qbSourceNDI();
		} catch ( const std::exception &e ) {
			return false;
		}
		newSrc->load( _app, _tex, mFlags );
		this->setSource(newSrc);
		return true;
	}
	void qbSourceSelector::setSource( qbSourceBase * newSrc )
	{
		mSrc = std::shared_ptr<qbSourceBase>( newSrc );
	}
	void qbSourceSelector::onFileDrop( ci::app::FileDropEvent & event )
	{
		if ( bCatchingFiles && event.getNumFiles() > 0 )
		{
			std::string theFile = event.getFile( 0 ).string();
			if ( this->load(theFile) )
				event.setHandled();
		}
	}
	
	// Source shortcut list
	void qbSourceSelector::setList( int _key, std::string _name )
	{
		mList[_key] = _name;
		if (cfgSelectorId >= 0)
			cfgSelectorPtr->setValueLabel( cfgSelectorId, _key,  getPathFileName(_name) );
	}
	void qbSourceSelector::useConfigSelector( int _id, ciConfig *_ptr, bool popValueList )
	{
		cfgSelectorId = _id;
		cfgSelectorPtr = ( _ptr ? _ptr : &_cfg );
		
		//cfgSelectorPtr->setValueLabels( cfgSelectorId, mList );
		if (popValueList)
		{
			std::map<int,std::string>::const_iterator it;
			for ( it = mList.begin() ; it != mList.end(); it++ )
				cfgSelectorPtr->setValueLabel( cfgSelectorId, it->first,  getPathFileName(it->second) );
		}
		else
			cfgSelectorPtr->setLimits(_id,0,mList.size()-1);
	}
	void qbSourceSelector::useConfigTrigger( int _id, ciConfig *_ptr )
	{
		cfgTriggerId = _id;
		cfgTriggerPtr = ( _ptr ? _ptr : &_cfg );
	}
	void qbSourceSelector::useConfigName( int _id, ciConfig *_ptr )
	{
		cfgNameId = _id;
		cfgNamePtr = ( _ptr ? _ptr : &_cfg );
	}
	//
	// Main Loop update
	void qbSourceSelector::update()
	{
		//
		// Config Selector, from source List
		bool playing = ( mSrc ? mSrc->isPlaying() : false );
		if (cfgSelectorId >= 0)
		{
			// selected on list
			if ( cfgSelectorPtr->isFresh(cfgSelectorId ) )
			{
				int i = cfgSelectorPtr->getInt(cfgSelectorId);
				if (mList.find(i) != mList.end())
				{
					// play by name?
					if (cfgNameId >= 0)
						cfgNamePtr->set( cfgNameId, mList[i] );
					// play now
					else if ( this->load( mList[i] ) )
						this->play( playing );
				}
			}
		}

		//
		// Config Selector by NAME
		if (cfgNameId >= 0)
		{
			// Source name is new??
			if ( cfgNamePtr->isFresh(cfgNameId ) )
			{
				std::string name = cfgNamePtr->getString(cfgNameId);
				if ( ! name.empty() )
				{
					if ( this->load( name ) )
					{
						this->play( playing );
						// select on list
						cfgSelectorPtr->set( cfgSelectorId, -1 );
						for ( auto it=mList.begin() ; it != mList.end() ; it++ )
							if ( name.compare(it->second) == 0 )
								cfgSelectorPtr->set( cfgSelectorId, it->first );
					}
				}
			}
		}

		//
		// Config Trigger -- PLAY / STOP
		if (cfgTriggerId >= 0)
		{
			bool shouldBePlaying = cfgTriggerPtr->getBool(cfgTriggerId);
			if ( ! mSrc->isPlaying() && shouldBePlaying )
				this->play();
			else if ( mSrc->isPlaying() && ! shouldBePlaying )
				this->stop();
		}
		
		// update Frame
		bNewFrame = mSrc->updateFrame();
	}

	
	///////////////////////////////////////////
	//
	// BASE
	//
	qbSourceBase::qbSourceBase()
	{
		bPlaying		= false;
		bBundled		= false;
		bHasAlpha		= false;
		bBackwards		= false;
		mBoundUnit		= 0;
		mSpawnedAtFrame = -1;
		mSize = Vec2f::zero();
		mTexFormat.setWrapS( GL_REPEAT );
		mTexFormat.setWrapT( GL_REPEAT );
		mTexFormat.setTarget( GL_TEXTURE_RECTANGLE_ARB );		// compatible with MovieGl/Syphon
		//mTexFormat.enableMipmapping();						// useless for RECTANGLE
		//mTexFormat.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);		// useless for RECTANGLE
		//mTexFormat.setMagFilter(GL_LINEAR_MIPMAP_LINEAR);		// useless for RECTANGLE
		mUV = Vec2f::zero();
		this->makeUV( 1.0f, 1.0f );
#ifdef VERBOSE_SOURCE
		std::cout << ">>>>>>> qbSourceBase CONSTRUCT this [" << this << "] " << mName << std::endl;
#endif
	}
	qbSourceBase::~qbSourceBase()
	{
#ifdef VERBOSE_SOURCE
		std::cout << ">>>>>>> qbSourceBase DESTRUCT this [" << this << "] " << mName << std::endl;
#endif
	}
	//
	// Draw Source info on screen
	void qbSourceBase::drawInfo( const Vec2f & p )
	{
		Vec2f pp = p + Vec2f(3,0);
		gl::enableAlphaBlending();
		if ( mName.length() )
			gl::drawString( mName, pp, Color::white(), _qb.mFont);
		else
			gl::drawString( "Source not loaded!", p, Color::white(), _qb.mFont);
		if ( mDesc.length() )
		{
			pp += Vec2f(0,10);
			gl::drawString( mDesc, pp, Color::white(), _qb.mFont);
		}
		if ( mDesc2.length() )
		{
			pp += Vec2f(0,10);
			gl::drawString( mDesc2, pp, Color::white(), _qb.mFont);
		}
		gl::disableAlphaBlending();
	}
	//
	// Make cube texture coordinates
	void qbSourceBase::makeUV( float _u, float _v )
	{
		if ( mUV.x != _u || mUV.y != _v )
		{
			mUV = Vec2f( _u, _v );
			for (int f = 0 ; f < QB_CUBE_FACES ; f++)
			{
				for (int v = 0 ; v < QB_QUAD_VERTICES ; v++)
				{
					mCubeTexes[f][v][0] = __qbCubeTexesQuad[f][v][0] * _u;
					mCubeTexes[f][v][1] = __qbCubeTexesQuad[f][v][1] * _v;
					if ( mTex )
						if( mTex.isFlipped() )
							mCubeTexes[f][v][1] = _v - mCubeTexes[f][v][1];
					//printf("makeUV uv[%.1f/%.1f]  f %d  v %d  tex[%.1f/%.1f]\n",_u,_v,f,v,mCubeTexes[f][v][0],mCubeTexes[f][v][1]);
				}
			}
		}
	}
	//
	// Bind saving/retrieving current target
	void qbSourceBase::bind(int unit)
	{
		if (mTex)
		{
			glGetBooleanv( mTex.getTarget(), &mOldTargetBinding );
			mTex.enableAndBind( unit );
			mBoundUnit = unit;
		}
	}
	void qbSourceBase::unbind()
	{
		if (mTex)
		{
			mTex.unbind( mBoundUnit );
			if( mOldTargetBinding )
				glEnable( mTex.getTarget() );
			else
				glDisable( mTex.getTarget() );
		}
	}

	//
	// MOTOLED
	// input x/y as prog (0.0 to 1.0)
	ColorA qbSourceBase::getColorProg( float _px, float _py )
	{
		const Surface8u & surf = this->getSurface();
		if ( ! surf )
			return ColorAf::white();
		int x = (int) (_px * mSize.x);
		int y = (int) (_py * mSize.y);
		ColorA c = surf.getPixel(Vec2i(x,y));
		//printf("____color %.2f %.2f = %d %d = %.2f %.2f %.2f\n",_px,_py,x,y,c.r,c.g,c.b);
		//return ( c.a == 0 ? ColorA::zero() : c );
		return c;
	}
	
	
	///////////////////////////////////////////
	//
	// IMAGE
	//
	bool qbSourceImage::load( const std::string & _f, char _flags )
	{
		std::string theFile = _qb.getFilePath( _f );
		if ( theFile.length() == 0 )
			return false;

		// try to load new image
		Texture newTex = gl::Texture( loadImage( loadFile( theFile ) ), mTexFormat );
		if ( ! newTex )
			return false;
		
		// save new image
		mTex = newTex;
		mSize = mTex.getSize();
		bHasAlpha = mTex.hasAlpha();
		mSurface = Surface8u();		// clear surface
		this->makeUV( mTex.getMaxU(), mTex.getMaxV() );

		// make name
		std::stringstream os;
		os << getPathFileName( theFile );
		if ( theFile.find(".app/Contents/Resources/") != std::string::npos )
			os << " (Bundle)";
		mName = os.str();

		// make desc
		os.str("");
		os << "Image: " << mSize.x << " x " << mSize.y << " " << ( bHasAlpha ? "RGBA" : "RGB" );
		mDesc = os.str();
		mDesc2 = "";
		mSpawnedAtFrame = app::getElapsedFrames();
		mFullPath = _f;
		this->play();	// Always playing
		
		printf("SOURCE Image [%s] loaded!\n",theFile.c_str());
		return true;
	}
	
	
	
	///////////////////////////////////////////
	//
	// MOVIE
	//
#ifdef QTXX
	bool qbSourceMovie::load( const std::string & _f, char _flags )
	{
		std::string theFile = _qb.getFilePath( _f );
		if ( theFile.length() == 0 )
			return false;
		
		// try to load new movie
		try {
			printf("SRC FLAG %d [%s]\n",_flags,_f.c_str());
			
			if ( TEST_FLAG( _flags, QBFLAG_SURFACE) )
			{
#if defined(QT64)
				mMovieGl = qtime::MovieGl();
#elif defined(QT32)
				mMovieGl = qtime::MovieGlHap();
				mMovieGl.setAsRect();
#endif
				mMovieSurface = qtime::MovieSurface( theFile );
				mMovie = & mMovieSurface;
			}
			else
			{
#if defined(QT64)
				mMovieGl = qtime::MovieGl( theFile );
#elif defined(QT32)
				mMovieGl = qtime::MovieGlHap( theFile );
				mMovieGl.setAsRect();
#endif
				mMovieSurface = qtime::MovieSurface();
				mMovie = & mMovieGl;
			}
			if ( ! mMovie )
				return false;
			mMovie->play();
			mMovie->stop();			// we're gonna play it frame by frame
			mMovie->setLoop();
			mMovie->setVolume(1.0f);
			mSize = mMovie->getSize();
			bHasAlpha = mMovie->hasAlpha();
			mFrameCount = mMovie->getNumFrames() + 1;
			mDuration = mMovie->getDuration();
			mFrameRate = (mFrameCount / mDuration);	//mMovie->getFramerate();
			mFrameTime = (mDuration / mFrameCount);
			mDurationQT = (mDuration - mFrameTime);
			mFullPath = _f;
			mCurrentTime = 0.0;
			mCurrentFrame = mLastRenderedFrame = 0;
			mFpsTime = mFpsFrames = 0;
		}
		catch( ... ) {
			printf("ERRO!!! MovieGl throws...\n");
			return false;
		}
		
		// grab a frame
		this->updateFrame(true);
		
		// make name
		std::stringstream os;
		os << getPathFileName( theFile );
		if ( theFile.find(".app/Contents/Resources/") != std::string::npos )
			os << " (Bundle)";
		mName = os.str();
		
		// make desc
		os.str("");
		os.setf(std::ios::fixed);
		os.precision(1);
		os << "Movie: " << mSize.x << " x " << mSize.y << ", " << mDuration << "s";
		mDesc = os.str();
		mSpawnedAtFrame = app::getElapsedFrames();

		printf("SOURCE Movie [%s] loaded as %s!\n",theFile.c_str(),(mMovieGl?"MovieGl":"MovieSurface"));
		return true;
	}

	//
	// Get new Frame
	// qbUpdateObject VIRTUAL
	bool qbSourceMovie::updateFrame( bool _force )
	{
		if ( ! mMovie )
			return false;
		
		bool newFrame = false;
		
		// start/stop
		/* no need to play as we do it frame by frame
		if ( bPlaying && ! mMovie->isPlaying() )
			mMovie->play();
		else if ( ! bPlaying && mMovie->isPlaying() )
			mMovie->stop();
		 */
		
		// maybe this shouldn't be playing...
		//if ( ( ! bPlaying || ! _qb.isPlaying() ) && ! _force )
		//	return false;
		
		// Time Profiler
		double d = (app::getElapsedSeconds() - mTimeProfiler);
		mTimeProfiler = app::getElapsedSeconds();

		// Always play frame by frame
		float t = ( _qb.shouldRenderFitSources() ? QB_ANIM_PROG * mDuration : QB_TIME );
		//if (_renderer.isRendering()) printf("PLAY   t0 %.8f    duration %.8f   qt %.8f\n",t,mDuration,mDurationQT);
		t = math<float>::fmod( t, mDuration );
		if (bBackwards)
			t = (mDurationQT - t);
		int fr = roundf(mFrameCount * (t/mDuration));
		//if (_renderer.isRendering()) printf("PLAY   t1 %.8f     fr %d\n",t,fr);
		// same frame, abort!
		if ( mLastRenderedFrame == fr && ! _force )
			return false;
		//mMovie->seekToTime( t );
		mMovie->seekToFrame( fr );
		mLastRenderedFrame = fr;
		newFrame = true;
		
		// load new frame?
		if ( newFrame )
		{
			if ( mMovieGl )
			{
				mTex = mMovieGl.getTexture();
				mSurface = Surface8u();
			}
			else
			{
				mSurface = mMovieSurface.getSurface();
				mTex = gl::Texture( mSurface );
			}
			//mCurrentTime = mMovie->getCurrentTime();
			//mCurrentFrame = ((mCurrentTime / mDuration) * mFrameCount);
			mCurrentTime = t;
			mCurrentFrame = fr;

			// calc UV?
			this->makeUV( mTex.getMaxU(), mTex.getMaxV() );
			
			// update desc
			std::stringstream os;
			os << mFrameCount << " f .. " << mCurrentFrame << " f / ";
			os.setf(std::ios::fixed);
			os.precision(1);
			os << mDuration << " s .. " << mCurrentTime << " s";
			mDesc2 = os.str();

			// Update current framerate
			mFpsFrames++;
			double now = app::getElapsedSeconds();
			double dt = now - mFpsTime;
			float interval = 0.25;
			if ( dt >= interval )
			{
				mCurrentFrameRate = mFpsFrames / interval;
				mFpsTime = now;
				mFpsFrames = 0;
			}
		}
//if (_renderer.isRendering())
//printf("SOURCE PLAY  time %.8f / fr %d\n",mCurrentTime,mCurrentFrame);;

		// Time Profiler
		d = (app::getElapsedSeconds() - mTimeProfiler);
		//rintf("%d) t(%.5f) QB source out...\n",app::getElapsedFrames(),(float)d);
		mTimeProfiler = app::getElapsedSeconds();

		return newFrame;
	}
#endif	// QTXX

	
	
	///////////////////////////////////////////
	//
	// SYPHON
	//
	bool qbSourceSyphon::load( const std::string & _app, char _flags )
	{
		return this->load( _app, "", _flags );
	}
	bool qbSourceSyphon::load( const std::string & _app, const std::string & _tex, char _flags )
	{
		mSyphonClient.setApplicationName( _app );
		mSyphonClient.setServerName(_tex);
		this->updateFrame(true);

		// source properties
		bHasAlpha = true;
		
		// Make name
		std::stringstream os;
		os << _app;
		if ( _tex.length() )
			os << " - " << _tex;
		mName = os.str();
		mDesc2 = "";
		mSpawnedAtFrame = app::getElapsedFrames();
		this->play();	// Always playing

		printf("SOURCE Syphon [%s] loaded!\n",_app.c_str());
		return true;
	}
	//
	// Get new Frame
	// qbUpdateObject VIRTUAL
	bool qbSourceSyphon::updateFrame( bool _force )
	{
		if ( ! _force && ! mSyphonClient.hasNewFrame() )
			return false;
		mSyphonClient.update();
		//printf("SyphonClient.update() > qbSourceSyphon\n");
		mTex = mSyphonClient.getTexture();
		mSize = mSyphonClient.getSize();
		if(mSurface)
			mSurface = Surface8u();		// clear surface
		// calc UV?
		if (mTex)
			this->makeUV( mTex.getMaxU(), mTex.getMaxV() );
		// update desc
		std::stringstream os;
		os << "Syphon: " << mSize.x << " x " << mSize.y;
		mDesc = os.str();
		return true;
	}
	
	
	///////////////////////////////////////////
	//
	// NDI
	//
	CinderNDIFinderPtr _CinderNDIFinder;
	qbSourceNDI::qbSourceNDI() : qbSourceBase()
	{
		// Create the NDI finder
		if (!_CinderNDIFinder)
		{
			CinderNDIFinder::Description finderDscr;
			_CinderNDIFinder = std::make_unique<CinderNDIFinder>( finderDscr );
		}
		
		mNDIVoice = ci::audio::Voice::create( [ this ] ( ci::audio::Buffer* buffer, size_t sampleRate ) {
			if( mCinderNDIReceiver ) {
				auto audioBuffer = mCinderNDIReceiver->getAudioBuffer();
				if( audioBuffer && ! audioBuffer->isEmpty() ) {
					buffer->copy( *audioBuffer.get() );
				}
			}
		}, ci::audio::Voice::Options().channels( 2 ));
		mNDIVoice->start();
		
		mDesc = "NDI";
	}

	const float	qbSourceNDI::getCurrentFrameRate()
	{
		return ( mCinderNDIReceiver ? mCinderNDIReceiver->getFrameRate() : 0 );
	}

	void qbSourceNDI::audioSetVolume(float volume)
	{
		if(mNDIVoice->getVolume() != volume)
			mNDIVoice->setVolume(volume);
	}

//	void qbSourceNDI::bind(int unit)
//	{
//		if(mCinderNDIReceiver)
//			mCinderNDIReceiver->bind(unit);
//	}
//	void qbSourceNDI::unbind()
//	{
//		if(mCinderNDIReceiver)
//			mCinderNDIReceiver->unbind();
//	}

	bool qbSourceNDI::load( const std::string & _app, char _flags )
	{
		return this->load( _app, "", _flags );
	}
	bool qbSourceNDI::load( const std::string & _app, const std::string & _tex, char _flags )
	{
		// Create the NDI receiver for this source
		NDISource source;
		if( !_CinderNDIFinder->getSource( _app, source ) )
		{
			printf("SOURCE NDI source ERROR [%s]!\n",_app.c_str());
			return false;
		}
		
		if( ! mCinderNDIReceiver )
		{
			CinderNDIReceiver::Description recvDscr;
			recvDscr.source = &source;
			mCinderNDIReceiver = std::make_unique<CinderNDIReceiver>( recvDscr );
		}
		else
		{
			mCinderNDIReceiver->connect( source );
		}
		
		this->updateFrame(true);
		
		// source properties
		bHasAlpha = true;
		
		// Make name
		std::stringstream os;
		os << _app;
		if ( _tex.length() )
			os << " - " << _tex;
		mName = os.str();
		mDesc2 = "";
		mSpawnedAtFrame = app::getElapsedFrames();
		this->play();	// Always playing
		
		printf("SOURCE NDI [%s] loaded!\n",_app.c_str());
		return true;
	}
	//
	// Get new Frame
	// qbUpdateObject VIRTUAL
	bool qbSourceNDI::updateFrame( bool _force )
	{
		auto videoTex = mCinderNDIReceiver != nullptr ? mCinderNDIReceiver->getVideoTexture() : nullptr;

//		if ( ! _force && ! videoTex )
		if ( !videoTex )
			return false;
		
		//printf("SyphonClient.update() > qbSourceNDI\n");
		mTex = *videoTex;
		mSize = videoTex->getSize();
		if(mSurface)
			mSurface = Surface8u();		// clear surface
		// calc UV?
		if (mTex)
			this->makeUV( mTex.getMaxU(), mTex.getMaxV() );
		// update desc
		std::stringstream os;
		os << "NDI: " << mSize.x << " x " << mSize.y;
		mDesc = os.str();
		
		if(this->canPlayAudio())
			this->audioSetVolume( _cfg.getBool(QBCFG_AUDIO_MUTE) ? 0 : _cfg.getFloat(QBCFG_AUDIO_VOLUME) );
		
		return true;
	}
	
	
} } // cinder::qb







