//
//  qbSvgOld.cpp
//
//  Created by Roger Sodre on 01/07/2011
//  Copyright 2011 Studio Avante. All rights reserved.
//

#include "qbSvg.h"
#include "qbCube.h"
#include "cinder/app/App.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"

#include <errno.h>
	
#define VERBOSE_SVG

#define SVG_xyz(x,y,z)		(SVG_Vec3f(Vec3f((x),(y),(z))))
#define SVG_Vec3f(v)		((v)-Vec3f(mViewBox.getX1(),mViewBox.getY1(),0))

enum enumPolytype {
	QBPOLY_NONE = -1,	// not a polygon (0 vertices)
	QBPOLY_POINT,		// single point
	QBPOLY_POLYLINE,	// open polygon (line strip)
	QBPOLY_POLYGON,		// closed generic polygon (line loop)
	QBPOLY_TRI,			// triangle
	QBPOLY_QUAD,		// generic quad
	QBPOLY_SQUARE,		// square
	QBPOLY_RECT,		// rectangle
	// NEW
	QBPOLY_SHAPE,		// Made from ci::Svg node
	// count
	QBPOLY_COUNT
};



namespace cinder { namespace qb {
	
	///////////////////////////////////
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
	

	//
	// Return polys loaded
	int qbSvgOld::parseSvg( const XmlTree  & _doc, Vec2f destSize ) {
		// grab SVG
		if ( ! _doc.hasChild("svg") )
		{
			printf(">>> QBSVG : doc is NOT SVG!\n");
			return 0;
		}
		XmlTree svg = _doc.getChild( "svg" );
		
		// Get doc data
		float w, h;
		if ( svg.hasAttribute( "viewBox" ) )
		{
			//
			// ATENCAO :: VIEWBOX NAO EH BOUNDS!!!!!!!
			//
			// <min-x>, <min-y>, <width> and <height>
			// http://www.w3.org/TR/SVG/coords.html#ViewBoxAttribute
			std::vector<std::string> vals = ci::split( svg.getAttributeValue<std::string>( "viewBox" ), " " );
			for (int n = 0 ; n < vals.size() ; n++)
				printf(">>> VIEWBOX  %d/%d =  %s\n",n,(int)vals.size(),vals[n].c_str());
			float x = atof(vals[0].c_str());
			float y = atof(vals[1].c_str());
			w = atof(vals[2].c_str());
			h = atof(vals[3].c_str());
			printf(">>> VIEWBOX  xywh  %.2f %.2f %.2f %.2f\n",x,y,w,h);
			mViewBox = Rectf( x, y, w, h );
		}
		else if ( svg.hasAttribute( "width" ) && svg.hasAttribute( "height" ) )
		{
			w = svg.getAttributeValue<float>( "width" );
			h = svg.getAttributeValue<float>( "height" );
			mViewBox = Rectf( 0, 0, w, h );
		}
		printf(">>> QBSVG : viewBox : %.2f, %.2f, %.2f, %.2f\n",mViewBox.getX1(),mViewBox.getX2(),mViewBox.getY1(),mViewBox.getY2());
		
		// Calc scale
		if ( destSize != Vec2f::zero() )
			mScale = Vec3f( destSize.x / w ,  destSize.y / h, 1.0f );
		else
			mScale = Vec3f::one();
		printf(">>> QBSVG : scale : %.4f %.4f\n",mScale.x,mScale.y);
		
		// Update viewbox
		mViewBox.x1 *= mScale.x;
		mViewBox.x2 *= mScale.x;
		mViewBox.y1 *= mScale.y;
		mViewBox.y2 *= mScale.y;

		// parse!!
		this->parseSvgLayer( svg );
		
		return (int) mPolys.size();
	}
	
