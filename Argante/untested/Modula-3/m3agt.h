#ifndef _M3AGT_H_
#define _M3AGT_H_

#define AGT_CPU_CNT 16
#define AGT_CPU_MAX AGT_CPU_CNT-1
#define cpu_getu( s ) cpu_get('u', (s))
#define cpu_gets( s ) cpu_get('s', (s))
#define cpu_getf( s ) cpu_get('f', (s))

#define t_cpu int

struct agt_cpus {
	int u_cpus[AGT_CPU_CNT];
	int s_cpus[AGT_CPU_CNT];
	int f_cpus[AGT_CPU_CNT];
};

int cpu_init( struct agt_cpus *cpus );
int cpu_get( char kind, struct agt_cpus *cpus );
void cpu_free( t_cpu cpu, struct agt_cpus *cpus );
char cpu_kind_nr( t_cpu cpu, int *nr );
char cpu_kind( t_cpu cpu );
char *cpun( t_cpu cpu );
#endif