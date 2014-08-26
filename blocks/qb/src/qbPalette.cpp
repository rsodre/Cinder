//
//  qbPalette.h
//
//  Created by Roger Sodre on 01/07/2011
//  Copyright 2011 Studio Avante. All rights reserved.
//

#include "qbPalette.h"
#include "qb.h"

#ifdef QB_COLOR_REDUCTION
#include "CinderOpenCV.h"
#endif

#define SAMPLES_COUNT	8


namespace cinder { namespace qb {
	
	
	//////////////////////////////////////////////////////////
	//
	// QB SOURCE
	//
	qbSourcePalette::qbSourcePalette()
	{
		mName = "qbSourcePalette";
		mDesc = "Generator: ? x ?";
	}
	qbSourcePalette::~qbSourcePalette()
	{
	}
	
	// virtual
	bool qbSourcePalette::updateFrame( bool _force )
	{
		if ( ! bPlaying && ! _force )
			return false;
		
		// update data
		_palette.update();
		mSurface = _palette.mSourceSurface;
		mTex = _palette.mSourceTex;
		mSize = _palette.getSize();
		this->makeUV( mSize.x, mSize.y );
		
		// make desc
		std::stringstream os;
		os << "qbPalette: " << (int)mSize.x << " x " << (int)mSize.y;
		mDesc = os.str();
		return true;
	}
	
	
	
	
	
	//////////////////////////////////////////////////////////
	//
	// PALETTE GENERATOR
	//
	void qbPalette::update()
	{
		// refresh once per frame
		if ( mUpdateFrame == app::getElapsedFrames() )
			return;
		mUpdateFrame = app::getElapsedFrames();

		// Update the palette
		this->updatePalette();
	}
	
	//
	// Update the actual palette
	void qbPalette::updatePalette()
	{
		// new!
		if ( ! mSourceSurface )
		{
			mOctaves = _cfg.getInt(QBCFG_PERLIN_OCTAVE);
			mPerlin = Perlin( mOctaves, mSeed );
			mSourceSize = Vec2i( 80, 60 );
			mSourceSurface = Surface8u ( mSourceSize.x, mSourceSize.y, true ); // width, height, alpha?
			mLastTime = -1.0f;
		}
		
		// already updated?
		mTimeSpan = (QB_ANIM_FRAMES * _cfg.get(QBCFG_PERLIN_SPEED));
		mTime = mTimeSpan * QB_ANIM_PROG;
		//if ( mTime == mLastTime )
		//	return;
		mLastTime = mTime;
		
		// Paused
		if (mTimeSpan == 0.0)
			mTimeSpan = 0.0001;

		// Perlin data
		mFrequency = (1 / 200.0f) * pow(2.0f, 6.0f);
		mFrequency /= pow(2.0f, _cfg.get(QBCFG_PERLIN_FREQ));

		//
		// Render Perlin
		Surface8u::Iter iter = mSourceSurface.getIter();
		while( iter.line() ) {
			while( iter.pixel() ) {
				//Vec3f pp = Vec3f( iter.x(), iter.y(), mTime ) * mFrequency;
				Vec2f pp = Vec2f( iter.x(), iter.y() );
				pp /= mSourceSize;
				pp += _cfg.getVector2( QBCFG_PERLIN_OFFSET );
				pp *= _cfg.getVector2( QBCFG_PERLIN_SCALE );
				pp *= mSourceSize;
				Vec3f ppp = Vec3f( pp, mTime ) * mFrequency;
				
				// get perlin value
				float p = mPerlin.fBmLoop( ppp.x, ppp.y, ppp.z, mTimeSpan * mFrequency );
				if (p < mMin) mMin = p;	// statistics
				if (p > mMax) mMax = p;	// statistics
				float r = 0.3f * (1.0 - _cfg.get(QBCFG_PERLIN_COMPRESS));
				float v = math<float>::clamp( lmap(p, -r, r, 0.0f, 1.0f) );
				
				// get color
				if ( ! _cfg.getBool(QBCFG_PERLIN_GRAYSCALE) )
				{
					Color c = this->getColorFromPalette(v);
					iter.r() = (uint8_t) (c.r * 255);
					iter.g() = (uint8_t) (c.g * 255);
					iter.b() = (uint8_t) (c.b * 255);
				}
				else
					iter.r() = iter.g() = iter.b() = (uint8_t) (v * 255);
				iter.a() = (uint8_t) (v * 255);

				//if ( iter.x() == 0 && iter.y() == 0 ) printf("perlin %.3f  val %.3f  range %.3f .. %.3f\n",p,v,mMin,mMax);
			}
		}
		if ( ! mSourceTex )
		{
			mSourceTex = gl::Texture( mSourceSurface );
			_cfg.mTexturePalette = mSourceTex;
		}
		else
			mSourceTex.update( mSourceSurface );
		_cfg.controlPaletteTexture->refresh();
		//printf("PALETTE PERLIN UPDATE  %d\n",app::getElapsedFrames());
	}
	
