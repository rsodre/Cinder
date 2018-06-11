//
//  qbPoly.cpp
//
//  Created by Roger Sodre on 01/07/2011
//  Copyright 2011 Studio Avante. All rights reserved.
//

#include "qbPoly.h"
#include "qbCube.h"
#include "cinder/app/App.h"
#include "cinder/Utilities.h"
#include "cinder/Shape2d.h"
#include "cinder/Triangulate.h"

#include <errno.h>
#include <map>

#define M_STEP		0.25		// Tesselation approximation (1.0 = 1:1 with screen space)

//#define IDENT_SVG				// define to dump SVG file tree


//#define QB_TRIANGULATOR
int bourkeTriangulateFromPoints( std::vector<ci::Vec3f> & points, std::vector<ci::Vec3f> * vertices );


namespace cinder { namespace qb {
	
	
	/////////////////////////////////////////////////////
	//
	// SVG LOADER
	//	By Roger Sodre
	//	sep/2011
	//
	// SVG Reference: http://www.w3.org/TR/SVG/
	//
	// Instructions for generating the SVG file:
	//	- Edit on Adobe Illustrator
	//		Each Base Layer will be interpreted as a different layer
	//		You can use the Base Layer name as a hint for anything, like depth
	//		Inside layers, each element is a unique Poly
	//		Groups inside base layers are treated as a single Poly
	//		Layers inside base layers will be treated as groups (single Poly)
	//	- Save as.. SVG
	//		SVG Profiles:	"SVG 1.1"
	//		Images:			"Link"
	//		Check:			"Preserve Illustrator Editing Capabilities"
	//		(this will output a lot of useless data, but will preserve the layer hierarchy)
	//
	

	//////////////////////////////////////////////////////////
	//
	// POLY GROUPS
	//
	int qbPolyGroup::loadFromSVG( const DataSourceRef & _res, Vec2f destSize )
	{
		return this->parseSvg( _res, destSize );
	}
	int qbPolyGroup::loadFromSVG( const std::string & _f, Vec2f destSize )
	{
		return this->parseSvg( app::loadResource( _f ), destSize );
	}
	
	//
	// Return polys loaded
	int qbPolyGroup::parseSvg( DataSourceRef dataSource, Vec2f destSize ) {
		mSvgDoc = svg::Doc::create( dataSource );
		
		// Calc size / scale
		if ( destSize != Vec2f::zero() )
			mScale = Vec2f( destSize.x / mSvgDoc->getWidth() ,  destSize.y / mSvgDoc->getHeight() );
		else
			mScale = Vec2f::one();
		mTransMatrix = MatrixAffine2f::makeScale( mScale.xy() );
		mSize = Vec2f(mSvgDoc->getSize()) * mScale;
		//mSvgDoc->setTransform( m );
		
#ifdef IDENT_SVG
		this->identSvg( mSvgDoc.get() );
#endif
		
		mLayerCount = 0;
		this->parseSvgLayer( mSvgDoc.get() );
		
		for (int p = 0 ; p < mPolys.size() ; p++)
			mPolys[p].makeLayerTexCoords( mLayers[mPolys[p].getLayer()].getBounds() );

		return 0;
	}
	
	void qbPolyGroup::identSvg( svg::Group * parent, int level ) {
		
		for( std::list<svg::Node*>::iterator it = parent->getChildren().begin(); it != parent->getChildren().end(); ++it )
		{
			svg::Node* node = *it;
			bool isGroup = ( typeid(*node) == typeid(svg::Group) );
			int numContours = (int) node->getShape().getNumContours();
			
			printf("## SVG %d |",level);
			for (int n=0;n<level;n++)
				printf("-");
			if (isGroup)
				printf(" %s [%s]",node->getTag().c_str(),node->getId().c_str());
			else
				printf(" %s = %s  (%d contours)",node->getTag().c_str(),node->getId().c_str(),numContours);
			
			if ( node->isDisplayNone() )	// hidden
			{
				printf(" (HIDDEN)\n");
				continue;
			}
			printf("\n");
			
			// open group
			if( isGroup )
				this->identSvg( static_cast<svg::Group*>(node), level+1 );
		}
	}
	
