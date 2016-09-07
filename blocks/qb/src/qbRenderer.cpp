//
//  qbRenderer.h
//
//  Created by Roger Sodre on 01/07/2011
//  Copyright 2011 Studio Avante. All rights reserved.
//
#include <stdlib.h>
#include "qbRenderer.h"
#include "qb.h"
#include "cinder/Filesystem.h"
#include "QuickTime/ImageCompression.h"

using namespace ci;
using namespace ci::gl;
#ifdef QTXX
using namespace ci::qtime;
#endif

namespace cinder { namespace qb {
	
	
	//////////////////////////////////////////////////////////////////
	//
	// RENDERER
	//
	qbRenderer::qbRenderer()
	{
		//mAudioStop = audio::load( loadResource( "audio_render_stopped.mp3" ) );
		//mAudioFinish = audio::load( loadResource( "audio_render_finished.mp3" ) );
		//mAudioScreenshot = audio::load( loadResource( "audio_screenshot.mp3" ) );
		// make file name base
		this->setFileNameBase( "__qb_render" );
		// Reset !
		this->reset();
		mStatus = "READY";
		mFramesString = "0";
		mAppFramerate = 60.0f;
	}
	qbRenderer::~qbRenderer()
	{
		if ( bOpened )
			this->finish();
	}
	void qbRenderer::setFolder( const std::string & _folder )
	{
		mFolder = _folder;
	}
	void qbRenderer::setFileNameBase( const std::string & _f )
	{
		mFileNameBase = _f;
	}
	void qbRenderer::reset()
	{
		bOpened = false;
		bIsRendering = false;
		mFramesMax = 0;
		mFramesStill = 0;
		mFramesAdded = 0;
		mFramesStarted = 0;
	}
	// Get render progression
	// before: sampling before actual animation
	float qbRenderer::getProg()
	{
		return (mFramesAdded-mFramesStarted) / (float)(mFramesMax-mFramesStarted-1);
	}
	
	//
	// Make name with current date and time
	std::string qbRenderer::makeFileNameTime( std::string _p, std::string _f, std::string _ext )
	{
		std::string name = getPathFileName( _f );
		std::string ext = ( _ext.length() ? _ext : getPathExtension( _f ) );
		time_t now;
		time ( &now );
		struct tm * t = localtime( &now );
		std::ostringstream os;
		os << _p << "/" << name << "_"
			<< (t->tm_year + 1900)
			<< "." << std::setfill('0') << std::setw(2) << (t->tm_mon + 1) 
			<< "." << std::setfill('0') << std::setw(2) << (t->tm_mday)
			<< "-" << std::setfill('0') << std::setw(2) << (t->tm_hour) 
			<< "." << std::setfill('0') << std::setw(2) << (t->tm_min) 
			<< "." << std::setfill('0') << std::setw(2) << (t->tm_sec) ;
		if ( _ext.length() > 0 )
			os << "." << ext;
		return os.str();
	}
	// Make framename for PNG export
	// mFileName is a Folder
	std::string qbRenderer::makeFileNameSerial()
	{
		char num[6];
		sprintf(num,"%05d",mFramesAdded);
		std::ostringstream os;
		os << mFileName << "/" << num << ".png";
		return os.str();
	}
	//
	// Open export file
	void qbRenderer::open()
	{
		// Open target file
		if ( ! _cfg.get(QBCFG_RENDER_PNG_SEQUENCE) )
		{
#ifdef QT32
			// codecs: https://developer.apple.com/library/mac/#documentation/QuickTime/Reference/QTRef_Constants/Reference/reference.html
			MovieWriter::Format format = MovieWriter::Format();
			format.setDefaultDuration( QB_FRAME_DURATION );
			//format.setCodec('rle ');	// Animation
			//format.setCodec('png ');
			format.setCodec('jpeg');
			format.setQuality( _cfg.get(QBCFG_RENDER_QUALITY) );
			mMovieWriter = qtime::MovieWriter( mFileName, QB_RENDER_WIDTH, QB_RENDER_HEIGHT, format );
			// new status
			std::ostringstream os;
			os << "RENDER OPEN [" << mFileName << "] "<<mMovieWriter.getWidth()<<" x "<<mMovieWriter.getHeight();
			printf("%s\n",os.str().c_str());
#endif
		}
		else
		{
			// Create output folder
			if( ! fs::exists( mFileName ) )
				_qb.createFolder(mFileName);
		}
		
		// New render
		mFramesAdded = 0;
		bOpened = true;
	}

	//
	// REC-HEAD
	void qbRenderer::startstop()
	{
		if ( ! bIsRendering )
			this->start();
		else 
			this->stop();
	}
	void qbRenderer::start( const std::string & _f )
	{
		// defaule folder name is beside app
		if ( mFolder.length() == 0 )
			this->setFolder( app::getAppPath().string() + "/.." );

		// Open new file
		if ( ! bOpened )
		{
			// Make file name
			if ( _f.c_str()[0] == '/' )
				mFileName = _f;
			else if ( ! _cfg.get(QBCFG_RENDER_PNG_SEQUENCE) )
				mFileName = makeFileNameTime( mFolder, mFileNameBase, "mov" );
			else
				mFileName = makeFileNameTime( mFolder, mFileNameBase, "" );
			// open file
			this->open();
		}
		
		// Start!
		mFramesStarted = mFramesAdded;
		mFramesMax = ( _qb.getRenderSeconds() ? mFramesAdded+(int)(_qb.getRenderSeconds()*QB_FRAMERATE) : 0 );
		mFramesStill =  (int)(_qb.getRenderStillSeconds()*QB_FRAMERATE);
		mTimeStart = app::getElapsedSeconds();

		// Start!
		mAppFramerate = app::App::get()->getFrameRate();
		app::App::get()->setFrameRate( 1000.0 );
		bIsRendering = true;
		
		// Play QB
		_qb.rewind();
		_qb.play();
		
		mStatus = "RENDERING...";
		printf("RENDER START max %d  still %d\n",mFramesMax,mFramesStill);
	}
	void qbRenderer::stop()
	{
		if ( bIsRendering )
		{
			app::App::get()->setFrameRate( mAppFramerate );
			bIsRendering = false;
			mStatus = "OK, SAVE IT!!!";
			if (mFramesMax == 0)
				mProgString = "";
			//audio::Output::play( mAudioStop );
			printf("RENDER STOPPED!\n");
		}
		this->updateStatus();
	}
	
