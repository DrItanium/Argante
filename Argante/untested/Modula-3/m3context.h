#ifndef _M3CONTEXT_H_
#define _M3CONTEXT_H_

/* node status masks */
#define STATUS_NOTCONST 0x0001
#define STATUS_PREPARING 0x0002
#define MASK_FULL 0xffff
#define set_mask(x,mask) (x) |= (mask)
#define clear_mask(x,mask) (x) &= MASK_FULL-(mask)
#define check_mask(x,mask) ((x) & (mask))

struct s_env {
	struct s_env *super_env;
	t_list *all_decls; /* contcatenation of all lists below */
	t_list *constdecls; /* constants */
	t_list *typedecls; /* types */
	t_list *exceptiondecls; /* exceptions */
	t_list *vardecls; /* variables */
	t_list *proceduredecls; /* procedures */
	t_list *revealdecls; /* reveals */
	t_list *actuals; /* actuals - if in procedure */

	t_type restype; /* type of result - if in function */
} ;

t_env *build_env( t_env *super_env, t_list *decls );
void analyse_tree( t_node *tree );
void analyse_decls( t_list *decls );
void analyse_statements( t_list *S, t_env *env );
void analyse_statement( t_node *stmt );
void analyse_block( t_node *block, t_env *super_env );
void analyse_expression( t_node *expr, t_env *env );
void analyse_type( t_node *type, t_env *env );
t_id extract_id( t_node *node );
t_node *find_id( t_id id, t_env *env );
t_node *find_id_depth( t_id id, t_env *env, int *d );

#endif 