	void qbPolyGroup::parseSvgLayer( svg::Group * parent, int level )
	{
		for( std::list<svg::Node*>::iterator it = parent->getChildren().begin(); it != parent->getChildren().end(); ++it )
		{
			svg::Node* node = *it;
			if ( node->getId().compare(0,5,"DEPTH") == 0 && ( typeid(*node) == typeid(svg::Group) ) )
			{
				float d = atof( node->getId().substr(5).c_str() );
				if (d)
					this->parseSvgDepthLayer( static_cast<svg::Group*>(node), d );
				continue;
			}
			if ( node->isDisplayNone() )	// hidden
				continue;
			// next group
			if( ( typeid(*node) == typeid(svg::Group) ) )
			{
				if ( level == 0 )
				{
					mLayerCount++;
					mCurrLayerName = node->getId();
				}
				this->parseSvgLayer( static_cast<svg::Group*>(node), level+1 );
			}
			// store object
			else
			{
				// new layer?
				bool newLayer = true;
				for ( int n = 0 ; n < mLayers.size() ; n++ )
					if ( mLayers[n].getName() == mCurrLayerName )
						newLayer = false;
				if ( newLayer )
				{
					qbPolyLayer l( mCurrLayerName );
					mLayers.push_back( l );
				}

				// make poly
				qbPoly apoly;
				apoly.make( this, node );
				
				// set layer
				int layerNum = mLayerCount-1;
				apoly.setLayer( layerNum );
				mLayers[layerNum].add( apoly.getBounds() );

				// set name
				if ( node->getId().length() )
					apoly.setName( node->getId() );
				else
					apoly.setName( "<" + node->getTag() + ">" );
				
				// Add poly!
				this->addPoly( apoly );
			}
		}
	}
	
	void qbPolyGroup::parseSvgDepthLayer( svg::Group * parent, float depth )
	{
		for( std::list<svg::Node*>::iterator it = parent->getChildren().begin(); it != parent->getChildren().end(); ++it )
		{
			svg::Node* node = *it;
			if( ( typeid(*node) == typeid(svg::Group) ) )
				this->parseSvgDepthLayer( static_cast<svg::Group*>(node), depth );
			else
			{
				Shape2d shape = (node)->getShape().transformCopy(mTransMatrix);
				std::vector<Path2d> cs = shape.getContours();
				for( std::vector<Path2d>::const_iterator contIt = cs.begin(); contIt != cs.end(); ++contIt ) {
					std::vector<Vec2f> ps = (*contIt).subdivide( M_STEP );
					for( std::vector<Vec2f>::iterator ptIt = ps.begin(); ptIt != ps.end(); ++ptIt )
					{
						Vec2f point = *ptIt;
						mVerticesDepth[point] = depth;
						//printf("DEPTH  %.1f %.1f  =  %.1f\n",point.x,point.y,depth);
					}
				}
			}
		}
	}
	
