/*
 * String hash table.
 * 
 * This is for ensuring only one copy of "hit me"
 * is around at the same time. We'll only ever need
 * to add strings to this.
 */

typedef struct _string *string;

extern string String(const char *a);
extern string StringEscaped(const char *s);

extern const char *StringToChar(string s);
extern unsigned StringLen(string s);
extern void PrintEscaped(string str, FILE *to);

extern string SymbolizeString(string s);
extern void DumpStringTable(void);

