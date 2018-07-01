/*
 syphonServer.h
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

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"

#ifdef __OBJC__
@class SyphonServer;
@class NSDictionary;
#else
class SyphonServer;
class NSDictionary;
#endif

class syphonServer {
	public:
	syphonServer();
	~syphonServer();
	
	bool isRunning()	{return (bool)mSyphon; }
	void shutdown();
	
	void setName (std::string n, bool privateServer=false);
	std::string getName();
	std::string getUUID();
	NSDictionary * getDescription();

	void publishScreen( bool displayMousePos=false );
    void publishTexture(ci::gl::Texture & inputTexture);
	
    
protected:
	SyphonServer	*mSyphon;

};
