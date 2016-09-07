//
//  qbRenderer.h
//
//  Created by Roger Sodre on 01/07/2011
//  Copyright 2011 Studio Avante. All rights reserved.
//
#pragma once

#include "cinder/Cinder.h"
#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
//#include "cinder/audio/Output.h"
//#include "cinder/audio/Io.h"
#if defined(QT64)
#include "Avf.h"	// https://github.com/calebjohnston/Cinder-AvfImpl
#define qtime avf
#elif defined(QT32)
#include "cinder/qtime/Quicktime.h"
#include "cinder/qtime/MovieWriter.h"
#endif

namespace cinder { namespace qb {
	
	class qbRenderer {
	public:
		qbRenderer();
		~qbRenderer();
		
		std::string &	getStatusString()			{ return mStatus; }
		std::string &	getProgString()				{ return mProgString; }
		std::string &	getFramestring()			{ return mFramesString; }
		std::string		getTimeEstimatedString()	{ return ( mFramesMax == 0 ? "?" : toTime(mTimeEstimated) ); }
		std::string		getTimeElapsedString()		{ return toTime(mTimeElapsed); }
		std::string		getTimeRemainingString()	{ return ( mFramesMax == 0 ? "?" : toTime(mTimeRemaining) ); }
		bool			isRendering()				{ return bIsRendering; }
		float			getProg();
		
		void			setFolder( const std::string & _folder );
		void			setFileNameBase( const std::string & _f );
		
		void			start( const std::string & _f="" );
		void			stop();
		void			finish();
		void			startstop();

		void			takeScreenshot();
		void			takeScreenshot( const ImageSourceRef & _aframe );
		void			takeScreenshot( const ImageSourceRef & _aframe, const std::string _filename );
		
		void			add();
		void			add( const ImageSourceRef & _aframe );
		
		
	private:
		
		void				reset();
		std::string			makeFileNameTime( std::string _p, std::string _f, std::string _ext="" );
		std::string			makeFileNameSerial();
		void				open();
		void				commit( const ImageSourceRef & _aframe );
		void				updateStatus();
		
#ifdef QT32
		qtime::MovieWriter	mMovieWriter;
#endif
		std::string			mStatus;
		std::string			mProgString;
		std::string			mFramesString;
		std::string			mFolder;
		std::string			mFileNameBase;
		std::string			mFileName;			// the actual file name being written
		int					mFramesMax;
		int					mFramesStill;
		int					mFramesAdded;
		int					mFramesStarted;		// for progress only
		double				mTimeStart, mTimeElapsed, mTimeEstimated, mTimeRemaining;
		bool				bOpened;
		bool				bIsRendering;
		float				mAppFramerate;
		
		//audio::SourceRef	mAudioStop;
		//audio::SourceRef	mAudioFinish;
		//audio::SourceRef	mAudioScreenshot;
		
	};
	
	
} } // cinder::qb


