
#define UREGVAL(W,A) 	W=cpu[c].uregs[A];


#define SREGVAL(W,A) 	W=cpu[c].sregs[A];


#define FREGVAL(W,A) 	W=*((int*)&cpu[c].fregs[A]);

#define IMMPTRVAL(W,A)	W=get_mem_value(c,A); if (got_nonfatal_round) return;

#define UPTRVAL(W,A)	W=get_mem_value(c,cpu[c].uregs[A]); \
                        if (got_nonfatal_round) return;

#define UREG(A)		cpu[c].uregs[A]

#define SREG(A)		cpu[c].sregs[A]

#define FREG(A)		*((int*)&cpu[c].fregs[A])

#define FFREG(A)	cpu[c].fregs[A]