	void qbPolyGroup::addPoly( qbPoly & _poly )
	{
		if (_poly.getVertexCount() > 0)
		{
			mPolys.push_back( _poly );
			mPerimeter += _poly.getPerimeter();
		}
	}
	void qbPolyGroup::draw()
	{
		for (int p = 0 ; p < mPolys.size() ; p++)
			qb::drawSolidPoly( mPolys[p] );
	}
	void qbPolyGroup::drawStroked()
	{
		for (int p = 0 ; p < mPolys.size() ; p++)
			qb::drawStrokedPoly( mPolys[p] );
	}

	
	
	
	//////////////////////////////////////////////////////////
	//
	// POLYS
	//
	// Make from SVG node
	void qbPoly::make( qbPolyGroup * _pg, svg::Node *node )
	{
		mNode = node;
		mBounds = node->getBoundingBoxAbsolute() * _pg->getScale().xy();
		mCenter = Vec3f( mBounds.getCenter(), 0 );
		bSmooth = ( node->getTag() == "circle" || node->getTag() == "ellipse" || node->getTag() == "path" );
		bClosed = ( node->getTag() == "circle" || node->getTag() == "ellipse" || node->getTag() == "polygon" || node->getTag() == "rect" );
		
		// Make contours
		Vec2f plast;
		qbPolyContour cont;
		Shape2d shape = node->getShape().transformCopy( _pg->getTransformMatrix() );
		std::vector<Path2d> cs = shape.getContours();
		for( std::vector<Path2d>::const_iterator contIt = cs.begin(); contIt != cs.end(); ++contIt ) {
			std::vector<Vec2f> ps = (*contIt).subdivide( M_STEP );
			for( std::vector<Vec2f>::iterator ptIt = ps.begin(); ptIt != ps.end(); ++ptIt )
			{
				Vec2f & point = *ptIt;
				if ( ptIt == ps.begin() ||  point != plast )
					cont.mPoints.push_back( Vec3f( point, 0 ) );
				plast = point;
			}
			if ( cont.mPoints.size() )
			{
				mContours.push_back( cont );
				cont.mPoints.clear();
			}
		}
		
		// make center
		/*
		int pc = 0;
		mCenter = Vec3f::zero();
		for( int c = 0 ; c < mContours.size() ; c++ )
		{
			for( int p = 0 ; p < mContours[c].mPoints.size() ; p++ )
			{
				mCenter += mContours[c].mPoints[p];
				pc++;
			}
		}
		mCenter /= pc;
		 */
		
		// Fill vertices
		for( int c = 0 ; c < mContours.size() ; c++ )
		{
			for( int p = 0 ; p < mContours[c].mPoints.size() ; p++ )
			{
				Vec3f & point = mContours[c].mPoints[p];
				this->addVertex( point );
				//printf(">> CONTOUR [%s]  %d / %d  =  %.6f, %.6f\n",mNode->getTag().c_str(),c,p,point.x,point.y);
			}
			// outer contour is closed?
			if ( c == 0 && mVertices.size() >= 3 )
				if ( mVertices.front() == mVertices.back() )
					bClosed = true;
		}
		
		// Make Front / Back Meshes
#ifdef QB_TRIANGULATOR
		qbTriMesh tris = this->triangulate( shape );					// paul bourke triangulator
#else
		TriMesh2d tris = Triangulator( shape, M_STEP ).calcMesh();		// libtess2
#endif
		mTrisFront.clear();
		mTrisBack.clear();
		Vec3f nv[3];	// vertice for normals
		for (int i = 0 ; i < tris.getNumIndices() ; i++)
		{
			int ix = (int) tris.getIndices()[i];
#ifdef QB_TRIANGULATOR
			Vec2f point = tris.getVertices()[ix].xy();		// paul bourke
#else
			Vec2f point = tris.getVertices()[ix];			// libtess2
#endif
			float z = _pg->getPointDepth( point );
			//printf("DEPTH  %.1f %.1f  =  %.1f\n",point.x,point.y,z);
			Vec3f v = Vec3f( point, z );
			Vec2f tDoc = Vec2f( v.x / _pg->getSize().x, v.y / _pg->getSize().y );
			Vec2f tPoly = Vec2f( lmap(v.x,mBounds.x1,mBounds.x2,0.0f,1.0f), lmap(v.y,mBounds.y1,mBounds.y2,0.0f,1.0f) );
			// front
			mTrisFront.appendIndex( i );
			mTrisFront.appendVertexBase( v );
			mTrisFront.appendTexCoordDoc( tDoc );
			mTrisFront.appendTexCoordPoly( tPoly );
			// back
			mTrisBack.appendIndex( i );
			mTrisBack.appendVertexBase( v );
			mTrisBack.appendTexCoordDoc( Vec2f( 1.0 - tDoc.x, tDoc.y ) );
			mTrisBack.appendTexCoordPoly( Vec2f( 1.0 - tPoly.x, tPoly.y ) );
			// normals
			nv[i%3] = v;
			if ( (i % 3) == 2 )
			{
				Vec3f n = triangleNormal( nv[0], nv[1], nv[2] );
				mTrisFront.appendNormalBase( n );
				mTrisFront.appendNormalBase( n );
				mTrisFront.appendNormalBase( n );
				mTrisBack.appendNormalBase( -n );
				mTrisBack.appendNormalBase( -n );
				mTrisBack.appendNormalBase( -n );
			}
		}
		mTrisFront.setTexType( TEX_COORD_DOC );
		mTrisBack.setTexType( TEX_COORD_DOC );
		
		// finish up
		this->finish();
	}
	void qbPoly::makeLayerTexCoords( Rectf bounds )
	{
		for (int i = 0 ; i < mTrisFront.getNumVertices() ; i++)
		{
			Vec3f v = mTrisFront.getVertices()[i];
			Vec2f tLayer = Vec2f( lmap(v.x,bounds.x1,bounds.x2,0.0f,1.0f), lmap(v.y,bounds.y1,bounds.y2,0.0f,1.0f) );
			mTrisFront.appendTexCoordLayer( tLayer );
			mTrisBack.appendTexCoordLayer( Vec2f( 1.0 - tLayer.x, tLayer.y ) );
		}
	}
	
