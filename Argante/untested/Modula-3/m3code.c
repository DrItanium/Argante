#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include "m3common.h"
#include "m3tree.h"
#include "m3types.h"
#include "m3parser.h"
#include "m3context.h"
#include "m3code.h"
#include "m3agt.h"

/* AR Structure */
#define AR_OFFS_VAR	0
#define AR_OFFS_DL 	1
#define AR_OFFS_ACT 2
#define AR_OFFS_SL	3
#define AR_OFFS_RES 4
#define AR_OFFS_EXC 5

static struct agt_cpus agt_cpus_data;
static struct agt_cpus *cpus=&agt_cpus_data;
static t_cpu AR_cpu;

int out_fd;


void pastefile(const char * fname)
{
char BUF[4096];
size_t in;
int f = open(fname,O_RDONLY);
if (!f)
	fatal_error("Can't open file %s",fname);
while ((in=read(f,BUF,512))) 
	write(out_fd,BUF,in);
close(f);
} /* sp_junction */

int unique(void)
{
	static int u=0;
	return ++u;
}

static char buf[4096];
void _voutput_( char *s, va_list vl )
{
	vsnprintf(buf,4096,s,vl);
	write(out_fd,buf,strlen(buf));
}
void _output_( char *s, ... )
{
	va_list vl;
	va_start(vl,s);
	_voutput_(s,vl);
	va_end(vl);
}

void LOG( char *s, ...)
{
	va_list vl ;
	time_t ttt=time(0);
	char *ct=ctime(&ttt);
	ct[strlen(ct)-1]=0; /* drop \n */
	
	_output_("#[%s] ",ct);
	va_start(vl,s);
	_voutput_(s,vl);
	va_end(vl);
	_output_("\n");
}

void COMM( char *s, ...)
{
	va_list vl;
	_output_("#");
	va_start(vl,s);
	_voutput_(s,vl);
	va_end(vl);
	_output_("\n");
}

void GEN( char *s, ... )
{
	va_list vl;
	if (!s || !s[0]) {
		_output_("\n");
		return ; 
	}
	if(s[0]!=':' && s[0]!='.' && s[0]!='\n')
		_output_("\t");
	va_start(vl,s);
	_voutput_(s,vl);
	va_end(vl);
	_output_("\n");
}	
void LINE(void)
{
	_output_("#-----------------------------------------\n");
}

void create_proc( t_node *proc, t_env *env )
{
	t_node *bl,*sig;
	t_id id ;
	int cnt;

	assert(proc);

	bl=proc->x.proceduredecl.Block;
	id=proc->x.proceduredecl.Id;
	sig=proc->x.proceduredecl.Signature;
	cnt=proc->x.proceduredecl.counter;
	if (!bl) {
		if (id < LIT_RESERVED) {
			/* xman, a co jesli formals niezgone ? */
			warning("Reserved procedure '%s' declared", give_literal(id));
		} else {
			warning("Procedure '%s' doesn't have body", give_literal(id));
			SAYONARA;
		}
	}
	COMM("--- BEGIN PROCEDURE %s[%d] ---",give_literal(id),cnt);
	GEN(":%s_%d",give_literal(id),cnt);
	create_block( bl );
	COMM("--- END PROCEDURE %s[%d] ---",give_literal(id),cnt);
}

void create_procs( t_env *env )
{
	t_list *procs=env->proceduredecls;
	while(procs) {
		create_proc( procs->node, env );
		procs=procs->next ;
	}
} 

int prepare_vars( t_list *vardecls )
{
	t_list *vd=vardecls;
	t_node *n;
	int varlen=0;
	while(vd) {
		n=vd->node;
		n->x.vardecl.offset=varlen;
		if (n->type != TYPE_INTEGER && n->type != TYPE_FLOAT)
			fatal_error("must be INTEGER or FLOAT");
		varlen++;
		vd=vd->next;
	}
	fprintf(stderr,"Block has len=%d\n",varlen);
	return varlen;
}
void init_vars( t_list *vardecls )
{
	t_list *vd=vardecls;
	t_node *n,*ex;
	int offs;
	while(vd) {
		n=vd->node;
		ex=n->x.vardecl.Expr;
		if (ex) {
			COMM("Initialising variable %s",give_literal(n->x.vardecl.Id));
			GEN("mov u14,*%s",cpun(AR_cpu));
			offs=n->x.vardecl.offset;
			if (offs)
				GEN("add u14,%d",offs);
			if (ex->nt==NT_INTEGER) {
				GEN("mov *u14,%d",ex->x.intval);
			} else if (ex->nt==NT_FLOAT) {
				GEN("mov *u14,%f",ex->x.floatval);
			} else
				fatal_error("Consts can only be int'n'float");
		}
		vd=vd->next;
	}
}

