/********************************************
 --------------------------------------------
ctw_bcoder_incr.cpp

            binary CTW method
       with Incremental Context Set



*********************************************/
#include <vector>
#include "ctw_bcoder_incr.h"
#include "errormac.h"
using namespace std;
/********************************************/
/*   binary CTW Incremental Context Set     */
/********************************************/

ctw_bcoder_incr :: ctw_bcoder_incr()
{
	eta = 0;
	MaxDepth = -1;
	MaxNodes  = -1;
}

/***************  initialize  ***************/
int ctw_bcoder_incr ::
	init
		(
			float_ex (*p_update_esti)(
				const float_ex &cur,
				int x,
				long ctr[]),
			bit_input *bi,
			bit_output *bo,
			int to_decode,
			int ieta, long MD, long MN
		)
{
	egf(ieta<=0);
	eta=ieta;
	Depth = 0;
	update_esti = p_update_esti;

	clear_past();
	delete_node(ctree);
	ctree=NULL;
	/* prepare only current symbol node */
	past = new past_node;
	past->next = NULL;

	/* set context tree as empty */
	ctree = NULL;

	/* prepare bit-input and output */
	p_bi = bi;
	p_bo = bo;
	Code = 0;
	if( to_decode )
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

	MaxDepth = MD;
	MaxNodes = MN;
	Nodes = 1;

	return 0;
f:
	return -1;
}

/**********  condition to add node  *********/
int ctw_bcoder_incr ::
	C( ctw_bnode *p_node, long d )
{
	return
		p_node->ctr[0] + p_node->ctr[1] >= eta
		&& ( MaxNodes < 0 || Nodes < MaxNodes )
		&& ( MaxDepth < 0 || d < MaxDepth );
}

/************  update past memory  **********/
void ctw_bcoder_incr :: update_past()
{
	past_node *p_past=past;
	unsigned int c1,c2=p_past->c;
	int d=0;

	/* shift past symbols */
	while( p_past->next ){
		c1 = p_past->next->c;
		p_past->next->c = c2;
		c2 = c1;
		p_past = p_past->next;
		d++;
	}
	if( d <= Depth+1 ){
		p_past->next = new past_node;
		p_past->next->next = NULL;
		c1 = p_past->next->c;
		p_past->next->c = c2;
		c2 = c1;
		p_past = p_past->next;
	}
}

#define NextC n->child[p_past->next->c]
#define nonNextC n->child[!p_past->next->c]

/**********  find Pc if input is 0  *********/
float_ex ctw_bcoder_incr ::
	find_Pc0(
		ctw_bnode *n,
		past_node *p_past,
		long d
	)
{
	float_ex tPe;
	double beta = 0.01;
	if( !n ) return 0.5;
	n->Pwgrad = gradient(NextC,p_past->next,d+1);
	tPe = update_esti( n->Pe, 0, n->ctr );
	vector<double> b(630,1/2.0);
	b.at(d) = b.at(d) + beta*n->Pwgrad.decimal;
	if(b.at(d)>1/2.0){
		b.at(d) = b.at(d);
	} else{
		b.at(d) = 1/2.0;
	}
	if( !n->child[0] && !n->child[1] ){
		//in dT
		if( C(n,d) )
			return ( tPe + n->Pe/2.0 )/2.0;
		else return tPe;
	} else //in T
		//cout << d << endl;
		return
			tPe*b.at(d) +
				find_Pc0( NextC, p_past->next, d+1 )
				*( nonNextC? nonNextC-> Pw: float_ex(1.0) )*(1-b.at(d));
}

/***********  update context tree  **********/
float_ex ctw_bcoder_incr ::
	update_tree(
		ctw_bnode *&n,
		int x,
		past_node *p_past,
		long d )
{
	if( !n ){
		n = create_node();
		n->ctr[x] = 1;
		n->Pe = n->Pw = 0.5;
		return n->Pe;
	}
	double alpha = 0.001;
	if( !n->child[0] && !n->child[1] ){
	  //in dT
		if( C(n,d) ){ //add new child node
			if( d+1 > Depth ) Depth++;
			Nodes+=2;
			//cout << d << endl;
			NextC = create_node(0);
			NextC ->ctr[x] = 1;
			NextC ->Pe = NextC ->Pw = n->Pe*0.5;			
			n->Pe = update_esti( n->Pe, x, n->ctr );
			n->ctr[ x ]++;
			return n->Pw = ( n->Pe + NextC->Pw )/2.0;

		} else {
			n->Pe = update_esti( n->Pe, x, n->ctr );
			n->ctr[ x ]++;	
			return n->Pw = n->Pe;
		}
	} else { //in T
		n->Pe = update_esti( n->Pe, x, n->ctr );
		n->ctr[ x ]++;
		n->Pwgrad = gradient(NextC,p_past->next,d+1);
		vector<double> a(Depth,1/2.0);
		a.at(d)=a.at(d) +alpha*n->Pwgrad.decimal;
		if(a.at(d)>1/2.0){
			a.at(d)=a.at(d);
		}else{
			a.at(d) = 1/2.0;
		}

		//cout << a.at(d) << endl;
		//vector<vector<double> > a(74, vector<double>(74,0.5));

		return n->Pw =
			n->Pe*a.at(d) +
				update_tree(NextC,x,p_past->next,d+1)
				*( nonNextC? nonNextC-> Pw: float_ex(1.0))*(1-a.at(d));
	}
}

/*float_ex gradient_array ::
	gradient(
		ctw_bnode *&n,
		int x,
		past_node *p_past,
		long d
	)
{
	if( !n ){
		n = create_node();
		n->ctr[x] = 1;
		n->Pe = n->Pw = 0.5;
		return n->Pe;
	}

	if( !n->child[0] && !n->child[1] ){
	  //in dT
		if( C(n,d) ){ //add new child node
			if( d+1 > Depth ) Depth++;
			Nodes+=2;
			cout << d << endl;
			NextC = create_node();
			NextC ->ctr[x] = 1;
			NextC ->Pe = NextC ->Pw = n->Pe*0.5;			
			n->Pe = update_esti( n->Pe, x, n->ctr );		
			n->ctr[ x ]++;
			return n->Pwgrad = n->Pe - NextC->Pw ;

		} else {
			n->Pe = update_esti( n->Pe, x, n->ctr );
			n->ctr[ x ]++;	
			return n->Pwgrad = n->Pe;
		}
	} else { //in T
		n->Pe = update_esti( n->Pe, x, n->ctr );
		n->ctr[ x ]++;
		vector<double> a(74,0.5);
		//vector<vector<double> > a(74, vector<double>(74,0.5));
		cout << d << endl;
		return n->Pwgrad =
			n->Pe -
				gradient(NextC,x,p_past->next,d+1)
				*( nonNextC? nonNextC-> Pw: float_ex(1.0));
	}
}*/

