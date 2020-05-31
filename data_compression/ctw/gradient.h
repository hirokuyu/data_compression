#include "ctw_bcoder.h"

class gradient_array : public virtual ctw_bcoder
{
public:
	float_ex gradient(
		ctw_bnode *n,
		past_node *p_past,
		long d=0
	);

	float_ex gradient0(
		ctw_bnode *&n,
		int x,
		past_node *p_past,
		long d=0
	);

};