void create_catchvar( t_cpu ucpu, t_id id, t_env *env )
{
	int depth=0,offs,u;
	t_node *n;
	
	n=find_id_depth(id,env,&depth);
	
	if (!n)
		fatal_error("Can't find id '%s'",give_literal(id));
	offs=n->x.vardecl.offset;
	COMM("Catching address of variable '%s', SL depth=%d, offs=%d",
		give_literal(id), depth, offs);
	if (!depth) { /* local variable */
		GEN("mov %s,*%s",cpun(ucpu),cpun(AR_cpu));
		offs=n->x.vardecl.offset;
		if(offs)
			GEN("add %s,%d",cpun(ucpu),offs);
	} else { /* somewhere in surrounding environment */
		if (depth > 1) { /* loop */
			u=unique();
			GEN("mov u0,%d",depth-1);
			GEN(":loo_%d",u);
		}
		GEN("mov %s,%s",cpun(ucpu),cpun(AR_cpu));	/* ucpu: current *(AR) */
		GEN("add %s,%d",cpun(ucpu),AR_OFFS_SL);		/* ucpu: current *(AR.SL) */
		GEN("mov %s,*%s",cpun(ucpu),cpun(ucpu));	/* ucpu: current AR.SL = top *AR */
		if (depth > 1) {
			GEN("loop :loo_%d",u); 
		} 
		/* now in cpu we have pointer to AR with id declared */
		GEN("mov %s,*%s",cpun(ucpu),cpun(ucpu));	/* ucpu: pointer to VAR with id */
		if (offs)
			GEN( "add %s,%d",ucpu,offs);			/* ucpu: pointer to id */
	}
	COMM("'%s'catched",give_literal(id));
} /* create_catchvar */

/****************************WARNING****************************\
* create_expression returns t_cpu, but caller HAS TO FREE IT!!! *
\***************************************************************/
t_cpu create_expression( t_node *expr, t_env *env ) 
{
	t_cpu c,c1,c2;
	char kind,k1,k2;
	t_node *e,*e1,*e2,*decl;
	t_id id ;
	int op;
	
	assert(expr);		

	switch(expr->nt) {
		case NT_EXPR : /* i guess only ( expr ) */
			e=expr->x.expr.Expr;
			return create_expression(e,env);
		case NT_EXPR1 : /* like +expr, -expr... */
			e=expr->x.expr.Expr;
			op=expr->x.expr.operand;
			switch( op ) {
				case KEY_PLUS :
					return create_expression(e,env); /* nothing to do */
				case KEY_MINUS :
					c1=create_expression(e,env);
					k1=cpu_kind(c1);
					c=cpu_get(k1,cpus);
					GEN("mov %s,0",cpun(c));
					GEN("sub %s,%s",cpun(c),cpun(c1));
					cpu_free(c1,cpus);
					return c1;

				break ;
				default :
					warning("Binary operand %s unimplemented",give_token( op ));
					SAYONARA;
			}
			break ;	/* NT_EXPR1 */
		case NT_EXPR2 :
			e1=expr->x.expr.Expr;
			e2=expr->x.expr.Expr2;
			op=expr->x.expr.operand;
			COMM("Generating '%s' expression from line %d",give_token(op),
				expr->yyline);
			switch( op ) {
				case KEY_PLUS :
				case KEY_MINUS :
				case KEY_MUL :
				case KEY_DIV :
				case DIV :
					c1=create_expression(e1,env);
					c2=create_expression(e2,env);
					k1=cpu_kind(c1);
					k2=cpu_kind(c2);
					if ( op==MOD || op==DIV )
						kind='s';
					else if ( op==KEY_DIV || k1=='f' || k2=='f' ) 
						kind='f';
					else	
						kind='s';
					c=cpu_get( kind, cpus );
					GEN("mov %s,%s", cpun(c), cpun(c1));
					switch(op) {
						case KEY_PLUS :
							GEN("add %s,%s",cpun(c),cpun(c2)); break;
						case KEY_MINUS :
							GEN("sub %s,%s",cpun(c),cpun(c2)); break;
						case KEY_MUL :
							GEN("mul %s,%s",cpun(c),cpun(c2)); break;
						default :
							GEN("div %s,%s",cpun(c),cpun(c2)); 
					}
					cpu_free( c1, cpus );
					cpu_free( c2, cpus );
					return c;
				default :
					warning("Expr2 '%s' unimplemented",give_token(expr->x.expr.operand));
					SAYONARA;
					break ;
			} 
			break ; /* NT_EXPR2 */
		case NT_ID : 
			id = expr->x.Id ;
			decl = find_id( expr->x.Id, env );
			if ( !decl )
				fatal_error( "Id '%s' unknown in line %d", 
					give_literal(id), expr->yyline);
			c1=cpu_getu( cpus );
			create_catchvar( c1, id, env );
			switch(decl->type) {
				case TYPE_INTEGER :
					kind='s'; break;
				case TYPE_FLOAT :
					kind='f'; break;
				default :
					fatal_error("unknown type %d of id %s",decl->type,give_literal(id));
			}
			c=cpu_get( kind, cpus );
			COMM("ID '%s' assignment",give_literal( id ));
			GEN("mov %s,*%s",cpun(c),cpun(c1));
			cpu_free(c1,cpus);
			return c;
			break ; /* NT_ID */
		case NT_INTEGER :
			c=cpu_gets( cpus );
			COMM("Used INTEGER constant %d",expr->x.intval);
			GEN("mov %s,%d",cpun(c),expr->x.intval);
			return c;
			break ; /* NT_INTEGER */
		case NT_FLOAT :
			c=cpu_getf( cpus );
			COMM("Used FLOAT constant %f",expr->x.floatval);
			GEN("mov %s,%f",cpun(c),expr->x.floatval);
			return c;
			break ; /* NT_INTEGER */
		default :
			warning("create_expression(%s) unimplemented",give_ntname(expr->nt));
			SAYONARA;
			break;
	}

//	SAYONARA;
} /* create_expression */

