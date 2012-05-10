//
//	qbPoly.h
//
//  Created by Roger Sodre on 01/07/2011
//	Copyright 2011 Studio Avante. All rights reserved.
//
#pragma once

#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/DataSource.h"
#include "cinder/svg/Svg.h"
#include "cinder/svg/SvgGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/TriMesh.h"

#import <string>
#import <vector>
#import <list>


namespace cinder { namespace qb {
	
	typedef struct {
		int p1,p2,p3;
	} ITRIANGLE;
	typedef struct {
		int p1,p2;
	} IEDGE;

	
	/////////////////////////////////
	//
	// A Poly Vertex
	class qbTriMesh : public TriMesh
	{
	public:
		
		qbTriMesh() : bTexAbsolute(false), TriMesh() {}
		
		void	appendVertexBase( const Vec3f &v );
		void	appendNormalBase( const Vec3f &n );
		void	appendIndex( const uint32_t i )		{ mIndices.push_back( i ); }
		void	clearVertices()						{ mNormals.clear(); }
		void	clearNormals()						{ mVertices.clear(); }
		void	loadBase();

		std::vector<Vec3f>&			getVerticesBase() { return mVerticesBase; }
		const std::vector<Vec3f>&	getVerticesBase() const { return mVerticesBase; }

		void	setTexAbsolute( bool abs=true );
		void	appendTexCoordAbs( const Vec2f &v ) { mTexCoordsAbs.push_back( v ); }
		void	appendTexCoordRel( const Vec2f &v ) { mTexCoordsRel.push_back( v ); }

	protected:
	
		bool					bTexAbsolute;
		std::vector<Vec3f>		mVerticesBase;		// base vertices
		std::vector<Vec3f>		mNormalsBase;		// base normals
		std::vector<Vec2f>		mTexCoordsRel;		// textures on poly
		std::vector<Vec2f>		mTexCoordsAbs;		// textures filling doc

	};
	
	
	
	//
	// A Poly Vertex
	class qbPolyVertex : public Vec3f
	{
	public:
		
		qbPolyVertex() : Vec3f(), mDist(0), mProg(0), bDup(false) {}
		qbPolyVertex( const Vec3f & src, bool dup ) : Vec3f(src), mDist(0), mProg(0), bDup(dup)  {}
		
		bool		bDup;			// duplicated?
		Vec2f		mTexRel;		// Relative (object bounds)
		Vec2f		mTexAbs;		// Absolute (document)
		
		// animation helpers
		float		mDist;			// The distance from poly start
		float		mProg;			// The prog from poly start	( distance / perimeter )
		
	protected:
		
	};
	
	/////////////////////////////////
	//
	// A Poly Vertex
	class qbPolyContour
	{
	public:
		std::vector<Vec3f>	mPoints;
	};
	
	/////////////////////////////////
	//
	// A Poly
	class qbPoly
	{
	public:
		
		qbPoly() : bClosed(false), mLayer(0), bSmooth(false), mNode(NULL) {}

		void			make ( svg::Node *node, Vec2f docSize, Vec2f docScale=Vec2f::one() );
		void			addVertex( Vec3f _v );
		void			close();

		void			setLayer( const int n )					{ mLayer = n; }
		void			setLayerName( const std::string & n )	{ mLayerName = n; }
		void			setName( const std::string & n )		{ mName = n; }
		void			setTexAbsolute( bool abs )				{ mTrisFront.setTexAbsolute(abs); mTrisBack.setTexAbsolute(abs); }

		int				getVertexCount()				{ return (int) mVertices.size(); }
		float			getPerimeter()					{ return mPerimeter; }
		bool			isClosed()						{ return bClosed; }
		bool			isSmooth()						{ return bSmooth; }
		int				getLayer()						{ return mLayer; }
		std::string &	getLayerName()					{ return mLayerName; }
		std::string &	getName()						{ return mName; }
		Rectf &			getBounds()						{ return mBounds; }
		Vec2f			getPos()						{ return mBounds.getUpperLeft(); }
		Vec2f			getSize()						{ return mBounds.getSize(); }
		Vec3f &			getCenter()						{ return mCenter; }
		
