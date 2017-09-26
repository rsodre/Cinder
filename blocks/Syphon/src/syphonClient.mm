/*
 syphonClient.mm
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

#include "syphonClient.h"

#import <Syphon/Syphon.h>
#import "SyphonNameboundClient.h"

#include "cinder/app/App.h"

using namespace cinder;


syphonClient::syphonClient()
{
	refreshFrame = -1;
	bSetup = false;
	bHasNewFrame = false;
}

syphonClient::~syphonClient()
{
    if(bSetup)
    {
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		
		[(SyphonNameboundClient*)mClient release];
		mClient = nil;
		
		[pool drain];
	}
}

void syphonClient::setup()
{
    // Need pool
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	mClient = [[SyphonNameboundClient alloc] init]; 
	
	bSetup = true;
    
    [pool drain];
}

void syphonClient::setApplicationName(std::string appName)
{
    if(bSetup)
    {
		mAppName = appName;
		
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        NSString *name = [NSString stringWithCString:appName.c_str() encoding:[NSString defaultCStringEncoding]];
        
        [(SyphonNameboundClient*)mClient setAppName:name];
		
		this->refresh(true);
		
        [pool drain];
    }
    
}
void syphonClient::setServerName(std::string serverName)
{
    if(bSetup)
    {
		mServerName = serverName;
		
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        NSString *name = [NSString stringWithCString:serverName.c_str() encoding:[NSString defaultCStringEncoding]];
		
        if([name length] == 0)
            name = nil;
        
        [(SyphonNameboundClient*)mClient setName:name];
		
        [pool drain];
    }    
}

//
// Dont forget to unbind() !!!!!
// ... or else the client will remain locked
void syphonClient::bind(int unit)
{
    if(bSetup)
    {
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
     	[(SyphonNameboundClient*)mClient lockClient];
		this->refresh();
		[pool drain];

		if (getTexture())
		{
			// Save old GL_TEXTURE_RECTANGLE_ARB binding or else we can mess GL_TEXTURE_2D used after
			glGetBooleanv( GL_TEXTURE_RECTANGLE_ARB, &mOldTargetBinding );
			
			getTexture().enableAndBind(unit);
		}
    }
    else
		std::cout<<"syphonClient is not setup, or is not properly connected to server.  Cannot bind.\n";
}

void syphonClient::unbind(int unit)
{
    if(bSetup)
    {
		if (getTexture())
		{
			getTexture().unbind(unit);
			
			if( mOldTargetBinding )
				glEnable( GL_TEXTURE_RECTANGLE_ARB );
			else
				glDisable( GL_TEXTURE_RECTANGLE_ARB );
		}
		
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        [(SyphonNameboundClient*)mClient unlockClient];
		[pool drain];
    }
    else
		std::cout<<"syphonClient is not setup, or is not properly connected to server.  Cannot unbind.\n";
}

//
// Draw Syphon texture
void syphonClient::draw( const Vec2f & pos )
{
	if(bSetup){
		if (getTexture())
		{
			this->bind();
			gl::draw( getTexture(), pos );
			this->unbind();
		}
	}
}
void syphonClient::draw( const Rectf & rect )
{
	if(bSetup){
		if (getTexture())
		{
			this->bind();
			gl::draw( getTexture(), rect );
			this->unbind();
		}
	}
}
void syphonClient::draw( const Area & srcArea, const Rectf & destRect )
{
	if(bSetup){
		if (getTexture())
		{
			this->bind();
			gl::draw( getTexture(), srcArea, destRect );
			this->unbind();
		}
	}
}


//
// Update client with latest data
// Call this from inside your app to get always the freshest data from this Client
void syphonClient::update()
{
    if(bSetup)
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		[(SyphonNameboundClient*)mClient lockClient];
		this->refresh();
		[(SyphonNameboundClient*)mClient unlockClient];
		[pool drain];
    }    
    else
		std::cout<<"syphonClient is not setup, or is not properly connected to server.  Cannot bind.\n";
}

//
// (PRIVATE)
// Refresh current texture from Syphon Client
// Sets all data for this client, like
// Can be called by all methods, will be updated just once per frame
void syphonClient::refresh( bool force )
{
	// refresh once per frame
	if ( refreshFrame == app::getElapsedFrames() && !force )
		return;
	refreshFrame = app::getElapsedFrames();
	
	// Get Client
	SyphonClient *client = [(SyphonNameboundClient*)mClient client];
	bHasNewFrame = [client hasNewFrame];
	if (bHasNewFrame)
	{
		mCurrentFrame = [(SyphonNameboundClient*)mClient currentFrame];
		
		// Grab Frame
		SyphonImage* latestImage = [client newFrameImage];
		//printf("SyphonImage [%d] current [%d] new [%d]\n",(int)latestImage,(int)mCurrentFrame,bHasNewFrame);
		if (latestImage)
		{
			NSSize texSize = [latestImage textureSize];
			GLuint m_id = [latestImage textureName];
			[latestImage release];
			//printf("frame [%d] texSize [%d]\n",mCurrentFrame,m_id);
			
			bool newTex = false;
			if ( !mTex )
				newTex = true;
			else if ( texSize.width != mTex.getWidth() || texSize.height != mTex.getHeight() || m_id != mTex.getId() )
				newTex = true;
			
			if (newTex)
			{
				mTex = gl::Texture(GL_TEXTURE_RECTANGLE_ARB, m_id, texSize.width, texSize.height, true);
				mTex.setFlipped();
#ifdef SYPHON_HARD_COPY
				gl::Fbo::Format fmt = gl::Fbo::Format();
				fmt.setTarget( GL_TEXTURE_RECTANGLE_ARB );
				fmt.setColorInternalFormat( mTex.getInternalFormat() );
				fmt.enableDepthBuffer( false );
				mFbo = gl::Fbo(texSize.width, texSize.height, fmt);
				mFbo.getTexture().setFlipped();
#endif
				//printf("Syphon NEW TEX id[%d] w[%d] h[%d]\n",(int)m_id,(int)texSize.width,(int)texSize.height);
			}
		}
		// Do not erase! Keep last received frame
		//	else if ( mTex )
		//		mTex = gl::Texture();
		
#ifdef SYPHON_HARD_COPY
		if (mTex && latestImage)
		{
			Area vp = gl::getViewport();
			gl::pushMatrices();
			
			mFbo.bindFramebuffer();
			gl::setMatricesWindow( mFbo.getSize() );
			gl::setViewport( mFbo.getBounds() );
			gl::clear( Color::yellow() );
			mTex.enableAndBind();
//			glPushMatrix();
//			gl::translate( 0, mFbo.getHeight() );
//			gl::scale( 1.0, -1.0 );
			gl::draw(mTex);
//			glPopMatrix();
			mTex.unbind();
			mFbo.unbindFramebuffer();
			
			gl::popMatrices();
			gl::setViewport( vp );
		}
#endif
	}
	
}

//
// Has a new frame since last update()
bool syphonClient::hasNewFrame() 
{
    if(bSetup)
    {
		this->update();
		return bHasNewFrame;
    }
	else 
		return false;
};

//
// Current Frame number
unsigned int syphonClient::getCurrentFrame() 
{
    if(bSetup)
    {
		this->update();
		return mCurrentFrame;
    }
	else 
		return 0;
};

//
// This client's FPS
// The FPS calculation is asynchronous and updated inside SyphonNameboundClient
// Even if my app is on a slower framerate, this will be accurate
float syphonClient::getCurrentFrameRate() 
{
	return ( mClient ? [(SyphonNameboundClient*)mClient currentFrameRate] : 0.0 );
};

