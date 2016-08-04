//
//  qbDomeMaster.h
//
//  Created by Roger Sodre on 01/07/2011
//  Copyright 2011 Studio Avante. All rights reserved.
//

#include "qbDomeMaster.h"
#include "cinder/Rand.h"
#include "cinder/CinderMath.h"
#include "qb.h"


namespace cinder { namespace qb {
	
	
	void qbDomeMaster::setup( int w, int h )
	{
		if ( w == 0 || h == 0 )
			return;
		mSize = Vec2i( w, h );
		mCenter = mSize * 0.5;
		mCenterf = Vec3f( mCenter.x, mCenter.y, 0 );
		mDiameter = h;
		mRadius = h * 0.5;
		mGridStep = 10;
		mMeshStep = 5;
		
		// Make Length of each pixel to the center
		mLength = (float*) realloc( mLength, sizeof(float) * w * h );
		for ( int y = 0 ; y < h ; y++ )
			for ( int x = 0 ; x < w ; x++ )
				mLength[DXYI(x,y)] = mCenter.distance( Vec2i( x, y ) );
		
		// Make mask
		{
			mMask = (bool*) realloc( mMask, sizeof(bool) * w * h );
			Surface8u mMaskSurface( w, h, true ); // width, height, alpha?
			Surface8u::Iter iter = mMaskSurface.getIter();
			while( iter.line() ) {
				while( iter.pixel() ) {
					int i = DXYI(iter.x(),iter.y());
#ifdef GEO
					mMask[i] = true;
#else
					mMask[i] = ( mLength[i] <= mRadius ? true : false );
#endif
					iter.r() = iter.g() = iter.b() = 0;
					iter.a() = ( mMask[i] ? 255 : 0 );
				}
			}
			mMaskTexture = gl::Texture( mMaskSurface );
		}
		
		// Make Vector of each pixel in the dome
		{
			mVectors = (Vec3f*) realloc( mVectors, sizeof(Vec3f) * w * h );
			mNormals = (Vec3f*) realloc( mNormals, sizeof(Vec3f) * w * h );
			mProjection = (Vec3f*) realloc( mProjection, sizeof(Vec3f) * w * h );
			Surface8u mVectorSurface( w, h, false ); // width, height, alpha?
			Surface8u::Iter iter = mVectorSurface.getIter();
			while( iter.line() ) {
				while( iter.pixel() ) {
					int i = DXYI(iter.x(),iter.y());
					if ( mMask[i] )
					{
#ifdef GEO
						mVectors[i] = qbDomeMaster::geoToDome( Vec2f(iter.x(),iter.y()) / mVectorSurface.getSize() );
						//mVectors[i] *= Vec3f( mVectorSurface.getSize(), mVectorSurface.getHeight() );
#else
						
						float d = mCenter.distance( Vec2i( iter.x(), iter.y() ) );
						mVectors[i].x = iter.x() - mCenter.x;
						mVectors[i].y = iter.y() - mCenter.y;
						mVectors[i].z = sqrt( mRadius * mRadius - d * d );
#endif
						mProjection[i] = mVectors[i];
						mVectors[i].normalize();
						mNormals[i].x = mVectors[i].x;
						mNormals[i].y = mVectors[i].y;
						mNormals[i].z = mVectors[i].z;
						iter.r() = lmap( mNormals[i].x, -1.0f, 1.0f, 0.0f, 1.0f ) * 255;
						iter.g() = lmap( mNormals[i].y, -1.0f, 1.0f, 0.0f, 1.0f ) * 255;
						iter.b() = lmap( mNormals[i].z, -1.0f, 1.0f, 0.0f, 1.0f ) * 255;
						
					}
					else
					{
						mVectors[i] = Vec3f::zero();
						mNormals[i] = Vec3f::zero();
						iter.r() = iter.g() = iter.b() = 0;
					}
				}
			}
			mNormalTexture = gl::Texture( mVectorSurface );
		}
	}
	
	
	//
	// Project a vector in space into the dome
	// p       = ( 0..w, 0..h)
	// returns = ( 0..w, 0..h)
	Vec3f qbDomeMaster::project2D( const Vec3f & p )
	{
		return (p - mCenterf).normalized() * mRadius + mCenterf;
	}
	// Mid-point between 2 points
	Vec3f qbDomeMaster::lerp2D( const float fact, const Vec3f & p0, const Vec3f & p1 )
	{
		Vec3f p = p0.lerp( fact, p1 );
		return this->project2D( p );
	}
	void qbDomeMaster::drawLineSegmented2D( const Vec3f & p0, const Vec3f & p1, int segments )
	{
		glBegin( GL_LINE_STRIP );
		for (int i = 0 ; i <= segments ; i++)
		{
			float prog = (i / (float)segments);
			Vec3f mid = this->lerp2D( prog, p0, p1 );
			//glNormal3f( this->getNormal( Vec2i(mid.x, mid.y) ) );
			glVertex3f( mid );
		}
		glEnd();
	}
	
	
	
