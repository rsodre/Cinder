/*
 Copyright (c) 2010, The Cinder Project, All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Portions Copyright (c) 2010, The Barbarian Group
 All rights reserved.

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

#pragma once

#include "cinder/Cinder.h"
#include "cinder/ChanTraits.h"
#include "cinder/Vector.h"
#include "cinder/CinderMath.h"

namespace cinder {

typedef	enum {
	CM_RGB, // Red[0 - 1.0] Green[0 - 1.0] Blue[0 - 1.0]
	CM_HSV  // Hue[0 - 1.0] Saturation[0 - 1.0] Value[0 - 1.0]
} ColorModel;

template<typename T>
class ColorT 
{
 public:
	T r,g,b;

	ColorT() : r( 0 ), g( 0 ), b( 0 ) {}
	ColorT( T aR, T aG, T aB ) 
		: r( aR ), g( aG ), b( aB )
	{}
	ColorT( const ColorT<T> &src ) 
		: r( src.r ), g( src.g ), b( src.b )
	{}

	ColorT( ColorModel cm, const Vec3f &v );
	ColorT( ColorModel cm, float x, float y, float z );

	template<typename FromT>
	ColorT( const ColorT<FromT> &src ) 
		: r( CHANTRAIT<T>::convert( src.r ) ), g( CHANTRAIT<T>::convert( src.g ) ), b( CHANTRAIT<T>::convert( src.b ) ) 
	{}
	
	void set( T ar, T ag, T ab )
	{
		r = ar; g = ag; b = ab;
	}
	
	void set( const ColorT<T> &rhs )
	{
		r = rhs.r; g = rhs.g; b = rhs.b;
	}

	void	set( ColorModel cm, const Vec3f &v );

	ColorT<T> operator=( const ColorT<T> &rhs ) 
	{
		r = rhs.r;
		g = rhs.g;
		b = rhs.b;
		return * this;
	}

	template<class FromT>
	ColorT<T> operator=( const ColorT<FromT> &rhs )
	{
		r = CHANTRAIT<T>::convert( rhs.r );
		g = CHANTRAIT<T>::convert( rhs.g );
		b = CHANTRAIT<T>::convert( rhs.b );
		return * this;
	}

	Vec3f get( ColorModel cm ) const;

	T& operator[]( int n )
	{
		assert( n >= 0 && n <= 2 );
		return (&r)[n];
	}

	const T& operator[]( int n ) const
	{
		assert( n >= 0 && n <= 2 );
		return (&r)[n];
	}

	T*  ptr() const { return &(const_cast<ColorT*>( this )->r); }

	ColorT<T>		operator+( const ColorT<T> &rhs ) const { return ColorT<T>( r + rhs.r, g + rhs.g, b + rhs.b ); }
	ColorT<T>		operator-( const ColorT<T> &rhs ) const { return ColorT<T>( r - rhs.r, g - rhs.g, b - rhs.b ); }
	ColorT<T>		operator*( const ColorT<T> &rhs ) const { return ColorT<T>( r * rhs.r, g * rhs.g, b * rhs.b ); }
	ColorT<T>		operator/( const ColorT<T> &rhs ) const { return ColorT<T>( r / rhs.r, g / rhs.g, b / rhs.b ); }
	const ColorT<T>&	operator+=( const ColorT<T> &rhs ) { r += rhs.r; g += rhs.g; b += rhs.b; return *this; }
	const ColorT<T>&	operator-=( const ColorT<T> &rhs ) { r -= rhs.r; g -= rhs.g; b -= rhs.b; return *this; }
	const ColorT<T>&	operator*=( const ColorT<T> &rhs ) { r *= rhs.r; g *= rhs.g; b *= rhs.b; return *this; }
	const ColorT<T>&	operator/=( const ColorT<T> &rhs ) { r /= rhs.r; g /= rhs.g; b /= rhs.b; return *this; }
	ColorT<T>		operator+( T rhs ) const { return ColorT<T>( r + rhs, g + rhs, b + rhs ); }
	ColorT<T>		operator-( T rhs ) const { return ColorT<T>( r - rhs, g - rhs, b - rhs ); }
	ColorT<T>		operator*( T rhs ) const { return ColorT<T>( r * rhs, g * rhs, b * rhs ); }
	ColorT<T>		operator/( T rhs ) const { return ColorT<T>( r / rhs, g / rhs, b / rhs ); }
	const ColorT<T>&	operator+=( T rhs ) { r += rhs; g += rhs; b += rhs; return *this; }
	const ColorT<T>&	operator-=( T rhs ) { r -= rhs; g -= rhs; b -= rhs; return *this; }
	const ColorT<T>&	operator*=( T rhs ) { r *= rhs; g *= rhs; b *= rhs; return *this; }
	const ColorT<T>&	operator/=( T rhs ) { r /= rhs; g /= rhs; b /= rhs; return *this; }

	bool operator==( const ColorT<T>& rhs ) const
	{
		return ( r == rhs.r ) && ( g == rhs.g ) && ( b == rhs.b );
	}

	bool operator!=( const ColorT<T>& rhs ) const
	{
		return ! ( *this == rhs );
	}

	typename CHANTRAIT<T>::Accum dot( const ColorT<T> &rhs ) const
	{
		return r*rhs.r + g*rhs.g + b*rhs.b;
	}

	float distance( const ColorT<T> &rhs ) const
	{
		return math<float>::sqrt( static_cast<float>( (r - rhs.r)*(r - rhs.r) + (g - rhs.g)*(g - rhs.g) + (b - rhs.b)*(b - rhs.b)) );
	}

	typename CHANTRAIT<T>::Accum distanceSquared( const ColorT<T> &rhs ) const
	{
		return (r - rhs.r) * (r - rhs.r) + (g - rhs.g) * (g - rhs.g) + (b - rhs.b) * (b - rhs.b);
	}

	float length() const 
	{
		return math<float>::sqrt( static_cast<float>( r*r + g*g + b*b ) );
	}

	typename CHANTRAIT<T>::Accum lengthSquared() const 
	{
		return r*r + g*g + b*b;
	}

	// tests for zero-length
	void normalize()
	{
		float s = length();
		if( s > 0.0f ) {
			r = static_cast<T>( r / s );
			g = static_cast<T>( g / s );
			b = static_cast<T>( b / s );
		}
	}

	ColorT<T> lerp( float fact, const ColorT<T> &d ) const
	{
		return ColorT<T>( r + ( d.r - r ) * fact, g + ( d.g - g ) * fact, b + ( d.b - b ) * fact );
	}

	static ColorT<T> max()
	{
		return ColorT<T>( std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), std::numeric_limits<T>::max() );
	}

	static ColorT<T> black()
	{
		return ColorT<T>( static_cast<T>( 0 ), static_cast<T>( 0 ), static_cast<T>( 0 ) );
	}

	static ColorT<T> white()
	{
		return ColorT<T>( CHANTRAIT<T>::max(), CHANTRAIT<T>::max(), CHANTRAIT<T>::max() );
	}

	static ColorT<T> gray( T value )
	{
		return ColorT<T>( value, value, value );
	}

	// ROGER
	static ColorT<T> red()
	{
		return ColorT<T>( CHANTRAIT<T>::max(), static_cast<T>( 0 ), static_cast<T>( 0 ) );
	}
	static ColorT<T> green()
	{
		return ColorT<T>( static_cast<T>( 0 ), CHANTRAIT<T>::max(), static_cast<T>( 0 ) );
	}
	static ColorT<T> blue()
	{
		return ColorT<T>( static_cast<T>( 0 ), static_cast<T>( 0 ), CHANTRAIT<T>::max() );
	}
	static ColorT<T> yellow()
	{
		return ColorT<T>( CHANTRAIT<T>::max(), CHANTRAIT<T>::max(), static_cast<T>( 0 ) );
	}
	static ColorT<T> cyan()
	{
		return ColorT<T>( static_cast<T>( 0 ), CHANTRAIT<T>::max(), CHANTRAIT<T>::max() );
	}
	static ColorT<T> magenta()
	{
		return ColorT<T>( CHANTRAIT<T>::max(), static_cast<T>( 0 ), CHANTRAIT<T>::max() );
	}
	static ColorT<T> orange()
	{
		return ColorT<T>( CHANTRAIT<T>::max(), CHANTRAIT<T>::max() * 0.5f, static_cast<T>( 0 ) );
	}
	
	// ROGER
	// color ordering -- does it make sense?
	T sum()
	{
		return r + ( g ? (g+CHANTRAIT<T>::max()) : 0 ) + ( b ? (b+CHANTRAIT<T>::max()*2) : 0 );
	}
	bool operator<( ColorT<T> &rhs )
	{
		return ( this->sum() < rhs.sum() );
	}
	bool operator>( ColorT<T> &rhs )
	{
		return ( this->sum() > rhs.sum() );
	}

	// ROGER
	float brightness()
	{
		return ( (r / (float)CHANTRAIT<T>::max()) + (g / (float)CHANTRAIT<T>::max()) + (b / (float)CHANTRAIT<T>::max()) ) / 3.0f;
	}

	//! Returns a color from a hexadecimal-encoded RGB triple. For example, red is 0xFF0000
	static ColorT<T> hex( uint32_t hexValue )
	{
		uint8_t red = ( hexValue >> 16 ) & 255;
		uint8_t green = ( hexValue >> 8 ) & 255;
		uint8_t blue = hexValue & 255;		
		return ColorT<T>( CHANTRAIT<T>::convert( red ), CHANTRAIT<T>::convert( green ), CHANTRAIT<T>::convert( blue ) );
	}

	operator T*(){ return (T*) this; }
	operator const T*() const { return (const T*) this; }
};


//////////////////////////////////////////////////////////////
// ColorAT

template<typename T>
class ColorAT {
 public: 
	T r,g,b,a;

	ColorAT() 
		: r( 0 ), g( 0 ), b( 0 ), a( 0 )
	{}
	ColorAT( T aR, T aG, T aB, T aA = CHANTRAIT<T>::convert( 1.0f ) )
		: r( aR ), g( aG ), b( aB ), a( aA )
	{}
	ColorAT( const ColorAT<T> &src ) 
		: r( src.r ), g( src.g ), b( src.b ), a( src.a )
	{}
	ColorAT( const ColorT<T> &col, T aA = CHANTRAIT<T>::convert( 1.0f ) )
		: r( col.r ), g( col.g ), b( col.b ), a( aA )
	{}

	ColorAT( ColorModel cm, float c1, float c2, float c3, float aA = CHANTRAIT<T>::convert( 1.0f ) );

	template<typename FromT>
	ColorAT( const ColorT<FromT> &src ) 
		: r( CHANTRAIT<T>::convert( src.r ) ), g( CHANTRAIT<T>::convert( src.g ) ), b( CHANTRAIT<T>::convert( src.b ) ), a( CHANTRAIT<T>::convert( 1.0f ) ) 
	{}


	template<typename FromT>
	ColorAT( const ColorAT<FromT>& src )
		: r( CHANTRAIT<T>::convert( src.r ) ), g( CHANTRAIT<T>::convert( src.g ) ), b( CHANTRAIT<T>::convert( src.b ) ), a( CHANTRAIT<T>::convert( src.a ) )
	{}

	void set( T ar, T ag, T ab , T aa )
	{
		r = ar; g = ag; b = ab; a = aa;
	}
	
	void set( const ColorAT<T> &rhs )
	{
		r = rhs.r; g = rhs.g; b = rhs.b; a = rhs.a;
	}
	
	ColorAT<T> operator=( const ColorAT<T>& rhs ) 
	{
		r = rhs.r;
		g = rhs.g;
		b = rhs.b;
		a = rhs.a;
		return * this;
	}

	template<class FromT>
	ColorAT<T> operator=( const ColorAT<FromT>& rhs ) 
	{
		r = CHANTRAIT<T>::convert( rhs.r );
		g = CHANTRAIT<T>::convert( rhs.g );
		b = CHANTRAIT<T>::convert( rhs.b );
		a = CHANTRAIT<T>::convert( rhs.a );
		return * this;
	}

	T& operator[]( int n )
	{
		assert( n >= 0 && n <= 3 );
		return (&r)[n];
	}

	const T& operator[]( int n ) const
	{
		assert( n >= 0 && n <= 3 );
		return (&r)[n];
	}

	T*  ptr() const { return &(const_cast<ColorAT*>( this )->r); }

	ColorAT<T>	operator+( const ColorAT<T> &rhs ) const { return ColorAT<T>( r + rhs.r, g + rhs.g, b + rhs.b, a + rhs.a ); }
	ColorAT<T>	operator-( const ColorAT<T> &rhs ) const { return ColorAT<T>( r - rhs.r, g - rhs.g, b - rhs.b, a - rhs.a ); }
	ColorAT<T>	operator*( const ColorAT<T> &rhs ) const { return ColorAT<T>( r * rhs.r, g * rhs.g, b * rhs.b, a * rhs.a ); }
	ColorAT<T>	operator/( const ColorAT<T> &rhs ) const { return ColorAT<T>( r / rhs.r, g / rhs.g, b / rhs.b, a / rhs.a ); }
	const ColorAT<T>&	operator+=( const ColorAT<T> &rhs ) { r += rhs.r; g += rhs.g; b += rhs.b; a += rhs.a; return *this; }
	const ColorAT<T>&	operator-=( const ColorAT<T> &rhs ) {	r -= rhs.r;	g -= rhs.g;	b -= rhs.b;	a -= rhs.a;	return *this; }
	const ColorAT<T>&	operator*=( const ColorAT<T> &rhs ) { r *= rhs.r; g *= rhs.g; b *= rhs.b; a *= rhs.a; return *this; }
	const ColorAT<T>&	operator/=( const ColorAT<T> &rhs ) {	r /= rhs.r;	g /= rhs.g;	b /= rhs.b;	a /= rhs.a;	return *this; }
	ColorAT<T>	operator+( T rhs ) const { return ColorAT<T>( r + rhs, g + rhs, b + rhs, a + rhs ); }
	ColorAT<T>	operator-( T rhs ) const { return ColorAT<T>( r - rhs, g - rhs, b - rhs, a - rhs ); }
	ColorAT<T>	operator*( T rhs ) const { return ColorAT<T>( r * rhs, g * rhs, b * rhs, a * rhs ); }
	ColorAT<T>	operator/( T rhs ) const { return ColorAT<T>( r / rhs, g / rhs, b / rhs, a / rhs ); }
	const ColorAT<T>&	operator+=( T rhs ) {	r += rhs; g += rhs; b += rhs; a += rhs; return *this; }
	const ColorAT<T>&	operator-=( T rhs ) {	r -= rhs; g -= rhs; b -= rhs; a -= rhs;	return * this; }
	const ColorAT<T>&	operator*=( T rhs ) { r *= rhs; g *= rhs; b *= rhs; a *= rhs; return * this; }
	const ColorAT<T>&	operator/=( T rhs ) { r /= rhs; g /= rhs; b /= rhs; a /= rhs;	return * this; }

	bool operator==( const ColorAT<T>& rhs ) const
	{
		return ( r == rhs.r ) && ( g == rhs.g ) && ( b == rhs.b ) && ( a == rhs.a );
	}

	bool operator!=( const ColorAT<T>& rhs ) const
	{
	return ! ( *this == rhs );
	}

	float length() const
	{
		return math<float>::sqrt( static_cast<float>( r*r + g*g + b*b ) );
	}

	// tests for zero-length
	void normalize()
	{
		float s = length();
		if( s > 0.0f ) {
			r = static_cast<T>( r / s );
			g = static_cast<T>( g / s );
			b = static_cast<T>( b / s );
		}
	}

	ColorAT<T> premultiplied() const
	{
		return ColorAT<T>( r * a, g * a, b * a, a );
	}

	typename CHANTRAIT<T>::Accum lengthSquared() const
	{
		return r * r + g * g + b * b;
	}

	ColorAT<T> lerp( T fact, const ColorAT<T> &d ) const
	{
		return ColorAT<T>( r + ( d.r - r ) * fact, g + ( d.g - g ) * fact, b + ( d.b - b ) * fact, a + ( d.a - a ) * fact );
	}

	static ColorAT<T> zero()
	{
		return ColorAT<T>( static_cast<T>( 0 ), static_cast<T>( 0 ), static_cast<T>( 0 ), static_cast<T>( 0 ) );
	}

	static ColorAT<T> black()
	{
		return ColorAT<T>( static_cast<T>( 0 ), static_cast<T>( 0 ), static_cast<T>( 0 ), CHANTRAIT<T>::max() );
	}

	static ColorAT<T> white()
	{
		return ColorAT<T>( CHANTRAIT<T>::max(), CHANTRAIT<T>::max(), CHANTRAIT<T>::max(), CHANTRAIT<T>::max() );
	}
	
	// ROGER
	static ColorAT<T> red()
	{
		return ColorAT<T>( CHANTRAIT<T>::max(), static_cast<T>( 0 ), static_cast<T>( 0 ), CHANTRAIT<T>::max() );
	}
	static ColorAT<T> green()
	{
		return ColorAT<T>( static_cast<T>( 0 ), CHANTRAIT<T>::max(), static_cast<T>( 0 ), CHANTRAIT<T>::max() );
	}
	static ColorAT<T> blue()
	{
		return ColorAT<T>( static_cast<T>( 0 ), static_cast<T>( 0 ), CHANTRAIT<T>::max(), CHANTRAIT<T>::max() );
	}
	static ColorAT<T> yellow()
	{
		return ColorAT<T>( CHANTRAIT<T>::max(), CHANTRAIT<T>::max(), static_cast<T>( 0 ), CHANTRAIT<T>::max() );
	}
	static ColorAT<T> cyan()
	{
		return ColorAT<T>( static_cast<T>( 0 ), CHANTRAIT<T>::max(), CHANTRAIT<T>::max(), CHANTRAIT<T>::max() );
	}
	static ColorAT<T> magenta()
	{
		return ColorAT<T>( CHANTRAIT<T>::max(), static_cast<T>( 0 ), CHANTRAIT<T>::max(), CHANTRAIT<T>::max() );
	}
	

	static ColorAT<T> gray( T value, T alpha = CHANTRAIT<T>::max() )
	{
		return ColorAT<T>( value, value, value, alpha );
	}

	//! Returns a ColorA from a hexadecimal-encoded RGB triple. For example, red is 0xFF0000
	static ColorAT<T> hex( uint32_t hexValue )
	{
		uint8_t red = ( hexValue >> 16 ) & 255;
		uint8_t green = ( hexValue >> 8 ) & 255;
		uint8_t blue = hexValue & 255;		
		return ColorAT<T>( CHANTRAIT<T>::convert( red ), CHANTRAIT<T>::convert( green ), CHANTRAIT<T>::convert( blue ), CHANTRAIT<T>::max() );
	}

	//! Returns a ColorA from a hexadecimal-encoded ARGB ordering. For example, 50% transparent red is 0x80FF0000
	static ColorAT<T> hexA( uint32_t hexValue )
	{
		uint8_t alpha = ( hexValue >> 24 ) & 255;;
		uint8_t red = ( hexValue >> 16 ) & 255;
		uint8_t green = ( hexValue >> 8 ) & 255;
		uint8_t blue = hexValue & 255;
		return ColorAT<T>( CHANTRAIT<T>::convert( red ), CHANTRAIT<T>::convert( green ), CHANTRAIT<T>::convert( blue ), CHANTRAIT<T>::convert( alpha ) );
	}

	operator T*(){ return (T*) this; }
	operator const T*() const { return (const T*) this; }
	operator ColorT<T>(){ return ColorT<T>( r, g, b ); }
};

// Operators
template <typename T, typename Y> inline ColorT<T>  operator*( Y s, const ColorT<T>& c )  { return ColorT<T>( s*c.r, s*c.g, s*c.b ); }
template <typename T, typename Y> inline ColorAT<T> operator*( Y s, const ColorAT<T>& c ) { return ColorAT<T>( s*c.r, s*c.g, s*c.b, s*c.a ); }

// Free Functions
extern ColorT<float> hsvToRGB( const Vec3f &hsv );
extern Vec3f rgbToHSV( const ColorT<float> &c );

extern std::ostream& operator<<( std::ostream &lhs, const ColorT<float> &rhs );
extern std::ostream& operator<<( std::ostream &lhs, const ColorAT<float> &rhs );
extern std::ostream& operator<<( std::ostream &lhs, const ColorT<uint8_t> &rhs );
extern std::ostream& operator<<( std::ostream &lhs, const ColorAT<uint8_t> &rhs );

typedef ColorT<float>		Color;
typedef ColorT<float>		Colorf;
typedef ColorT<uint8_t>		Color8u;
typedef ColorAT<float>		ColorA;
typedef ColorAT<float>		ColorAf;
typedef ColorAT<uint8_t>	ColorA8u;

} // namespace cinder
