#ifndef _M3TYPES_H_
#define _M3TYPES_H_

#define MAX_TYPES 1024
#define TYPE_UNKNOWN 0
#define TYPE_REFANY 1
#define TYPE_INTEGER 2
#define TYPE_FLOAT 3
#define TYPE_STRING 4
#define TYPE_CARDINAL 5
#define TYPE_BOOLEAN 6
#define TYPES_PREDECLARED (TYPE_BOOLEAN+1)

void types_init(void);
t_type types_add( t_node *type );
int types_is_assignable( t_type type_from, t_type type_to );
int types_is_equal( t_node *type1, t_node *type2 );
t_type types_expr_result( t_type t1, t_type t2 );

#endif 