	//
	// Draw segmented line in 3D space with minimum segment size
	// p0/p1 = ( 0..w, 0..h)
	void qbDomeMaster::drawLineSegmented( const Vec3f & p0, const Vec3f & p1, int segments )
	{
		glBegin( GL_LINE_STRIP );
		for (int i = 0 ; i <= segments ; i++)
		{
			float prog = (i / (float)segments);
			Vec3f mid = p0.lerp( prog, p1 ).normalized();
			//glNormal3f( this->getNormal( Vec2i(mid.x, mid.y) ) );
			glVertex3f( mid );
		}
		glEnd();
	}
	
	
	
	//
	// Intersection of a line startint from p0 to p1 into the dome
	// http://knol.google.com/k/koen-samyn/line-sphere-intersection-c/2lijysgth48w1/118#
	// http://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
	Vec3f qbDomeMaster::getIntersectionFrom( const Vec3f & lp1, const Vec3f & lp2, bool closest )
	{
		return rayIntersectSphere( lp1, lp2, Vec3f::zero(), 1.0 );
		/*
		Vec3f res = Vec3f::zero();
		float R = 1.0f;					// Radius
		Vec3f m = Vec3f::zero();		// dome center
		Vec3f d = lp1 - m;
		Vec3f ldir = lp2- lp1;
		float a = ldir.lengthSquared();
		float b = 2 * ldir.dot(d);
		float c = d.lengthSquared() - R*R;
		float D = b*b - 4*a*c;
		if ( D < 0 ) {
			res = Vec3f::zero();
		} else if ( D < FLT_EPSILON ){
			float t = (-b)/(2*a);
			Vec3f p1 = lp1 + t*ldir;
			res = p1;
		} else {
			float sq_D = sqrt(D);
			float t1 = (-b+sq_D)/(2*a);
			Vec3f p1 = lp1 + t1 * ldir;
			float t2 = (-b-sq_D)/(2*a);
			Vec3f p2 = lp1 + t2 * ldir;
			if ( closest )
				res = ( (p1 - lp1).length() < (p2 - lp1).length() ? p1 : p2 );	// return closest
			else
				res = ( (p1 - lp1).length() > (p2 - lp1).length() ? p1 : p2 );	// return farthest
		}
		return res;
		 */
	}
	// Intersects ray r = p + td, |d| = 1, with sphere s and, if intersecting,
	// returns t value of intersection and intersection point q
	// From: Real-Time Collision Detection, p. 178
	// in:	p1 = origin, p0 = dest, BIZRRO -- invertido!?
	// out: intersection point, or center if does not intersect
	Vec3f qbDomeMaster::rayIntersectSphere( Vec3f p1, Vec3f p0, Vec3f center, float r )
	{
		Vec3f d = (p1 - p0).normalized();		// direction
		Vec3f m = p0 - center;
		float b = m.dot(d);
		float c = m.dot(m) - r * r;
		// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0)
		if (c > 0.0 && b > 0.0) return center;	// no intersection
		float discr = b * b - c;
		// A negative discriminant corresponds to ray missing sphere
		if (discr < 0.0) return center;	// no intersection
		// Ray now found to intersect sphere, compute smallest t value of intersection
		float t = -b - sqrt(discr);
		// If t is negative, ray started inside sphere so clamp t to zero
		if (t < 0.0) t = 0.0;
		Vec3f q = p0 + t * d;
		return q;
	}
	
	
	
