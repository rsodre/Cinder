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

enum enumTexCoordType
{
	TEX_COORD_DOC = 0,
	TEX_COORD_LAYER,
	TEX_COORD_POLY,
	// count
	TEX_COORD_COUNT
};


namespace cinder { namespace qb {
	
	
	/////////////////////////////////
	//
	// A Poly TriMesh
	//
	class qbTriMesh : public TriMesh
	{
	public:
		
		qbTriMesh() : mTexType(TEX_COORD_DOC), TriMesh() {}
		
		void	appendVertexBase( const Vec3f &v );
		void	appendNormalBase( const Vec3f &n );
		void	appendIndex( const uint32_t i )				{ mIndices.push_back( i ); }
		void	clearVertices()								{ mNormals.clear(); }
		void	clearNormals()								{ mVertices.clear(); }
		void	loadBase();
		
		std::vector<Vec3f>&			getVerticesBase()		{ return mVerticesBase; }
		const std::vector<Vec3f>&	getVerticesBase() const { return mVerticesBase; }
		
		void	setTexType( int type );
		void	appendTexCoordDoc( const Vec2f &v )			{ mTexCoordsDoc.push_back( v ); }
		void	appendTexCoordLayer( const Vec2f &v )		{ mTexCoordsLayer.push_back( v ); }
		void	appendTexCoordPoly( const Vec2f &v )		{ mTexCoordsPoly.push_back( v ); }
		
	protected:
		
		int						mTexType;
		std::vector<Vec3f>		mVerticesBase;		// base vertices
		std::vector<Vec3f>		mNormalsBase;		// base normals
		std::vector<Vec2f>		mTexCoordsDoc;		// textures filling doc
		std::vector<Vec2f>		mTexCoordsLayer;	// textures filling layer
		std::vector<Vec2f>		mTexCoordsPoly;		// textures on poly
		
	};
	
	

	/////////////////////////////////
	//
	// A PolyGroup Layer
	//
	class qbPolyLayer {
	public:
		
		qbPolyLayer(std::string & _name): mName(_name), bEmpty(true) {}
		
		void add(Vec3f _p)
		{
			this->add( Rectf( _p.x, _p.y, _p.x, _p.y ) );
		}
		
		void add(Rectf _b)
		{
			if (bEmpty)
			{
				mBounds = _b;
				bEmpty = false;
			}
			else
			{
				if (_b.x1 < mBounds.x1)
					mBounds.x1 = _b.x1;
				if (_b.x2 > mBounds.x2)
					mBounds.x2 = _b.x2;
				if (_b.y1 < mBounds.y1)
					mBounds.y1 = _b.y1;
				if (_b.y2 > mBounds.y2)
					mBounds.y2 = _b.y2;
			}
		}
		
		void draw()
		{
			gl::color( Color::red() );
			gl::drawStrokedRect( mBounds );
		}
		
		Vec2f		getSize()		{ return mBounds.getSize(); }
		Rectf		getBounds()		{ return mBounds; }
		std::string	getName()		{ return mName; }
		
	private:
		
		std::string		mName;
		bool			bEmpty;
		Rectf			mBounds;
		
	};
	
	
	/////////////////////////////////
	//
	// A Poly Vertex
	//
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
	// A Poly Contour
	//
	class qbPolyContour
	{
	public:
		std::vector<Vec3f>	mPoints;
	};
	
	/////////////////////////////////
	//
	// A Poly
	//
	class qbPolyGroup;
	class qbPoly
	{
	public:
		
		qbPoly() : bClosed(false), mLayer(0), bSmooth(false), mNode(NULL) {}

		void			make ( qbPolyGroup * _pg, svg::Node * node );
		void			addVertex( Vec3f _v );
		void			makeLayerTexCoords( Rectf bounds );
		void			close();

		void			setLayer( const int n )				{ mLayer = n; }
		void			setName( const std::string & n )	{ mName = n; }
		void			setTexType( int type )				{ mTrisFront.setTexType(type); mTrisBack.setTexType(type); }

		int				getVertexCount()				{ return (int) mVertices.size(); }
		float			getPerimeter()					{ return mPerimeter; }
		bool			isClosed()						{ return bClosed; }
		bool			isSmooth()						{ return bSmooth; }
		int				getLayer()						{ return mLayer; }
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

		svg::Node *		mNode;							// Original SVG node
		std::vector<qbPolyVertex>	mVertices;			// Border vertices
		std::vector<qbPolyContour>	mContours;			// Original 2D contours, no duplicates
		std::vector<Vec3f>			mLinesExtrude;		// lines between extruded faces
		qbTriMesh		mTrisFront;						// Front faces, textures on poly / filling doc
		qbTriMesh		mTrisBack;						// Back faces, textures on poly / filling doc
		TriMesh			mTrisExtrude;					// Extrude triangles
		std::string		mName;
		Rectf			mBounds;
		Vec3f			mCenter;
		int				mLayer;
		bool			bClosed;
		float			mPerimeter;
		bool			bSmooth;		// Curves are smoooothed
	};
	
	/////////////////////////////////
	//
	// A Poly Group
	//
	class qbPolyGroup {
	public:
		qbPolyGroup() : mPerimeter(0), mScale(Vec2f::one()) {};
		
		int			loadFromSVG( const DataSourceRef & _res, Vec2f destSize=Vec2f::zero() );
		int			loadFromSVG( const std::string & _f, Vec2f destSize=Vec2f::zero() );
		
		void		clear()						{ mPolys.clear(); }
		void		addPoly( qbPoly & _poly );
		void		draw();
		void		drawStroked();
		
		svg::Doc		& getSvgDoc()				{ return *mSvgDoc; };
		Vec2f			& getSize()					{ return mSize; }
		Vec2f			& getScale()				{ return mScale; }
		MatrixAffine2f	& getTransformMatrix()		{ return mTransMatrix; }
		float			getPointDepth( Vec2f p )	{ std::map<Vec2f,float>::iterator it=mVerticesDepth.find(p); return ( it != mVerticesDepth.end() ? (*it).second : 0.0f ); }
		int				getLayerCount()				{ return (int) mLayers.size(); }
		float			getPerimeter()				{ return mPerimeter; }
		int				getPolyCount()				{ return (int) mPolys.size(); }
		qbPoly &		getPoly( int n )			{ return mPolys[n]; }
		
	protected:
		
		int			parseSvg( DataSourceRef dataSource, Vec2f destSize );
		void		parseSvgLayer( svg::Group * parent, int level=0 );
		void		parseSvgDepthLayer( svg::Group * parent, float depth );
		void		identSvg( svg::Group * parent, int level=0 );
		
		svg::DocRef					mSvgDoc;
		std::vector<qbPolyLayer>	mLayers;		// Layer names
		std::vector<qbPoly>			mPolys;			// Polys
		std::string	mCurrLayerName, mLastLayerName;
		float						mPerimeter;		// Length from start to finish
		int							mLayerCount;
		Vec2f						mSize;
		Vec2f						mScale;
		MatrixAffine2f				mTransMatrix;
		std::map<Vec2f,float>		mVerticesDepth;
	};

} } // cinder::qb

