/********************************************
 --------------------------------------------
btree_source.h

             binary tree source



*********************************************/

#if !defined __btree_source_h__
#define __btree_source_h__

#include <iostream>
using namespace std;
/********************************************/
/*            node of tree source           */
/********************************************/
struct tsrc_node {
	double theta;
		//parameter;
		//  probability that 1 will be generated
	tsrc_node *child[2];

	tsrc_node();
	~tsrc_node();

	/* set parameter on the node of depth d
			 for the suffix of the context */
	double set_param(char *ctxt,double v,int d);

	/* check fair or illegal */
	int is_illegal();

	/* get the parameter for the context */
	double get_param( char *ctxt );

	/* output parameter */
	int output_param( char *ctxt, ostream &os );
};

/********************************************/
/*               tree source                */
/********************************************/
class tree_source : public tsrc_node
{
public:

	int Depth;

	tree_source();

	/* laod parameter file */
	int load_param_file( const char *pfile );

	/* build random tree source */
	int build_random_tree( int D );

	/* output parameter file */
	int output_param_file( const char *pfile );

	/* get a bit */
	int get_a_bit( char *ctxt, double *clength );
};

#endif
