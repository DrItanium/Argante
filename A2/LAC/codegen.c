#include "autocfg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lstring.h"
#include "main.h"
#include "function.h"

extern int yynerrs;

mVar *VarResolve(mVar *q, function *in) {
	mVar *r;

	if (q->kind == VarConst || q->kind == VarErrno) return q;

	if (!q->id) {
		EM_Error("unnamed var?!");
		return NULL;
	}
	if ((r=FindVarInFunc(q->id, in))) return r;
	if ((r=FindVarInFunc(q->id, global))) return r;
	yynerrs++;
	EM_Error("undefined variable %s", StringToChar(q->id));
	return NULL;
}

void PrintVar(mVar *v, int deref) {
	switch(v->kind) {
		case VarConst:
			if (v->strreftype==STRREF_ADDR) {
				fprintf(codeout, "u::%s", StringToChar(v->id));
				break;
			}
			
			if (v->type.basetype == TypeUnsigned || (v->type.pointerct > deref))
				fprintf(codeout, "u:%d", v->u.constnt_u);
			else if (v->type.basetype == TypeSigned)
				fprintf(codeout, "s:%d", v->u.constnt_s);
			else if (v->type.basetype == TypeFloat)
				fprintf(codeout, "f:%f", v->u.constnt_f);
			else {
				fprintf(codeout, "ERR:???");
				EM_Error("internal error? (PrintVar)");
			}
			break;
		case VarRegister:
			if (v->type.basetype == TypeUnsigned || (v->type.pointerct > deref))
				fprintf(codeout, "u:");
			else if (v->type.basetype == TypeSigned)
				fprintf(codeout, "s:");
			else if (v->type.basetype == TypeFloat)
				fprintf(codeout, "f:");
			else {
				fprintf(codeout, "ERR:");
				EM_Error("internal error? (PrintVar)");
			}
			fprintf(codeout, "r%d", v->u.registr);
			break;
		case VarErrno:
			fprintf(codeout, "u:r%d", ERRNO_REG);
			break;
		case VarGlobal:
			if (v->type.basetype == TypeUnsigned || (v->type.pointerct > deref))
				fprintf(codeout, "*u:");
			else if (v->type.basetype == TypeSigned)
				fprintf(codeout, "*s:");
			else if (v->type.basetype == TypeFloat)
				fprintf(codeout, "*f:");
			else {
				fprintf(codeout, "*ERR:");
				EM_Error("internal error? (PrintVar)");
			}
			fprintf(codeout, ":%s", StringToChar(v->id));
			break;
	}
}

void PrintUVar(mVar *v) {
	switch(v->kind) {
		case VarConst:
			if (v->strreftype==STRREF_ADDR)
				fprintf(codeout, "u::%s", StringToChar(v->id));
			else
				fprintf(codeout, "u:%d", v->u.constnt_u);
			break;
		case VarRegister:
			fprintf(codeout, "u:r%d", v->u.registr);
			break;
		case VarErrno:
			fprintf(codeout, "u:r%d", ERRNO_REG);
			break;
		case VarGlobal:
			fprintf(codeout, "*u::%s", StringToChar(v->id));
			break;
	}
}


static inline int does_fn_contain_code(function *f) {
	Stm *s;
	s=f->code;
	while(s) {
		if (s->kind != StmVarDef && s->kind != StmLabel &&
			!(s->kind == StmReglist && !s->u.reglist.r) /* odd BUG */ ) {
			return 1;
		}
		s=s->next;
	}
	return 0;
}

void GenStackMan_Main() {
	fprintf(codeout, ".define STACKGROW 32\n");
	if (!does_fn_contain_code(global)) return;
	fprintf(codeout, 
		".define INITSTACKSZ 64\n"
		":_regstack_ptr\n"
			"\tINITSTACKSZ\n"
		":_regstack_size\n"
			"\tINITSTACKSZ\n"
		":_regstack_free\n"
			"\tINITSTACKSZ\n");
}

/* The stack management code can use NO REGISTERS.
 * Zilch. Zip. Zero. That would clobber stuff. */
void GenStackMan_CallStart(int reg_start, int reg_stop) {
	int i;
	fprintf(codeout, 
			/* Need to allow symbol to have content,
			 * otherwise NAGT calls it a prototype :( */
		"\tifabo *:_regstack_free, %d\n"
		"{\tjmp :_init_b\n" /* Also have to put jmp in same block as def,
				       or NAGT prototypes it globally (cute, that) */
		"\tadd *:_regstack_free, STACKGROW\n"
		"\tadd *:_regstack_size, STACKGROW\n"
		"\trealloc *:_regstack_ptr, *:_regstack_size\n"
		"\tstack *:_regstack_ptr, *:_regstack_size\n"
		"\n:_init_b\n"
		"\tsub *:_regstack_free, %d\n"
		"# End stackman code\n",
		reg_stop - reg_start,
		reg_stop-reg_start);
	for(i=reg_start;i<reg_stop;i++) {
		fprintf(codeout, "\tpush u:r%d\n", i); /* I hope 'u' can hold everything */
	}
}