	//
	// LAYER
	int lay = -1;
	void qbSvgOld::parseSvgLayer( const XmlTree  & _l ) {
		// get a name
		std::string layerName = "<" + _l.getTag() + ">";
		if ( _l.hasAttribute( "id" ) )
			layerName = _l.getAttributeValue<std::string>( "id" );
		mCurrLayerName = layerName;
		// is off??
		if ( _l.hasAttribute( "display" ) )
		{
			std::string display = _l.getAttributeValue<std::string>( "display" );
			if ( display == "none" )
			{
				printf(">>> QBSVG : %s : is OFF!\n",layerName.c_str());
				return;
			}
		}
		// Parse children
		std::list<XmlTree> children = _l.getChildren();
		std::list<XmlTree>::iterator p;
		for ( p = children.begin() ; p != children.end(); p++ )
		{
			qbPolyOld apoly;
			// get a name
			std::string tag = (*p).getTag();
			if ( (*p).hasAttribute( "id" ) )
				apoly.setName( (*p).getAttributeValue<std::string>( "id" ) );
			else
				apoly.setName( "<" + tag + ">" );
			// parse
			if ( tag == "g" || tag == "switch" )
			{
				this->parseSvgLayer( *p );
				mCurrLayerName = layerName;
			}
			else if ( tag == "polygon" )
				this->parseSvgPolygon( *p, true, &apoly );
			else if ( tag == "polyline" )
				this->parseSvgPolygon( *p, false, &apoly );
			else if ( tag == "rect" )
				this->parseSvgRect( *p, &apoly );
			else if ( tag == "line" )
				this->parseSvgLine( *p, &apoly );
			else if ( tag == "circle" )
				this->parseSvgCircle( *p, &apoly );
			else if ( tag == "ellipse" )
				this->parseSvgEllipse( *p, &apoly );
			else if ( tag == "path" )
				this->parseSvgPath( *p, &apoly );
			else
			{
#ifdef VERBOSE_SVG
				printf(">>> QBSVG : %s : %s : UNKNOWN TAG\n",mCurrLayerName.c_str(),apoly.getName().c_str());
#endif
				//continue;
			}
			// Add poly!
			if ( apoly.getType() != QBPOLY_NONE )
			{
				// set layer
				if ( lay == -1 || mCurrLayerName != mLastLayerName )
				{
					lay++;
					mLastLayerName = mCurrLayerName;
					printf(">>> QBSVG : NEW LAYER %d [%s]\n",lay,mCurrLayerName.c_str());
				}
				apoly.setLayerName( mCurrLayerName );
				apoly.setLayer( lay );
				// Add poly!
				this->addPoly( apoly );
			}
		}
	}
	//
	// <polygon fill="none" stroke="#ED1C24" stroke-width="2" stroke-miterlimit="10" points="1854.75,340.249 1954.734,340.249 2042.234,627.698 1767.245,627.698 	"/>
	// <polyline fill="none" stroke="#ED1C24" stroke-width="2" stroke-miterlimit="10" points="1854.75,340.249 1954.734,340.249 2042.234,627.698 1767.245,627.698 	"/>
	void qbSvgOld::parseSvgPolygon( const XmlTree  & _p, bool _closed, qbPolyOld *apoly ) {
		std::vector<std::string> points = ci::split( _p.getAttributeValue<std::string>( "points" ), " \t\n\r" );
#ifdef VERBOSE_SVG
		printf(">>> QBSVG : %s : %s : POLYGON (%d vertices)...\n",mCurrLayerName.c_str(),apoly->getName().c_str(),(int)points.size());
#endif
		for (int p = 0 ; p < points.size() ; p++)
		{
			std::vector<std::string> vals = ci::split( points[p], "," );
			printf("    %d : points (%d) = [%s]\n",p,(int)vals.size(),points[p].c_str());
			if (vals.size() == 2)
			{
				float x = atof(vals[0].c_str()) * mScale.x;
				float y = atof(vals[1].c_str()) * mScale.y;
				apoly->addVertex( SVG_xyz( x, y, 0.0f ) );
			}
		}
		apoly->finishOptimized();
		if ( _closed )
			apoly->close();
	}
	
