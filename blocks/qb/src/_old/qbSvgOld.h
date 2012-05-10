//
//	qbSvgOld.h
//
//  Created by Roger Sodre on 01/07/2011
//	Copyright 2011 Studio Avante. All rights reserved.
//
#pragma once

#include "qbPoly.h"
#include "cinder/svg/Svg.h"
#include "cinder/svg/SvgGl.h"

namespace cinder { namespace qb {
	
	
	//
	// A Poly
	class qbPolyOld : public qbPoly
	{
	public:
		
		void copyFrom ( qbPoly & src, bool _asPolygons=false );
		void finish();
		void finishOptimized();
		void randomize( int _max=10 );
		
		void makeRect ( float x, float y, float w, float h )		{ this->makeRect( Vec3f(x,y,0), w, h); }
		void makeCircle ( float x, float y, float rx, float ry )	{ this->makeCircle( Vec3f(x,y,0), rx, ry); }
		//void makeLine ( float x0, float y0, float x1, float y1 )	{ this->makeLine( Vec3f(x0,y0,0), Vec3f(x1,y1,0)); }
		
		void makeTriangle ( Vec3f v0, Vec3f v1, Vec3f v2 );
		void makeRect ( Vec3f v, float w, float h );
		void makeCircle ( Vec3f v, float rx, float ry );
		//void makeLine ( Vec3f v0, Vec3f v0 );
		void makeCurve ( Vec3f p0, Vec3f p1, Vec3f c0, Vec3f c1 );
		
		void addVertex (Vec3f _v );
		void close();

		
	protected:
		
		void calcBounds();
		void makeBounds (float x1, float y1, float x2, float y2);
		void makeMesh();
		
	};

	/////////////////////////////////
	//
	// SVG IMPORTER
	//
	class qbSvgOld : public qbPolyGroup {
	public:
		qbSvgOld() : qbPolyGroup(), mType(QBPOLY_NONE), mLayerCount(0) {};
		
		void	randomize();

		int		getType()						{ return mType; }

	protected:
		int		mType;
		Rectf	mViewBox;		// Viewport

		// OLD
		int		parseSvg( const XmlTree  & _doc, Vec2f destSize );
		void	parseSvgLayer( const XmlTree  & _l );
		void	parseSvgPolygon( const XmlTree  & _p, bool _closed, qbPoly * apoly );
		void	parseSvgRect( const XmlTree  & _p, qbPoly * apoly );
		void	parseSvgLine( const XmlTree  & _p, qbPoly * apoly );
		void	parseSvgCircle( const XmlTree  & _p, qbPoly * apoly );
		void	parseSvgEllipse( const XmlTree  & _p, qbPoly * apoly );
		void	parseSvgPath( const XmlTree  & _p, qbPoly * apoly );
		std::vector<float> splitPathValues( std::string & _values );
		
		
	};
	
} } // cinder::qb