	//
	// Poly Construction
	void qbPoly::addVertex( Vec3f _v )
	{
		bool dup = false;
		for (int v = 0 ; v < mVertices.size() ; v++)
			if ( mVertices[v] == _v )
				dup = true;
		mVertices.push_back( qbPolyVertex( _v, dup ) );
	}
	//
	// Finishing touches
	void qbPoly::finish()
	{
		// calc perimeter
		float perim = 0;
		for (int n = 0 ; n < mVertices.size() ; n++)
		{
			if ( n > 0 )
				perim += mVertices[n].distance( mVertices[n-1] );
			mVertices[n].mDist = perim;
		}
		mPerimeter = perim;
		
		// calc vertexes distance based on perimeter
		for (int n = 0 ; n < mVertices.size() ; n++)
		{
			mVertices[n].mProg = ( mVertices[n].mDist / mPerimeter );
			//printf("__prog  perim %.2f  vert %d  dist %.2f  prog %.2f\n",mPerimeter,n,mVertices[n].mDist,mVertices[n].mProg);
		}
		
		//
		// Make extrusion
		mTrisExtrude.clear();
		for( int c = 0 ; c < mContours.size() ; c++ )
		{
			bool outer = ( c % 2 == 0 );
			std::vector<Vec3f> & ps = mContours[c].mPoints;
			if ( ps.size() == 1 )
			{
				Vec3f p0( ps[0].xy(), 0.5f );
				Vec3f p1( ps[0].xy(), -0.5f );
				mLinesExtrude.push_back( p0 );
				mLinesExtrude.push_back( p1 );
			}
			else if ( ps.size() >= 2 )
			{
				std::vector<Vec3f> normals;	// per face
				for( int p = 0 ; p < ps.size() ; p++ )
				{
					int i0 = p;
					int i1 = ( p < ps.size()-1 ? p+1 : 0 );
					// continuation?
					if ( ps[i0] == ps[i1] )
						continue;
					// Indices
					int i = (int) mTrisExtrude.getNumVertices();
					mTrisExtrude.appendTriangle( i+0, i+1, i+2 );
					mTrisExtrude.appendTriangle( i+1, i+2, i+3 );
					// Vertices
					Vec3f p0( ps[i0].xy(), outer? 0.5f : -0.5f );
					Vec3f p1( ps[i0].xy(), outer? -0.5f : 0.5f );
					Vec3f p2( ps[i1].xy(), outer? 0.5f : -0.5f );
					Vec3f p3( ps[i1].xy(), outer? -0.5f : 0.5f );
					//printf(">> EXTRUDE vertex [%s]  p  %d / %d  =  %.6f, %.6f / %.6f, %.6f\n",mNode->getTag().c_str(),i0,i1,p0.x,p0.y,p2.x,p2.y);
					mTrisExtrude.appendVertex( p0 );
					mTrisExtrude.appendVertex( p1 );
					mTrisExtrude.appendVertex( p2 );
					mTrisExtrude.appendVertex( p3 );
					mTrisExtrude.appendTexCoord( Vec2f( 0, 0 ) );
					mTrisExtrude.appendTexCoord( Vec2f( 0, 1 ) );
					mTrisExtrude.appendTexCoord( Vec2f( 1, 0 ) );
					mTrisExtrude.appendTexCoord( Vec2f( 1, 1 ) );
					// Save face Normal
					normals.push_back( triangleNormal(p0, p1, p2) );
					// line from up face to bottom face
					mLinesExtrude.push_back( p0 );
					mLinesExtrude.push_back( p1 );
				}
				// Append normals
				for( int face = 0 ; face < normals.size() ; face++ )
				{
					if ( bSmooth )
					{
						Vec3f n;
						int i0, i1;
						// face point 1
						i0 = (int) ( face > 0 ? face-1 : ( bClosed ? normals.size()-1 : face ) );
						i1 = face;
						n = normals[i0].lerp( 0.5, normals[i1] ).normalized();
						mTrisExtrude.appendNormal( n );
						mTrisExtrude.appendNormal( n );
						//printf(">> EXTRUDE normals [%s]  %d  face  %d/1  =  %d / %d\n",mNode->getTag().c_str(),c,face,i0,i1);
						// face point 2
						i0 = face;
						i1 = ( face < normals.size()-1 ? face+1 : ( bClosed ? 0 : face ) );
						n = normals[i0].lerp( 0.5, normals[i1] ).normalized();
						mTrisExtrude.appendNormal( n );
						mTrisExtrude.appendNormal( n );
						//printf(">> EXTRUDE normals [%s]  %d  face  %d/2  =  %d / %d\n",mNode->getTag().c_str(),c,face,i0,i1);
					}
					else
					{
						mTrisExtrude.appendNormal( normals[face] );
						mTrisExtrude.appendNormal( normals[face] );
						mTrisExtrude.appendNormal( normals[face] );
						mTrisExtrude.appendNormal( normals[face] );
					}
				}
			}
		}
	}
	
