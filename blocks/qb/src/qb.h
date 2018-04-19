//
//  qb.h
//
//  Created by Roger Sodre on 01/07/2011
//  Copyright 2011 Studio Avante. All rights reserved.
//
//
//	PREPROCESSOR MACROS :
//	QB_DM	-- enables DomeMaster 
//	QB_COLOR_REDUCTION	-- enables OpenCV
//		blocks: Cinder-OpenCV, osc
//
//
//
//
#pragma once

#include "cinder/Cinder.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/gl.h"

namespace cinder { namespace qb {

///////////////////////////////////
//
// MISC DEFINES
// 
// Global access to QB Manager and QB Config
#define _qb						(qb::__qb)
#define _cfg					(*(qb::__qb.mConfig))
#define _dome					(qb::__qb.mDomeMaster)
#define _palette				(qb::__qb.mPalette)
#define _renderer				(qb::__qb.mRenderer)
#define _src(s)					(qb::__qb.source(s))
#define _touch					(qb::__qb.mTouch)
#define _sys					(qb::__qb.sys())

// Render unit: The rendered scene size in pixels
#define QB_RENDER_WIDTH			(_qb.getRenderWidth())
#define QB_RENDER_HEIGHT		(_qb.getRenderHeight())
#define QB_RENDER_SIZE			(_qb.getRenderSize())
#define QB_RENDER_ASPECT		(_qb.getRenderAspect())
#define QB_RENDER_BOUNDS		(_qb.getRenderBounds())

// Render unit: The rendered scene size in pixels
#define QB_WINDOW_WIDTH			(_qb.getWindowWidth())
#define QB_WINDOW_HEIGHT		(_qb.getWindowHeight())

// Metric unit: Screen size, your actual scene measure, in any unit you like
#define QB_WIDTH				(_qb.getMetricWidth())
#define QB_HEIGHT				(_qb.getMetricHeight())
#define QB_THROW				(_qb.getMetricThrow())
#define QB_ASPECT				(_qb.getMetricAspect())
#define QB_SCALE				(_qb.getMetricScale())
#define QB_SIZE					(_qb.getMetricSize())
#define QB_BOUNDS				(_qb.getMetricBounds()+_qb.getCameraOffset().xy())
#define QB_CENTER				(_qb.getMetricCenter())

// Animation
#define QB_FRAMERATE			(_qb.getRenderFrameRate())
#define QB_FRAME_DURATION		(1.0/QB_FRAMERATE)
#define QB_TIME					(_qb.getTime())
#define QB_PROG					(_qb.getTime()/(_qb.getRenderSeconds()-QB_FRAME_DURATION))	// 0.0 .. 1.0
#define QB_ANIM_PROG			fmod(_qb.getTime()/_qb.getRenderSeconds(),1.0)				// 0.0 .. 0.99 (looper)
#define QB_ANIM_FRAMES			(QB_FRAMERATE*_qb.getRenderSeconds())
#define QB_ANIM_DURATION		(_qb.getRenderSeconds())

// misc
#define QB_VERBOSE				(_qb.bVerbose)
#define QB_APP_NAME				(sysinfo::SysInfo::getAppName())
#define QB_APP_VERSION			(sysinfo::SysInfo::getAppVersion())
#define QB_APP_VERSION_LONG		(sysinfo::SysInfo::getAppVersionLong())
#define QB_CAPTURE_FOLDER		"/Users/roger/Movies/CAPTURE/"
#define QB_FONT					(_qb.mFont)

// Flags
#define TEST_FLAG(src,f)		((src)&(f))
#define QBFLAG_SURFACE			0x01
#define QBFLAG_2					0x02
#define QBFLAG_3					0x04
#define QBFLAG_4					0x08
#define QBFLAG_5					0x10
#define QBFLAG_6					0x20
#define QBFLAG_7					0x40
#define QBFLAG_8					0x80
	
	
///////////////////////////////////
//
// MULTI PURPOSE DEFINES
// 
#ifndef TWO_PI
#define TWO_PI (M_PI*2.0f)
#endif
#ifndef HALF_PI
#define HALF_PI (M_PI/2.0f)
#endif
	

	
	///////////////////////////////////
	//
	// MISC LIB
	// 
	void	qbLog(std::string s);
	void	qbCheckGLError(const char *file, int line, const char *func);
#define CHECK_GL_ERROR qb::qbCheckGLError(__FILE__, __LINE__, __FUNCTION__)
	//
	// Text
	void	writeFPS();
	void	writeStuff(std::string text, int x, int y);
	//
	// Transformations
	Rectf	rectToFit( Rectf src, Rectf dst );
	//
	// fx
	void superFastBlur(unsigned char *pix, int w, int h, int radius);


} } // cinder::qb

#include "qbMain.h"
#include "qbCube.h"