	Color qbPalette::getColorFromPalette( float v )
	{
		Color c1, c2;
		float l1 = 0.0;
		float l2 = 0.0;
		int colorCount = _cfg.getInt(QBCFG_PALETTE_COUNT);
		// 2 values
		if ( colorCount == 2 )
		{
			c1 = _cfg.getColor(QBCFG_PALETTE_1);
			c2 = _cfg.getColor(QBCFG_PALETTE_2);
			l1 = 0.0f;
			l2 = 1.0f;
		}
		// 3 values
		else if ( colorCount == 3 )
		{
			if ( v < 0.5 )
			{
				c1 = _cfg.getColor(QBCFG_PALETTE_1);
				c2 = _cfg.getColor(QBCFG_PALETTE_2);
				l1 = 0.0f;
				l2 = 0.5f;
			}
			else
			{
				c1 = _cfg.getColor(QBCFG_PALETTE_2);
				c2 = _cfg.getColor(QBCFG_PALETTE_3);
				l1 = 0.5f;
				l2 = 1.0f;
			}
		}
		// 4 values
		else if ( colorCount == 4 )
		{
			if ( v < 0.3333 )
			{
				c1 = _cfg.getColor(QBCFG_PALETTE_1);
				c2 = _cfg.getColor(QBCFG_PALETTE_2);
				l1 = 0.0f;
				l2 = 0.3333f;
			}
			else if ( v < 0.6666 )
			{
				c1 = _cfg.getColor(QBCFG_PALETTE_2);
				c2 = _cfg.getColor(QBCFG_PALETTE_3);
				l1 = 0.3333f;
				l2 = 0.6666f;
			}
			else
			{
				c1 = _cfg.getColor(QBCFG_PALETTE_3);
				c2 = _cfg.getColor(QBCFG_PALETTE_4);
				l1 = 0.6666f;
				l2 = 1.0f;
			}
		}
		// 5 values
		else if ( colorCount == 5 )
		{
			if ( v < 0.25 )
			{
				c1 = _cfg.getColor(QBCFG_PALETTE_1);
				c2 = _cfg.getColor(QBCFG_PALETTE_2);
				l1 = 0.0f;
				l2 = 0.25f;
			}
			else if ( v < 0.5 )
			{
				c1 = _cfg.getColor(QBCFG_PALETTE_2);
				c2 = _cfg.getColor(QBCFG_PALETTE_3);
				l1 = 0.25f;
				l2 = 0.5f;
			}
			else if ( v < 0.75 )
			{
				c1 = _cfg.getColor(QBCFG_PALETTE_3);
				c2 = _cfg.getColor(QBCFG_PALETTE_4);
				l1 = 0.5f;
				l2 = 0.75f;
			}
			else
			{
				c1 = _cfg.getColor(QBCFG_PALETTE_4);
				c2 = _cfg.getColor(QBCFG_PALETTE_5);
				l1 = 0.75f;
				l2 = 1.0f;
			}
		}
		// get val
		float m = l1 + ((l2-l1) * 0.5);
		l1 = lerp( l1, m, _cfg.get(QBCFG_PALETTE_COMPRESS) );
		l2 = lerp( l2, m, _cfg.get(QBCFG_PALETTE_COMPRESS) );
		float v2 = math<float>::clamp( lmap( v, l1, l2, 0.0f, 1.0f ) );
		float v1 = 1.0 - v2;
		return Color( c1.r*v1 + c2.r*v2, c1.g*v1 + c2.g*v2, c1.b*v1 + c2.b*v2 );
	}
	
