#include "m3common.h"
#include "m3tree.h"
#include "m3types.h"

struct m3type {
	t_node *Type ;
	int size ;
};
struct m3type types_tab[MAX_TYPES];
t_type types_top=TYPES_PREDECLARED;

void types_init(void)
{
	
}

t_type types_add( t_node *type )
{
	int i;
	for (i=0;i<types_top;i++)
		if ( types_is_equal( type, types_tab[i].Type ))
			return i;
	types_top++;
	types_tab[i].Type=type;
	return i;
}

/* xman */
int types_is_assignable( t_type type_from, t_type type_to )
{
	if ( type_to == type_from )
		return 1;
	if ( type_to == TYPE_FLOAT && types_is_assignable (type_from, TYPE_INTEGER))
		return 1;
	if ( type_to == TYPE_INTEGER && type_from == TYPE_CARDINAL )
		return 1;
	return 0;
}

/* xman */
int types_is_equal( t_node *type1, t_node *type2 ) 
{
	return 0;
}

/* xman ? */
t_type types_expr_result( t_type t1, t_type t2 )
{
	if (!t1 || !t2) return TYPE_UNKNOWN;
	if ( types_is_assignable( t1, t2 )) return t2;
	if ( types_is_assignable( t2, t1 )) return t1;
	return TYPE_UNKNOWN;
}