	void qbDomeMaster::drawLineSegmentedFrom( const Vec3f & from, const Vec3f & p0, const Vec3f & p1, int segments, bool closest )
	{
		glBegin( GL_LINE_STRIP );
		for (int i = 0 ; i <= segments ; i++)
		{
			float prog = (i / (float)segments);
			Vec3f mid = getIntersectionFrom( from, p0.lerp( prog, p1 ), closest );
			if ( mid != Vec3f::zero() )
				glVertex3f( mid );
		}
		glEnd();
	}
	void qbDomeMaster::drawLineSegmentedFrom( const Vec3f & from, const Vec3f & p0, const Vec3f & p1, const Vec3f & op0, const Vec3f & op1, int segments, bool closest )
	{
		glBegin( GL_LINE_STRIP );
		for (int i = 0 ; i <= segments ; i++)
		{
			float prog = (i / (float)segments);
			Vec3f pp = p0.lerp( prog, p1 );
			Vec3f mid = getIntersectionFrom( from, pp, closest );
			/*if ( mid == Vec3f::zero() )
			{
				Vec3f opp = op0.lerp( prog, op1 );
				for (int ii = 0 ; ii <= points ; ii++)
				{
					prog = (1.0f / points) * ii;
					Vec3f ppp = pp.lerp( prog, opp );
					Vec3f mid = this->getIntersectionFrom( from, ppp, true );
					if ( mid != Vec3f::zero() )
					{
						glVertex3f( mid );
						break;
					}
				}
			}*/
			if ( mid != Vec3f::zero() )
				glVertex3f( mid );
		}
		glEnd();
	}
	
	
	
	
	//
	// Draw Normals + mouse normal
	void qbDomeMaster::drawNormals()
	{
		this->drawNormals( QB_BOUNDS );
	}
	void qbDomeMaster::drawNormals( Rectf bounds )
	{
		float sz = QB_HEIGHT / 10.0f;
		gl::color( Color::white() );
		gl::draw( mNormalTexture, bounds );
		Vec2f mouse = _qb.getMousePos();
		Vec3f normal = _dome.getNormal( mouse / QB_SCALE );
		gl::drawLine( Vec3f(mouse,0), Vec3f(mouse,0) + normal * sz  );
	}
	void qbDomeMaster::drawMask()
	{
		gl::color( Color::white() );
		gl::enableAlphaBlending();
		gl::draw( mMaskTexture, QB_BOUNDS );
		gl::disableAlphaBlending();
	}
	void qbDomeMaster::drawHorizonLine( float horizon )
	{
		// make?
		glLineWidth( 1 );
		gl::color( Color::gray(0.5f) );
		//glLineWidth( 2 );
		//gl::color( Color::red() );
		if ( mMeshBorder.start( horizon ) )
		{
			// horizontais altitude (Latitude)
			//int lat = 0;
			float lat_h = horizonToAltitudeDeg( horizon );
			glBegin( GL_LINE_STRIP );
			for ( float lng = 0 ; lng <= 360 ; lng += mMeshStep )
			{
				Vec3f p = LATLNG_TO_DOME_DEG( lat_h, lng );
				glVertex3f( p );
			}
			glEnd();
			mMeshBorder.end();
			//printf("HORIZON %.1f  lat %.1f\n",horizon,lat);
		}
		// draw!
		mMeshBorder.draw();
	}
	void qbDomeMaster::drawGrid( float horizon )
	{
		// make?
		glLineWidth( 1 );
		if ( mMeshGrid.start( horizon ) )
		{
			float lat_h = horizonToAltitudeDeg( horizon );
			// verticais (Longitude)
			for ( int lng = 0 ; lng <= 360 ; lng += mGridStep )
			{
				bool yellow = (lng % 30 == 0);
				gl::color( yellow ? Color::yellow()*0.9f : Color::gray(0.5f) );
				glBegin( GL_LINE_STRIP );
				for ( float lat = lat_h ; lat <= 90.0 ; lat += mMeshStep )
				{
					Vec3f p = LATLNG_TO_DOME_DEG( lat, lng );
					glVertex3f( p );
					// 1st time, adjust altitude (Latitude) to step
					float diff = fmod( (lat + 360.0), mMeshStep );
					if ( diff != 0.0 )
						lat -= diff;
				}
				glEnd();
			}
			// horizontais altitude (Latitude)
			for ( float lat = lat_h ; lat < 90 ; lat += mGridStep )
			{
				bool yellow = fmod(lat, 30.0) == 0.0;
				gl::color( yellow ? Color::yellow()*0.9f : Color::white()*0.5f );
				glBegin( GL_LINE_STRIP );
				for ( int lng = 0 ; lng <= 360 ; lng += mMeshStep )
				{
					Vec3f p = LATLNG_TO_DOME_DEG( lat, lng );
					glVertex3f( p );
				}
				glEnd();
				// 1st time, adjust altitude (Latitude) to step
				float diff = fmod( (lat + 360.0), mGridStep );
				if ( diff != 0.0 )
					lat -= diff;
			}
			mMeshGrid.end();
		}
		// draw!
		gl::color( Color::white() );
		mMeshGrid.draw();
	}
	void qbDomeMaster::drawMesh( Vec2f uv, float horizon )
	{
		// Make displaylist?
		if ( mMeshDome.start( horizon ) )
		{
			// horizontais altitude (Latitude)
			bool esfera = (horizon == 360.0);
			float lat_h = horizonToAltitudeDeg( horizon );
			for ( float lat = lat_h ; lat < 90.0f ; lat += mMeshStep )
			{
				float d = fmod( lat, mMeshStep );
				glBegin( GL_TRIANGLE_STRIP );
				for ( int lng = 0 ; lng <= 360 ; lng += mMeshStep )
				{
					Vec3f p0 = LATLNG_TO_DOME_DEG( lat, lng );
					Vec3f p1 = LATLNG_TO_DOME_DEG( lat+mMeshStep-d, lng );
					Vec3f n0 = ( esfera ? p0 : -p0 );
					Vec3f n1 = ( esfera ? p1 : -p1 );
					// TODO:: TESTAR SEM SHADER !!! - vai dar problema no displaylist
					//Vec2f t0 = ( esfera ? domeToGeo( p0 ) : domeToTexel( p0 ) );
					//Vec2f t1 = ( esfera ? domeToGeo( p1 ) : domeToTexel( p1 ) );
					//t0 *= uv;
					//t1 *= uv;
					//glTexCoord2f( t0 );
					glNormal3f( n0 );
					glVertex3f( p0 );
					//glTexCoord2f( t1 );
					glNormal3f( n1 );
					glVertex3f( p1 );
				}
				glEnd();
				// 1st time, adjust dorison diff
				lat -= d;
			}
			mMeshDome.end();
		}
		// Draw!
		mMeshDome.draw();
	}
	