void create_assignst( t_node *stmt )
{
	t_node *e1,*e2,*decl;
	t_cpu target_cpu,expr_cpu,fcpu;
	
	assert(stmt);
	
	e1=stmt->x.assignst.Expr_left;
	e2=stmt->x.assignst.Expr_right;
	if (e1->nt != NT_ID ) {
		warning("Assignment can only be done to IDs at line %d", e1->yyline);
		SAYONARA;
	}
	COMM("Assigning to '%s' at line %d",give_literal(e1->x.Id),e1->yyline);
	expr_cpu=create_expression( e2, stmt->env );
	target_cpu=cpu_getu( cpus );
	create_catchvar( target_cpu, e1->x.Id, stmt->env );
	decl=find_id( e1->x.Id, stmt->env );
	if (( decl->type == TYPE_FLOAT && cpu_kind(expr_cpu) == 'f')
	|| ( decl->type == TYPE_INTEGER && cpu_kind(expr_cpu) == 's')) {
		GEN("mov *%s,%s",cpun(target_cpu),cpun(expr_cpu));
	} else /* decl-float, expr-integer, needed conversion */ {
		fcpu=cpu_getf( cpus );
		COMM("Converting from int to float");
		GEN("mov %s,%s",cpun(fcpu),cpun(expr_cpu));
		GEN("mov *%s,%s",cpun(target_cpu),cpun(fcpu));
		cpu_free( fcpu, cpus );
	}
	cpu_free( expr_cpu, cpus );
	cpu_free( target_cpu, cpus );
} /* create_assignst */

void create_xxx_writeintst( t_node *stmt )
{
	t_node *ex;
	t_cpu expr_cpu;
	char kind;
	
	assert(stmt);
	ex=stmt->x.expr.Expr;
	COMM("Prepare expression to print...");
	expr_cpu = create_expression( ex, stmt->env );
	kind=cpu_kind(expr_cpu);
	if ( kind != 's' ) 
		{	warning("Kind of proc for xxx_WRITEINT must be 's'"); SAYONARA; }
	COMM("Print out number");
	GEN("mov u0,%s",cpun(expr_cpu));
	GEN("syscall $IO_PUTINT");
	cpu_free(expr_cpu,cpus);
} /* create_xxx_writeintst */


void create_stmt( t_node *stmt )
{
	assert(stmt);

	switch(stmt->nt) {
		case NT_ASSIGNST :
			create_assignst( stmt );
			break;
		case NT_xxx_WRITEINTST :
			create_xxx_writeintst( stmt );
			break ;
		default :
			warning("create_stmt incomplete (%s)", give_ntname(stmt->nt));
			SAYONARA;
	}
}

void create_block( t_node *bl )
{
	int varlen;
	t_list *vl,*sl;
	vl=bl->env->vardecls;
	varlen=prepare_vars( vl );
	
/* allocate space for variables */
	COMM("Allocating space for variables - total size=%d",varlen);
	if (varlen) {
		GEN("alloc %d,3",varlen);
		GEN("mov *u15,u1"); /* xman - obvious memory leak - no freeing */
	}
/* initialise variables */
	create_procs( bl->env );
	sl=bl->x.block.S;
	init_vars( vl );
	while(sl) {
		create_stmt( sl->node );
		sl=sl->next;
	}
}

void header(void) {
	pastefile(M3HeaderFile);
	LOG("MAG - Modula ArGante Compiler by eru");
	LOG("Starting generating code...");
	LOG("Compiling for Argante with %d processors",AGT_CPU_CNT);
	LOG("Processor used for AR - %s\n",cpun(AR_cpu));
	LINE();
	COMM("Allocating space for stack, saving ptr to beginning of it in %s",cpun(AR_cpu));
	GEN("mov u0,*:M3SMax");
	GEN("alloc u0,3");
	GEN("mov *:M3SId,u0");
	GEN("mov *:M3Stack,u1");
	GEN("mov %s,u1",cpun(AR_cpu));
}
void footer(void) {
	LINE();
	LOG("Finished generating code");
	pastefile(M3FooterFile);
}
void create_code( t_node *tree )
{
	info("Started creating code");
	out_fd=1; /* stdout */
	AR_cpu=cpu_init(cpus);
	header();
	if (!tree)
		return ;
	if (tree->nt == NT_MODULE) {
		COMM("Parsing TOP LEVEL module");
		create_block( tree->x.module.Block );
	}
	else
		fatal_error("Must be module to create code");
	footer();
	info("Finished creating code");
} /* create_code */

