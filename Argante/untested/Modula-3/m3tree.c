//#include <stdlib.h>
#include <stdio.h>
#include "m3common.h"
#include "m3tree.h"
#include "m3types.h"
#include "m3parser.h"
#include "m3context.h"
#include "m3code.h"

t_node *m3tree;
extern int yyparse( void * );

static char *reserved_literals[LIT_RESERVED] = {
	"_unknown_",
	"ABS","ADDRESS","ADR","ADRSIZE","BITSIZE","BOOLEAN",
	"BYTESIZE","CARDINAL","CEILING","CHAR","DEC","DISPOSE",
	"EXTENDED","FALSE","FIRST","FLOAT","FLOOR","INC",
	"INTEGER","ISTYPE","LAST","LONGREAL","LOOPHOLE","MAX",
	"MIN","MUTEX","NARROW","NIL","NULL",
	"NUMBER","ORD","REAL","REFANY","ROUND","SUBARRAY",
	"TEXT","TRUE","TRUNC","TYPECODE","VAL",
/* Moje */
	"WriteInt"
	};

static char *ntnames[] = { 
	"_","Integer","Float","Char","String",
	"Id", "_","_","_","_",
	"Interface","Inst_Interface","Module","Inst_Module","AsImport",
	"FromImport","Block","ConstDecl","TypeDecl","ExceptionDecl",
	"VariableDecl","ProcedureDecl","RevealDecl","QualId","Signature",
	"Formal","AssignSt","CallSt","CaseSt","ExitSt",
	"EvalSt","ForSt","IfSt","IfCase","LockSt",
	"LoopSt","RaiseSt","RepeatSt","ReturnSt","TCaseSt",
	"TryXptSt","TryFinSt","WhileSt","WithSt","Actual",
	"Case","Label","TCase","Handler","Binding",
	"Any","Alias","ArrayType","PackedType","EnumType",
	"ObjectType","ProcedureType","RecordType","RefType","SetType",
	"SubrangeType","Method","Override","Field","Expr",
	"Expr1","Expr2","ExprSelected","Selector","Constructor",
	"SetElt","RecordElt","NewExpr","xxx_WRITEINT"
	 };
#define MAX_LITERALS 4096
#define MAX_LITERALS_LEN 65536
char *lit_tab[MAX_LITERALS];
char lit_area[MAX_LITERALS_LEN], *lit_area_top=lit_area;
int lits=0;
	
int level=0;
static char spaces[] = 
"                                                                              "
;
char *mar=spaces+78;

void print_tree( t_node *node );

void print_list( t_list *lst )
{
	t_list *l=lst;
	while(l) {
		print_tree( l->node );
		l=l->next;
	}
}
void print_id_list( t_list *lst )
{
	t_list *l=lst;
	while(l) {
		fprintf(stderr," %s",lit_tab[ (int)l->node ]);
		l=l->next;
	}
	fprintf(stderr,"\n");
}
void print_qualid( t_node *qualid )
{
	if (qualid->x.qualid.Id_qualifier)
		fprintf(stderr,"%s.%s\n",lit_tab[qualid->x.qualid.Id_qualifier],
			lit_tab[qualid->x.qualid.Id]);
	else
		fprintf(stderr,"%s\n",lit_tab[qualid->x.qualid.Id]);
}