void GenStackMan_CallStop(int reg_start, int reg_stop) {
	int i;
	fputs(":_end\n", codeout);
	i=reg_stop;
	while(i > reg_start) {
		i--;
		fprintf(codeout, "\tpop u:r%d\n", i); /* I hope 'u' can hold everything */
	}
	fprintf(codeout, "\tadd *:_regstack_free, %d\n"
		"\tret 0\n", reg_stop - reg_start);
}
/*
static int compare_types(mType *a, mType *b) {
	if (a->basetype == TypeIgnore || b->basetype == TypeIgnore) EM_Error("bad use of ignore");
	if (a->pointerct != b->pointerct) return 1;
	if (a->basetype != b->basetype) return 1;
	return 0;
} */

void CodeGen(function *f) {
	Stm *s;
	mVar *a, *b;
	int i;
	
	if (f->id) {
		fprintf(codeout, ":%s\n", StringToChar(f->id));
		GenStackMan_CallStart(f->declregs, f->usedregs);
	} else {
		/* Test to see if global func contains any code. If not,
		 * we're an object, we don't need our own main stackman code. */
		if (does_fn_contain_code(f)) {
			fprintf(codeout, "\talloc *:_regstack_ptr, 3\n"
				"\tstack *:_regstack_ptr, *:_regstack_size\n");
		}
	}
	EM_FileName=f->code->file;
	s=f->code;
	while(s) {
		EM_LineNo=s->lineno;
		switch(s->kind) {
			case StmVarDef: /* Pointless little test */
				if(f != global && FindVarInFunc(s->u.vardef.id, global))
					EM_Warn("%s shadows a global", StringToChar(s->u.vardef.id));
				break;
			case StmWait:
				fputs("\tWAIT ", codeout);
				a=VarResolve(s->u.wait.w, f);
				if (!a) goto next;
				if (a->type.basetype != TypeUnsigned) EM_Warn("wait needs unsigned value, casting.");

				if (s->u.assign.to->ptrderef) {
					putc('*', codeout);
					if (!a->type.pointerct) EM_Warn("dereferencing nonpointer");
				}
				switch(a->kind) {
					case VarConst:
						if (a->strreftype==STRREF_ADDR) {
							fprintf(codeout, "u::%s", StringToChar(a->id));
							break;
						}
						fprintf(codeout, "u:%d", a->u.constnt_u);
					break;
					case VarRegister:
					fprintf(codeout, "u:");
					fprintf(codeout, "r%d", a->u.registr);
					break;
					case VarErrno:
					fprintf(codeout, "u:r%d", ERRNO_REG);
					break;
					case VarGlobal:
					fprintf(codeout, "*u:");
					fprintf(codeout, ":%s", StringToChar(a->id));
					break;
				}
				break;
			case StmRaise:
				fputs("\tRAISE ", codeout);
				a=VarResolve(s->u.wait.w, f);
				if (!a) goto next;
				if (a->type.basetype != TypeUnsigned) EM_Warn("raise needs unsigned value, casting.");

				if (s->u.assign.to->ptrderef) {
					putc('*', codeout);
					if (!a->type.pointerct) EM_Warn("dereferencing nonpointer");
				}
				switch(a->kind) {
					case VarConst:
						if (a->strreftype==STRREF_ADDR) {
							fprintf(codeout, "u::%s", StringToChar(a->id));
							break;
						}
						fprintf(codeout, "u:%d", a->u.constnt_u);
					break;
					case VarRegister:
					fprintf(codeout, "u:");
					fprintf(codeout, "r%d", a->u.registr);
					break;
					case VarErrno:
					fprintf(codeout, "u:r%d", ERRNO_REG);
					break;
					case VarGlobal:
					fprintf(codeout, "*u:");
					fprintf(codeout, ":%s", StringToChar(a->id));
					break;
				}
				break;
			case StmAssign:
				a=VarResolve(s->u.assign.to, f);
				b=VarResolve(s->u.assign.from, f);
				if (!a || !b) goto next;
				if (a->kind == VarConst) EM_Error("assigning to constant?");

				putc('\t', codeout);
				switch(s->u.assign.kind) {
					case AEq:
						if ((a->type.pointerct - s->u.assign.to->ptrderef)
							!= (b->type.pointerct - s->u.assign.from->ptrderef))
							EM_Warn("pointer mismatch in =");
						else if ((a->type.pointerct - s->u.assign.to->ptrderef)
							&& a->type.basetype != b->type.basetype)
							EM_Warn("pointer basetype mismatch in =");
						
						fputs("MOV", codeout);
						break;
					case APlus: fputs("ADD", codeout); break;
					case AMinus: fputs("SUB", codeout); break;
					case AMul: fputs("MUL", codeout); break;
					case ADiv: fputs("DIV", codeout); break;
					case AXor: fputs("XOR", codeout); break;
					case AOr: fputs("OR", codeout); break;
					case AAnd: fputs("AND", codeout); break;
					case AShl: fputs("SHL", codeout); break;
					case AShr: fputs("SHR", codeout); break;
					case ARol: fputs("ROL", codeout); break;
					case ARor: fputs("ROR", codeout); break;
					case AMod: fputs("MOD", codeout); break;
					default:
						   fputs("UNKNOWN_ASSIGN", codeout);
						   EM_Error("internal error? (unknown assignment)");
				}
				putc(' ', codeout);
				
				if (s->u.assign.to->ptrderef) {
					putc('*', codeout);
					if (!a->type.pointerct) EM_Warn("dereferencing nonpointer");
					PrintVar(a,1); 
				} else PrintVar(a, 0); 
				fputs(", ", codeout);

				if (s->u.assign.from->ptrderef) {
					putc('*', codeout);
					if (!b->type.pointerct) EM_Warn("dereferencing nonpointer");
					PrintVar(b,1); 
				} else PrintVar(b, 0); 
				break;
			case StmIf:
				a=VarResolve(s->u.sif.a1, f);
				b=VarResolve(s->u.sif.a2, f);
				if (!a || !b) goto next;

				if ((a->type.pointerct - s->u.sif.a1->ptrderef)
					!= (b->type.pointerct - s->u.sif.a2->ptrderef))
					EM_Warn("pointer mismatch in compare");
				else if ((a->type.pointerct - s->u.sif.a1->ptrderef)
					&& a->type.basetype != b->type.basetype)
					EM_Warn("pointer basetype mismatch in compare");

				putc('\t', codeout);
				switch(s->u.sif.kind) {
					case IEq:
						fputs("IFEQ", codeout);
						break;
					case INEq:
						fputs("IFNEQ", codeout);
						break;
					case IGt:
						fputs("IFABO", codeout);
						break;
					case ILt:
						fputs("IFBEL", codeout);
						break;
					default:
						fputs("IFWTF", codeout);
						EM_Error("internal error? (unknown comparison)");
				}
				putc(' ', codeout);

				if (s->u.sif.a1->ptrderef) {
					putc('*', codeout);
					if (!a->type.pointerct) EM_Warn("dereferencing nonpointer");
					PrintVar(a,1); 
				} else PrintVar(a, 0); 
				fputs(", ", codeout);

				if (s->u.sif.a2->ptrderef) {
					putc('*', codeout);
					if (!b->type.pointerct) EM_Warn("dereferencing nonpointer\n");
					PrintVar(b,1); 
				} else PrintVar(b, 0); 
				fprintf(codeout, "\n\tJMP :%s", StringToChar(s->u.sif.label));
				break;
			case StmReglist: {
				RegList *rg;
				i=0;
				rg=s->u.reglist.r;
				if (s->u.reglist.kind == RSet) {
					while(rg) {
						a=VarResolve(rg->var, f);
						if (!a) goto next;
						if (a->type.basetype != TypeIgnore) {
						fprintf(codeout, "\tMOV %c:r%d, ", 
							(a->type.basetype == TypeUnsigned ||
							 a->type.pointerct > rg->var->ptrderef) ? 'u' :
							(a->type.basetype == TypeSigned) ? 's' :
							(a->type.basetype == TypeFloat) ? 'f' : '!',
							i);
						
						if (rg->var->ptrderef) {
							putc('*', codeout);
							if (!a->type.pointerct)
								EM_Warn("dereferencing nonpointer");
							PrintVar(a,1); 
						} else
							PrintVar(a, 0); 
							
						if (rg->next) putc('\n', codeout);
						}
						i++;
						rg=rg->next;
					}
				} else {
					while(rg) {
						a=VarResolve(rg->var, f);
						if (!a) goto next;
						if (a->type.basetype != TypeIgnore) {
							if (a->kind == VarConst) EM_Error("assigning to constant?");
							fputs("\tMOV ", codeout);
							if (rg->var->ptrderef) {
								putc('*', codeout);
								if (!a->type.pointerct)
									EM_Warn("dereferencing nonpointer");
								PrintVar(a,1); 
							} else
								PrintVar(a, 0); 
							fprintf(codeout, ", %c:r%d",  
								(a->type.basetype == TypeUnsigned ||
								 a->type.pointerct > rg->var->ptrderef) ? 'u' :
								(a->type.basetype == TypeSigned) ? 's' :
								(a->type.basetype == TypeFloat) ? 'f' : '!',
								i);
							if (rg->next) putc('\n', codeout);
						}
						i++;
						rg=rg->next;
					}
				}
				break; }
			case StmLabel:
				fprintf(codeout, ":%s", StringToChar(s->u.label));
				break;
			case StmHandler:
				fprintf(codeout, "\tHANDLER :%s", StringToChar(s->u.label));
				break;
			case StmGoto:
				fprintf(codeout, "\tJMP :%s", StringToChar(s->u.label));
				break;
			case StmCall:
				if (Call_IsSyscall(s->u.func)) {
					fprintf(codeout, "\tSYSCALL $%s", StringToChar(s->u.func));
				} else {
					fprintf(codeout, "\tCALL :%s", StringToChar(s->u.func));
				}
				break;
			case StmReturn:
				fputs("\tJMP :_end", codeout);
				break;
			case StmFinalize:
			case StmUnfinalize:
				a=VarResolve(s->u.fin.addr, f);
				if (!a) goto next;
				if (a->kind == VarConst) EM_Warn("changing the permissions of a constant");
				putc('\t', codeout);
				fputs("REALLOC ", codeout);
				
				if (s->u.fin.addr->ptrderef) {
					putc('*', codeout);
					if (!a->type.pointerct) EM_Warn("dereferencing nonpointer\n");
				}	
				PrintUVar(a); 
				
				fputs(", s:", codeout);
				if (s->kind == StmFinalize)
					fputc('1', codeout); /* read only */
				else
					fputc('3', codeout); /* read write */
				break;
			case StmDealloc:
				a=VarResolve(s->u.fin.addr, f);
				if (!a) goto next;
				if (a->kind == VarConst) EM_Warn("freeing a constant");
				putc('\t', codeout);
				fputs("FREE ", codeout);
				
				if (s->u.fin.addr->ptrderef) {
					putc('*', codeout);
					if (!a->type.pointerct) EM_Warn("dereferencing nonpointer\n");
				}	
				PrintUVar(a); 
				break;
			case StmAlloc:
			case StmRealloc:
				a=VarResolve(s->u.alloc.addrdest, f);
				b=VarResolve(s->u.alloc.size, f);
				if (!a || !b) goto next;
				if (a->kind == VarConst) EM_Error("assigning to constant?");
				if (!a->type.pointerct) EM_Warn("allocating into a nonpointer?");

				putc('\t', codeout);
				if (s->kind == StmAlloc) {
					fputs("MOV ", codeout);
					if (s->u.alloc.addrdest->ptrderef) {
						putc('*', codeout);
						if (!a->type.pointerct) EM_Warn("dereferencing nonpointer\n");
					}
					PrintUVar(a);
					fputs(", ", codeout);
					if (s->u.alloc.size->ptrderef) {
						putc('*', codeout);
						if (!b->type.pointerct) EM_Warn("dereferencing nonpointer\n");
					}
					PrintUVar(b);
					fputs("\n\tALLOC ", codeout);
					if (s->u.alloc.addrdest->ptrderef) {
						putc('*', codeout);
						if (!a->type.pointerct) EM_Warn("dereferencing nonpointer\n");
					}
					PrintUVar(a);
					fputs(", u:3", codeout);
				} else {
					fputs("REALLOC ", codeout);
					if (s->u.alloc.addrdest->ptrderef) {
						putc('*', codeout);
						if (!a->type.pointerct) EM_Warn("dereferencing nonpointer\n");
					}
					PrintUVar(a);
					fputs(", ", codeout);
					if (s->u.alloc.size->ptrderef) {
						putc('*', codeout);
						if (!b->type.pointerct) EM_Warn("dereferencing nonpointer\n");
					}
					PrintUVar(b);
				}
				break;
			default:
				goto next2;
		}
next:
		putc('\n', codeout);
next2:
		s=s->next;
	}
	if (f->id) {
		GenStackMan_CallStop(f->declregs, f->usedregs);
		fputs("}\n", codeout);
	} else {
		/* Still write a HALT command into objects - the other objects
		 * might not be too well behaved and might run into us... */
		fputs("\tHALT\n", codeout);
	}
}

void GenGlobals() {
	mVarH *h;
	function *f=global;
	int i;
	for(i=0;i<f->varhashsz;i++) {
		h=f->varhash[i];
		while(h) {
			fprintf(codeout, ":%s\n\t0\n", StringToChar(h->v->id));
			h=h->next;
		}
	}
}

void GenAllCode() {
	function *f;
	/* Do global first! V.Imp! */
	CodeGen(global);
	f=chain;
	while(f) {
		if (f!=global) CodeGen(f);
		f=f->next;
	}
}


