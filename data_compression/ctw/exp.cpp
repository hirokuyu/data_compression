/********************************************
 --------------------------------------------
exp.cpp

             experiment program



*********************************************/

#include <time.h>
#include <math.h>
#include <sys/timeb.h>
#include <fstream>
#include <iomanip>
#include "btree_source.h"
#include "bitio_file.h"
#include "ctw_bcoder_incr.h"
#include "KrTr_esti.h"
#include "my_rand.h"
#include "errormac.h"
using namespace std;
int expn=0;
char tsf[]="exp/ts00",Laf[]="exp/La00",
	Rf[]="exp/Rc0_00",Ravf[]="exp/Rc0_av";

/***********  parameter analysis  ***********/
int analyse_parameter( int argc, char *argv[] )
{
	int i, D=0;
	egf( argc == 1 );
	for( i=1; i < argc; i++ )
		if( argv[i][0]=='-' || argv[i][0]=='/' ){
			switch( argv[i][1] ){
				case 'a': // input data length
					egf( ++i >= argc );
					expn = atol(argv[i]);
					break;
			}
		}
	return 0;
f:
	return -1;
}

/********** create tree source file *********/
int create_tsrc_file
	( tree_source *ts, int D, int times, int N )
{
	bit_output_file bo;
	fstream fLa;
	double La, dL;
	int i, d, t, nextt, nextvt, skip;
	char ctxt[256];

	for( i=0; i < times; i++ ){
		tsf[6]='0'+i/10; tsf[7]='0'+i%10;
		cerr << "creating " << tsf << ".\n";
		emgf( bo.init(tsf,0) );
		Laf[6]='0'+i/10; Laf[7]='0'+i%10;
		fLa.open(Laf,ios::out); fLa.clear();
		La=0;
		nextt=1; nextvt=0; skip = 1;
		for( d=0; d < D; d++ )
			ctxt[d]=(double)my_rand()/MY_RAND_MAX<0.5;
		for( t=1; t <= N; t++ ){
			bo.output_bit(ts->get_a_bit(ctxt,&dL));
			La += dL;
			for( d=0; d < D; d++ ) ctxt[d]=ctxt[d+1];
			if( t==nextt ){
				nextt += skip;
				if( nextt >= skip*100 ) skip *= 10;
				fLa << La << endl;
			}
			if( t>=nextvt ){
				cerr<<"\rts"<<i<<":"<<t*100/N<<" % ";
				nextvt += N/20;
			}
		}
		cerr << endl;
		bo.finish();
		fLa.close();
	}
	return 1;
f:
	return 0;
}

/****** create 0-run prob. source file *****/
int create_0run_src_file
	( double a, int times, int N )
{
	bit_output_file bo;
	fstream fLa;
	double La;
	int i, t, nextt, nextvt, skip,m,c;

	for( i=0; i < times; i++ ){
		m=0;
		tsf[6]='0'+i/10;
		tsf[7]='0'+i%10;
		cerr << "creating " << tsf << ".\n";
		emgf( bo.init(tsf,0) );
		Laf[6]='0'+i/10;
		Laf[7]='0'+i%10;
		fLa.open(Laf,ios::out);
		fLa.clear();
		La=0;
		nextt=1;
		nextvt=0;
		skip = 1;
		m=0;
		for( t=1; t <= N; t++ ){
			bo.output_bit(c=
				(double)my_rand()/MY_RAND_MAX<1.0/(a*m+2));
			if(c){
				La += -log(1.0/(a*m+2))/log(2);
				m=0;
			} else {
				La += -log(1-1.0/(a*m+2))/log(2);
				m++;
			}
			if( t==nextt ){
				nextt += skip;
				if( nextt >= skip*100 ) skip *= 10;
				fLa << La << endl;
			}
			if( t>=nextvt ){
				cerr << "\rts" << i << ":"<<t*100/N<<
					" % ";
				nextvt += N/20;
			}
		}
		cerr << endl;
		bo.finish();
		fLa.close();
	}
	return 1;
f:
	return 0;
}

/********** prepare find Redundancy *********/
void prepare_find_red( int num, int i,
 bit_input_file &bi, fstream &fR, fstream &fLa )
{
	Rf[6]='0'+num;
	tsf[6]='0'+i/10; tsf[7]='0'+i%10;
	Laf[6]='0'+i/10; Laf[7]='0'+i%10;
	Rf[8]='0'+i/10; Rf[9]='0'+i%10;
	bi.init(tsf,0);
	fR.open(Rf,ios::out); fR.clear();
	fLa.open(Laf,ios::in);
}
/******* prepare fixed depth ctw coder ******/
ctw_bcoder *prepare_fixed_coder( int depth )
{
	ctw_bcoder *coder=new ctw_bcoder;
	coder->init(depth,KrTr_bupdate,NULL,NULL,0);
	return coder;
}
/*** prepare incremental context ctw coder **/
ctw_bcoder *prepare_incr_coder( int eta )
{
	ctw_bcoder_incr *coder=new ctw_bcoder_incr;
	coder->
	 init(KrTr_bupdate,NULL,NULL,0,eta,-1,-1);
	return coder;
}

