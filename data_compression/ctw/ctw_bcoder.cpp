/********************************************
 --------------------------------------------
ctw_bcoder.cpp

          binary CTW method coder



*********************************************/

#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <iomanip>
#include "ctw_bcoder.h"
#include "errormac.h"
using namespace std;
#define If(X) if(X) (X)

static long bn_block=0x4000, pn_block=0x4000;
static unsigned long bn_count=0, pn_count=0;
static char *bn_head=NULL, *pn_head=NULL;
static long bn_rest = 0, pn_rest = 0;
static char *bn_next = (char *) &bn_head,
            *pn_next = (char *) &pn_head;

#define opnew(N)\
	void *ret;\
	if( N##_rest == 0 ){\
		e0gf( *(void**) N##_next =\
			             malloc( s*N##_block+4 ) );\
		N##_next = *(char**) N##_next;\
		*(void**)( N##_next+s*N##_block ) = NULL;\
		N##_rest = N##_block;\
	}\
	ret = N##_next;\
	N##_next += s;\
	N##_rest--;\
	N##_count++;\
	return ret;\
f:\
	return NULL;

#define opdel(N,ST)\
	char *p1, *p2;\
	if( --N##_count == 0 ){\
		p1 = (char*) N##_head;\
		while( p1 ){\
			p2 = *(char**)\
				  ( p1 + sizeof(ST)*N##_block );\
			free(p1);\
			p1 = p2;\
		}\
		N##_head = NULL;\
		N##_next = (char *) &N##_head;\
		N##_rest = 0;\
	}

/********************************************/
/*   node of binary weighted context tree   */
/********************************************/
/**** customization of memory allocation ****/
void *ctw_bnode :: operator new( size_t s )
{
	opnew(bn)
}
void ctw_bnode :: operator delete( void *p )
{
	opdel(bn,ctw_bnode)
}
/********************************************/
/*        node of past symbols list         */
/********************************************/
/**** customization of memory allocation ****/
void *past_node :: operator new( size_t s )
{
	opnew(pn)
}
void past_node :: operator delete( void *p )
{
	opdel(pn,past_node)
}

/********************************************/
/*      binary CTW encoder and decoder      */
/********************************************/

ctw_bcoder :: ctw_bcoder()
{
	p_bi = NULL;
	p_bo = NULL;
	verbose = 1;
	update_esti = NULL;
	Depth = 0;
	Nodes = 0;
	ctree = NULL;
	past = NULL;
	source_length=-1;
}

ctw_bcoder :: ~ctw_bcoder()
{
	clear_past();
	delete_node(ctree);
	ctree=NULL;
}

/***************  initialize  ***************/
int ctw_bcoder ::
	init
		( long depth,
			float_ex (*p_update_esti)(
				const float_ex &cur,
				int x,
				long ctr[]),
			bit_input *bi,
			bit_output *bo,
			int to_decode
		)
{
	past_node *p_past;
	int d;

	emgf(Depth);
	Depth = depth;
	update_esti = p_update_esti;

	clear_past();
	delete_node(ctree);
	ctree=NULL;
	/* initialize past symbols as ...000 */
	p_past = past = new past_node;
	for(d=1;d<=Depth;d++,p_past=p_past->next){
		p_past->next = new past_node;
		p_past->c = 0;
	}
	p_past->next = NULL;
	p_past->c = 0;

	/* set context tree as empty */
	ctree = NULL;
	Nodes = 0;

	/* prepare bit-input and output */
	p_bi = bi;
	p_bo = bo;
	Code = 0;
	if( to_decode ) //load Code
		for( int i=0; i < code_figure; i++ )
			Code = Code<<1 | (p_bi-> input_bit()==1);

	/* initialize status */
	Low = 0;
	High = top;
	prevPw = 1.0;

	t = 0;
	L = 0;

	prevt = 0;
	tn = 0;
	source_length = -1;

	return 0;
f:
	return -1;
}

/*****  simulate to encode each symbol  *****/
long ctw_bcoder ::
	sim_encode_a_symbol( int x )
{
	past->c = x;
	t++;
	/* update context tree */
	update_tree( ctree, x, past );
	update_past();

	return (L=-ctree->Pw.exp);
}

/***********  encode each symbol  ***********/
void ctw_bcoder ::
	encode_a_symbol(
		int x,
		long *code_length
	)
{
	unsigned int Range,
		Pc;   // coding probability
	float_ex Pup, alpha=0.001;
	past->c = x;

	t++;
	
	Range = High - Low + 1;
	/** CTW part **/
	//cout << x << endl;
	/* find coding probability */
	if( x==0 ){
		Pup = update_tree( ctree, 0, past );
		Pc = (int)
			( Pup
				* (double) Range
				/ prevPw
			);
	} else {
		Pc = (int)
			( find_Pc0( ctree, past )
				* (double) Range
				/ prevPw
			);
		//cout << find_Pc0( ctree, past ) << endl;
		Pc = Range-Pc;
		update_tree( ctree, 1, past );
	}

	/* underflow */
	if( Pc < 1 ) Pc = 1;
	else if( Pc > Range-1 ) Pc = Range-1;

	/* narrow code range */
	if( x == 0 ) High = Low + Pc - 1;
	else Low = High - Pc + 1;

	/** arithmetic coding part **/

	/* scaling */
	while(1){

		if( High < half ){
			If(p_bo)-> output_bit(0); L++;
		} else if( Low >= half ){
			Low -= half;
			High -= half;
			If(p_bo)-> output_bit(1); L++;
		} else if( first_qtr <= Low
			         && High < third_qtr ){
			Low -= first_qtr;
			High -= first_qtr;
			If(p_bo)-> incr_reverse_bits(); L++;
		} else break;

		Low = Low * 2;
		High = High * 2 + 1;

	}

	prevPw = ctree-> Pw;
	
	update_past();

	if( code_length ) *code_length = L;
	//cout << L << endl;
}

/***********  decode each symbol  ***********/
int ctw_bcoder :: decode_a_symbol()
{
	unsigned int Range,
	            // range of code is [Low,High+1)
		Pc0;   // coding probability if input is 0
	int x;

	Range = High - Low + 1;

	/** CTW part **/

	/* find cumulative probability */
	Pc0 = (int)
		( find_Pc0( ctree, past )
			* (double) Range
			/ prevPw
		);

	/* underflow */
	if( Pc0 < 1 ) Pc0 = 1;
	else if( Pc0 > Range-1 ) Pc0 = Range-1;

	/* decide source symbol */
	x = past->c = Code >= Low + Pc0;
	t++;

	/* narrow code range */
	if( x == 0 ) High = Low + Pc0 - 1;
	else Low += Pc0;

	/* update context-tree */
	update_tree( ctree, x, past );

	/** arithmetic coding part **/

	/* scaling */
	while(1){

		if( High < half );
		else if( Low >= half ){
			Low -= half;
			High -= half;
			Code -= half;
		} else if( first_qtr <= Low
			         && High < third_qtr ){
			Low -= first_qtr;
			High -= first_qtr;
			Code -= first_qtr;
		} else break;

		Low = Low * 2;
		High = High * 2 + 1;
		Code = Code * 2 | (p_bi->input_bit()==1);

	}

	prevPw = ctree-> Pw;
	update_past();

	return x;
}

/*************  finish encoding  ************/
void ctw_bcoder :: finish_encoding()
{
	/* decide final range */
	p_bi-> finish();
	If(p_bo)-> incr_reverse_bits(); L++;
	If(p_bo)-> output_bit( Low>=first_qtr ); L++;
	If(p_bo)-> set_source_length(t);
	If(p_bo)-> finish();
}

/*************  finish decoding  ************/
void ctw_bcoder :: finish_decoding()
{
	p_bi-> finish();
	If(p_bo)-> finish();
}

/**********  encode input sequence  *********/
int ctw_bcoder :: encode( long source_length )
{
	int x;

	if( verbose ) print_encode(cerr);

	while(( x = p_bi->input_bit() ) >= 0
	 && (source_length<0||(long)t<source_length)){

		encode_a_symbol(x);
		if( verbose ) print_encode(cerr);
	}

	finish_encoding();
	if( verbose ) print_encode(cerr,1);

	return L;
}

/**********  decode input sequence  *********/
int ctw_bcoder :: decode()
{
	int x;

	source_length = p_bi-> get_source_length();

	if( verbose ) print_decode(cerr);

	while( t < source_length ){

		x = decode_a_symbol();
		If(p_bo)-> output_bit( x );

		if( verbose ) print_decode(cerr);
	}
	if( verbose ) print_decode(cerr,1);
	finish_decoding();

	return 0;
}

/***** print current status on encoding *****/
void ctw_bcoder ::
	print_encode( ostream &out, int over )
{
	if( over || (source_length >=0
	   && t >= source_length*tn/20) ||
	   (source_length < 0 && t-prevt > 1000) ){
		prevt = t; tn++;
		out << "\r" << (p_bi?p_bi->get_name():" ")
		    << ": D " << Depth
		    << ": N " << Nodes
			  << ": t " << t
			  << ": L " << L
		    << ": L/t "
		    << setw(3) << L*100/t << "%";
		if( over )
			out << ": done       \n";
		else if( source_length >= 0 )
			out << ": worked "
			    << setw(3)
			    << t*100/source_length << "%";

		cerr.flush();
	}
}
/***** print current status on decoding *****/
void ctw_bcoder ::
	print_decode( ostream &out, int over )
{
	if( over || (source_length >=0
	   && t >= source_length*tn/20) ||
	   (source_length < 0 && t-prevt > 1000) ){
			prevt = t; tn++;
			out << "\r" << (p_bo?p_bo->get_name():" ")
			    << ": D " << Depth
			    << ": N " << Nodes
			    << ": n " << source_length
			    << ": t " << t;
			if( over )
				out << ": done       \n";
			else
				out << ": worked "
				    << setw(3)
				    << t*100/source_length << "%";
			cerr.flush();
		}
}

/************  update past memory  **********/
void ctw_bcoder :: update_past()
{
	past_node *p_past=past;
	unsigned int c1,c2=p_past->c;

	/* shift past symbols */
	while( p_past->next ){
		c1 = p_past->next->c;
		p_past->next->c = c2;
		c2 = c1;
		p_past = p_past->next;
	}
}
/************  clear past memory  ***********/
void ctw_bcoder :: clear_past()
{
	past_node *p_past;
	if( past ){
		p_past = past;
		while( p_past ){
			past = p_past->next;
			delete p_past;
			p_past = past;
		}
		past = NULL;
	}
}

/************  create (sub-)tree  ***********/
ctw_bnode *ctw_bcoder ::
	create_node( long depth )
{
	ctw_bnode *p_node = new ctw_bnode;

	if( depth ){
		p_node-> child[0] = create_node( depth-1 );
		p_node-> child[1] = create_node( depth-1 );
	} else p_node->child[0]=p_node->child[1]=NULL;

	p_node-> ctr[0] = p_node-> ctr[1] = 0;
	p_node-> Pe = p_node-> Pw = 1;
	p_node-> Pwgrad = 0;

	return p_node;
}

/************  delete (sub-)tree  ***********/
void ctw_bcoder ::
	delete_node( ctw_bnode *p_node )
{
	if( p_node ){
		delete_node( p_node->child[0] );
		delete_node( p_node->child[1] );
		delete p_node;
	}
}

#define NextC n->child[p_past->next->c]
#define nonNextC n->child[!p_past->next->c]

/**********  find Pc if input is 0  *********/
float_ex ctw_bcoder ::
	find_Pc0(
		ctw_bnode *n,
		past_node *p_past,
		long d
	)
{
	float_ex tPe;

	if( !n ) return 0.5;

	tPe = update_esti(n->Pe,0,n->ctr);
	if( d < Depth )
		return
			(tPe +
				find_Pc0( NextC, p_past->next, d+1 )
				*( nonNextC? nonNextC-> Pw: float_ex(1.0) )
			) /2.0;
	else return tPe;
}

/***********  update context tree  **********/
float_ex ctw_bcoder ::
	update_tree(
		ctw_bnode *&n,
		int x,
		past_node *p_past,
		long d
	)
{
	if( !n ){ n = create_node(); Nodes++; }
	n->Pe = update_esti( n->Pe, x, n->ctr );
	n->ctr[ x ]++;

	if( d < Depth )
		return n->Pw =
			(n->Pe +
				update_tree(NextC,x,p_past->next,d+1)
				*( nonNextC? nonNextC-> Pw: float_ex(1.0) )
			) /2.0;
	else return n->Pw = n->Pe;
}