	/////////////////////////////////////////////////////////////////////
	//
	// STATICS
	//
	// lat = horizontal = -90  .. +90
	// lng = vertical   = -180 .. +180
	// Returns -1.0 .. 1.0
	Vec3f qbDomeMaster::getPosFromLatLngDeg( float lat, float lng )
	{
		return qbDomeMaster::getPosFromLatLng( toRadians(lat), toRadians(lng) );
	}
	Vec3f qbDomeMaster::getPosFromLatLng( float lat, float lng )
	{
		float r = cos( lat );
		float x = cos( lng ) * r;
		float y = sin( lng ) * r;
		float z = sin( lat );
		return Vec3f( x, y, z );
	}
	//
	// CONVERSIONS
	//
	// st (Texel coord):	0.0 .. 1.0
	// dc (Dome coord):		-1.0 .. 1.0
	//
	Vec2f qbDomeMaster::texelToUnit( Vec2f st )
	{
		//return Vec2f( st.x * 2.0f - 1.0f, (1.0-st.y) * 2.0f - 1.0f );
		return st * 2.0 - Vec2f::one();
	}
	Vec2f qbDomeMaster::unitToTexel( Vec2f st )
	{
		//return Vec2f( (st.x + 1.0f) * 0.5f, 1.0f - ((st.y + 1.0f) * 0.5f) );
		return ( st + Vec2f::one() ) * 0.5f;
	}
	// pbourke bangalore.pdf pg 14
	// http://paulbourke.net/miscellaneous/domefisheye/fisheye/
	// From:	Dome 3D coordinates (-1.0 .. 1.0, -1.0 .. 1.0, 0.0 .. 1.0)
	// To:		Texel (0.0 .. 1.0)
	Vec2f qbDomeMaster::domeToTexel( Vec3f pos, float horizon )
	{
		float theta = atan2f( sqrt( pos.x * pos.x + pos.y * pos.y ), pos.z);
		float phi = atan2f(pos.y, pos.x);
		float r = theta / (horizon * 0.5);
		Vec2f dc = Vec2f( r * cos(phi), r * sin(phi) );
		return qbDomeMaster::unitToTexel( dc );
	}

