/*
 syphonServer.mm
 Cinder Syphon Implementation
 
 Created by astellato on 2/6/11
 
 Copyright 2011 astellato, bangnoise (Tom Butterworth) & vade (Anton Marini).
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <Foundation/Foundation.h>
#import <OpenGL/OpenGL.h>
#import <Syphon/Syphon.h>
#include "syphonServer.h"

#include "cinder/Cinder.h"
#include "cinder/app/AppBasic.h"

#ifdef QB
#include "qb.h"
#else
#define QB_RENDER_DENSITY 1
#endif


syphonServer::syphonServer()
{
	mSyphon = nil;
}

syphonServer::~syphonServer()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	this->shutdown();
	[pool drain];
}

NSDictionary * syphonServer::getDescription()
{
	return ( mSyphon ? mSyphon.serverDescription : nil );
}


void syphonServer::shutdown()
{
    [mSyphon stop];
    [mSyphon release];
	mSyphon = nil;
}


void syphonServer::setName(std::string n, bool privateServer)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
	NSString *title = [NSString stringWithCString:n.c_str() 
										 encoding:[NSString defaultCStringEncoding]];
	
	if (!mSyphon)
	{
		NSDictionary *options = nil;
		if ( privateServer )
			// http://rypress.com/tutorials/objective-c/data-types/nsdictionary.html
			options = @{ SyphonServerOptionIsPrivate : [NSNumber numberWithBool:TRUE] };
		mSyphon = [[SyphonServer alloc] initWithName:title context:CGLGetCurrentContext() options:options];
	}
	else
	{
		[mSyphon setName:title];
	}
    
    [pool drain];
}

std::string syphonServer::getName()
{
	std::string name;
	if (mSyphon)
	{
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		name = [[mSyphon name] cStringUsingEncoding:[NSString defaultCStringEncoding]];
		[pool drain];
	}
	return name;
}

std::string syphonServer::getUUID()
{
	if (!mSyphon)
		return "";
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	NSString * u = [mSyphon.serverDescription objectForKey:SyphonServerDescriptionUUIDKey];
	std::string uuid = std::string( [u UTF8String] );
	[pool drain];
	return uuid;
}

void syphonServer::publishScreen( bool displayMousePos )
{
	ci::Surface surf = ci::app::copyWindowSurface();
	if (displayMousePos)
	{
		ci::Surface::Iter it( surf.getIter() );
		ci::Vec2i mousePos = ci::app::AppBasic::get()->getMousePosMainWindow() * QB_RENDER_DENSITY;
		float maxDistance = 1;
		while( it.line() ) {
			while( it.pixel() ) {
				ci::Vec2f pos = it.getPos();
				float distance = fmin( abs(mousePos.x - pos.x), abs(mousePos.y - pos.y) );
				if ( distance <= maxDistance )
				{
					it.r() = 255;
					it.g() = 255;
					it.b() = 255;
				}
			}
		}
	}
	ci::gl::Texture mTex =  ci::gl::Texture( surf );
	this->publishTexture( mTex );
}


void syphonServer::publishTexture( ci::gl::Texture & inputTexture )
{
	if(inputTexture){
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		GLuint texID = inputTexture.getId();
		if (!mSyphon)
		{
			mSyphon = [[SyphonServer alloc] initWithName:@"Untitled" context:CGLGetCurrentContext() options:nil];
		}
		[mSyphon publishFrameTexture:texID
									   textureTarget:inputTexture.getTarget()
										 imageRegion:NSMakeRect(0, 0, inputTexture.getWidth(), inputTexture.getHeight())
								   textureDimensions:NSMakeSize(inputTexture.getWidth(), inputTexture.getHeight()) 
											 flipped:!inputTexture.isFlipped()];
		[pool drain];
	} else {
		ci::app::console()<<"syphonServer is not setup, or texture is not properly backed.  Cannot draw.\n";
	}
}

