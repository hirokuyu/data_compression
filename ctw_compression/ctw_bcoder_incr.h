/********************************************
 --------------------------------------------
ctw_bcoder_incr.h

            binary CTW method
       with Incremental Context Set



*********************************************/

#if !defined __ctw_bcoder_incr_h__
#define __ctw_bcoder_incr_h__
#include "gradient.h"
#include "ctw_bcoder.h"

/********************************************/
/*   binary CTW Incremental Context Set     */
/********************************************/

class ctw_bcoder_incr : public virtual ctw_bcoder, public gradient_array
{
public:

	long eta, //C(s)= n >= eta
		MaxDepth, MaxNodes;
		// MaxDepth:maximum depth
		// MaxNode:maximum number of nodes

	ctw_bcoder_incr();
	int init
		(
			float_ex (*p_update_esti)(
				const float_ex &cur,
				int x,
				long ctr[]),
			bit_input *bi,
			bit_output *bo,
			int to_decode,
			int ieta, long MD, long MN
	);

	/* condition to add node */
	virtual int C( ctw_bnode *p_node, long d );

	/* update past memory */
	void update_past();

	/** operation for context tree **/

	/* find Pc if input is 0 */
	float_ex find_Pc0(
		ctw_bnode *n,
		past_node *p_past,
		long d=0
	);

	/* update context tree */
	float_ex update_tree(
		ctw_bnode *&n,
		int x,
		past_node *p_past,
		long d=0
	);

	


};





#endif