	//
	// QB TRIANGULATION
	qbTriMesh qbPoly::triangulate( Shape2d & shape )
	{
		// Get contour points
		std::vector<Vec3f> points;
		std::vector<Path2d> paths = shape.getContours();
		for( int p = 0 ; p < paths.size() ; p++ )
		{
			std::vector<Vec2f> ps = paths[p].subdivide( M_STEP );
			if ( ps.empty() && paths[p].getNumPoints() > 0 )
				ps = paths[p].getPoints();
			for( int pt = 0 ; pt < ps.size(); pt++ )
				points.push_back( Vec3f( ps[pt].xy(), 0 ) );
		}
		
		// Triangulate all vertices
		std::vector<Vec3f> vertices;
		bourkeTriangulateFromPoints( points, &vertices );
		
		// Make Trimesh2d
		qbTriMesh tri;
		for (int t = 0 ; t < vertices.size()/3 ; t++)
		{
			int i = t*3;
			Vec3f p0( vertices[i+0] );
			Vec3f p1( vertices[i+1] );
			Vec3f p2( vertices[i+2] );
			Vec2f c( triangleCentroid( p0.xy(), p1.xy(), p2.xy() ) );
			//printf(" TRRI   %d / %d  =  %.1f/%.1f  %.1f/%.1f  %.1f/%.1f  ==  c %.1f/%.1f  in %d\n",t,i,p0.x,p0.y,p1.x,p1.y,p2.x,p2.y,c.x,c.y,(int)shape.contains(c));
			// good triangle
			if ( shape.contains( c ) )
			{
				tri.appendIndex( (uint32_t) tri.getNumVertices() );
				tri.appendVertex( p0 );
				tri.appendIndex( (uint32_t) tri.getNumVertices() );
				tri.appendVertex( p1 );
				tri.appendIndex( (uint32_t) tri.getNumVertices() );
				tri.appendVertex( p2 );
			}
		}
		return tri;
	}
	

	
	
	//////////////////////////////////////////////////////////
	//
	// POLYS GETTERS
	//
	Vec3f qbPoly::getPointAt( float prog )
	{
		float p = math<float>::clamp( prog, 0.0f, 1.0f );
		if ( p == 0.0 )
			return mVertices.front();
		for ( int i = 1 ; i < mVertices.size() ; i++ )
		{
			if( p <= mVertices[i].mProg )
			{
				float pp = lmap<float>(p, mVertices[i-1].mProg, mVertices[i].mProg, 0, 1);
				return  mVertices[i-1].lerp( pp, mVertices[i] );
			}
		}
		return mVertices.back();
	}
	float qbPoly::getLoopingProgAt( float prog )
	{
		if ( bClosed )
			return prog;
		else if ( prog <= 0.5f )
			return ( prog * 2.0f );
		else
			return ( 1.0 - ((prog-0.5f) * 2.0f) );
	}
	
	float qbPoly::getLoopingDistanceAt(float dist)
	{
		if ( bClosed )
			return dist;
		// clamp
		float d = dist;
		while ( d < -mPerimeter * 0.5f )
			d += mPerimeter;
		while (d > mPerimeter * 0.5f )
			d -= mPerimeter;
		if ( d < 0.0f )
			return lmap( d, -mPerimeter * 0.5f, 0.0f, mPerimeter, 0.0f );
		else if ( d <= mPerimeter * 0.5f )
			return lmap( d, 0.0f, mPerimeter * 0.5f, 0.0f, mPerimeter );
		else
			return lmap( d, mPerimeter * 0.5f, mPerimeter, mPerimeter, 0.0f );
	}

	Vec3f qbPoly::getPointAtDistance( float dist )
	{
		if (mPerimeter == 0)
			return ( mVertices.size() ? mVertices[0] : Vec3f::zero() );
		// clamp
		float d = dist;
		while ( d < 0 )
			d += mPerimeter;
		while (d > mPerimeter )
			d -= mPerimeter;
		if ( d == 0 )
			return mVertices.front();
		for ( int i = 1 ; i < mVertices.size() ; i++ )
		{
			if( d <= mVertices[i].mDist )
			{
				float dd = lmap<float>(d, mVertices[i-1].mDist, mVertices[i].mDist, 0, 1);
				return  mVertices[i-1].lerp( dd, mVertices[i] );
			}
		}
		return mVertices.back();
	}
	

	
	//////////////////////////////////////////////////////////
	//
	// TRIMESH
	//
	void qbTriMesh::setTexType( int type )
	{
		// erase current
		if ( ( type == TEX_COORD_DOC && mTexType != TEX_COORD_DOC && mTexCoordsDoc.size() ) || 
			 ( type == TEX_COORD_LAYER && mTexType != TEX_COORD_LAYER && mTexCoordsLayer.size() ) || 
			 ( type == TEX_COORD_POLY && mTexType != TEX_COORD_POLY && mTexCoordsPoly.size() ) )
		{
			mTexCoords.clear();
			mTexType = type;
		}
		// update textures
		if ( mTexCoords.empty() )
		{
			if ( mTexType == TEX_COORD_DOC )
				for( int i = 0; i < mTexCoordsDoc.size() ; ++i )
					mTexCoords.push_back( mTexCoordsDoc[i] );
			else if ( mTexType == TEX_COORD_LAYER )
				for( int i = 0; i < mTexCoordsLayer.size() ; ++i )
					mTexCoords.push_back( mTexCoordsLayer[i] );
			else if ( mTexType == TEX_COORD_POLY )
				for( int i = 0; i < mTexCoordsPoly.size() ; ++i )
					mTexCoords.push_back( mTexCoordsPoly[i] );
		}
	}

