/********************************************
 --------------------------------------------
float_ex.h

     double decimal, 32bits exponent float

  This class can be used
	  as a substitution of float or double.
	The four basic operations with double
		  are defined.
	To get double type value, use Double().
*********************************************/

#if !defined __float_ex_h__
#define __float_ex_h__

/********************************************/
/*  double decimal, 32-bit exponent float   */
/********************************************/
class float_ex
{
public:

	/* the value is 0.decimal * 2^exp */
	double decimal;
	int exp;

	float_ex( double i=0 );
	float_ex( double decmali, int expi );
	float_ex( const float_ex &one );

	float_ex operator-() const;
	float_ex &operator=( const float_ex &one );
	float_ex &operator=( double one );
	double Double() const;
	operator int() const;
	float_ex operator+(const float_ex &one) const;
	float_ex &operator+=( const float_ex &one );
	float_ex operator-(const float_ex &one) const;
	float_ex &operator-=( const float_ex &one );
	float_ex operator*(const float_ex &one) const;
	float_ex operator*( const double one ) const;
	float_ex &operator*=( const float_ex &one );
	float_ex operator/(const float_ex &one) const;
	float_ex operator/( const double one ) const;
	float_ex &operator/=( const float_ex &one );

	/** inner opearation **/

	void add(
		double &d, int &e,
		double d1, int e1,
		double d2, int e2
	) const;
	void mul(
		double &d, int &e,
		double d1, int e1,
		double d2, int e2
	) const;
	void div(
		double &d, int &e,
		double d1, int e1,
		double d2, int e2
	) const;

};

#endif
