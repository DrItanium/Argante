#include <assert.h>
#include "m3common.h"
#include "m3tree.h"
#include "m3types.h"
#include "m3context.h"
#include "m3parser.h"

/* xman */
#define DECL_CONST 1
#define DECL_TYPE 2
#define DECL_EXCEPTION 3
#define DECL_VAR 4
#define DECL_PROCEDURE 5

/* counter of functions */
static int proc_counter;

t_id extract_id( t_node *node )
{
	switch (node->nt) {
		case NT_CONSTDECL : 
		case NT_VARDECL :
			return node->x.vardecl.Id;
		case NT_TYPEDECL :
			return node->x.typedecl.Id;
		case NT_EXCEPTIONDECL :
			return node->x.exceptiondecl.Id;
		case NT_PROCEDUREDECL :
			return node->x.proceduredecl.Id;
		case NT_FORMAL :
			return node->x.formal.Id;
		case NT_REVEALDECL :
			return node->x.revealdecl.QualId->x.qualid.Id; /* xman */
		default :
			fatal_error("Can't find ID in nt:%s", give_ntname( node->nt ));
	}
	return 0;
}

/* main functions in context analysis */

t_type analyse_call( t_node *ex, t_list *acts, t_env *env) 
{
	t_list *formals;
	t_node *proc,*sig,*a,*f;
	t_id id;

//	fprintf(stderr,"analysing call, ex->nt=%s\n",give_ntname(ex->nt));
	if (ex->nt != NT_ID ) 
		{	warning("Expression for '(' can only be id in line %d",ex->yyline); SAYONARA; }
	id=ex->x.Id;
	proc=find_id( ex->x.Id, env );
	if (!proc) 
		{	warning("Can't find id '%s'", give_literal( id )); SAYONARA; }
	if (proc->nt != NT_PROCEDUREDECL )
		{	warning("ID '%s' is not a procedure",give_literal(id)); SAYONARA; }
	sig=proc->x.proceduredecl.Signature;
	formals=sig->x.signature.Formals;
	while(acts) {
		if (!formals)
			{	warning("To many parameters for function %s in line %d", give_literal(id),ex->yyline); SAYONARA; }
		a=acts->node;
		f=formals->node;
		if (a->x.actual.Id) {	
			warning("Sorry, don't have name-bindings for procedure arguments('%s') at line %d", 
				give_literal(a->x.actual.Id),a->yyline); 
			SAYONARA; 
		}
		analyse_expression(a->x.actual.Expr, env);
		fprintf(stderr,"types: %d %d\n", a->x.actual.Expr->type, f->type);
		if (!types_is_assignable(a->x.actual.Expr->type, f->type)) {
			warning("Can't assign value to '%s' argument of '%s' in line %d",
				give_literal(f->x.formal.Id), give_literal( id ), ex->yyline);
			SAYONARA;
		}
		formals=formals->next;
		acts=acts->next;
	}	
	if (formals) {	
		warning("To little parameters for function %s in line %d", 
			give_literal(id),ex->yyline); 
		SAYONARA; 
	}
	if (!sig->x.signature.Type)	
		return TYPE_UNKNOWN;
	return sig->x.signature.Type->type;
} /* analyse_call */


void analyse_type ( t_node *type, t_env *env )
{
	if (!type) return ;
	switch( type->nt ) {
		case NT_QUALID : 
			if ( ! type->x.qualid.Id_qualifier ) {
				switch ( type->x.qualid.Id ) {
					case LIT_CARDINAL :
						type->type=TYPE_CARDINAL ; return ;
					case LIT_INTEGER : 
						type->type=TYPE_INTEGER ; return ;
					case LIT_FLOAT :
						type->type=TYPE_FLOAT ; return ;
					case LIT_BOOLEAN :
						type->type=TYPE_BOOLEAN ; return ;
					case LIT_CHAR :
						type->type=TYPE_STRING ; return ;
					case LIT_REFANY	:
						type->type=TYPE_REFANY ; return ;					
				}
			}
		case NT_ARRAYTYPE :
		case NT_PACKEDTYPE :
		case NT_ENUMTYPE :
		case NT_OBJECTTYPE :
		case NT_PROCEDURETYPE :
		case NT_RECORDTYPE :
		case NT_REFTYPE :
		case NT_SETTYPE :
		case NT_SUBRANGETYPE :
		default :
			warning("Type kind '%s' unimplemented in line %d",
				give_ntname( type->nt ), type->yyline );
			SAYONARA ;
	}
}