		std::vector<qbPolyVertex> &	getVertices()		{ return mVertices; }
		std::vector<qbPolyContour> &getContours()		{ return mContours; }
		std::vector<Vec3f> & getLinesExtrude()			{ return mLinesExtrude; }
		qbPolyVertex &	getVertex(int v)				{ return mVertices[v]; }
		qbTriMesh &		getTrisFront()					{ return mTrisFront; }
		qbTriMesh &		getTrisBack()					{ return mTrisBack; }
		TriMesh &		getTrisExtrude()				{ return mTrisExtrude; }
		
		float			getLoopingProgAt(float prog);
		float			getDistanceAt(float prog)		{ return (mPerimeter * prog); }
		float			getLoopingDistanceAt(float prog);
		Vec3f			getPointAt(float prog);
		Vec3f			getPointAtDistance(float dist);
		
	protected:
		
		void			finish();
		
		qbTriMesh		triangulate( Shape2d & shape );
		int				bourkeTriangulateFromPoints( std::vector<Vec3f> & points, std::vector<Vec3f> * vertices );
		int				bourkeTriangulate(int nv, Vec3f *pxyz,ITRIANGLE *v,int *ntri);
		int				bourkeCircumCircle(double xp,double yp,
										   double x1,double y1,
										   double x2,double y2,
										   double x3,double y3,
										   double *xc,double *yc,double *rsqr);
		
		svg::Node *		mNode;							// Original SVG node
		std::vector<qbPolyVertex>	mVertices;			// Border vertices
		std::vector<qbPolyContour>	mContours;			// Original 2D contours, no duplicates
		std::vector<Vec3f>			mLinesExtrude;		// lines between extruded faces
		qbTriMesh		mTrisFront;						// Front faces, textures on poly / filling doc
		qbTriMesh		mTrisBack;						// Back faces, textures on poly / filling doc
		TriMesh			mTrisExtrude;					// Extrude triangles
		std::string		mLayerName;
		std::string		mName;
		Rectf			mBounds;
		Vec3f			mCenter;
		int				mLayer;
		bool			bClosed;
		float			mPerimeter;
		float			mStep;			// Tesselation approximation (1.0 = 1:1 with screen space)
		bool			bSmooth;		// Curves are smoooothed
	};
	
	/////////////////////////////////
	//
	// POLY GROUP
	//
	class qbPolyGroup {
	public:
		qbPolyGroup() : mPerimeter(0) {};
		
		int			loadFromSVG( const DataSourceRef & _res, Vec2f destSize=Vec2f::zero() );
		int			loadFromSVG( const std::string & _f, Vec2f destSize=Vec2f::zero() );
		
		void		clear()						{ mPolys.clear(); }
		void		addPoly( qbPoly & _poly );
		void		draw();
		void		drawStroked();
		
		svg::Doc	& getSvgDoc()				{ return *mSvgDoc; };
		int			getLayerCount()				{ return mLayers.size(); }
		float		getPerimeter()				{ return mPerimeter; }
		int			getPolyCount()				{ return mPolys.size(); }
		qbPoly &	getPoly( int n )			{ return mPolys[n]; }
		
	protected:
		
		int			parseSvg( DataSourceRef dataSource, Vec2f destSize );
		void		parseSvgLayer( svg::Group * parent, int level=0 );
		void		identSvg( svg::Group * parent, int level=0 );
		
		svg::DocRef					mSvgDoc;
		std::vector<std::string>	mLayers;		// Layer names
		std::vector<qbPoly>			mPolys;			// Polys
		std::string	mCurrLayerName, mLastLayerName;
		float						mPerimeter;		// Length from start to finish
		int							mLayerCount;
		Vec2f						mSize;
		Vec2f						mScale;
	};

} } // cinder::qb

