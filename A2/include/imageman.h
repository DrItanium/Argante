extern unsigned imageman_loadimage(char *from, struct vcpu *into);
extern void imageman_unload(struct vcpu *from);
extern int imageman_unload_al(struct vcpu *from, unsigned alib_id);
extern int validate_bcode(struct vcpu *curr_cpu);

#define A2_MAX_ALID 0xffff

