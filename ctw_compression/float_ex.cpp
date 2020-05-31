/********************************************
 --------------------------------------------
float_ex.cpp

     double decimal, 32bits exponent float



*********************************************/

#include <float.h>
#include <math.h>
#include "float_ex.h"
using namespace std;
#define sgn(x) ((x)? ((x)>0?1:-1) :0)

#define normalize(d,e)    \
if( d != 0 ){             \
	while( fabs(d) >= 1 ){  \
		d /= 2;               \
		e++;                  \
	}                       \
	while( fabs(d) < 0.50 ){\
		d *= 2;               \
		e--;                  \
	}                       \
} else

/***************  constructor  **************/
float_ex :: float_ex( double i )
{
	*this = i;
}
float_ex :: float_ex(double decimali,int expi)
{
	decimal = decimali;
	exp = expi;
}
float_ex :: float_ex( const float_ex &one )
{
	decimal = one.decimal;
	exp = one.exp;
}

/*****************  operator  ***************/
float_ex float_ex :: operator-() const
{
	return float_ex( -decimal, exp );
}
float_ex &float_ex ::
operator=( const float_ex &one )
{
	decimal = one.decimal;
	exp = one.exp;
	return *this;
}
float_ex &float_ex :: operator=( double one )
{
	decimal = frexp( one, &exp );
	return *this;
}
double float_ex :: Double() const
{
	return ldexp( decimal, exp );
}
float_ex :: operator int() const
{
	return (int)((*this).Double());
}
float_ex float_ex ::
operator+( const float_ex &one ) const
{
	double d;
	int e;
	add( d,           e,
	     decimal,     exp,
	     one.decimal, one.exp );
	return float_ex( d, e );
}
float_ex &float_ex ::
operator+=( const float_ex &one )
{
	add( decimal,     exp,
	     decimal,     exp,
			 one.decimal, one.exp );
	return *this;
}
float_ex float_ex ::
operator-( const float_ex &one ) const
{
	double d;
	int e;
	add( d,           e,
	     decimal,     exp,
	    -one.decimal, one.exp );
	return float_ex( d, e );
}
float_ex &float_ex ::
operator-=( const float_ex &one )
{
	add( decimal,     exp,
	     decimal,     exp,
			-one.decimal, one.exp );
	return *this;
}
float_ex float_ex ::
operator*( const float_ex &one ) const
{
	double d;
	int e;
	mul( d,           e,
	     decimal,     exp,
	     one.decimal, one.exp );
	return float_ex( d, e );
}
float_ex float_ex ::
operator*( const double one ) const
{
	double d, d2;
	int e, e2;
	d2 = frexp( one, &e2 );
	mul( d,       e,
	     decimal, exp,
	     d2,      e2 );
	return float_ex( d, e );
}
float_ex &float_ex ::
operator*=( const float_ex &one )
{
	mul( decimal,     exp,
	     decimal,     exp,
			 one.decimal, one.exp );
	return *this;
}
float_ex float_ex ::
operator/( const float_ex &one ) const
{
	double d;
	int e;
	div( d,           e,
	     decimal,     exp,
	     one.decimal, one.exp );
	return float_ex( d, e );
}
float_ex float_ex ::
operator/( const double one ) const
{
	double d, d2;
	int e, e2;
	d2 = frexp( one, &e2 );
	div( d,       e,
	     decimal, exp,
	     d2,      e2 );
	return float_ex( d, e );
}
float_ex &float_ex ::
operator/=( const float_ex &one )
{
	div( decimal,     exp,
	     decimal,     exp,
			 one.decimal, one.exp );
	return *this;
}

/*************  inner operation  ************/
void float_ex ::
	add(
		double &d, int &e,
		double d1, int e1,
		double d2, int e2
	) const
{
	double edif;

	if( d1 == 0 ){
		d = d2; e = e2;
		return;
	} else if( d2 == 0 ){
		d = d1; e = e1;
		return;
	} else
		if( e1 >= e2 ){
			edif = pow(2,e2-e1);
			d = d1 + d2 * edif;
			e = e1;
			normalize(d,e);
			return;
		} else {
			edif = pow(2,e1-e2);
			d = d2 + d1 * edif;
			e = e2;
			normalize(d,e);
			return;
		}
}

void float_ex ::
	mul(
		double &d, int &e,
		double d1, int e1,
		double d2, int e2
	) const
{
	if( d1 == 0 || d2 == 0 ){
		d = 0; e = 0;
		return;
	}
	d = d1 * d2;
	e = e1 + e2;
	normalize(d,e);
	return;
}

void float_ex ::
	div(
		double &d, int &e,
		double d1, int e1,
		double d2, int e2
	) const
{
	if( d1 == 0 ){
		d = 0;
		e = 0;
	} else {
		d = d1 / d2;
		e = e1 - e2;
		normalize( d, e );
	}
	return;
}
