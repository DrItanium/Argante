struct symbol {
	struct stable s;
	unsigned alib_id;
	struct symbol *next;
	struct relocent *reloc;
};

extern struct symbol *symman_find_symbol(struct vcpu *in, char *name, unsigned alib_id);
extern struct symbol *symman_add_symbol(struct vcpu *to, struct stable *sym, unsigned alib_id); 
extern int symman_add_reloc(struct vcpu *in, struct stable *sym, struct symbol *sdata, struct reloc *r, unsigned alib_id); 
extern void symman_unload(struct vcpu *in);
extern void symman_unload_al(struct vcpu *in, unsigned alib_id);