	//
	// <rect x="137" y="571.495" width="749.999" height="125.005"/>
	void qbSvgOld::parseSvgRect( const XmlTree  & _p, qbPolyOld *apoly ) {
		try {
			float x = _p.getAttributeValue<float>( "x" ) * mScale.x;
			float y = _p.getAttributeValue<float>( "y" ) * mScale.y;
			float w = _p.getAttributeValue<float>( "width" ) * mScale.x;
			float h = _p.getAttributeValue<float>( "height" ) * mScale.y;
#ifdef VERBOSE_SVG
			printf(">>> QBSVG : %s : %s : RECT xy %.1f %.1f  wh %.1f %.1f\n",mCurrLayerName.c_str(),apoly->getName().c_str(),x,y,w,h);
#endif
			apoly->makeRect( SVG_xyz(x,y,0), w, h );
		} catch ( XmlTree::Exception ) {
			printf(">>> QBSVG : RECT exception\n");
		}
	}
	
	//
	// <line fill="none" stroke="#000000" stroke-miterlimit="10" x1="33.5" y1="338.5" x2="55.5" y2="445.5"/>
	void qbSvgOld::parseSvgLine( const XmlTree  & _p, qbPolyOld *apoly ) {
		try {
			float x1 = _p.getAttributeValue<float>( "x1" ) * mScale.x;
			float y1 = _p.getAttributeValue<float>( "y1" ) * mScale.y;
			float x2 = _p.getAttributeValue<float>( "x2" ) * mScale.x;
			float y2 = _p.getAttributeValue<float>( "y2" ) * mScale.y;
#ifdef VERBOSE_SVG
			printf(">>> QBSVG : %s : %s : LINE xy %.1f %.1f .. w %.1f %.1f\n",mCurrLayerName.c_str(),apoly->getName().c_str(),x1,y1,x2,y2);
#endif
			apoly->addVertex( SVG_xyz( x1, y1, 0 ) );
			apoly->addVertex( SVG_xyz( x2, y2, 0 ) );
			apoly->finishOptimized();
		} catch ( XmlTree::Exception ) {
			printf(">>> QBSVG : LINE exception\n");
		}
	}
	
	// <circle fill="none" stroke="#000000" stroke-miterlimit="10" cx="282" cy="380" r="45.5"/>
	void qbSvgOld::parseSvgCircle( const XmlTree  & _p, qbPolyOld *apoly ) {
		try {
			float x = _p.getAttributeValue<float>( "cx" ) * mScale.x;
			float y = _p.getAttributeValue<float>( "cy" ) * mScale.y;
			float rx = _p.getAttributeValue<float>( "r" ) * mScale.x;
			float ry = _p.getAttributeValue<float>( "r" ) * mScale.y;
#ifdef VERBOSE_SVG
			printf(">>> QBSVG : %s : %s : CIRCLE xy %.1f %.1f  r %.1f %.1f\n",mCurrLayerName.c_str(),apoly->getName().c_str(),x,y,rx,ry);
#endif
			apoly->makeCircle( SVG_xyz(x,y,0), rx, ry );
		} catch ( XmlTree::Exception ) {
			printf(">>> QBSVG : CIRCLE exception\n");
		}
	}
	
	// <ellipse fill="none" stroke="#000000" stroke-miterlimit="10" cx="407" cy="377.5" rx="16" ry="57.5"/>
	void qbSvgOld::parseSvgEllipse( const XmlTree  & _p, qbPolyOld *apoly ) {
		try {
			float x = _p.getAttributeValue<float>( "cx" ) * mScale.x;
			float y = _p.getAttributeValue<float>( "cy" ) * mScale.y;
			float rx = _p.getAttributeValue<float>( "rx" ) * mScale.x;
			float ry = _p.getAttributeValue<float>( "ry" ) * mScale.y;
#ifdef VERBOSE_SVG
			printf(">>> QBSVG : %s : %s : ELLIPSE xy %.1f %.1f  rxy %.1f %.1f\n",mCurrLayerName.c_str(),apoly->getName().c_str(),x,y,rx,ry);
#endif
			apoly->makeCircle( SVG_xyz(x,y,0), rx, ry );
		} catch ( XmlTree::Exception ) {
			printf(">>> QBSVG : ELLIPSE exception\n");
		}
	}
	
