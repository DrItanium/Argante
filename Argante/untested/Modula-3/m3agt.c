#include "m3agt.h"
#include "m3common.h"

#define AGT_CPU_NEEDED 8
#define AGT_CPU_BOTTOM 4

#define AGT_CPU_U_OFFS (0*AGT_CPU_CNT)
#define AGT_CPU_S_OFFS (1*AGT_CPU_CNT)
#define AGT_CPU_F_OFFS (2*AGT_CPU_CNT)

#define CPU_STR_LEN 5
#define CPU_STR_BUFLEN 4
static char cpu_str_buf[CPU_STR_LEN][CPU_STR_BUFLEN];
static int cpu_str_pos=0;

char cpu_kind( t_cpu cpu )
{
	return cpu_kind_nr( cpu, 0 );
}

char cpu_kind_nr( t_cpu cpu, int *nr )
{
	if (cpu>=AGT_CPU_U_OFFS && cpu<=AGT_CPU_U_OFFS+AGT_CPU_MAX) {
		if (nr) *nr=cpu-AGT_CPU_U_OFFS;
		return 'u';
	} else if (cpu>=AGT_CPU_S_OFFS && cpu<=AGT_CPU_S_OFFS+AGT_CPU_MAX) {
		if (nr) *nr=cpu-AGT_CPU_S_OFFS;
		return 's';
	} else if (cpu>=AGT_CPU_F_OFFS && cpu<=AGT_CPU_F_OFFS+AGT_CPU_MAX) {
		if (nr) *nr=cpu-AGT_CPU_F_OFFS;
		return 'f';
	} else
		fatal_error("cpu %d from out of range (%d..%d)",cpu,
			AGT_CPU_U_OFFS,AGT_CPU_F_OFFS+AGT_CPU_MAX);
	return 0;
}

/* Buforowane do CPU_STR_BUFLEN wywolan */
char* cpun( t_cpu cpu ) 
{	
	int cpunr;
	char k=cpu_kind_nr(cpu,&cpunr);
	cpu_str_pos++;
	cpu_str_pos %= CPU_STR_BUFLEN;
	snprintf(cpu_str_buf[cpu_str_pos],CPU_STR_LEN,"%c%d",k,cpunr);
	return cpu_str_buf[cpu_str_pos];
}

/* inicjuje i zwraca numer ostatniego unsigned (na AR)*/
t_cpu cpu_init( struct agt_cpus *cpus )
{
	if (AGT_CPU_CNT < AGT_CPU_NEEDED)	
		fatal_error("Must be compiled for Argante machines with at least %d processors (is %d)",
			AGT_CPU_NEEDED, AGT_CPU_CNT);
	memset(cpus->u_cpus,0,sizeof(int)*AGT_CPU_CNT);
	memset(cpus->s_cpus,0,sizeof(int)*AGT_CPU_CNT);
	memset(cpus->f_cpus,0,sizeof(int)*AGT_CPU_CNT);
	return cpu_getu( cpus );
}

t_cpu cpu_get( char kind, struct agt_cpus *cpus )
{
	int i;
	
	switch( kind ) {
		case 'u' :
			for (i=AGT_CPU_MAX;i>AGT_CPU_BOTTOM;i--) {
				if (!cpus->u_cpus[i]) {
					cpus->u_cpus[i]=1;
					return i + AGT_CPU_U_OFFS;
					}
			}
			break; 
		case 's' :
			for (i=AGT_CPU_MAX;i>AGT_CPU_BOTTOM;i--) {
				if (!cpus->s_cpus[i]) {
					cpus->s_cpus[i]=1;
					return i + AGT_CPU_S_OFFS;
				}
			}
			break ;
		case 'f':
			for (i=AGT_CPU_MAX;i>AGT_CPU_BOTTOM;i--) {
				if (!cpus->f_cpus[i]) {
					cpus->f_cpus[i]=1;
					return i + AGT_CPU_F_OFFS;
				}
			}
			break ;
		default :
			fatal_error("Unknown cpu type '%c'",kind);
	}	
	fatal_error("No empty processors of type '%c'", kind );
	return 0;
}

void cpu_free( t_cpu cpu, struct agt_cpus *cpus )
{
	int nr;
	char kind=cpu_kind_nr( cpu, &nr );
	switch (kind) {
		case 'u' :
			if (cpus->u_cpus[nr]) {
				cpus->u_cpus[nr]=0; return;
			} 
			break;
		case 's' :
			if (cpus->s_cpus[nr]) {
				cpus->s_cpus[nr]=0; return;
			} 
			break;
		case 'f' :
			if (cpus->f_cpus[nr]) {
				cpus->f_cpus[nr]=0; return;
			} 
			break;
		default :
			fatal_error("Unknown cpu type '%c'",kind);
	}
	fatal_error("Trying to free '%c' cpu #%d, which is not used",
		kind,nr);
}