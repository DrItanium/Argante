#ifndef ARGANTE_EVALUATE_H__
#define ARGANTE_EVALUATE_H__

#define UREGVAL(W,A) 	W=curr_cpu_p->uregs[A];


#define SREGVAL(W,A) 	W=curr_cpu_p->sregs[A];


#define FREGVAL(W,A) 	W=(curr_cpu_p->fregs[A].i);

#define IMMPTRVAL(W,A)	W=get_mem_value(curr_cpu,A); if (got_nonfatal_round) return;

#define UPTRVAL(W,A)	W=get_mem_value(curr_cpu,curr_cpu_p->uregs[A]); \
                        if (got_nonfatal_round) return;

#define UREG(A)		curr_cpu_p->uregs[A]

#define SREG(A)		curr_cpu_p->sregs[A]

#define FREG(A)		curr_cpu_p->fregs[A].i

#define FFREG(A)	curr_cpu_p->fregs[A].f

#endif // end !defined(ARGANTE_EVALUATE_H__)
