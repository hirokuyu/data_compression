/********************************************
 --------------------------------------------
ctw_bcoder.h

          binary CTW method coder

	Initialize with
	  bit_input,
	  bit_output if necessary,
		and estimate updater.
	Then, call encode() or decode().

*********************************************/

#if !defined __ctw_bcoder_h__
#define __ctw_bcoder_h__

#if !defined NULL
#define NULL 0
#endif

#include <iostream>
#include "float_ex.h"
#include "bitio.h"

/********************************************/
/*   node of binary weighted context tree   */
/********************************************/
struct ctw_bnode
{
	long ctr[2];
 	float_ex Pe, Pw, Pwgrad;
	        //estimate and weighted probability
	ctw_bnode *child[2];
	                // child node for 0s and 1s

	/* customization of memory allocation */
	void *operator new( size_t s );
	void operator delete( void *p );
};

/********************************************/
/*        node of past symbols list         */
/********************************************/
struct past_node
{
	unsigned int c;
	past_node *next;

	/* customization of memory allocation */
	void *operator new( size_t s );
	void operator delete( void *p );
};

/********************************************/
/*      binary CTW encoder and decoder      */
/********************************************/
class ctw_bcoder
{
public:

	bit_input *p_bi;   // bit-input
	bit_output *p_bo; // bit-output

	int verbose;

	/* estimate updator */
	float_ex (*update_esti)(
		const float_ex &cur,
		int x,
		long ctr[]
	);

	/* status */
	long Depth,	       // depth of context-trees
	 Nodes;   // number of nodes of context-tree
	ctw_bnode *ctree;          // context-trees
	past_node *past;
	   //current and past symbols list t,t-1,...

	long t,              // index of source data
	     L;                    // length of code
	unsigned int Low, High, Code;
	            // range of code is [Low,High+1)
	float_ex prevPw;
	long prevt, tn;
	long source_length; // length of source

	ctw_bcoder();
	virtual ~ctw_bcoder();

	/* initilalize */
	int init
		( long depth,
			float_ex (*p_update_esti)(
				const float_ex &cur,
				int x,
				long ctr[]),
			bit_input *bi,
			bit_output *bo,
			int to_decode
		);

	/* simulate to encode each symbol */
	long sim_encode_a_symbol( int x );
	/* encode each symbol */
	void encode_a_symbol(
		int x,
		long *code_length = NULL //return L+2
	);
	/* decode each symbol */
	int decode_a_symbol();
	/* finish encoding */
	void finish_encoding();
	/* finish decoding */
	void finish_decoding();

	/* encode input sequence */
	virtual int encode( long source_length=-1 );
		       //return length of compressed data
	/* decode input sequence */
	virtual int decode();

	/* print current status on encoding */
	void print_encode( ostream &out, int over=0 );
	/* print current status on decoding */
	void print_decode( ostream &out, int over=0 );

	/* update past memory */
	virtual void update_past();
	/* clear past memory */
	virtual void clear_past();

	/** operation for context tree **/

	/* create (sub-)tree */
	virtual ctw_bnode *create_node(long Depth=0);
	/* delete (sub-)tree */
	virtual void delete_node( ctw_bnode *p_node );

	/* find Pc if input is 0 */
	virtual float_ex find_Pc0(
		            //return tPw(lambda)
		ctw_bnode *n,
		past_node *p_past,
		            //past node corresponds to d
		long d=0    //depth of the node in tree
	);
	/* update context tree */
	virtual float_ex update_tree(
		            //return Pw(lambda)
		ctw_bnode *&n,
		int x,      //x_t, input symbol
		past_node *p_past,
		            //past node corresponds to d
		long d=0    //depth of the node in tree
	);

	

};

/* code precision */
static const unsigned int
	code_figure = 31,
	top = (((unsigned int)1<<code_figure)-1),
	first_qtr = (top/4+1),
	half      = (2*first_qtr),
	third_qtr = (3*first_qtr);

#endif
