/********************************************
 --------------------------------------------
ctw.cpp

      main routain for binary ctw coder



*********************************************/

#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#if defined _WIN32
#include <io.h>
#endif
#include "ctw_bcoder_incr.h"
#include "bitio_file.h"
#include "KrTr_esti.h"
#include "errormac.h"

using namespace std;

#if !defined MAX
#define MAX(A,B) ((A)>(B)?(A):(B))
#endif

long depth=0, algo=0, x=0, source_length=-1,
	// depth: depth of context-tree if a 0
	// algo:type of algorithm
	// x:decompress(1) or compress(0)
	// source_length:source length
	no_out=0, max_depth=-1, max_nodes=-1, eta=1,
	// no_out: if 1, not output.
	// max_depth:maximum depth if a 1
	// max_nodes:maximum number of nodes if a 1
  // eta:condition;n>=eta
	verbose=1;
char *ofile_name=NULL, *ifile_name=NULL;
	// output file name and input file name

/***************  show usage  ***************/
void usage()
{
	cerr <<
		"Usage: ctw [-h][-?][-D depth][-MD depth]"
		"[-MN num][-eta num][-i size][-a n][-c][-s]"
		"[-x][-T trees] -o file1 file2\n"
		"  -h,-? : show this.\n"
		"  -D depth : depth of context tree."
		    " default 0.\n"
		"  -MD depth : maximum depth if a 1."
		    " default no limitation.\n"
		"  -MN num : maximum number of nodes"
			" if a 1. default no limitation.\n"
		"  -eta num : condition for incremental>=1;"
			" n>=eta."
		"default 1.\n"
		"  -i size : input data size (bits)."
		    " default until eof.\n"
		"  -a 0 : use fixed-depth context set"
		    "(default).\n"
		"  -a 1 : use incremental CTW.\n"
		"  -c : no output.\n"
		"  -s : silent mode.\n"
		"  -x : decompress. default compress.\n"
			" default 1.\n"
		"  -o file1 : output file.\n"
		"  file2 : input file.\n";
}

#define load_int(V) \
	egf( ++i >= argc );V = atol(argv[i]);break

/***********  parameter analysis  ***********/
int analyse_parameter( int argc, char *argv[] )
{
	int i;
	egf( argc == 1 );
	for( i=1; i < argc; i++ )
		if( argv[i][0]=='-' || argv[i][0]=='/' ){
			switch( argv[i][1] ){
				case 'a': load_int(algo);
				case 'D': load_int(depth);
				case 'e':
					if(argv[i][2]=='t'&&argv[i][3]=='a')
						load_int(eta);
					break;
				case 'M':
					switch( argv[i][2] ){
						case 'D': load_int(max_depth);
						case 'N': load_int(max_nodes);
						default: goto f;
					}
					break;
				case 'c': no_out=1; break;
				case 'i': load_int(source_length);
				case 'x': x = 1; break;
				case 's': verbose = 0; break;
				case 'o':
					egf( ++i >= argc || ofile_name );
					ofile_name = argv[i];
					break;
				case 'h': case '?': default: // usage
					goto f;
			}
		} else { // input-file name
			egf( ifile_name );
			ifile_name = argv[i];
		}
	return 0;
f:
	return -1;
}

/***** prepare bit-input for data output ****/
bit_input *prepare_input()
{
	bit_input_file *p_bif = NULL;

	e0gf( ifile_name );
	e0gfn( p_bif = new bit_input_file(), 1);

	if( p_bif-> init(ifile_name,x)<0 ){
		cerr<<"Can't open "<<ifile_name<<".\n";
		goto f;
	}
	if( !x && source_length < 0 )
		source_length = p_bif-> get_length();
	return p_bif;
f1:
	cerr << "memory error.\n";
f:
	delete p_bif;
	return NULL;
}

/**** prepare bit-output for data output ****/
bit_output *prepare_output()
{
	bit_output_file *p_bof = NULL;

	e0gf( ofile_name);
	e0gfn( p_bof = new bit_output_file(), 1);

	if( p_bof-> init(ofile_name,!x)<0 ){
		cerr<<"Can't open "<<ofile_name<<".\n";
		goto f;
	}
	return p_bof;
f1:
	cerr << "memory error.\n";
f:
	delete p_bof;
	return NULL;
}

/************  prepare ctw coder  ***********/
ctw_bcoder *
	prepare_ctw_bcoder
		(bit_input *p_bi,bit_output *p_bo)
{
	ctw_bcoder *p_ctw = NULL;
	ctw_bcoder_incr *p_ctwi = NULL;

	switch( algo ){
		case 0: // fixed-depth ctw
			p_ctw = new ctw_bcoder();
			e0gfn(p_ctw,1);
			emgfn( p_ctw->
				init(depth,
					KrTr_bupdate,p_bi,p_bo,x), 2 );
			return p_ctw;
		case 1: // incremental ctw
			p_ctwi = new ctw_bcoder_incr();
			e0gfn(p_ctwi,1);
			emgfn( p_ctwi->
				init(
					KrTr_bupdate,p_bi,p_bo,x,
						eta,max_depth,max_nodes), 2 );
			return p_ctwi;
		default:
			usage();
			goto f;
	}
f1:
	cerr << "memory error.\n"; goto f;
f2:
	cerr << "Can't build the coder.\n";
f:
	delete p_ctw; delete p_ctwi;
	return NULL;
}

/********************************************/
/*               main routain               */
/********************************************/
int main( int argc, char *argv[] )
{
	bit_input *p_bi = NULL;
	bit_output *p_bo = NULL;
	ctw_bcoder *p_ctw = NULL;
	time_t t;

	emgfn( analyse_parameter(argc,argv), 1);
	e0gf( p_bi = prepare_input() );
	if( !no_out ) e0gf( p_bo = prepare_output() );
	e0gf( p_ctw = prepare_ctw_bcoder(p_bi,p_bo) );
	if( !verbose ) p_ctw->verbose = 0;

	/** encode or decode **/
	t=time(NULL);
	if( !x ) p_ctw-> encode(source_length);
	else     p_ctw-> decode();
	if( verbose )
		cerr << "time: " << time(NULL)-t
				 << "[sec]\n";
		cerr.flush();

	delete p_bi; delete p_bo; delete p_ctw;
	return 0;
f1:
	usage();
f:
	delete p_bi; delete p_bo; delete p_ctw;
	return -1;
}
