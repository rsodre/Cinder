//
//  qbTouch.h
//
//  Created by Roger Sodre on 01/07/2011
//  Copyright 2011 Studio Avante. All rights reserved.
//
#pragma once

#include "cinder/Cinder.h"
#include "cinder/app/AppBasic.h"

#include <vector>
#include <map>
#include <list>

//////////////////////////////////////
//
// Source Classes
//
namespace cinder { namespace qb {
	
	class qbTouch {
	public:
		
		qbTouch();
		~qbTouch();
		
		void	enable( app::AppBasic::Settings *settings );
		void	setupEvents();
		
		void	touchesBegan( app::TouchEvent & event );
		void	touchesMoved( app::TouchEvent & event );
		void	touchesEnded( app::TouchEvent & event );
		
		int		getCount()			{ return mPoints.size(); }	// Number of touches
		Vec2f	getPos()			{ return mPos; }			// Average pos from all touches
		Vec2f	getPosPrev()		{ return mPosPrev; }		// Average pos from all touches
		Vec2f	getPanDelta()		{ return mPanDelta; }
		float	getZoomDelta();
		
	private:
		
		std::map<uint32_t,app::TouchEvent::Touch>	mPoints;
		Vec2f	mPos, mPosPrev;
		Vec2f	mPanDelta;
		
		void	calcPos();
		
	};

} } // cinder::qb