/************** find redundancy *************/
void find_redundancy(
	ctw_bcoder *coder,int N, int i,
	bit_input &bi, fstream &fR,fstream &fLa)
{
	time_t st=time(NULL);
	double La;
	int nextt=1,nextvt=0,skip=1,t,L;

	coder->source_length = N;
	for( t=1; t <= N; t++ ){
		L=coder->
			sim_encode_a_symbol(bi.input_bit());
		if( t==nextt ){
			nextt += skip;
			if( nextt >= skip*100 ) skip *= 10;
			fLa >> La;
			fR << L + 2 - La << endl;
		}
		if( t >= nextvt ){
			coder->print_encode(cerr);
			cerr << " to ts" << i;
			nextvt+=N/20;
		}
	}
	coder->print_encode(cerr,1);
	cerr<<" time:"<<time(NULL)-st<<"[sec]"<<endl;
	fR << "#Depth=" << coder->Depth
	   << " Nodes=" << coder->Nodes
		 << " Time="  << time(NULL)-st << endl;
	bi.finish();
	delete coder;
	fLa.close();
	fR.close();
}

/******** take average of redundancy ********/
void take_av_of_R( int num, int times, int N )
{
	fstream fRav,fRs[100];
	double Rd, dRd;
	int i, t, nextvt, skip;

	Rf[6]=Ravf[6]='0'+num;
	fRav.open(Ravf,ios::out);
	for( i=0; i < times; i++ ){
		Rf[8]='0'+i/10; Rf[9]='0'+i%10;
		fRs[i].open(Rf,ios::in);
	}
	skip=1; nextvt=0;
	for( t=1; t <= N; t += skip ){
		if( t >= skip*100 ) skip *= 10;
		if( t >= nextvt ){
			cerr << "\rtaking average of R" << num
			     << " : " << t*100/N << " %";
			nextvt += N/20;
		}
		for( i=0,Rd=0; i < times; i++ ){
			fRs[i] >> dRd; Rd += dRd;
		}
		fRav << t << " " << Rd/times << endl;
	}
	cerr << "\rtaking average of R" << num
	     << " : done      \n";
	for( i=0; i < times; i++ ) fRs[i].close();
	fRav.close();
}

/**************  experiment 0  **************/
void exp0()
{
	const int de=3;
	tree_source *tsrc=new tree_source;
	ctw_bcoder *coder;
	bit_input_file bi;
	fstream fR,fLa;
	int d, N=1000000, times=5;
	
	tsrc->build_random_tree(8);
	tsrc->output_param_file("exp/prm");
	create_tsrc_file(tsrc,8,times,N);
	delete tsrc;
	for( int i=0; i < times; i++ ){
		prepare_find_red(0,i,bi,fR,fLa);
		coder=prepare_fixed_coder(7);
		find_redundancy(coder,N,i,bi,fR,fLa);
		for( d=0; d < de; d++ ){
			prepare_find_red(d+1,i,bi,fR,fLa);
			coder=prepare_fixed_coder(8+d*4);
			find_redundancy(coder,N,i,bi,fR,fLa);
		}
		for( d=0; d < 2; d++ ){
			prepare_find_red(d+de+1,i,bi,fR,fLa);
			coder=prepare_incr_coder(d+1);
			find_redundancy(coder,N,i,bi,fR,fLa);
		}
	}
	for( d=0; d < de+3; d++ )
		take_av_of_R(d,times,N);
}

/**************  experiment 1  **************/
void exp1()
{
	const int de=3;
	ctw_bcoder *coder;
	bit_input_file bi;
	fstream fR,fLa;
	double a=0.65;
	int d, N=100000, times=3;

	create_0run_src_file(a,times,N);
	for( int i=0; i < times; i++ ){
		for( d=0; d < de; d++ ){
			prepare_find_red(d,i,bi,fR,fLa);
			coder=prepare_fixed_coder(8+d*8);
			find_redundancy(coder,N,i,bi,fR,fLa);
		}
		for( d=0; d < 2; d++ ){
			prepare_find_red(d+de,i,bi,fR,fLa);
			coder=prepare_incr_coder(d+1);
			find_redundancy(coder,N,i,bi,fR,fLa);
		}
	}
	for( d=0; d < de+2; d++ )
		take_av_of_R(d,times,N);
}

/********************************************/
/*               main routain               */
/********************************************/
int main( int argc, char *argv[] )
{
	timeb tm;
	/** initialize random sequence **/
	ftime(&tm);
	my_srand((unsigned int)time(NULL)
		^(tm.millitm*256));
	time_t t=time(NULL);

	if( analyse_parameter(argc,argv) ) goto ue;

	switch( expn ){
		case 0: exp0(); break;
		case 1: exp1(); break;
	}
	cerr << "total:" << time(NULL)-t << "[sec]\n";
	return 0;
ue:
	return -1;
}
