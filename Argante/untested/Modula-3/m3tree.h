#ifndef _M3TREE_H_
#define _M3TREE_H_

/* reserved literals */
#define LIT_UNKNOWN 0
#define LIT_ABS 1
#define LIT_ADDRESS 2
#define LIT_ADR 3
#define LIT_ADRSIZE 4
#define LIT_BITSIZE 5
#define LIT_BOOLEAN 6
#define LIT_BYTESIZE 7
#define LIT_CARDINAL 8
#define LIT_CEILING 9
#define LIT_CHAR 10
#define LIT_DEC 11
#define LIT_DISPOSE 12
#define LIT_EXTENDED 13
#define LIT_FALSE 14
#define LIT_FIRST 15
#define LIT_FLOAT 16
#define LIT_FLOOR 17
#define LIT_INC 18
#define LIT_INTEGER 19
#define LIT_ISTYPE 20
#define LIT_LAST 21
#define LIT_LONGREAL 22
#define LIT_LOOPHOLE 23
#define LIT_MAX 24
#define LIT_MIN 25
#define LIT_MUTEX 26
#define LIT_NARROW 27
#define LIT_NIL 28
#define LIT_NULL 29
#define LIT_NUMBER 30
#define LIT_ORD 31
#define LIT_REAL 32
#define LIT_REFANY 33
#define LIT_ROUND 34
#define LIT_SUBARRAY 35
#define LIT_TEXT 36
#define LIT_TRUE 37
#define LIT_TRUNC 38
#define LIT_TYPECODE 39
#define LIT_VAL 40
#define LIT_WRITEINT 41
#define LIT_RESERVED (LIT_WRITEINT+1)

/* node types */
#define NT_UNKNOWN			0
#define NT_INTEGER			1
#define NT_FLOAT			2
#define NT_CHAR				3
#define NT_STRING			4
#define NT_ID				5
#define NT_INTERFACE		10
#define NT_INST_INTERFACE	11
#define NT_MODULE			12
#define NT_INST_MODULE		13
#define NT_ASIMPORT         14
#define NT_FROMIMPORT		15
#define NT_BLOCK			16
#define NT_CONSTDECL		17
#define NT_TYPEDECL			18
#define NT_EXCEPTIONDECL	19
#define NT_VARDECL			20
#define NT_PROCEDUREDECL	21
#define NT_REVEALDECL		22
#define NT_QUALID			23
#define NT_SIGNATURE		24
#define NT_FORMAL			25
#define NT_ASSIGNST			26
#define NT_CALLST			27
#define NT_CASEST			28
#define NT_EXITST			29
#define NT_EVALST			30
#define NT_FORST			31
#define NT_IFST				32
#define NT_IFCASE			33
#define NT_LOCKST			34
#define NT_LOOPST			35
#define NT_RAISEST			36
#define NT_REPEATST			37
#define NT_RETURNST			38
#define NT_TCASEST			39
#define NT_TRYXPTST			40
#define NT_TRYFINST			41
#define NT_WHILEST			42
#define NT_WITHST			43
#define NT_ACTUAL			44
#define NT_CASE				45
#define NT_LABEL			46
#define NT_TCASE			47
#define NT_HANDLER			48
#define NT_BINDING			49
#define NT_ANY				50
#define NT_ALIAS			51
#define NT_ARRAYTYPE		52
#define NT_PACKEDTYPE		53
#define NT_ENUMTYPE			54
#define NT_OBJECTTYPE		55
#define NT_PROCEDURETYPE	56
#define NT_RECORDTYPE		57
#define NT_REFTYPE			58
#define NT_SETTYPE			59
#define NT_SUBRANGETYPE		60
#define NT_METHOD			61
#define NT_OVERRIDE			62
#define NT_FIELD			63	
#define NT_EXPR				64
#define NT_EXPR1			65
#define NT_EXPR2			66
#define NT_EXPRSELECTED		67
#define NT_SELECTOR			68
#define NT_CONSTRUCTOR		69
#define NT_SETELT			70
#define NT_RECORDELT		71
#define NT_NEWEXPR			72
#define NT_xxx_WRITEINTST	73

#define MODE_VALUE 0 
#define MODE_VAR 1
#define MODE_READONLY 2
#define BRAND_NAMELESS -1
#define ARRAYELT_DOTDOT -1

#define t_node struct s_node
#define t_list struct s_list

extern long yylineno;