	float qbPalette::getPerlinProg( float _px, float _py )
	{
		ColorA c = ColorA::zero();
		if ( mSourceSurface )
		{
			int x = (int) (_px * mSourceSize.x);
			int y = (int) (_py * mSourceSize.y);
			c = mSourceSurface.getPixel( Vec2i(x,y) );
		}
		return c.a;
	}

	

	
	/////////////////////////////////////
	//
	// OPENCV REDUCE COLORS
	//
	void qbPalette::reduce( std::string src, int colorCount )
	{
		mInputImage = Surface( loadImage( src ) );
		this->reduce( colorCount );
	}
	void qbPalette::reduce( gl::Texture src, int colorCount )
	{
		if ( ! src )
			return;
		if ( src.getWidth() == 0 || src.getHeight() == 0 )
			return;
		// Issue 182 workaround
		// https://github.com/cinder/Cinder/issues/182
		if ( false )
		{
			mInputImage = Surface( src );
		}
		else
		{
			if (  mSurfFbo )
				if ( src.getWidth() != mSurfFbo.getWidth() || src.getHeight() != mSurfFbo.getHeight() )
					mSurfFbo = gl::Fbo();
			if ( ! mSurfFbo )
				mSurfFbo = gl::Fbo( src.getWidth(), src.getHeight() );
			mSurfFbo.bindFramebuffer();
			gl::setViewport( mSurfFbo.getBounds() );
			gl::setMatricesWindow( mSurfFbo.getSize() );
			gl::draw( src, mSurfFbo.getBounds() );
			mSurfFbo.unbindFramebuffer();
			//mInputImage = Surface( mSurfFbo.getTexture() );
			mSurfFromFbo = Surface( mSurfFbo.getTexture() );			// save fbo
			mInputImage = Surface( src.getWidth(), src.getHeight(), true, SurfaceChannelOrder::RGBA );	// make empty
			/*
			 mInputImage = Surface( src.getData(), src.getWidth(), src.getHeight(), 4, SurfaceChannelOrder::RGBA );
			 */
		}
		this->reduce( colorCount );
	}
	void qbPalette::reduce( int colorCount )
	{
#ifdef QB_COLOR_REDUCTION
		double startTime = app::getElapsedSeconds();
		
		// no image!
		if ( ! mInputImage )
			return;
		// no colors!
		if ( colorCount <= 0 || colorCount > QB_PALETTE_COUNT )
			colorCount = QB_PALETTE_COUNT;
		
		//
		// reduce size
		int szMax = 100;
		cv::Size sz( mInputImage.getWidth(), mInputImage.getHeight() );
		if ( sz.width < sz.height && sz.width > szMax )
		{
			float scale = szMax / (float) sz.width;
			sz = cv::Size( sz.width * scale, sz.height * scale );
		}
		else if ( sz.height < sz.width && sz.height > szMax )
		{
			float scale = szMax / (float) sz.height;
			sz = cv::Size( sz.width * scale, sz.height * scale );
		}
		
		//
		// blur
		cv::Mat input, output;
		if ( mSurfFromFbo )
			cv::resize( toOcv( mSurfFromFbo ), input, sz );
		else
			cv::resize( toOcv( mInputImage ), input, sz );
		//cv::medianBlur( input, output, 11 );
		//mInputImage = fromOcv( output );
		mInputImage = fromOcv( input );
		
		//
		// get samples
		const int sampleSize = mInputImage.getHeight() * mInputImage.getWidth();
		cv::Mat colorSamples( sampleSize, 1, CV_32FC3 );
		Surface::ConstIter imageIt = mInputImage.getIter();
		cv::MatIterator_<cv::Vec3f> sampleIt = colorSamples.begin<cv::Vec3f>();
		while( imageIt.line() )
			while( imageIt.pixel() )
				*sampleIt++ = cv::Vec3f( imageIt.r(), imageIt.g(), imageIt.b() );
		
		//
		// reduce with kmeans
		// cv::KMEANS_RANDOM_CENTERS || cv::KMEANS_PP_CENTERS
		cv::Mat labels, clusters;
		cv::kmeans( colorSamples, SAMPLES_COUNT, labels, cv::TermCriteria( cv::TermCriteria::COUNT, 8, 0 ), 2, cv::KMEANS_PP_CENTERS, clusters );
		
		//
		// Quantitize
		//printf(">>> MAT  rows %d  cols %d  ch %d  total %d\n",(int)mat.rows,(int)mat.cols,(int)mat.channels(),(int)mat.total());
		int quant[SAMPLES_COUNT];
		memset(quant,0,sizeof(quant));
		for (int i = 0 ; i < labels.total() ; i++)
			quant[labels.at<int>(i)]++;
		
		//
		// order samples by quantity
		std::multimap<int,Color> smap;
		for( int i = 0; i < SAMPLES_COUNT ; ++i )
		{
			Color c = Color( Color8u( clusters.at<cv::Vec3f>(i,0)[0], clusters.at<cv::Vec3f>(i,0)[1], clusters.at<cv::Vec3f>(i,0)[2] ) );
			if ( quant[i] )
				smap.insert( std::pair<int,Color>( quant[i], c ) );
		}
		
		//
		// save all samples (ordered by multimap)
		//printf("SAMPLES-----------------------\n");
		mSamples.clear();
		std::multimap<int,Color>::reverse_iterator it;
		for ( it=smap.rbegin() ; it != smap.rend(); it++ )
		{
			Color c = (*it).second;
			mSamples.push_back( c );
			//printf("SAMPLE  count %d  =  %.1f %.1f %.1f\n",(*it).first,c.r,c.g,c.b);
		}
		
		//
		// save colors within threshold
		mColors.clear();
		float min = _cfg.get(QBCFG_PALETTE_MIN);
		float max = _cfg.get(QBCFG_PALETTE_MAX);
		for( int i = 0; i < mSamples.size() ; ++i )
		{
			Color c = mSamples[i];
			float b = c.brightness();
			if ( b >= min && b <= max )
				mColors.push_back( c );
		}
		
		//
		// select best colors
		for ( int i = 0 ; i < mColors.size() ; ++i )
		{
			for ( int ii = i ; ii < mColors.size() ; ++ii )
			{
				Color c1 = mColors[ii];
				Color c2 = mColors[ii+1];
				Vec3f hsv1 = rgbToHSV(c1);
				Vec3f hsv2 = rgbToHSV(c2);
				//float p1 = (hsv1.y + hsv1.z) * ((float)i/mColors.size());
				//float p2 = (hsv2.y + hsv2.z) * ((float)i/mColors.size());
				float p1 = (hsv1.y + hsv1.z);
				float p2 = (hsv2.y + hsv2.z);
				if ( p1 < p2 )
				{
					Color c = mColors[ii];
					mColors[ii] = mColors[ii+1];
					mColors[ii+1] = c;
				}
			}
		}
		// reduce to desired color count
		while ( mColors.size() > colorCount )
			mColors.pop_back();
		// fill blacks to desired color count
		while ( mColors.size() < colorCount )
			mColors.push_back( Color::black() );
		// order colors by hue
		/*
		 for ( int i = 0 ; i < mColors.size() ; ++i )
		 {
		 for ( int ii = 0 ; ii < mColors.size() ; ++ii )
		 {
		 Color c1 = mColors[ii];
		 Color c2 = mColors[ii+1];
		 if ( rgbToHSV(c1).x > rgbToHSV(c2).x )		// hue
		 {
		 Color c = mColors[ii];
		 mColors[ii] = mColors[ii+1];
		 mColors[ii+1] = c;
		 }
		 }
		 }
		 */
		
		//
		// update GUI
		for (int n = 0 ; n < mColors.size() && n < colorCount ; n++)
			_cfg.set( QBCFG_PALETTE_1+n, mColors[n] );
		
		// make reduced surface
		/*
		 Surface result( mInputImage.getWidth(), mInputImage.getHeight(), false );
		 Surface::Iter resultIt = result.getIter();
		 cv::MatIterator_<int> labelIt = labels.begin<int>();
		 while( resultIt.line() ) {
		 while( resultIt.pixel() ) {
		 resultIt.r() = mColors[*labelIt].r;
		 resultIt.g() = mColors[*labelIt].g;
		 resultIt.b() = mColors[*labelIt].b;
		 ++labelIt;
		 }
		 }
		 mReducedTex = gl::Texture( result );
		 */
		
		// statistics
		double elapsed = (app::getElapsedSeconds()-startTime);
		_cfg.set( QBCFG_PALETTE_REDUCE_TIME, elapsed );
		printf("qbPalette::reduce - Reduce Time %.4f s\n",elapsed);
		
		// Send to network
		// NO NEED! sent by QBCFG_PALETTE_X
		//this->sendOsc();
#else
		printf("!!! qbPalette::reduce ERROR !!! OPenCV not enabled! Declare QB_COLOR_REDUCTION on PREPROCESSOR_MACROS\n");
#endif	// QB_COLOR_REDUCTION
	}
	