	void qbTriMesh::appendVertexBase( const Vec3f &v )
	{
		mVerticesBase.push_back( v );
		mVertices.push_back( v );
	}
	
	void qbTriMesh::appendNormalBase( const Vec3f &n )
	{
		mNormalsBase.push_back( n );
		mNormals.push_back( n );
	}
	
	void qbTriMesh::loadBase()
	{
		if ( ! mVerticesBase.empty() )
		{
			mVertices.clear();
			this->appendVertices(&(mVerticesBase[0]),mVerticesBase.size());
		}
		if ( ! mNormalsBase.empty() )
		{
			mNormals.clear();
			this->appendNormals(&(mNormalsBase[0]),mNormalsBase.size());
		}
	}
	
	
	
	
	

} } // cinder::qb






//////////////////////////////////////////////////////
//
// Triangulation by Paul Bourke
// http://paulbourke.net/papers/triangulate/
// 
// Triangulation subroutine
// Takes as input NV vertices in array pxyz
// Returned is a list of ntri triangular faces in the array v
// These triangles are arranged in a consistent clockwise order.
// The triangle array 'v' should be malloced to 3 * nv
// The vertex array pxyz must be big enough to hold 3 more points
// The vertex array must be sorted in increasing x values say
//
using namespace cinder;
typedef struct {
	int p1,p2,p3;
} ITRIANGLE;
typedef struct {
	int p1,p2;
} IEDGE;
int bourkeTriangulate(int nv, Vec3f *pxyz,ITRIANGLE *v,int *ntri);
int bourkeCircumCircle(double xp,double yp,double x1,double y1,double x2,double y2,double x3,double y3,double *xc,double *yc,double *rsqr);
int bourkeXYZCompare(const void *v1,const void *v2);
int bourkeTriangulateFromPoints( std::vector<Vec3f> & points, std::vector<Vec3f> * vertices )
{
	int i;
	int ntri = 0;
	ITRIANGLE *v;
	Vec3f *p = NULL;
	int nv = 0;
	
	for ( nv = 0 ; nv < points.size() ; nv++ )
	{
		// duplicated?
		for ( int nv2 = 0 ; nv2 < nv ; nv2++ )
			if ( p[nv] == p[nv2] )
				break;
		// add
		p = (Vec3f*) realloc( p, (nv+1)*sizeof(Vec3f) );
		p[nv] = points[nv];
	}
	if (nv < 3)
		return 0;
	p = (Vec3f*) realloc(p,(nv+3)*sizeof(Vec3f));
	v = (ITRIANGLE*) malloc(3*nv*sizeof(ITRIANGLE));
	qsort(p,nv,sizeof(Vec3f),bourkeXYZCompare);
	bourkeTriangulate(nv,p,v,&ntri);
	//fprintf(stderr,"Formed %d triangles\n",ntri);
	/* Write triangles in geom format */
	for (i=0;i<ntri;i++) {
		/*
		 printf("TRIS >>>>>>>>> %d = f3 %g %g %g %g %g %g %g %g %g 1 1 1\n",
		 i,
		 p[v[i].p1].x,p[v[i].p1].y,p[v[i].p1].z,
		 p[v[i].p2].x,p[v[i].p2].y,p[v[i].p2].z,
		 p[v[i].p3].x,p[v[i].p3].y,p[v[i].p3].z);
		 */
		vertices->push_back( Vec3f( p[v[i].p1].x, p[v[i].p1].y, p[v[i].p1].z ) );
		vertices->push_back( Vec3f( p[v[i].p2].x, p[v[i].p2].y, p[v[i].p2].z ) );
		vertices->push_back( Vec3f( p[v[i].p3].x, p[v[i].p3].y, p[v[i].p3].z ) );
	}
	free(p);
	free(v);
	return ntri;
}
int bourkeXYZCompare(const void *v1,const void *v2)
{
	ci::Vec3f *p1,*p2;
	p1 = (ci::Vec3f*) v1;
	p2 = (ci::Vec3f*) v2;
	if (p1->x < p2->x)
		return(-1);
	else if (p1->x > p2->x)
		return(1);
	else 
		return(0); 
}       
int bourkeTriangulate(int nv,Vec3f *pxyz,ITRIANGLE *v,int *ntri)
{
	int *complete = NULL;
	IEDGE *edges = NULL;
	int nedge = 0;
	int trimax,emax = 200;
	int status = 0;
	int inside;
	int i,j,k;
	double xp,yp,x1,y1,x2,y2,x3,y3,xc,yc,r;
	double xmin,xmax,ymin,ymax,xmid,ymid;
	double dx,dy,dmax;
	/* Allocate memory for the completeness list, flag for each triangle */
	trimax = 4 * nv;
	if ((complete = (int*) malloc(trimax*sizeof(int))) == NULL) {
		status = 1;
		goto skip;
	}
	/* Allocate memory for the edge list */
	if ((edges = (IEDGE*) malloc(emax*(long)sizeof(IEDGE))) == NULL) {
		status = 2;
		goto skip;
	}
	/*
	 Find the maximum and minimum vertex bounds.
	 This is to allow calculation of the bounding triangle
	 */
	xmin = pxyz[0].x;
	ymin = pxyz[0].y;
	xmax = xmin;
	ymax = ymin;
	for (i=1;i<nv;i++) {
		if (pxyz[i].x < xmin) xmin = pxyz[i].x;
		if (pxyz[i].x > xmax) xmax = pxyz[i].x;
		if (pxyz[i].y < ymin) ymin = pxyz[i].y;
		if (pxyz[i].y > ymax) ymax = pxyz[i].y;
	}
	dx = xmax - xmin;
	dy = ymax - ymin;
	dmax = (dx > dy) ? dx : dy;
	xmid = (xmax + xmin) / 2.0;
	ymid = (ymax + ymin) / 2.0;
	/*
	 Set up the supertriangle
	 This is a triangle which encompasses all the sample points.
	 The supertriangle coordinates are added to the end of the
	 vertex list. The supertriangle is the first triangle in
	 the triangle list.
	 */
	pxyz[nv+0].x = xmid - 20 * dmax;
	pxyz[nv+0].y = ymid - dmax;
	pxyz[nv+0].z = 0.0;
	pxyz[nv+1].x = xmid;
	pxyz[nv+1].y = ymid + 20 * dmax;
	pxyz[nv+1].z = 0.0;
	pxyz[nv+2].x = xmid + 20 * dmax;
	pxyz[nv+2].y = ymid - dmax;
	pxyz[nv+2].z = 0.0;
	v[0].p1 = nv;
	v[0].p2 = nv+1;
	v[0].p3 = nv+2;
	complete[0] = false;
	*ntri = 1;
	/*
	 Include each point one at a time into the existing mesh
	 */
	for (i=0;i<nv;i++) {
		
		xp = pxyz[i].x;
		yp = pxyz[i].y;
		nedge = 0;
		/*
		 Set up the edge buffer.
		 If the point (xp,yp) lies inside the circumcircle then the
		 three edges of that triangle are added to the edge buffer
		 and that triangle is removed.
		 */
		for (j=0;j<(*ntri);j++) {
			if (complete[j])
				continue;
			x1 = pxyz[v[j].p1].x;
			y1 = pxyz[v[j].p1].y;
			x2 = pxyz[v[j].p2].x;
			y2 = pxyz[v[j].p2].y;
			x3 = pxyz[v[j].p3].x;
			y3 = pxyz[v[j].p3].y;
			inside = bourkeCircumCircle(xp,yp,x1,y1,x2,y2,x3,y3,&xc,&yc,&r);
			if (xc < xp && ((xp-xc)*(xp-xc)) > r)
				complete[j] = true;
			if (inside) {
				/* Check that we haven't exceeded the edge list size */
				if (nedge+3 >= emax) {
					emax += 100;
					if ((edges = (IEDGE*) realloc(edges,emax*(long)sizeof(IEDGE))) == NULL) {
						status = 3;
						goto skip;
					}
				}
				edges[nedge+0].p1 = v[j].p1;
				edges[nedge+0].p2 = v[j].p2;
				edges[nedge+1].p1 = v[j].p2;
				edges[nedge+1].p2 = v[j].p3;
				edges[nedge+2].p1 = v[j].p3;
				edges[nedge+2].p2 = v[j].p1;
				nedge += 3;
				v[j] = v[(*ntri)-1];
				complete[j] = complete[(*ntri)-1];
				(*ntri)--;
				j--;
			}
		}
		/*
		 Tag multiple edges
		 Note: if all triangles are specified anticlockwise then all
		 interior edges are opposite pointing in direction.
		 */
		for (j=0;j<nedge-1;j++) {
			for (k=j+1;k<nedge;k++) {
				if ((edges[j].p1 == edges[k].p2) && (edges[j].p2 == edges[k].p1)) {
					edges[j].p1 = -1;
					edges[j].p2 = -1;
					edges[k].p1 = -1;
					edges[k].p2 = -1;
				}
				/* Shouldn't need the following, see note above */
				if ((edges[j].p1 == edges[k].p1) && (edges[j].p2 == edges[k].p2)) {
					edges[j].p1 = -1;
					edges[j].p2 = -1;
					edges[k].p1 = -1;
					edges[k].p2 = -1;
				}
			}
		}
		/*
		 Form new triangles for the current point
		 Skipping over any tagged edges.
		 All edges are arranged in clockwise order.
		 */
		for (j=0;j<nedge;j++) {
			if (edges[j].p1 < 0 || edges[j].p2 < 0)
				continue;
			if ((*ntri) >= trimax) {
				status = 4;
				goto skip;
			}
			v[*ntri].p1 = edges[j].p1;
			v[*ntri].p2 = edges[j].p2;
			v[*ntri].p3 = i;
			complete[*ntri] = false;
			(*ntri)++;
		}
	}
	/*
	 Remove triangles with supertriangle vertices
	 These are triangles which have a vertex number greater than nv
	 */
	for (i=0;i<(*ntri);i++) {
		if (v[i].p1 >= nv || v[i].p2 >= nv || v[i].p3 >= nv) {
			v[i] = v[(*ntri)-1];
			(*ntri)--;
			i--;
		}
	}
skip:
	if (edges)
		free(edges);
	if (complete)
		free(complete);
	return(status);
}