void add_signature( t_env *env, t_node *sig )
{
	t_id id;
	t_list *l;
	t_node *n,*t,*ex;
	int offs=0;
	int mode;

	if (!sig) 
		fatal_error("Strange, no signature to add");
	for(l=sig->x.signature.Formals;l;l=l->next) {
		n=l->node;
		id = n->x.formal.Id ;
		t = n->x.formal.Type ;
		ex = n->x.formal.ConstExpr;
		mode = n->x.formal.Mode;
		if ( t ) analyse_type( t, env->super_env );
		assert(t||ex);
		analyse_expression( ex, env->super_env );
		if (mode==MODE_VAR && ex ) 
			fatal_error( "Can't use VAR formal '%s' with default expression at line %d", give_literal( id ), n->yyline );
		if ( ex && check_mask( ex->status, STATUS_NOTCONST ))
			fatal_error( "Can't evaluate '%s' value as constant at line %d", give_literal( id ), n->yyline );
		if ( ex && t ) {
			if ( ! types_is_assignable( ex->type, t->type ) )
				fatal_error( "Can't assign value to '%s' at line %d", give_literal( id ), n->yyline );
					n->type = t->type;
		} else if ( ex ) {
			n->type = ex->type ;
		} else {
			n->type = t->type ;
		}
		if ( n->type != TYPE_INTEGER && n->type != TYPE_FLOAT ) {
			warning("There can be only int'n'float as formals, at line %d",sig->yyline);
			SAYONARA;
		}
		n->x.formal.offset=offs;
		offs+=1;
		fprintf(stderr,"Adding formal '%s' of type %d, with mode %d\n", give_literal( id ), n->type, mode );
		env->actuals=list_add(env->actuals,l->node);
		env->all_decls=list_add(env->all_decls,l->node);
	}
	t=sig->x.signature.Type;
	if (t) {
		analyse_type( t, env->super_env);
		env->restype=t->type;
	}
} /* add_signature */

t_env *build_env( t_env *super_env, t_list *decls )
{
	t_list *l,*l2,*ldecls;
	t_env *env,*procenv;
	t_node *n,*ex,*t,*bl;
	t_id id;

	env=my_alloc(sizeof(t_env));
	/* inherit environment */
	env->super_env=super_env;
	/* analyse declarations */
	for(l=decls;l;l=l->next) {
		n=l->node;
		id=extract_id(n);
		if ( id < LIT_RESERVED )
			fatal_error( "Trying to re-declare reserved identifier: '%s' at line %d",
				give_literal( id ), n->yyline );
		/* xman - powinno rozpoznawac deklaracje procedury */
		for (l2=env->all_decls;l2;l2=l2->next) {
			if ( id == extract_id( l2->node ))
				fatal_error( "Declaration of '%s' at line %d conflits with one from line %d",
					give_literal( id ), n->yyline, l2->node->yyline );					
		}
		/* add declaration */
		env->all_decls=list_add(env->all_decls,n);
		set_mask(n->status,STATUS_PREPARING);
		switch( n->nt ) {
			case NT_CONSTDECL :
			case NT_VARDECL :
				t = n->x.vardecl.Type ;
				ex = n->x.vardecl.Expr;
//				fprintf(stderr,"Parsing declaration of '%s'\n", give_literal( id ));
				if ( t ) analyse_type( t, env );
				assert(t||ex);
				analyse_expression( ex, env );
				if ( n->nt == NT_CONSTDECL && check_mask( ex->status, STATUS_NOTCONST ))
					fatal_error( "Can't evaluate '%s' value as constant at line %d", give_literal( id ), n->yyline );
				if ( ex && t ) {
					if ( ! types_is_assignable( ex->type, t->type ) )
						fatal_error( "Can't assign value to '%s' at line %d", give_literal( id ), n->yyline );
					n->type = t->type;
				} else if ( ex ) {
					n->type = ex->type ;
				} else {
					n->type = t->type ;
				}
				fprintf(stderr,"Adding var '%s' of type %d\n", give_literal( id ), n->type );
				if (n->nt==NT_CONSTDECL)
					env->constdecls=list_add(env->constdecls,n);
				else
					env->vardecls=list_add(env->vardecls,n);
				break ;
			case NT_PROCEDUREDECL :
				bl=n->x.proceduredecl.Block;
/*
				if (bl)
					procenv=build_env( env, bl->x.block.Decls );
				else
					procenv=build_env( env, 0 );
*/				
				ldecls=(bl)?bl->x.block.Decls:0;
				procenv=build_env(env,ldecls);
				add_signature( procenv, n->x.proceduredecl.Signature );
				if (bl) {
					analyse_statements( bl->x.block.S, procenv );
					bl->env=procenv;
				}
				env->proceduredecls=list_add(env->proceduredecls,n);
				n->x.proceduredecl.counter=++proc_counter;
				break ; 
			case NT_TYPEDECL :
			case NT_EXCEPTIONDECL :
			case NT_REVEALDECL :
			default :
				warning(" Declaration of '%s' unimplemented",give_ntname(n->nt));
				SAYONARA;
		}
		clear_mask(n->status,STATUS_PREPARING);
	}
	return env;
} /* build_env */

