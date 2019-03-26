//
//  qbPalette.h
//
//  Created by Roger Sodre on 01/07/2011
//  Copyright 2011 Studio Avante. All rights reserved.
//
#pragma once
#ifndef NO_QB_PALETTE
#define QB_PALETTE

#include "cinder/Cinder.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Surface.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Perlin.h"
#include "cinder/CinderMath.h"
#include "OscSender.h"
#include "OscListener.h"
#include "qbSource.h"

namespace cinder { namespace qb {
	
#define QB_SOURCE_PALETTE	"qbPalette"

	class qbPalette {
	public:
		
		qbPalette() : mUpdateFrame(-1) {}
		
		
		void	reduce( std::string src, int colorCount=0 );
		void	reduce( gl::Texture src, int colorCount=0 );
		void	reduce( int colorCount=0 );
		void	drawReduced( Rectf bounds );

		void	setupOsc();
		bool	sendOsc( int colorNum=-1 );
		bool	receiveOsc();
		
		int				getSamplesCount()	{ return (int) mSamples.size(); }
		Color &			getSample( int n )	{ return mSamples[n]; }
		
		int				getColorCount()		{ return (int) mColors.size(); }
		Color &			getColor( int n )	{ return mColors[n]; }
		
		Vec2i			getSize()			{ return mSourceSize; }
		float			getPerlinProg( float _px, float _py );

		Surface			mInputImage;
		gl::Texture		mReducedTex;
		
		
		//
		// Source Generator
		void			update();
		
		Surface			mSourceSurface;
		gl::Texture		mSourceTex;
		Vec2i			mSourceSize;


	private:

		std::vector<Color>	mSamples;		// complete samples, ordered by size
		std::vector<Color>	mColors;		// selected colors, ordered by hue
		
		osc::Sender			mOscSender;
		osc::Listener		mOscListener;

		// Issue 182 workaround
		Surface mSurfFromFbo;
		gl::Fbo	mSurfFbo;
		
		// Source Generator
		void		updatePalette();
		Color		getColorFromPalette( float v );
		
		int			mUpdateFrame;		// last frame when this have been updated
		Perlin		mPerlin;
		int			mSeed;
		int			mOctaves;
		float		mTimeSpan;
		float		mTime;
		float		mFrequency;
		float		mMin, mMax;			// peaked values (statistics only)
		float		mLastTime;

	};
	
	//
	// IMAGE Source
	class qbSourcePalette : public qbSourceBase {
	public:
		qbSourcePalette();
		~qbSourcePalette();
		
		// virtuals
		bool	updateFrame( bool _force=false );
		
	private:

};
	

	
} } // cinder::qb

#endif