/*
 Return TRUE if a point (xp,yp) is inside the circumcircle made up
 of the points (x1,y1), (x2,y2), (x3,y3)
 The circumcircle centre is returned in (xc,yc) and the radius r
 NOTE: A point on the edge is inside the circumcircle
 */
int bourkeCircumCircle(double xp,double yp,
							   double x1,double y1,double x2,double y2,double x3,double y3,
							   double *xc,double *yc,double *rsqr)
{
	double m1,m2,mx1,mx2,my1,my2;
	double dx,dy,drsqr;
	double fabsy1y2 = math<double>::abs(y1-y2);
	double fabsy2y3 = math<double>::abs(y2-y3);
	
	/* Check for coincident points */
	if (fabsy1y2 < EPSILON && fabsy2y3 < EPSILON)
		return(false);
	
	if (fabsy1y2 < EPSILON) {
		m2 = - (x3-x2) / (y3-y2);
		mx2 = (x2 + x3) / 2.0;
		my2 = (y2 + y3) / 2.0;
		*xc = (x2 + x1) / 2.0;
		*yc = m2 * (*xc - mx2) + my2;
	} else if (fabsy2y3 < EPSILON) {
		m1 = - (x2-x1) / (y2-y1);
		mx1 = (x1 + x2) / 2.0;
		my1 = (y1 + y2) / 2.0;
		*xc = (x3 + x2) / 2.0;
		*yc = m1 * (*xc - mx1) + my1;
	} else {
		m1 = - (x2-x1) / (y2-y1);
		m2 = - (x3-x2) / (y3-y2);
		mx1 = (x1 + x2) / 2.0;
		mx2 = (x2 + x3) / 2.0;
		my1 = (y1 + y2) / 2.0;
		my2 = (y2 + y3) / 2.0;
		*xc = (m1 * mx1 - m2 * mx2 + my2 - my1) / (m1 - m2);
		if (fabsy1y2 > fabsy2y3) {
			*yc = m1 * (*xc - mx1) + my1;
		} else {
			*yc = m2 * (*xc - mx2) + my2;
		}
	}
	
	dx = x2 - *xc;
	dy = y2 - *yc;
	*rsqr = dx*dx + dy*dy;
	
	dx = xp - *xc;
	dy = yp - *yc;
	drsqr = dx*dx + dy*dy;
	
	// Original
	//return((drsqr <= *rsqr) ? TRUE : FALSE);
	// Proposed by Chuck Morris
	return((drsqr - *rsqr) <= EPSILON ? true : false);
}