void analyse_decls( t_list *decls )
{
	t_id Id ;
	t_list *turtle=decls,*apollo;
	while(turtle) {
		Id = extract_id(turtle->node);
		for(apollo=decls; apollo!=turtle; apollo=apollo->next) {
			if ( Id == extract_id( apollo->node ))
				fatal_error("'%s' in line %d already declared in %d",
					give_literal(Id),turtle->node->yyline,apollo->node->yyline
					);
		}
		/* xman - tu powinna byc analiza deklaracji */
		turtle = turtle->next ;
	}
} /* analyse_decls */

/* xman - do przerobki */
int is_writable( t_node *expr, t_env *env )
{
	if (!expr) return 0;
	switch (expr->nt) {
		case NT_ID : 	
			/* xman - trzeba sprawdzic czy zmienna jest writable */
			return 1 ; 
		case NT_EXPRSELECTED :
			{	t_node *sel=expr->x.expr.Selector;
				switch (sel->x.selector.operand) {
					case KEY_UP : 
						return 1 ;
					case KEY_LSPAR : 
					case KEY_DOT :
						return is_writable(expr->x.expr.Expr,env);
				}
			}
	}
	return 0;
} /* is_writable */

t_node *find_id_depth( t_id id, t_env *env, int *depth )
{
	t_list *l;
	if (!env) return 0;
	for(l=env->all_decls;l;l=l->next)
		if ( id == extract_id(l->node))
			return l->node;
	(*depth)++;
	return find_id_depth( id, env->super_env, depth );

}

t_node *find_id( t_id id, t_env *env )
{
	t_list *l;
//	fprintf(stderr," id %d\n",id);
	if (!env) return 0;
	for(l=env->all_decls;l;l=l->next) 
		if ( id == extract_id(l->node))
			return l->node;
	return find_id( id, env->super_env);
}

int is_bool_operand( int op )
{
	return ( op == AND || op == OR || op == NOT
		|| op == KEY_EQ || op == KEY_NEQ || op == KEY_LT || op == KEY_GT 
		|| op == KEY_LEQ || op == KEY_GEQ || op == IN
	) ;
} /* is_bool_operand */

int is_bool_expr( t_node *e, t_env *env )
{
	if ( e->type == TYPE_BOOLEAN )
		return 1;
/*	if ( e->nt == NT_EXPR2 && is_bool_operand( e->x.expr.operand )) return 1;
	if ( e->nt == NT_EXPR1 && e->x.expr.operand == NOT ) return 1;
*/
/* xman: id, id() */
	return 0;
} /* is_bool_expr */

