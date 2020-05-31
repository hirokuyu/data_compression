/********************************************
 --------------------------------------------
KrTr_esti.h

       Krichevsky-Trofimov estimate



*********************************************/

#if !defined __KrTr_esti_h__
#define __KrTr_esti_h__

#include "float_ex.h"

/********************************************/
/*       Krichevsky-Trofimov estimate       */
/********************************************/

/* update for increase in a count for binary */
float_ex
	KrTr_bupdate(        //return new value
		const float_ex &cur,//current value
		int x,              //input data
		long ctr[]          //counter(ctr[0],ctr[1])
	)
{
	return cur *
		((ctr[x]*2.0+1.0)/
		 (((double)ctr[0]+ctr[1]+1.0)*2.0));
}

#endif
