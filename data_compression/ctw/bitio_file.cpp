/********************************************
 --------------------------------------------
bitio_file.cpp

              bit io for file



*********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "bitio_file.h"
#include "errormac.h"

#if !defined _O_BINARY
#define _O_BINARY 0
#endif
using namespace std;
/********************************************/
/*             bit input for file           */
/********************************************/

/***************  initialize  ***************/

int bit_input_file ::
	init( const char *filename, int x )
{
	emgf( handle =
		_open( filename, _O_BINARY | _O_RDONLY ) );

	free(name);
	name = (char *) malloc(strlen(filename)+1);
	strcpy(name,filename);

	compressed = x;

	if( x )
		emgf( _read( handle, &source_length, 4 ) );

	return 0;
f:
	return -1;
}

/***************  input a bit  **************/

int bit_input_file :: input_bit()
{
	int t;

	emgf( handle );

	if( cycle_ctr == 0 ){
		e0gf( _read(handle,&byte_buffer,1) );
		cycle_ctr = 8;
	}

	t = (byte_buffer & 0x80) != 0;
	byte_buffer <<= 1;
	cycle_ctr--;
	return t;
f:
	return -1;
}

/********************************************/
/*           bit output for file            */
/********************************************/

/***************  initialize  ***************/

int bit_output_file ::
	init( const char *filename, int x, long sl )
{
	emgf( handle =
		_open( filename,
			_O_BINARY | _O_CREAT | _O_WRONLY |_O_TRUNC,
			_S_IREAD | _S_IWRITE ) );

	free(name);
	name = (char *)malloc(strlen(filename)+1);
	strcpy(name,filename);

	compressed = x;
	source_length = sl >=0? sl : 0;

	cycle_ctr = 8;
	reverse_bits = 0;

	if( compressed )
		emgf(_write( handle, &source_length, 4 ));

	return 0;
f:
	return -1;
}

/************  finish outputting  ***********/

void bit_output_file :: finish()
{
	char c;

	if( cycle_ctr != 8 ){
		c=byte_buffer<<cycle_ctr;
		emr( _write( handle, &c, 1) );
	}

	if( compressed ){
		emr( lseek(handle,0,SEEK_SET) );
		emr( _write(handle,&source_length,4) );
	}

	_close(handle); handle=-1;
}

/**************  output a bit  **************/

int bit_output_file :: output_bit( int bit )
{
	egf( handle<0 );

	for( inner_output_bit(bit);
	     reverse_bits > 0;
			 inner_output_bit(!bit),reverse_bits-- );

	return 0;
f:
	return -1;
}

void bit_output_file ::
	inner_output_bit( int bit )
{
	byte_buffer <<= 1;
	if( bit ) byte_buffer |= 1;

	if( --cycle_ctr == 0 ){
		emr( _write( handle, &byte_buffer, 1) );
		byte_buffer = 0;
		cycle_ctr = 8;
	}
}