int is_arithm_operand( int op )
{
	return ( op == DIV || op == MOD || op == KEY_PLUS || op == KEY_MINUS
		|| op == KEY_MUL || op == KEY_DIV 
	) ;
} /* is_arithm_operand */

int is_arithm_expr( t_node *e, t_env *env )
{
	if ( types_is_assignable( e->type, TYPE_FLOAT ))
		return 1;
/*	if ( e->nt == NT_EXPR1 && is_arithm_operand( e->x.expr.operand )) return 1;
	if ( e->nt == NT_EXPR2 && is_arithm_operand( e->x.expr.operand )) return 1;
*/
/* xman: id, id() */
	return 0;
} /* is_bool_expr */

/* xman - mocno sprawdzic */
void analyse_expression( t_node *expr, t_env *env )
{
	t_node *e,*e1,*e2,*decl,*sel;
	t_type t;
	int op; 
	
	if (!expr) return ;
	switch( expr->nt ) {
		case NT_EXPR :
			e=expr->x.expr.Expr;
			analyse_expression( e, env );
			expr->status=e->status;
			expr->type=e->type;
			return;
		case NT_EXPR1 : 
			e=expr->x.expr.Expr;
			analyse_expression( e, env );
			expr->status=e->status;
			op=expr->x.expr.operand;
			switch ( op ) {
				case NOT : /* NOT e */
					if ( ! is_bool_expr( e, env ))
						fatal_error("NOT improper in line %d",expr->yyline);
					break; 
				case KEY_PLUS :
				case KEY_MINUS : /* +e , -e */
					if ( ! is_arithm_expr( e, env ))
						fatal_error("1arg +/- improper in line %d",expr->yyline);
					break ;
				default :
					warning("operand %s unimplemented",give_token( op ));
					SAYONARA;
			}
		case NT_EXPR2 : /* xman */
			e1=expr->x.expr.Expr;
			e2=expr->x.expr.Expr2;
			op=expr->x.expr.operand;
			analyse_expression( e1, env );
			analyse_expression( e2, env );
			expr->status=
				e1->status | e2->status;
			switch ( op ) {
				case DIV :
				case MOD :
				case KEY_PLUS :
				case KEY_MINUS :
				case KEY_MUL :
				case KEY_DIV :
					if ( ! is_arithm_expr( e1, env ) 
						|| ! is_arithm_expr( e2, env )) {
						fprintf(stderr,"e1[%d]-%d\n", e1->type, is_arithm_expr( e1, env )); 
						fprintf(stderr,"e2[%d]-%d\n", e2->type, is_arithm_expr( e2, env ));
						print_tree(e1); print_tree(e2);
						fatal_error("2arg +|-|*|/ improper in line %d", expr->yyline);
					}
					if ((op==DIV || op==MOD) && 
						(e1->type != TYPE_INTEGER || e2->type != TYPE_INTEGER))
						fatal_error("Can't use MUL/DIV operand with not INTEGER at line %d",expr->yyline);
					if (op!=KEY_DIV)
						t=types_expr_result(e1->type,e2->type);
					else
						t=TYPE_FLOAT; 
					if (!t)
						fatal_error("Can't determine 2arg +|-|*|/ type in line %d",expr->yyline);
					expr->type=t;
					break;
				case KEY_EQ :
				case KEY_NEQ :
				case KEY_LT :
				case KEY_GT :
				case KEY_LEQ :
				case KEY_GEQ :
				case IN :
				case OR :
				case AND : 
				case KEY_AND :
				default :
					warning("'%s' unimplemented",give_token(expr->x.expr.operand));
					SAYONARA;
					break ;
			}
			break ;
		case NT_INTEGER :
			expr->type=TYPE_INTEGER;
			break;
		case NT_FLOAT :
			expr->type=TYPE_FLOAT;
			break;
		case NT_STRING :	
			expr->type=TYPE_STRING;
			break;
		case NT_ID : /* xman */
			decl = find_id( expr->x.Id, env );
			if ( !decl )
				fatal_error( "Id '%s' unknown in line %d", 
					give_literal(expr->x.Id), expr->yyline);
			expr->type=decl->type;
			if ( decl->nt == NT_VARDECL )
				set_mask( expr->status, STATUS_NOTCONST );
			break ;
		case NT_EXPRSELECTED : /* xman */
			e=expr->x.expr.Expr;
			sel=expr->x.expr.Selector;
			op=sel->x.selector.operand ;
			switch( op ) {
				case KEY_LPAR : /* function call, compare Actls and Formals */
					expr->type=analyse_call(e,sel->x.selector.Actuals,env);
					break;
				case KEY_UP :
				case KEY_DOT :
				case KEY_LSPAR :
				default :
					warning("selector '%s' unimplemented",give_token( op ));
					SAYONARA;
			}
			break ;
		default :
			warning("analyse_expression(%s) unimplemented",give_ntname(expr->nt));
			SAYONARA;
			break;
	}
	return ;
}