void print_tree( t_node *node )
{
	if (!node)
		return ;
	if (node->nt != NT_ID)
		fprintf(stderr,"%s\\ %s [%d]\n",mar,ntnames[node->nt],node->yyline);
	else
		fprintf(stderr,"%s\\ Id [%d]: %s\n",mar,node->yyline,lit_tab[(int)node->x.Id]);
	mar-=4;
	switch(node->nt) {
		case NT_INTERFACE : 
			fprintf(stderr,"%sId=%s\n",mar,lit_tab[node->x.interface.Id]);
			fprintf(stderr,"%sUnsafe=%d\n",mar,node->x.interface.unsafe);
			break;
		case NT_MODULE :
			fprintf(stderr,"%sId=%s\n",mar,lit_tab[node->x.module.Id]);
			fprintf(stderr,"%sUnsafe=%d\n",mar,node->x.module.unsafe);
			fprintf(stderr,"%sExports>",mar);
				print_id_list( node->x.module.Exports );
			fprintf(stderr,"%sImports>\n",mar);
				print_list( node->x.module.Imports );
			fprintf(stderr,"%sGenFmls>",mar);
				print_id_list( node->x.module.GenFmls );
			fprintf(stderr,"%sBlock>\n",mar);
				print_tree(node->x.module.Block);
			break;
		case NT_BLOCK :
			fprintf(stderr,"%sDecls>\n",mar);
				print_list(node->x.block.Decls);
			fprintf(stderr,"%sS>\n",mar);
				print_list(node->x.block.S);
			break ;
		case NT_REVEALDECL :
			fprintf(stderr,"%sId=",mar);
				print_qualid(node->x.revealdecl.QualId);
			fprintf(stderr,"%sType>\n",mar);
				print_tree(node->x.revealdecl.Type);
			break ;
		case NT_PROCEDUREDECL :
			fprintf(stderr,"%sId=%s\n",mar,lit_tab[node->x.proceduredecl.Id]);
			fprintf(stderr,"%sSignature>\n",mar);
				print_tree(node->x.proceduredecl.Signature);
			fprintf(stderr,"%sBlock>\n",mar);
				print_tree(node->x.proceduredecl.Block);
			break;
		case NT_RETURNST :
			fprintf(stderr,"%sExpr>\n",mar);
				print_tree(node->x.expr.Expr);
			break;
		case NT_ASSIGNST :
			fprintf(stderr,"%sExpr left>\n",mar);
				print_tree(node->x.assignst.Expr_left);
			fprintf(stderr,"%sExpr right>\n",mar);
				print_tree(node->x.assignst.Expr_right);
			break ;
		case NT_INTEGER : 
			fprintf(stderr,"%sValue=%d\n",mar,node->x.intval );
			break;
		case NT_FLOAT : 
			fprintf(stderr,"%sValue=%f\n",mar,node->x.floatval );
			break;
		case NT_STRING : 
			fprintf(stderr,"%sValue='%s'\n",mar,give_literal(node->x.stringval) );
			break;
		case NT_EXPR2 : 
			fprintf(stderr,"%sOperand='%s'\n",mar,give_token(node->x.expr.operand));
			fprintf(stderr,"%sExpr left>\n",mar);
				print_tree(node->x.expr.Expr);
			fprintf(stderr,"%sExpr right>\n",mar);
				print_tree(node->x.expr.Expr2);
			break;		
	}
	mar +=4;
	return ;
}

t_node *new_node( int node_type )
{
	t_node *n;
#ifdef M3_DEBUG_NODE
	fprintf(stderr,"[%d]     n> Node type nr: %3d - %s\n",yylineno,
			node_type,ntnames[node_type]);
#endif
	fflush(stdout);
	n=my_alloc(sizeof(t_node));
	n->yyline=yylineno;
	n->nt=node_type;
	return n;
}
t_list *list_new( t_node *node )
{
	t_list *l;
	l = my_alloc( sizeof(t_list));
	l->next=0;
	l->node=node;
	return l;
}
t_list *list_join( t_list *l1, t_list *l2 )
{
	t_list *p=l1;
	if (!p)
		return l2;
	while(p->next) p = p->next ;
	p->next=l2;
	return l1;
}
t_list *list_add( t_list *list, t_node *node)
{
	return list_join(list,list_new(node));
}


t_id save_literal( const char *literal )
{
	int k=-1,slen;
	while (++k<lits)
		if (!strcmp(lit_tab[k],literal)) {
#ifdef M3_DEBUG_TOKEN
			fprintf(stderr,"[%d]       l> '%s' found at %d\n",yylineno,literal,k);
#endif
			return k;
		}
	if (lits>=MAX_LITERALS-1)
		fatal_error("MAX_LITERALS (%d)  exceeded",MAX_LITERALS);
	slen=strlen(literal);
	if (lit_area_top+slen+1-lit_area > MAX_LITERALS_LEN)
		fatal_error("MAX_LITERALS_LEN (%d) exceeded",MAX_LITERALS_LEN);
	strcpy(lit_area_top,literal);
#ifdef M3_DEBUG_TOKEN
	fprintf(stderr,"[%d]       l> '%s' added at %d\n",yylineno,literal,k);
#endif
	lit_tab[lits++]=lit_area_top;
	lit_area_top+=slen+1;
	return lits-1;
}
void print_literals()
{
	char *p=lit_area;
	int i=0;
	while(p!=lit_area_top) {
		if (i>=LIT_RESERVED)
			fprintf(stderr,",Literals[%d]=%s\n",i,p);
		i++;
		p+=strlen(p)+1;
	}
}
void init_literals()
{
	int i;
	for (i=0;i<LIT_RESERVED;i++)
		save_literal(reserved_literals[i]);
}