	void qbPalette::drawReduced( Rectf bounds )
	{
		if ( mReducedTex )
			gl::draw( mReducedTex, bounds );
	}
	
	
	
	
	///////////////////////////////////////////////
	//
	// OSC SEND/RECEIVE
	//
	// Format:
	//	/qb/palette  colorNum R G B
	//
	void qbPalette::setupOsc()
	{
		mOscSender.shutdown();
		mOscListener.shutdown();
		// start new
		std::string host = _cfg.getString(QBCFG_PALETTE_NET_HOST);
		int port = _cfg.getInt(QBCFG_PALETTE_NET_PORT);
		switch( _cfg.getInt(QBCFG_PALETTE_NET_STATE) )
		{
			case PALETTE_NET_SEND:
				printf("qbPalette::setupOsc() - Network Send  host [%s]  port [%d]\n",host.c_str(),port);
				mOscSender.setup( host, port );
				this->sendOsc();
				break;
			case PALETTE_NET_RECEIVE:
				printf("qbPalette::setupOsc() - Network Receive  port [%d]\n",port);
				mOscListener.setup( port );
				this->receiveOsc();
				break;
			default:
				printf("qbPalette::setupOsc() - Network OFF\n");
				break;
		}
	}
	bool qbPalette::sendOsc( int colorNum )
	{
		if ( _cfg.getInt(QBCFG_PALETTE_NET_STATE) == PALETTE_NET_SEND )
		{
			osc::Message message;
			for (int n = 0 ; n < QB_PALETTE_COUNT ; n++)
			{
				if ( colorNum == n || colorNum == -1 )
				{
					message.clear();
					message.setAddress("/qb/palette");
					//message.setRemoteEndpoint(host, port);
					message.addIntArg( n );
					Color8u c = _cfg.getColor(QBCFG_PALETTE_1+n);
					message.addIntArg( c.r );
					message.addIntArg( c.g );
					message.addIntArg( c.b );
					// Send!
					mOscSender.sendMessage(message);
					printf(">> OSC PALETTE SEND :: frame %d :: addr [%s]  color %d  rgb %d %d %d\n",app::getElapsedFrames(),message.getAddress().c_str(),message.getArgAsInt32(0),message.getArgAsInt32(1),message.getArgAsInt32(2),message.getArgAsInt32(3));
					return true;
				}
			}
		}
		return false;
	}
	bool qbPalette::receiveOsc()
	{
		// Receive from network
		if ( _cfg.getInt(QBCFG_PALETTE_NET_STATE) == PALETTE_NET_RECEIVE )
		{
			while (mOscListener.hasWaitingMessages())
			{
				osc::Message message;
				mOscListener.getNextMessage(&message);
				if ( message.getAddress() == "/qb/palette" && message.getNumArgs() == 4 )
				{
					int colorNum = message.getArgAsInt32(0);
					int r = message.getArgAsInt32(1);
					int g = message.getArgAsInt32(2);
					int b = message.getArgAsInt32(3);
					_cfg.set( QBCFG_PALETTE_1+colorNum, Color8u(r,g,b) );
					printf(">> OSC PALETTE RECEIVE :: frame %d :: addr [%s]  color %d  rgb %d %d %d\n",app::getElapsedFrames(),message.getAddress().c_str(),colorNum,r,g,b);
					return true;
				}
			}
		}
		return false;
	}

} } // cinder::qb