struct t_interface {
	t_id Id ;	
	char unsafe ;
	t_list *Imports ;
	t_list *Decls ;
	t_list *GenFmls ;
};
struct t_instance {
	t_id Id ;
	char unsafe ;
	t_id Id_generic ;
	t_list *Exports ;
	t_list *GenActls ;
};
struct t_module {
	t_id Id ;
	char unsafe ;
	t_list *Exports ;
	t_list *Imports ;
	t_list *GenFmls ;
	t_node *Block ;
};
struct t_alias {
	t_id Id ;
	t_id Alias ;
};
struct t_fromimport {
	t_id Id_from;
	t_list *IdList;
};
struct t_block {
	t_list	*Decls ;
	t_list	*S ;
};
struct t_typedecl {
	t_id Id ;
	t_node *Type ;
	char subtyping ;
};
struct t_exceptiondecl {
	t_id Id ;
	t_node *Type ;
};
struct t_vardecl {
	t_id Id ;
	t_node *Type ;
	t_node *Expr ;
	int offset ;
};
struct t_proceduredecl {
	t_id Id ;
	int counter ;
	t_node *Signature;
	t_node *Block;
};
struct t_revealdecl {
	t_node *QualId ;
	t_node *Type ;
	char subtyping ;
};
struct t_qualid {	
	t_id Id_qualifier ;
	t_id Id ;
};
struct t_signature {
	t_list *Formals ;
	t_node *Type ;
	t_list *Raises ;
	int formlen;
};
struct t_formal {
	t_id Id ;
	int	Mode ;
	t_node *Type ;
	t_node *ConstExpr ;
	int offset;
};
struct t_assignst {
	t_node *Expr_left;
	t_node *Expr_right;
};
struct t_callst {
	t_node *Expr ;
	t_list *Actuals ;
};
struct t_casest {
	t_node *Expr ;
	t_list *Cases ;
	t_list *S_else ;
};
struct t_forst {
	t_id Id ;
	t_node *Expr_from ;
	t_node *Expr_to ;
	t_node *Expr_by ;
	t_list *S ;
};
struct t_ifst {
	t_list *IfCases ;
	t_list *S_else ;
};
struct t_raisest {
	t_node *QualId ;
	t_node *Expr ;
};
struct t_tryxptst {
	t_list *S ;
	t_list *Handlers ;
	t_list *S_else ;
};
struct t_tryfinst {
	t_list *S ;
	t_list *S_final ;
};
struct t_withst {
	t_list *Bindings ;
	t_list *S ;
};
struct t_actual {
	t_id Id ;
	t_node *Expr ;
};
struct t_case {
	t_list *Labels ;
	t_list *S ;
};
struct t_label {
	t_node *ConstExpr ;
	t_node *ConstExpr_to ;
};
struct t_tcase {
	t_list *Types ;
	t_id Id ;
	t_list *S ;
};
struct t_handler {
	t_list *QualIds ;
	t_id Id ;
	t_list *S ;
};
struct t_binding {
	t_id Id ;
	t_node *Expr ;
};
struct t_arraytype {
	t_list *Types ;
	t_node *Type ;
};
struct t_packedtype {
	t_node *Type ;
	t_node *ConstExpr ;
};
struct t_objecttype {
	t_node *super ;
	t_node *Brand ;
	t_list *Fields ;
	t_list *Methods ;
	t_list *Overrides ;
};
struct t_method {
	t_id Id ;
	t_node *Signature ;
	t_node *ConstExpr ;
};
struct t_override {
	t_id Id ;
	t_node *ConstExpr ;
};
struct t_field {
	t_list *IdList ;
	t_node *Type ;
	t_node *ConstExpr ;
};
struct t_subrangetype {
	t_node *Expr_from ;
	t_node *Expr_to ;
};
struct t_reftype {
	char untraced ;
	t_node *Brand ;
	t_node *Type ;
};
struct t_selector {
	int operand ;
	t_list *Exprs;
	t_list *Actuals;
	t_id Id;
};
struct t_constructor {
	t_node *Type ;
	t_list *ConsList ;
};

struct t_expr_S {
	t_node *Expr ;
	t_list *S ;
};
struct t_expr {
	t_node *Expr ;
	t_node *Expr2 ;
	int operand ;
	t_node *Selector ;
};	
struct t_newexpr {
	int status;	
	t_node *Type ;
	t_list *Actuals ;
};

struct s_node{
	int yyline;
	int nt;
	int status;
	t_type type;
	t_env *env;
	union {
		struct t_interface interface;
		struct t_instance instance; 
		struct t_module module;
		struct t_fromimport fromimport;
		struct t_block block;
		struct t_typedecl typedecl;
		struct t_exceptiondecl exceptiondecl;
		struct t_vardecl vardecl;
		struct t_proceduredecl proceduredecl;
		struct t_revealdecl revealdecl;
		struct t_qualid qualid;
		struct t_signature signature;
		struct t_formal formal;
		struct t_assignst assignst;
		struct t_callst callst;
		struct t_casest casest;
		struct t_forst forst;
		struct t_ifst ifst;
		struct t_raisest raisest;
		struct t_tryxptst tryxptst;
		struct t_tryfinst tryfinst;	
		struct t_withst withst;
		struct t_actual actual;
		struct t_case case_ ;
		struct t_label label ;
		struct t_tcase tcase;
		struct t_handler handler;
		struct t_binding binding;
		struct t_alias alias;
		struct t_arraytype arraytype;
		struct t_packedtype packedtype;
		struct t_objecttype objecttype;
		struct t_method method;
		struct t_override override;
		struct t_field field;		
		struct t_subrangetype subrangetype;
		struct t_reftype reftype;
		struct t_selector selector;
		struct t_constructor constructor;
		struct t_newexpr newexpr;
		struct t_expr_S expr_S;
		struct t_expr expr;
		
		t_id Id ;
		t_node *Type;
		t_node *Signature;
		t_list *List;

		int intval;
		double floatval;
		int stringval;
		char charval;
		int boolval;
	} x ;
} ;
struct s_list {
	struct s_list *next;
	struct s_node *node;
} ;

t_list *list_new( t_node *node );
t_list *list_join( t_list *l1, t_list *l2 );
t_list *list_add( t_list *list, t_node *node );
t_node *new_node( int node_type );

t_id save_literal( const char *literal );
char *give_literal( t_id id );
char *give_ntname( int nt );
char *give_token( int token );
void print_tree( t_node *tree );

#endif 
