//
//  qbPlayhead.h
//
//  Created by Roger Sodre on 01/07/2011
//  Copyright 2011 Studio Avante. All rights reserved.
//
#include "qbPlayhead.h"
#include "qb.h"

namespace cinder { namespace qb {
	
	
	//////////////////////////////////////////////////////////////////
	//
	// QB MANAGER WRAPPERS
	//
	// Play / Pause
	void qbMain::playSwitch()
	{
		this->play( ! mPlayhead.isPlaying() );
	}
	void qbMain::stop()
	{
		this->play(false);
	}
	void qbMain::play( bool _p )
	{
		if (_p)
			mPlayhead.resume();
		else
			mPlayhead.stop();
		// Sources
		/*
		std::map<int,qbSourceSelector>::const_iterator it;
		for ( it = mSources.begin() ; it != mSources.end(); it++ )
		{
			int i = it->first;
			if (this->source(i))
				this->source(i).play(_p);
		}
		*/
	}
	void qbMain::rewindSources()
	{
		std::map<int,qbSourceSelector>::const_iterator it;
		for ( it = mSources.begin() ; it != mSources.end(); it++ )
		{
			int i = it->first;
			if (this->source(i))
				this->source(i).rewind();
		}
		// TODO:: Rewind AUDIO
	}
	
	
	
	
	//////////////////////////////////////////////////////////////////
	//
	// PLAYHEAD
	//
	qbPlayhead::qbPlayhead()
	{
		this->rewind();
		this->stop();
	}
	
	double qbPlayhead::getSeconds()
	{
		if ( _cfg.getBool(QBCFG_PLAY_BACKWARDS) && QB_ANIM_DURATION )
			return QB_ANIM_DURATION - mSeconds - QB_FRAME_DURATION;
		else
			return mSeconds;
	}

	
	//
	// Rewind + Start
	void qbPlayhead::start()
	{
		bPlaying = true;
		this->rewind();
	}
	
	//
	// Rewind
	void qbPlayhead::rewind()
	{
		mLastTime = CFAbsoluteTimeGetCurrent();
		mSeconds = 0.0;
		mCurrentFrame = 0;
		bShouldRewind = false;
		//printf("PLAYHEAD << REWIND\n");
	}
	
	//
	// Play = Rewind
	void qbPlayhead::stop()
	{
		if ( bPlaying )
			bPlaying = false;
	}
	
	//
	// Play = Rewind
	void qbPlayhead::resume()
	{
		if ( ! bPlaying )
		{
			bPlaying = true;
			mLastTime = CFAbsoluteTimeGetCurrent();
		}
	}
	
	//
	// Update
	void qbPlayhead::update()
	{
		// switch play
		if ( _cfg.isFresh(QBCFG_PLAYING) && bPlaying != _cfg.getBool(QBCFG_PLAYING) )
			bPlaying = _cfg.getBool(QBCFG_PLAYING);
		// scratch!
		if ( _cfg.guiIsInteracting(QBCFG_CURRENT_PROG) )
		{
			this->seekToProg( _cfg.get(QBCFG_CURRENT_PROG) );
			//printf("PROG  %.3f   secs %.3f   fr %d\n",_cfg.get(QBCFG_CURRENT_PROG),mSeconds,mCurrentFrame);
		}
		// move playhead
		else if ( bPlaying )
		{
			double now = CFAbsoluteTimeGetCurrent();
			// Realtime = Play continuously, may lose frames
			if ( _qb.isPreviewRealtime() )
			{
				mSeconds += (now - mLastTime);
				mCurrentFrame = (int) (mSeconds * QB_FRAMERATE);
			}
			// Not Realtime = Play frame by frame
			else
			{
				mCurrentFrame++;
				mSeconds = ( mCurrentFrame * QB_FRAME_DURATION );
			}
			// Loop!
			if ( _qb.getRenderSeconds() > 0.0 && mSeconds >= _qb.getRenderSeconds() )
				bShouldRewind = true;
			// Remember when we rendered for realtime preview
			mLastTime = now;
		}
		// Rewind!!
		if ( bShouldRewind )
		{
			this->rewind();
			_qb.rewindSources();
		}
		//if ( _renderer.isRendering() )
		//	printf("PLAYHEAD  >  secs %.6f / %.6f    fr %d / %d\n",mSeconds,(_qb.getRenderSeconds()-QB_FRAME_DURATION),mCurrentFrame,_qb.getRenderFrames());
		// Update GUI
		_cfg.set(QBCFG_PLAYING, bPlaying);
		_cfg.set(QBCFG_CURRENT_TIME, mSeconds);
		_cfg.set(QBCFG_CURRENT_PROG, QB_PROG);
		std::stringstream os;
		os << (mCurrentFrame+1) << "/" << ( _qb.getRenderFrames() ? toString(_qb.getRenderFrames()) : std::string("?") );
		_cfg.set(DUMMY_CURRENT_FRAME, os.str());
		// freshness
		bFresh = (mLastFrame != mCurrentFrame);
		mLastFrame = mCurrentFrame;
	}
	void qbPlayhead::seekToProg( float _prog )
	{
		//double t = _qb.getRenderSeconds() * _prog;
		//this->seekToTime( t );
		mCurrentFrame = (int) ((QB_ANIM_FRAMES-1) * _prog);
		mSeconds = ( mCurrentFrame * QB_FRAME_DURATION );
		mLastTime = CFAbsoluteTimeGetCurrent();
	}
	void qbPlayhead::seekToTime( double _s )
	{
		mSeconds = _s;
		mCurrentFrame = (int) (mSeconds * QB_FRAMERATE);
		mLastTime = CFAbsoluteTimeGetCurrent();
	}

		
	
} } // cinder::qb







