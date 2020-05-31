/********************************************
 --------------------------------------------
bitio.h

            bit input and output

  bit_input and bit_output should be used
    as a base class.
  Befor input or output data,
    init(.) has to be called.
  After input or output all data,
    finish() has to be called.
	Each derived class should has
    its own init(.) and finish().
	Compressed data has source length
	  of 4 bytes on head.
	bit_output:
	  To set the source length at head,
		  the output data can be buffered.
		If buffering is required(default),
		  the data are actually output in finish().
  incr_reverse_bits() is involved
	  with the arithmetic coding.

*********************************************/

#if !defined __bitio_h__
#define __bitio_h__

#include <stdlib.h>
#include <string.h>
#if defined WIN32
#include <strstrea.h>
#else
#include <strstream>
#endif
using namespace std;
/********************************************/
/*                bit input                 */
/********************************************/
class bit_input
{
public:

	char *name;              // name for display
	long source_length;
	   // source data length for compressed data
	int compressed;         // 1:compressed data

	/* status */
	unsigned char byte_buffer, cycle_ctr;

	bit_input(){
		name = NULL;
		source_length = -1;
		compressed = 0;
		cycle_ctr = 0;
	}

	/* finish inputting */
	virtual void finish()=0;

	/* name */
	char *get_name(){ return name; }
	/* set name */
	virtual void set_name( char *name1 ){
		free(name);
		name = (char*)malloc(strlen(name1)+1);
		strcpy(name,name1);
	}

	/* get input data length(bit) */
	virtual long get_length()=0;

	/* get source data length
	           for compressed data(bit) */
	long get_source_length()
		{ return source_length; }

	/* input a bit */
	virtual int input_bit()=0;

};

/********************************************/
/*                bit output                */
/********************************************/
class bit_output
{
public:

	char *name;              // name for display
	long source_length;
	            // data length before compressed
	int compressed;         // 1:compressed data
	int buffering;                // 1:buffering
	strstream *buffer;       // temporary output

	/* status */
	unsigned char byte_buffer, cycle_ctr;

	/* how many reverse bits follow output bit,
	  for value in [first_qtr,third_qtr) */
	long reverse_bits;

	bit_output(){
		name = NULL;
		source_length = -1;
		compressed = 0;
		buffering = 0;
		buffer = NULL;
		cycle_ctr = 8;
		reverse_bits = 0;
	}

	/* finish outputting */
	virtual void finish()=0;

	/* name */
	char *get_name(){ return name; }

	/* set name */
	virtual void set_name( char *name1 ){
		free(name);
		name = (char*) malloc(strlen(name1)+1);
		strcpy(name,name1);
	}

	/* set source length */
	void set_source_length( long l )
		{ source_length = l; }

	/* increase reverse_bits */
	long incr_reverse_bits(){
		return ++reverse_bits;
	}

	/* output a bit */
	virtual int output_bit( int bit )=0;

};

#endif