void analyse_statement( t_node *stmt)
{
	t_env *env=stmt->env;
	t_node *e1,*e2,*ex,*sel;
	int oper;
	
	if (!stmt) return ;
//	fprintf(stderr,"Analysing node:"); print_tree(stmt);
	switch (stmt->nt) {
		case NT_BLOCK :
			return analyse_block(stmt, env);
		case NT_ASSIGNST :
			e1=stmt->x.assignst.Expr_left;
			e2=stmt->x.assignst.Expr_right;
			analyse_expression(e1,env);
			analyse_expression(e2,env);
			fprintf(stderr,"ASS: ltype = %d, rtype = %d\n", e1->type, e2->type );
			if (!is_writable(stmt->x.assignst.Expr_left,env))
				fatal_error("Not a writable designator at line %d",stmt->yyline);
			if (!types_is_assignable(e2->type,e1->type))
				fatal_error("Types are not assignable at line %d",stmt->yyline);
			/* xman: assignability */
			break ;
		case NT_RETURNST :
			ex=stmt->x.expr.Expr;
			analyse_expression(ex,env);
//			print_tree(ex);
			if (env->restype) {
				if(!ex)
					fatal_error("Procedure should return value at line %d",stmt->yyline);
					if (!types_is_assignable(ex->type,env->restype))
						fatal_error("Return expression not assignable to result type at line %d",stmt->yyline);
			} else {
				if (ex)
					fatal_error("Procedure should NOT return value at line %d",stmt->yyline);
			}
			break; /* NT_RETURNST */
		case NT_EXPRSELECTED :
			ex=stmt->x.expr.Expr;
			sel=stmt->x.expr.Selector;
			assert(sel);
			oper=sel->x.selector.operand;
			switch(oper) {
				case KEY_LPAR: /* ex(...) */
					if(analyse_call(ex,sel->x.selector.Actuals,env)) {
						warning("Uncatched return value at line %d",ex->yyline);
						SAYONARA;
					}
					break ;
				default :
					warning("Selector '%s' not implemented",give_token(oper));
					SAYONARA;
			}
			break ; /* NT_EXPRSELECTED */
		case NT_xxx_WRITEINTST :
			ex=stmt->x.expr.Expr;
			analyse_expression(ex,env);
			if (ex->type != TYPE_INTEGER) {
				warning("Argument of xxx_WRITEINT is not INTEGER at line %d",ex->yyline);
			}
			break ;
		default :
			warning("analyse_statement incomplete (%s)", give_ntname(stmt->nt));
			SAYONARA;
	}
} /* analyse_statement */

void analyse_statements ( t_list *S, t_env *env )
{
	for(;S;S=S->next) {
		S->node->env=env;
		analyse_statement(S->node);
	}
} /* analyse_statements */

void analyse_block( t_node *block, t_env *super_env )
{
	t_env *env ;

	if (!block) return ;

	env = build_env( super_env, block->x.block.Decls );
	block->env=env;
	analyse_statements( block->x.block.S, env );

} /* analyse_block */

void analyse_tree( t_node *tree )
{
	if (!tree)
		return ;
	if (tree->nt == NT_MODULE)
		analyse_block( tree->x.module.Block, 0 );
	else
		fatal_error("Must be module to analyse it");
	return ;
} /* analyse_tree */