	// From: http://www.w3.org/TR/SVG/paths.html#PathData
	// <path fill="none" stroke="#000000" stroke-miterlimit="10" d="M190.5,356.5c0,53.062,42.49,96,95,96"/>
	void qbSvgOld::parseSvgPath( const XmlTree  & _p, qbPolyOld *apoly ) {
		try {
#ifdef VERBOSE_SVG
			printf(">>> QBSVG : %s : %s : PATH...\n",mCurrLayerName.c_str(),apoly->getName().c_str());
#endif
			char cmd;
			Vec3f v, last_v;
			Vec3f c0, c1, last_c;
			std::string d = _p.getAttributeValue<std::string>( "d" );
			std::string values;
			std::string::iterator it;
			for ( it=d.begin() ; it < d.end(); it++ )
			{
				char c = *it;
				if ( isalpha(c) )
				{
					cmd = c;
					values.clear();
				}
				else
					values += c;
				// jump following spaces
				while ( isspace(*(it+1)) )
					it++;
				// Execure command!
				if ( isalpha(*(it+1)) || (it+1) == d.end() )
				{
					std::vector<float> vals = splitPathValues( values );
#ifdef VERBOSE_SVG
					printf("            %c = [%s] = %d vals (",cmd,values.c_str(),(int)vals.size());
					for (int n = 0 ; n < vals.size() ; n++)
						 printf("%s%.1f",(n==0?" ":", "),vals[n]);
					printf(" )\n");
#endif
					switch (cmd)
					{
						case 'm':		// Relative
						case 'M':		// Absolute
						case 'l':		// Relative
						case 'L':		// Absolute
							// http://www.w3.org/TR/SVG/paths.html#PathDataMovetoCommands
							// http://www.w3.org/TR/SVG/paths.html#PathDataLinetoCommands
							// 2 points: (x y)
							for (int n = 0 ; n < vals.size() / 2 ; n++)
							{
								v = Vec3f( vals[n+0], vals[n+1], 0 ) * mScale;
								if ( islower(cmd) )
									v += last_v;
								apoly->addVertex( SVG_Vec3f(v) );
								last_v = last_c = v;	// save last
							}
							break;
						case 'h':		// Relative
						case 'H':		// Absolute
							// http://www.w3.org/TR/SVG/paths.html#PathDataLinetoCommands
							// 1 point: (x)
							for (int n = 0 ; n < vals.size() / 1 ; n++)
							{
								v = Vec3f( vals[n+0] * mScale.x, last_v.y, 0 );
								if ( islower(cmd) )
									v.x += last_v.x;
								apoly->addVertex( SVG_Vec3f(v) );
								last_v = last_c = v;	// save last
							}
							break;
						case 'v':		// Relative
						case 'V':		// Absolute
							// http://www.w3.org/TR/SVG/paths.html#PathDataLinetoCommands
							// 1 point: (y)
							for (int n = 0 ; n < vals.size() / 1 ; n++)
							{
								v = Vec3f( last_v.x, vals[n+0] * mScale.y, 0 );
								if ( islower(cmd) )
									v.y += last_v.y;
								apoly->addVertex( SVG_Vec3f(v) );
								last_v = last_c = v;	// save last
							}
							break;
						case 'c':		// Relative
						case 'C':		// Absolute
							// http://www.w3.org/TR/SVG/paths.html#PathDataCubicBezierCommands
							// 6 points: (x1 y1 x2 y2 x y)
							for (int n = 0 ; n < vals.size() / 6 ; n++)
							{
								c0 = Vec3f( vals[n+0], vals[n+1], 0 ) * mScale;
								c1 = Vec3f( vals[n+2], vals[n+3], 0 ) * mScale;
								v = Vec3f( vals[n+4], vals[n+5], 0 ) * mScale;
								if ( islower(cmd) )
								{
									v += last_v;
									c0 += last_v;
									c1 += last_v;
								}
								apoly->makeCurve( SVG_Vec3f(last_v), SVG_Vec3f(v), c0, c1 );
								last_v = v;		// save last
								last_c = c1;	// save last
							}
							break;
						case 's':		// Relative
						case 'S':		// Absolute
							// http://www.w3.org/TR/SVG/paths.html#PathDataCubicBezierCommands
							// 4 points: (x2 y2 x y)
							for (int n = 0 ; n < vals.size() / 4 ; n++)
							{
								c0 = last_v - Vec3f( last_c.x-last_v.x, last_c.y-last_v.y, 0);	// reflection
								c1 = Vec3f( vals[n+0], vals[n+1], 0 ) * mScale;
								v = Vec3f( vals[n+2], vals[n+3], 0 ) * mScale;
								if ( islower(cmd) )
								{
									v += last_v;
									c1 += last_v;
								}
								apoly->makeCurve( SVG_Vec3f(last_v), SVG_Vec3f(v), c0, c1 );
								last_v = v;		// save last
								last_c = c1;	// save last
							}
							break;
						case 'z':
						case 'Z':
							// http://www.w3.org/TR/SVG/paths.html#PathDataClosePathCommand
							apoly->close();
							break;
						default:
							printf(">>> QBSVG : command [%c] NOT IMPLEMENTED!!!\n",cmd);
							break;
					}
				}
			}
			apoly->finishOptimized();
		} catch ( XmlTree::Exception ) {
			printf(">>> QBSVG : PATH exception\n");
		}
	}
	// Split Path values string
	// Values are separated by comma or minus (-) sign of next value
	// From: http://www.w3.org/TR/SVG/paths.html#PathData
	// <path fill="none" stroke="#000000" stroke-miterlimit="10" d="M190.5,356.5c0,53.062,42.49,96,95,96"/>
	std::vector<float> qbSvg::splitPathValues( std::string & _values )
	{
		std::vector<float> theValues;
		std::string v;
		std::string::iterator it;
		for ( it=_values.begin() ; it < _values.end(); it++ )
		{
			char c = *it;
			if ( isnumber(c) || c == '.' || (c == '-' && v.size()==0) )
				v += c;
			if ( c == ',' || 
				 c == ' ' || 
				( c == '-' && (v.size()>1 || (v.size()>0 && isnumber(v[0]))) ) || 
				(it+1) == _values.end() )
			{
				float fv = atof( v.c_str() );
				//printf("                 %.3f\n",fv);
				theValues.push_back(fv);
				v.clear();
				if ( c == '-' )
					v += c;
			}
		}
		return theValues;
	}
	
	
	
	
	
	
	
	
	
	
	void qbPolyGroup::randomize()
	{
		std::vector<qbPoly> newPolys;
		for (int n = 0 ; n < this->getPolyCount() ; n++)
		{
			qbPoly p;
			p.copyFrom( this->getPoly(n), true );
			p.randomize();
			newPolys.push_back( p );
		}
		mPolys = newPolys;
	}

	
	
