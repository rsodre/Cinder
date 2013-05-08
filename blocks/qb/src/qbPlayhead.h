//
//  qbPlayhead.h
//
//  Created by Roger Sodre on 01/07/2011
//  Copyright 2011 Studio Avante. All rights reserved.
//
#pragma once

#include "cinder/Cinder.h"
#include "cinder/Timer.h"

namespace cinder { namespace qb {
	
	class qbPlayhead {
	public:
		qbPlayhead();
		
		void			update();
		void			seekToProg( float _prog );
		void			seekToTime( double _s );
		void			scheduleRewind()			{ bShouldRewind = true; }

		bool			isPlaying()					{ return bPlaying; }
		bool			isFresh()					{ return bFresh; }
		int				getCurrentFrame()			{ return mCurrentFrame; }
		double			getSeconds();
		
		void			start();
		void			stop();
		void			resume();
		
	private:
		
		bool			bPlaying;
		bool			bShouldRewind;
		double			mLastTime;
		double			mSeconds;
		int				mCurrentFrame;
		int				mLastFrame;
		bool			bFresh;
		
		void			rewind();
	};
	
	
} } // cinder::qb