char *give_literal(t_id id)
{
	return lit_tab[id];
}

char *give_token( int token )
{
	switch( token ) {
		case AND : return "AND" ;
		case ANY : return "ANY" ;
		case ARRAY : return "ARRAY" ;
		case AS : return "AS" ;
		case BEGINB : return "BEGIN" ;
		case BITS : return "BITS" ;
		case BRANDED : return "BRANDED" ;
		case BY : return "BY" ;
		case CASE : return "CASE" ;
		case CONST : return "CONST" ;
		case DIV : return "DIV" ;
		case DO : return "DO" ;
		case ELSE : return "ELSE" ;
		case ELSIF : return "ELSIF" ;
		case END : return "END" ;
		case EVAL : return "EVAL" ;
		case EXCEPT : return "EXCEPT" ;
		case EXCEPTION : return "EXCEPTION" ;
		case EXIT : return "EXIT" ;
		case EXPORTS : return "EXPORTS" ;
		case FINALLY : return "FINALLY" ;
		case FOR : return "FOR" ;
		case FROM : return "FROM" ;
		case GENERIC : return "GENERIC" ;
		case IF : return "IF" ;
		case IMPORT : return "IMPORT" ;
		case IN : return "IN" ;
		case INTERFACE : return "INTERFACE" ;
		case LOCK : return "LOCK" ;
		case LOOP : return "LOOP" ;
		case METHODS : return "METHODS" ;
		case MOD : return "MOD" ;
		case MODULE : return "MODULE" ;
		case NOT : return "NOT" ;
		case OBJECT : return "OBJECT" ;
		case OF : return "OF" ;
		case OR : return "OR" ;
		case OVERRIDES : return "OVERRIDES" ;
		case PROCEDURE : return "PROCEDURE" ;
		case RAISE : return "RAISE" ;
		case RAISES : return "RAISES" ;
		case READONLY : return "READONLY" ;
		case RECORD : return "RECORD" ;
		case REF : return "REF" ;
		case REPEAT : return "REPEAT" ;
		case RETURN : return "RETURN" ;
		case REVEAL : return "REVEAL" ;
		case ROOT : return "ROOT" ;
		case SET : return "SET" ;
		case THEN : return "THEN" ;
		case TO : return "TO" ;
		case TRY : return "TRY" ;
		case TYPE : return "TYPE" ;
		case TYPECASE : return "TYPECASE" ;
		case UNSAFE : return "UNSAFE" ;
		case UNTIL : return "UNTIL" ;
		case UNTRACED : return "UNTRACED" ;
		case VALUE : return "VALUE" ;
		case VAR : return "VAR" ;
		case WHILE : return "WHILE" ;
		case WITH : return "WITH" ;
		case KEY_PLUS : return "+" ;
		case KEY_MINUS : return "-" ;
		case KEY_MUL : return "*" ;
		case KEY_DIV : return "/" ;
		case KEY_LT : return "<" ;
		case KEY_GT : return ">" ;
		case KEY_LEQ : return "<=" ;
		case KEY_GEQ : return ">=" ;
		case KEY_LBRA : return "{" ;
		case KEY_LPAR : return "(" ;
		case KEY_LSPAR : return "[" ;
		case KEY_EQ : return "=" ;
		case KEY_RBRA : return "}" ;
		case KEY_RPAR : return ")" ;
		case KEY_RSPAR : return "]" ;
		case KEY_SEMI : return ";" ;
		case KEY_PIPE : return "|" ;
		case KEY_UP : return "^" ;
		case KEY_DOT : return "." ;
		case KEY_DOTDOT : return ".." ;
		case KEY_ASSIGN : return ":=" ;
		case KEY_COMMA : return "," ;
		case KEY_AND : return "&" ;
		case KEY_COL : return ":" ;
		case KEY_SUB : return "<:" ;
		case KEY_IMP : return "=>" ;
		case KEY_NEQ : return "<> or #" ;
		default : 
			fprintf(stderr,"Unknown token %d, please add to give_token",token);
			return "";
	
	}

}


char *give_ntname( int nt )
{
	return ntnames[nt];
}

int main (void) 
{
	fprintf(stderr,"M3Compiler by eru\n");
	
	init_literals();

	yyparse(0);
#ifdef M3_PRINT_TREE
	print_tree(m3tree);
#endif
#ifdef M3_PRINT_LITERALS
	print_literals();
#endif
	analyse_tree(m3tree);
	create_code(m3tree);
	return 1;
}
