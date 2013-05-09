//
//  qbDomeMaster.h
//
//  Created by Roger Sodre on 01/07/2011
//  Copyright 2011 Studio Avante. All rights reserved.
//
#pragma once

#include "cinder/Cinder.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Surface.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/DisplayList.h"

#define DXYI(X,Y)					((Y)*mSize.x+(X))

#define LATLNG_TO_XYZ(lat,lng)		(qbDomeMaster::getPosFromLatLng(lat,lng))

namespace cinder { namespace qb {
	
	class qbDomeMaster {
	public:
		
		qbDomeMaster() : mMask(NULL), mLength(NULL), mVectors(NULL), mNormals(NULL) {}
		
		void			setup( int w, int h );

		bool			isEnabled()							{ return ( mSize != Vec2i::zero() ); }

		// for PIXELS ( 0..w, 0..h)
		bool			getMask( const Vec2i & p )			{ return mMask[DXYI(p.x,p.y)]; }
		float			getLength( const Vec2i & p )		{ return mLength[DXYI(p.x,p.y)]; }
		Vec3f &			getVector( const Vec2i & p )		{ return mVectors[DXYI(p.x,p.y)]; }
		Vec3f &			getNormal( const Vec2i & p )		{ return mNormals[DXYI(p.x,p.y)]; }
		Vec3f			getProjection( const Vec2i & p )	{ return mProjection[DXYI(p.x,p.y)] + mCenterf; }

		Vec3f			project2D( const Vec3f & p );
		Vec3f			lerp2D( const float fact, const Vec3f & p0, const Vec3f & p1 );
		
		void			drawLineSegmented( const Vec3f & p0, const Vec3f & p1, int segments );
		void			drawLineSegmented2D( const Vec3f & p0, const Vec3f & p1, int segments );

		Vec3f			getIntersectionFrom( const Vec3f & p0, const Vec3f & p1, bool closest=false );
		void			drawLineSegmentedFrom( const Vec3f & from, const Vec3f & p0, const Vec3f & p1, int segments, bool closest=false );
		void			drawLineSegmentedFrom( const Vec3f & from, const Vec3f & p0, const Vec3f & p1, const Vec3f & op0, const Vec3f & op1, int segments, bool closest=false );

		
		gl::Texture &	getMaskTexture()					{ return mMaskTexture; }
		gl::Texture &	getNormalTexture()					{ return mNormalTexture; }

		void			drawNormals();
		void			drawNormals( Rectf bounds );
		void			drawMask();
		void			drawGrid( bool esfera=false );
		void			drawMesh( Vec2f uv=Vec2f::one(), bool esfera=false );
		
		Vec2i			mSize;
		Vec2i			mCenter;
		Vec3f			mCenterf;
		int				mRadius;
		int				mDiameter;
		int				mGridStep;
		int				mMeshStep;
		
		// Static
		static Vec3f	getPosFromLatLng( float lat, float lng );
		static Vec3f	getPosFromLatLngRad( float lat, float lng );
		static Vec2f	texelToDome( Vec2f st );
		static Vec2f	domeToTexel( Vec2f dc );
		static Vec2f	domeToTexel( Vec3f pos );
		static Vec2f	domeToGeo( Vec3f pos );
		static float	domeRadius( Vec2f dc );
		static float	getPixelAngle( Vec2f dc );
		
		static Vec3f	geoToDome( Vec2f st );
		
		
		bool *			mMask;			// for x,y in a DM: Is inside circle?
		float *			mLength;		// for x,y in a DM: distance from center
		Vec3f *			mVectors;		// for x,y in a DM: Vector from center
		Vec3f *			mNormals;		// for x,y in a DM: normal when projected on dome
		Vec3f *			mProjection;	// for x,y in a DM: position projected on dome
		
	private:
		
		void			makeMask();
		
		gl::Texture			mMaskTexture;
		gl::Texture			mNormalTexture;
	
		gl::DisplayList		mMeshGrid[2];		// 2 for dome/esfera
		gl::DisplayList		mMeshDome[2];		// 2 for dome/esfera

	};
	
	
	
} } // cinder::qb