	//
	// Poly Construction
	void qbPolyOld::addVertex (Vec3f _v )
	{
		bool dup = false;
		for (int v = 0 ; v < mVertices.size() ; v++)
		{
			if ( mVertices[v] == _v )
			{
				dup = true;
				// close if got back to 1st point
				if ( v == 0 && mVertices.size() > 2 )
					bClosed = true;
				//printf("...duplicated vertex!\n");
				//return;
			}
		}
		mVertices.push_back( qbPolyVertex( _v, dup ) );
	}

	void qbPolyOld::close()
	{
		if ( mVertices.front() != mVertices.back() )
			this->addVertex( this->getVertex(0) );
		bClosed = true;
	}

	
	void qbPolyOld::copyFrom( qbPolyOld & src, bool _asPolygons )
	{
		mVertices.empty();
		bClosed = src.isClosed();
		mLayerName = src.getLayerName();
		mLayer = src.getLayer();
		mName = src.getName();
		int t = src.getType();
		// convert quads to polygon?
		if ( _asPolygons && (t == QBPOLY_QUAD || t == QBPOLY_SQUARE || t == QBPOLY_RECT) )
		{
			mType = QBPOLY_POLYGON;
			this->addVertex( src.getVertex(0) );
			this->addVertex( src.getVertex(1) );
			this->addVertex( src.getVertex(3) );
			this->addVertex( src.getVertex(2) );
			this->close();
			this->finish();
		}
		// copy as is
		else
		{
			for (int v = 0 ; v < src.getVertexCount() ; v++)
				this->addVertex( src.getVertex(v) );
			this->finishOptimized();
		}
	}
	void qbPolyOld::makeTriangle ( Vec3f v0, Vec3f v1, Vec3f v2 )
	{
		mType = QBPOLY_TRI;
		mVertices.empty();
		this->addVertex( v0 );
		this->addVertex( v1 );
		this->addVertex( v2 );
		this->close();
		this->finish();
	}
	void qbPolyOld::makeRect( Vec3f v, float w, float h )
	{
		mType = ( w == h ? QBPOLY_SQUARE : QBPOLY_RECT );
		mVertices.empty();
		this->addVertex( v + Vec3f( 0, 0, 0) );
		this->addVertex( v + Vec3f( w, 0, 0) );
		this->addVertex( v + Vec3f( w, h, 0) );
		this->addVertex( v + Vec3f( 0, h, 0) );
		this->close();
		//mBounds = Rectf( x, y, x+w, y+h );
		//mPerimeter = (w * 2) + (h * 2);
		this->finish();
	}
	void qbPolyOld::makeCircle( Vec3f v, float rx, float ry )
	{
		mType = QBPOLY_POLYGON;
		mVertices.empty();
		int pts = 8 + ( std::max<float>(rx,ry) / 20.0f ) * 4;
		for ( int n = 0 ; n < pts ; n++ )
		{
			float a = n * (M_TWO_PI / pts);
			float xx = v.x + rx * sin(a);
			float yy = v.y + ry * cos(a);
			this->addVertex( Vec3f( xx, yy, v.z) );
		}
		this->close();
		//mBounds = Rectf( x-rx, y-ry, x+rx, y+ry );
		//mPerimeter = 2.0f * M_PI * sqrt( (rx*rx + ry*ry) / 2.0f );	// http://www.mathsisfun.com/geometry/ellipse-perimeter.html
		this->finish();
	}
	void qbPolyOld::makeCurve ( Vec3f p0, Vec3f p1, Vec3f c0, Vec3f c1 )
	{
		bClosed = false;
		mType = QBPOLY_POLYGON;
		mVertices.empty();
		int pts = 2 + ( p0.distance( p1 ) / 21.0f );
		for (int n = 0 ; n < pts ; n++)
		{
			float t =  n / (float)(pts-1);
			Vec3f v = bezierInterpRef<Vec3f,float>( p0, c0, c1, p1, t );
			this->addVertex( v );
			//printf("  bezier %d / %.2f = %.1f %.1f %.1f\n",n,t,v.x,v.y,v.z);
		}
	}
	
