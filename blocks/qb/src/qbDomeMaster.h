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

#define LATLNG_TO_DOME(lat,lng)		(qbDomeMaster::getPosFromLatLng((lat),(lng)))
#define LATLNG_TO_DOME_DEG(lat,lng)	(qbDomeMaster::getPosFromLatLngDeg((lat),(lng)))

#define DOME_TO_LATLNG(dc)			Vec2f(getAltitude((dc).x),getAzimuth((dc).y))

// Returns azimuth (longitude) of a dome position: 0 .. TWOPI
// dc = dome coordinates ( -1.0 .. +1.0 )
#define getAzimuth(dc)				( clampRadians(atan2(dc.y,dc.x)) )

// Returns altitude (latitude) of a dome position: -QUARTERPI .. +QUARTERPI
// dc = dome coordinates ( -1.0 .. +1.0 )
#define getAltitude(dc)				( asin( dc.z / 1.0 ) )
#define horizonToAltitude(h)		( M_HALF_PI - ((h) * 0.5) )
#define horizonToAltitudeDeg(h)		( 90.0 - ((h) * 0.5) )

#define ceil_power_of_x(n,x)		( (x) * ceil( log(n, x)) )


namespace cinder { namespace qb {
	
	class qbDomeMesh {
	public:
		
		bool start( float h ) {
			bool isNew = ( ! mMesh || h != mHorizon );
			if ( isNew )
			{
				mMesh = gl::DisplayList( GL_COMPILE );
				mMesh.newList();
				mHorizon = h;
			}
			return isNew;
		}
		void end()		{ mMesh.endList(); }
		void draw()		{ mMesh.draw(); }
		
	private:
		gl::DisplayList	mMesh;
		float			mHorizon;
	};
	
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

		static Vec3f	getIntersectionFrom( const Vec3f & p0, const Vec3f & p1, bool closest=false );
		static Vec3f	rayIntersectSphere( Vec3f p1, Vec3f p0, Vec3f center, float r );
		static void		drawLineSegmentedFrom( const Vec3f & from, const Vec3f & p0, const Vec3f & p1, int segments, bool closest=false );
		static void		drawLineSegmentedFrom( const Vec3f & from, const Vec3f & p0, const Vec3f & p1, const Vec3f & op0, const Vec3f & op1, int segments, bool closest=false );

		
		gl::Texture &	getMaskTexture()					{ return mMaskTexture; }
		gl::Texture &	getNormalTexture()					{ return mNormalTexture; }

		void			drawNormals();
		void			drawNormals( Rectf bounds );
		void			drawMask();
		void			drawHorizonLine( float horizon=180.0, Color lineColor=Color::gray(0.5f) );
		void			drawGrid( float horizon=180.0 );
		void			drawMesh( Vec2f uv=Vec2f::one(), float horizon=180.0 );
		
		Vec2i			mSize;
		Vec2i			mCenter;
		Vec3f			mCenterf;
		int				mRadius;
		int				mDiameter;
		int				mGridStep;
		int				mMeshStep;
		
		// Static
		static Vec3f	getPosFromLatLng( float lat, float lng );
		static Vec3f	getPosFromLatLngDeg( float lat, float lng );
		static Vec2f	texelToUnit( Vec2f st );
		static Vec2f	unitToTexel( Vec2f dc );
		static Vec2f	domeToTexel( Vec3f pos, float horizon=M_PI );
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
	
		qb::qbDomeMesh		mMeshBorder;	// mesh for dome border
		qb::qbDomeMesh		mMeshGrid;		// mesh for grid
		qb::qbDomeMesh		mMeshDome;		// mesh for dome

	};
	
	
	
} } // cinder::qb