	// Equirectangular projection
	// https://github.com/Flightphase/ofxPuffersphere
	// https://github.com/Flightphase/ofxPuffersphere/blob/master/spinningSquareExample/bin/data/shaders/offaxis.frag
	// From:	Dome 3D coordinates (-1.0 .. 1.0, -1.0 .. 1.0, 0.0 .. 1.0)
	// To:		Texel (0.0 .. 1.0)
	Vec2f qbDomeMaster::domeToGeo( Vec3f p )
	{
		float u = 0.5 + atan2f(p.x,p.y) / M_TWO_PI;		// atan with 2 parameters; range: -PI to +PI
		float v = 1.0 - ( cos(p.z) / M_PI );			// tig: changed from 0.5 + atan...
		return Vec2f(u, v);
	}
	// From:	Texel (0.0 .. 1.0)
	// To:		Dome 3D coordinates (-1.0 .. 1.0, -1.0 .. 1.0, -1.0 .. 1.0)
	Vec3f qbDomeMaster::geoToDome( Vec2f st )
	{
		float x = sin(st.y * M_PI) * cos(st.x * M_TWO_PI);
		float y = -sin(st.y * M_PI) * sin(st.x * M_TWO_PI);			// tig: changed to -sin
		float z = cos(st.y * M_PI);
		return Vec3f(x, y, z);
		
		// ROGER
		/*
		float x = sin(st.x * M_TWO_PI);
		float y = cos(st.x * M_TWO_PI);
		float z = cos(st.y * M_PI);
		return Vec3f(x, y, z);
		 */
	}
	
	//
	float qbDomeMaster::domeRadius( Vec2f dc )
	{
		return sqrt( dc.x * dc.x + dc.y * dc.y );
	}
	float qbDomeMaster::getPixelAngle( Vec2f dc )
	{
		float a = -atan2( dc.y, dc.x ) ;
		if ( a < 0.0f )
			a += M_TWO_PI;
		return a;
	}
	

} } // cinder::qb