	//
	// FINISHING A POLY
	void qbPolyOld::finish()
	{
		this->calcBounds();
		this->calcPerimeter();
		// Make Cinder Mesh
		this->makeMesh();
	}
	
	// Optimize will 
	void qbPolyOld::finishOptimized()
	{
		this->calcBounds();
		
		// Set TYPE
		if ( mVertices.size() == 0 )
		{
			mType = QBPOLY_NONE;
		}
		else if ( mVertices.size() == 1 )
		{
			mType = QBPOLY_POINT;
		}
		else if ( mVertices.size() == 2 )
		{
			mType = QBPOLY_POLYLINE;
		}
		else
		{
			if ( ! bClosed )
			{
				mType = QBPOLY_POLYLINE;
			}
			else
			{
				if ( mVertices.size() == 3 )
				{
					mType = QBPOLY_TRI;
				}
				else if (mVertices.size() == 4)
				{
					// order vertices
					// let's try to find a vertex for each corner
					// 0 > 1
					//   / 
					// 2 > 3
					float dist = 0.0;
					int order_i[4];
					Vec3f order_v[4];
					for (int o = 0 ; o < 4 ; o++)
					{
						// get corner vertex
						// TODO:: incluir isto em Rectf como mBounds[o]
						Vec2f p = ( o==0 ? mBounds.getUpperLeft() : 
								   ( o==1 ? mBounds.getUpperRight() : 
									( o==2 ? mBounds.getLowerLeft() : mBounds.getLowerRight() ) ) );
						// find closer vertex from poly
						for (int v = 0 ; v < 4 ; v++)
						{
							float d = p.distance( mVertices[v].xy() );
							if ( v == 0 || d < dist )
							{
								dist = d;
								order_i[o] = v;
								order_v[o] = mVertices[v];
							}
						}
					}
					// ordering does not give us a vertex per corner: make polygon
					if ( order_i[0] == order_i[1] || 
						order_i[0] == order_i[2] || 
						order_i[0] == order_i[3] || 
						order_i[1] == order_i[2] || 
						order_i[1] == order_i[3] || 
						order_i[2] == order_i[3] )
					{
						mType = QBPOLY_POLYGON;
					}
					// perfect square/rectangle
					else if ( mVertices[0].x == mVertices[1].x && 
							 mVertices[2].x == mVertices[3].x && 
							 mVertices[0].y == mVertices[2].y && 
							 mVertices[1].y == mVertices[3].y )
					{
						mType = ( mBounds.getWidth() == mBounds.getHeight() ? QBPOLY_SQUARE : QBPOLY_RECT );
					}
					// optimized quad for qb::drawQuad()
					else
					{
						// save reorder
						for (int v = 0 ; v < 4 ; v++)
							mVertices[v] = order_v[v];
						mType = QBPOLY_QUAD;
					}
				}
				else
				{
					mType = QBPOLY_POLYGON;
				}
			}
		}
		
		this->calcPerimeter();
		
		// Make Cinder Mesh
		this->makeMesh();
	}
	
