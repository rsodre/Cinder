/*
 Copyright (c) 2010, The Barbarian Group
 All rights reserved.

 Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "cinder/Cinder.h"
#include "cinder/Utilities.h"
#include "cinder/Unicode.h"
#include "cinder/app/Platform.h"

#if defined( CINDER_COCOA )
	#include "cinder/cocoa/CinderCocoa.h"
#endif

#include <vector>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

using std::vector;
using std::string;
using std::wstring;

namespace cinder {

fs::path expandPath( const fs::path &path )
{
	return app::Platform::get()->expandPath( path );
}

fs::path getHomeDirectory()
{
	return app::Platform::get()->getHomeDirectory();
}

fs::path getDocumentsDirectory()
{
	return app::Platform::get()->getDocumentsDirectory();
}

void launchWebBrowser( const Url &url )
{
	app::Platform::get()->launchWebBrowser( url );
}

	// ROGER
	void launchDefaultApplication( const std::string &path )
	{
		[[NSWorkspace sharedWorkspace] openFile:[NSString stringWithCString:path.c_str() encoding:NSUTF8StringEncoding]];
	}

std::map<std::string, std::string> getEnvironmentVariables()
{
	return app::Platform::get()->getEnvironmentVariables();
}

std::vector<std::string> split( const std::string &str, char separator, bool compress )
{
	return split( str, string( 1, separator ), compress );
}

std::vector<std::string> split( const std::string &str, const std::string &separators, bool compress )
{
	vector<string> result;

	boost::algorithm::split( result, str, boost::is_any_of(separators), 
		compress ? boost::token_compress_on : boost::token_compress_off );

	return result;
}

string loadString( const DataSourceRef &dataSource )
{
	Buffer buffer( dataSource );
	const char *data = static_cast<const char *>( buffer.getData() );

	return string( data, data + buffer.getSize() );
}

	// ROGER
	void isoToUtf8(unsigned char *in, unsigned char *out)
	{
		while (*in)
			if (*in<128)
				*out++=*in++;
			else
				*out++=0xc2+(*in>0xbf), *out++=(*in++&0x3f)+0x80;
		*out = 0;
	}
	
	// ROGER
	std::string toString(unsigned char ch)
	{
		unsigned char iso[2] = { ch, 0 };
		unsigned char utf[4];	// double size
		isoToUtf8( iso, utf );
		std::string str( (char*) utf );
		return str;
	}
	
	// ROGER
	std::string toTime(const float v)
	{
		int h = (int)( v / 3600.0f );
		int m = (int)( (v - (h * 3600.0f)) / 60.0f );
		int s = (int)( v - (h * 3600.0f) - (m * 60.0f) );
		std::ostringstream ss;
		if (h)
			ss << h << ":" << std::setfill('0') << std::setw(2) << m << ":" << std::setfill('0') << std::setw(2) << s;
		else
			ss << m << ":" << std::setfill('0') << std::setw(2) << s;
		return ss.str();
	}
	std::string toTimecode(const float v, const float framerate)
	{
		int f = (int)roundf( (v-(int)v) * framerate );// + 1;
		std::ostringstream ss;
		ss << toTime(v);
		ss << "." << std::setfill('0') << std::setw(2) << f;
		return ss.str();
	}
	
	// ROGER
	unsigned char hex2byte(const char* hex)
	{
		unsigned short byte = 0;
		std::istringstream iss((char*)hex);
		iss >> std::hex >> byte;
		return byte % 0x100;
	}
	void hex(const unsigned char * from, unsigned char * to, int bytes)
	{
		std::stringstream x;
		for (int i = 0 ; i < bytes ; i++)
			x << std::setw(2) << std::setfill('0') << std::hex << (int)from[i];
		memcpy( to, x.str().c_str(), bytes*2 );
	}
	void unhex(const unsigned char * from, unsigned char * to, int bytes)
	{
		unsigned char c[2];
		for (int i = 0 ; i < bytes ; i++)
		{
			c[0] = from[i*2+0];
			c[1] = from[i*2+1];
			to[i] = hex2byte( (char*)c );
		}
	}
	// ROGER
	std::string ltrim( const std::string & s ) {
		std::string t = s;
        t.erase(t.begin(), std::find_if(t.begin(), t.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return t;
	}
	std::string rtrim( const std::string & s ) {
		std::string t = s;
        t.erase(std::find_if(t.rbegin(), t.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), t.end());
        return t;
	}
	std::string trim( const std::string & s ) {
        return ltrim(rtrim(s));
	}
	// ROGER
	std::string toLower( const std::string & s )
	{
		std::string low = s;
		boost::algorithm::to_lower( low );
		return low;
	}
	std::string toUpper( const std::string & s )
	{
		std::string up = s;
		boost::algorithm::to_upper( up );
		return up;
	}
	// ROGER
	float clampRadians( float a )
	{
		while (a >= M_TWO_PI)
			a -= M_TWO_PI;
		while (a < 0.0)
			a += M_TWO_PI;
		return a;
	}
	Vec2f clampRadians( Vec2f a )
	{
		return Vec2f( clampRadians(a.x), clampRadians(a.y) );
	}
	Vec3f clampRadians( Vec3f a )
	{
		return Vec3f( clampRadians(a.x), clampRadians(a.y), clampRadians(a.z) );
	}
	float clampDegrees( float a )
	{
		while (a >= 360.0)
			a -= 360.0;
		while (a < 0.0)
			a += 360.0;
		return a;
	}
	Vec2f clampDegrees( Vec2f a )
	{
		return Vec2f( clampDegrees(a.x), clampDegrees( a.y) );
	}
	Vec3f clampDegrees( Vec3f a )
	{
		return Vec3f( clampDegrees(a.x), clampDegrees(a.y), clampDegrees(a.z) );
	}
	// !!!TESTAR!!!
	int compareAnglesDegrees( float a, float b )
	{
		a = clampDegrees(a);
		b = clampDegrees(b);
		if (a > b)
			return ( a - b < 180.0 ? 1 : -1 );
		else if (b > a)
			return ( b - a > 180.0 ? 1 : -1 );
		else
			return 0;
	}
	// !!!TESTAR!!!
	int compareAnglesRadians( float a, float b )
	{
		a = clampRadians(a);
		b = clampRadians(b);
		if (a > b)
			return ( a - b < M_PI ? 1 : -1 );
		else if (b > a)
			return ( b - a > M_PI ? 1 : -1 );
		else
			return 0;
	}
	// !!!TESTAR!!!
	float diffAnglesDegrees( float a, float b )
	{
		float d = clampDegrees(a) - clampDegrees(b);
		if ( d > 180.0)
			d -= 360.0;
		else if ( d <= -180.0)
			d += 360.0;
		return d;
	}
	// !!!TESTAR!!!
	float diffAnglesRadians( float a, float b )
	{
		float d = clampRadians(a) - clampRadians(b);
		if ( d > M_PI)
			d -= M_TWO_PI;
		else if ( d <= -M_PI)
			d += M_TWO_PI;
		return d;
	}
	
	
	
void sleep( float milliseconds )
{
	app::Platform::get()->sleep( milliseconds );
}

vector<string> stackTrace()
{
	return app::Platform::get()->stackTrace();
}

int16_t swapEndian( int16_t val ) { 
	return (int16_t) (	(((uint16_t) (val) & (uint16_t) 0x00ffU) << 8) | 
						(((uint16_t) (val) & (uint16_t) 0xff00U) >> 8) );
}

uint16_t swapEndian( uint16_t val ) { 
	return (uint16_t) (	(((uint16_t) (val) & (uint16_t) 0x00ffU) << 8) | 
						(((uint16_t) (val) & (uint16_t) 0xff00U) >> 8) );
}

int32_t swapEndian( int32_t val ) { 
	return (int32_t)((((uint32_t) (val) & (uint32_t) 0x000000FFU) << 24) |
					 (((uint32_t) (val) & (uint32_t) 0x0000FF00U) <<  8) |
					 (((uint32_t) (val) & (uint32_t) 0x00FF0000U) >>  8) |
					 (((uint32_t) (val) & (uint32_t) 0xFF000000U) >> 24));
}

uint32_t swapEndian( uint32_t val ) { 
	return (uint32_t)((((uint32_t) (val) & (uint32_t) 0x000000FFU) << 24) |
					 (((uint32_t) (val) & (uint32_t) 0x0000FF00U) <<  8) |
					 (((uint32_t) (val) & (uint32_t) 0x00FF0000U) >>  8) |
					 (((uint32_t) (val) & (uint32_t) 0xFF000000U) >> 24));
}

float swapEndian( float val ) { 
	uint32_t temp = swapEndian( * reinterpret_cast<uint32_t*>( &val ) );
	return *(reinterpret_cast<float*>( &temp ) );
}

double swapEndian( double val ) {
	union {
		double d;
		struct {  
			uint32_t a;
			uint32_t b;
		} i;
	} s1, s2;
	s1.d = val;
	s2.i.a = swapEndian( s1.i.b );
	s2.i.b = swapEndian( s1.i.a	);
	return s2.d;
}

void swapEndianBlock( uint16_t *blockPtr, size_t blockSizeInBytes )
{
	size_t blockSize = blockSizeInBytes / sizeof(uint16_t);

	for( size_t b = 0; b < blockSize; b++ ) {
		*blockPtr = swapEndian( *blockPtr );
		blockPtr++;
	}
}

void swapEndianBlock( float *blockPtr, size_t blockSizeInBytes )
{
	size_t blockSize = blockSizeInBytes / sizeof(float);

	for( size_t b = 0; b < blockSize; b++ ) {
		*(reinterpret_cast<uint32_t*>(blockPtr)) = swapEndian( *(reinterpret_cast<uint32_t*>(blockPtr)) );
		blockPtr++;
	}
}

} // namespace cinder

