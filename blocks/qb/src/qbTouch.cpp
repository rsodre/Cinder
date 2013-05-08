//
//  qbTouch.h
//
//  Created by Roger Sodre on 01/07/2011
//  Copyright 2011 Studio Avante. All rights reserved.
//

#include "qbTouch.h"

using namespace ci::app;

//////////////////////////////////////
//
// Source Classes
//
namespace cinder { namespace qb {
	
	
	qbTouch::qbTouch()
	{
	}
	
	qbTouch::~qbTouch()
	{
	}
	
	void qbTouch::enable( AppBasic::Settings *settings )
	{
		settings->enableMultiTouch( true );
		AppBasic::get()->registerTouchesBegan( this, &qbTouch::touchesBegan );
		AppBasic::get()->registerTouchesMoved( this, &qbTouch::touchesMoved );
		AppBasic::get()->registerTouchesEnded( this, &qbTouch::touchesEnded );
	}
	
	bool qbTouch::touchesBegan( TouchEvent event )
	{
		mPoints.clear();
		for( std::vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt ) {
			mPoints.insert( std::make_pair( touchIt->getId(), *touchIt ) );
			//console() << "NEW POINT: " << touchIt->getId() << std::endl;
		}
		//console() << "Began: " << event << ", size = " << mPoints.size() << std::endl;
		this->calcPos();
		return false;
	}
	bool qbTouch::touchesMoved( TouchEvent event )
	{
		for( std::vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt )
		{
			mPoints.erase( touchIt->getId() );
			mPoints.insert( std::make_pair( touchIt->getId(), *touchIt ) );
		}
		//console() << "Moved: " << event << ", size = " << mPoints.size() << std::endl;
		this->calcPos();
		return false;
	}
	bool qbTouch::touchesEnded( TouchEvent event )
	{
		for( std::vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt ) {
			mPoints.erase( touchIt->getId() );
		}
		//console() << "Ended: " << event << ", size = " << mPoints.size() << std::endl;
		this->calcPos();
		return false;
	}
	
	void qbTouch::calcPos()
	{
		if ( mPoints.size() == 1 )
		{
			std::map<uint32_t,TouchEvent::Touch>::iterator it = mPoints.begin();
			mPos = it->second.getPos();
			mPosPrev = it->second.getPrevPos();
		}
		else
		{
			Vec2f sum, sumPrev;
			int k = 0;
			for( std::map<uint32_t,TouchEvent::Touch>::iterator it = mPoints.begin(); it != mPoints.end(); ++it )
			{
				sum += (it->second).getPos();
				sumPrev += (it->second).getPrevPos();
				k++;
			}
			mPos = sum / k;
			mPosPrev = sumPrev / k;
		}
		mPanDelta = mPos - mPosPrev;
	}
	
	float qbTouch::getZoomDelta()
	{
		if ( mPoints.size() == 2 )
		{
			std::map<uint32_t,TouchEvent::Touch>::iterator it = mPoints.begin();
			TouchEvent::Touch & t0 = it->second;
			TouchEvent::Touch & t1 = (++it)->second;
			float distNow = t0.getPos().distance( t1.getPos() );
			float distPrev = t0.getPrevPos().distance( t1.getPrevPos() );
			float d = distNow - distPrev;
			//printf("distPrev %.2f  /  distNow %.2f  /  D = %.1f\n",distPrev,distNow,d);
			return d;
		}
		else
			return 0.0;
	}

} } // cinder::qb
