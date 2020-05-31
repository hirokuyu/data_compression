#include <vector>
#include "gradient.h"
#include "errormac.h"
using namespace std;
#define NextC n->child[p_past->next->c]
#define nonNextC n->child[!p_past->next->c]
float_ex gradient_array ::
	gradient(
		ctw_bnode *n,
		past_node *p_past,
		long d
	)
{
	float_ex tPe;

	if( !n ) return 0;

	tPe = update_esti( n->Pe, 0, n->ctr );
	if( !n->child[0] && !n->child[1] ){
		//in dT
		return  tPe - n->Pe/2.0 ;
	} else //in T
		//cout << d << endl;
		return
			(tPe -
				gradient( NextC, p_past->next, d+1 )
				*( nonNextC? nonNextC-> Pw: float_ex(1.0) )
			);
}

float_ex gradient_array ::
	gradient0(
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
	double alpha = 0.1;
	if( !n->child[0] && !n->child[1] ){
	  //in dT
		n->Pe = update_esti( n->Pe, x, n->ctr );
		n->ctr[ x ]++;	
		return n->Pw = n->Pe;
		
	} else { //in T
		n->Pe = update_esti( n->Pe, x, n->ctr );
		n->ctr[ x ]++;
		//cout << d << endl;
		return n->Pe -
				gradient0(NextC,x,p_past->next,d+1)
				*( nonNextC? nonNextC-> Pw: float_ex(1.0));
	}
}
