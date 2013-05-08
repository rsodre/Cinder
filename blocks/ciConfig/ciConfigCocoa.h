//
//	ciConfig.h
//
//  Created by Roger Sodre on 08/04/2010
//  Copyright 2011 Studio Avante. All rights reserved.
//
#pragma once

////////////////////////////////////////
//
// ciConfig
//
// Originally ofxConfig, made for openFrameworks
// Adapted to work on Cinder by typecasting some OF classes
// QB is for Cinder, so I'll make it a Cinder lib by default
// OF support is now broken, sorry.
//

#include "ciConfig.h"

// Cococa integration
#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#endif

// Cocoa
// Convert NSString <-> C char* string
#define STR2NSS(str)			([NSString stringWithUTF8String:(const char*)(str)])
#define NSS2STR(nsstr)			((char*)[(nsstr) UTF8String])

namespace cinder {

class ciConfigCocoa : public ciConfig
{
public:

	
#ifdef __OBJC__
	// Cococa integration
	void cocoaSetupSlider(int id, NSSlider *slider);
	void cocoaSetupButton(int id, NSButton *button);
	void cocoaSetupPopup(int id, NSPopUpButton *pop);
	void cocoaSetupSeg(int id, NSSegmentedControl *pop);

	void cocoaReadSlider(int id, NSSlider *slider);
	void cocoaReadButton(int id, NSButton *button);
	void cocoaReadPopup(int id, NSPopUpButton *pop);
	void cocoaReadSeg(int id, NSSegmentedControl *pop);
#endif

};


} // namespace cinder