	void qbPolyOld::calcBounds()
	{
		float dMin, dMax;
		for (int v = 0 ; v < mVertices.size() ; v++)
		{
			if ( v == 0 )
			{
				mBounds = Rectf( mVertices[v].xy(), mVertices[v].xy() );
				dMin = dMax = mVertices[v].z;
			}
			else
			{
				if (mVertices[v].x < mBounds.x1)
					mBounds.x1 = mVertices[v].x;
				if (mVertices[v].x > mBounds.x2)
					mBounds.x2 = mVertices[v].x;
				if (mVertices[v].y < mBounds.y1)
					mBounds.y1 = mVertices[v].y;
				if (mVertices[v].y > mBounds.y2)
					mBounds.y2 = mVertices[v].y;
				if (mVertices[v].z < dMin)
					dMin = mVertices[v].z;
				if (mVertices[v].z > dMax)
					dMax = mVertices[v].z;
			}
		}
		mCenter = Vec3f( mBounds.getUpperLeft() + mBounds.getSize() * 0.5f, (dMax-dMin) * 0.5f );
	}
	
	//
	// Make Noise
	void qbPolyOld::randomize( int _max )
	{
		std::vector<qbPolyVertex> newVertices;
		for (int n = 0 ; n < mVertices.size() ; n++)
		{
			Vec3f off = Vec3f( Rand::randFloat( -_max, _max ), Rand::randFloat( -_max, _max ), 0 );
			newVertices.push_back( qbPolyVertex( mVertices[n] + off ) );
		}
		mVertices = newVertices;
		this->finishOptimized();
	}
	
	
	
	
	//
	// Coinder Mesh
	void qbPolyOld::makeMesh()
	{
		Path2d path;
		for (int v = 0 ; v < mVertices.size() ; v++)
		{
			if (v == 0)
				path.moveTo( mVertices[v].xy() );
			else
				path.lineTo( mVertices[v].xy() );
		}
		if (bClosed)
			path.close();
		
		TriMesh2d mesh = Triangulator( path ).calcMesh();
		mMesh = gl::VboMesh( mesh );
	}
	
	
	
	
	
	
	
	
} } // cinder::qb


