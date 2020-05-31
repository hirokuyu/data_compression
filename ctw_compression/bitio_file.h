/********************************************
 --------------------------------------------
bitio_file.h

              bit io for file

	Initialize bit_input_file with file name.
	Initialize bit_output_file with file name.

*********************************************/

#if !defined __bitio_file_h__
#define __bitio_file_h__

#if defined WIN32
#include <io.h>
#elif defined unix
#include "forunix.h"
#endif
#include "bitio.h"
#include "forunix.h"
/********************************************/
/*             bit input for file           */
/********************************************/
class bit_input_file : public bit_input
{
public:

	int handle;                    // input file

	bit_input_file(){ handle=-1; }

	/* initialize
	   filename : input file name
		 x : 1:compressed */
	int init( const char *filename, int x );
	/* finish inputting */
	void finish()
	{ _close(handle); handle=-1; }

	/* get length of input file */
	long get_length()
		{ return _filelength(handle)*8; }

	/* set name */
	void set_name( char *name1 ){}

	/* input a bit */
	int input_bit();

};

/********************************************/
/*           bit output for file            */
/********************************************/
class bit_output_file : public bit_output
{
public:

	int handle;                   // output file

	bit_output_file(){ handle=-1; }

	/* initialize
	   filename : output file name
		 x : 1:compressed
		 sl : source length; if unknown, -1*/
	int init
		(const char *filename,int x,long sl=-1);
	/* finish outputting */
	void finish();

	/* set name */
	void set_name( char *name1 ){}

	/* output a bit */
	int output_bit( int bit );

	void inner_output_bit( int bit );

};

#endif