	//
	// Finish export
	void qbRenderer::finish()
	{
		if ( mFramesAdded == 0 || ! bOpened )
		{
			mStatus = "Nothing to save!";
			mProgString = "";
			printf("NO FRAMES TO FINISH!!!\n");
			return;
		}
#ifdef QT32
		if ( ! _cfg.get(QBCFG_RENDER_PNG_SEQUENCE) )
			mMovieWriter.finish();
#endif
		mStatus = "SAVED!";
		mProgString = "";
		mFramesString = "";
		mTimeEstimated = 0;
		mTimeElapsed = 0;
		mTimeRemaining = 0;
		this->reset();
		_cfg.setRenderTexture( NULL );
		//audio::Output::play( mAudioFinish );
		printf("RENDER FINISHED!!! [%s]\n",mFileName.c_str());
	}
	
	//
	// Add frame to movie
	void qbRenderer::add()
	{
		this->add( app::copyWindowSurface() );
	}
	void qbRenderer::add( const ImageSourceRef & _aframe )
	{
		if ( ! bIsRendering )
			return;
		
		std::ostringstream os;
		if ( mFramesMax == 0)
		{
			os << "RENDER add frame "<<mFramesAdded<<" (ulimited)";
			printf("%s\n",os.str().c_str());
			this->commit( _aframe );
			mFramesAdded++;
		}
		else if (mFramesAdded < mFramesMax)
		{
			os << "RENDER add frame "<<mFramesAdded<<"/"<<mFramesMax<<"  "<<(int)(this->getProg()*100)<<"%";
			printf("%s\n",os.str().c_str());
			this->commit( _aframe );
			mFramesAdded++;
			// Reached the end
			if ( mFramesAdded == mFramesMax )
			{
				// add still frames
				if ( mFramesStill > 0 )
				{
					//float stillSecs = (mFramesStill * QB_FRAME_DURATION);
					//mMovieWriter.addFrame( _aframe, stillSecs );
					for (int n = 0 ; n < mFramesStill ; n++)
						this->commit( _aframe );
					mFramesAdded += mFramesStill;
					os.str("");
					os << "RENDER still "<<mFramesStill<<" frames";
					printf("%s\n",os.str().c_str());
				}
				// HACK!! MovieWriter not saving last frame, add again!
				else if ( ! _cfg.get(QBCFG_RENDER_PNG_SEQUENCE) )
					this->commit( _aframe );
				// Stop!!
				this->stop();
			}
		}
		mTimeElapsed = app::getElapsedSeconds() - mTimeStart;
		this->updateStatus();
	}
	void qbRenderer::commit( const ImageSourceRef & _aframe )
	{
		if ( _cfg.get(QBCFG_RENDER_PNG_SEQUENCE) )
			this->takeScreenshot( _aframe, this->makeFileNameSerial() );
#ifdef QT32
		else
			mMovieWriter.addFrame( _aframe, QB_FRAME_DURATION );
#endif
	}
	
	void qbRenderer::updateStatus()
	{
		if ( bIsRendering )
		{
			mStatus = "RENDERING...";
			if ( mFramesMax == 0)
				mProgString = "unlimited";
			else
			{
				std::ostringstream os;
				os << (int)(this->getProg()*100)<<"%";
				mProgString = os.str();
			}
		}
		
		// Frame count
		std::ostringstream os;
		if ( bIsRendering )
		{
			// Progress
			os << mFramesAdded << " / " << ( mFramesMax == 0 ? std::string("?") : toString(mFramesMax) );
			mFramesString = os.str();
			// Time
			mTimeEstimated = ( mFramesMax == 0 ? 0 : mTimeElapsed / this->getProg() );
			mTimeRemaining = ( mFramesMax == 0 ? 0 : mTimeEstimated - mTimeElapsed );
		}
		else
		{
			os.setf(std::ios::fixed);
			os.precision(1);
			os << mFramesAdded;
			mFramesString = os.str();
			// Time
			mTimeEstimated = 0;
			mTimeRemaining = 0;
		}
	}

	//
	// Save Screenshot
	void qbRenderer::takeScreenshot()
	{
		this->takeScreenshot( _qb.getFboTexture() );
	}
	void qbRenderer::takeScreenshot( const ImageSourceRef & _aframe)
	{
		std::string filename = this->makeFileNameTime( mFolder, mFileNameBase, "png" );
		this->takeScreenshot( _aframe, filename );
	}
	void qbRenderer::takeScreenshot( const ImageSourceRef & _aframe, const std::string _filename )
	{
		//printf("RENDER SCREENSHOT...\n");
		writeImage( _filename, _aframe );
		//audio::Output::play( mAudioScreenshot );
		printf("RENDER SCREENSHOT [%s]\n",_filename.c_str());
	}
	
} } // cinder::qb







