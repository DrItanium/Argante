/*
 * String hash table.
 * 
 * This is for ensuring only one copy of "hit me" is around
 * at the same time, which makes it much more efficient to
 * find functions et al.
 *
 * We'll only ever need to add strings to this.
 */
#include "autocfg.h"
#include "compat/alloca.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lstring.h"
#include "main.h"


#define TABSIZE 150

struct _string {
	char *ch;
	unsigned len;
	string next;
	int AppUseId; /* Does this go into string table? What name? */
};

static string table[TABSIZE];

static unsigned hash(const char *s)
{
	return (((unsigned) s[0] << 8) | (unsigned) s[1]) % TABSIZE;
}

static string findString(int x, const char *s, unsigned len)
{
	string z=table[x];

	while(z)
	{
		if (z->len == len && !memcmp(z->ch, s, len)) return z;
		z=z->next;
	}
	return NULL;
}

static string StringMem(const char *s, unsigned len) {
	string a;
	string b;
	unsigned x;
	
	x=hash(s);

	b=findString(x, s, len);
	if (b) return b;

	a=malloc(sizeof(struct _string));
	a->ch=malloc(len + 1);
	memcpy(a->ch, s, len);
	a->ch[len]=0; /* LAME */
	a->len=len;
	a->AppUseId=0;
	a->next=table[x];
	table[x]=a;
	
	return a;

}

string String(const char *s)
{
	unsigned slen;
	slen=strlen(s);
	return StringMem(s, slen);
}

const char *StringToChar(string s)
{
	if (!s) return "(nuLL>";
	return s->ch;
}

extern unsigned StringLen(string s) {
	if (!s) return 0;
	return s->len;
}

/* Un-escapes an escaped string. */
string StringEscaped(const char *s) {
	char *dup, *o;
	const char *i;
	int sz;
	ALLOCA_STACK;

	sz=strlen(s) + 1;
	dup=alloca(sz);
   o=dup;
	i=s;

	sz=0;

	while(1) {
		*o=*i;
		if (!*i) break;
		else if (*i == '\\') {
			i++;
			/* There's a few I missed, I'm sure */
			switch(*i) {
				case 'x':
				{ /* Shellcode in Argante? Cool! */
					static char nu[3];
					char *ret;
					i++;
					if (!(nu[0]=*i)) goto xdie;
					i++;
					if (!(nu[1]=*i)) goto xdie;
					nu[2]=0;
					*o=(char) strtol(&nu[0], &ret, 0x10); /* Hex radix */
					if (ret && *ret) goto xdie; /* Trailing garbage? */
					break;
				xdie:
					EM_Error("Bad \\x sequence.");
					break;
				}
				case 'n': *o='\n'; break;
				case 'r': *o='\r'; break;
				case 'e': *o=27; break;
				case 'b': *o='\b'; break;
				case 't': *o='\t'; break;
				default: *o=*i;
			}
		}
		i++;
		o++;
		sz++;
	}
	return StringMem(dup, sz);
}

/* Prints a string in escaped form */
#include <ctype.h>
void PrintEscaped(string str, FILE *to) {
	const char *s;
	int i;
	s=StringToChar(str);
	i=StringLen(str);
	while(i) {
		switch(*s) {
			case '"':
			case '\'':
			case '\\':
				putc('\\', to);
				putc(*s, to);
				break;
			case '\n':
				putc('\\', to);
				putc('n', to);
				break;
			case '\r':
				putc('\\', to);
				putc('r', to);
				break;
			case 27:
				putc('\\', to);
				putc('e', to);
				break;
			case '\b':
				putc('\\', to);
				putc('b', to);
				break;
			case '\t':
				putc('\\', to);
				putc('t', to);
				break;
			default:
				if (isprint((unsigned) *s))
					putc(*s, to);
				else
					fprintf(to, "\\x%2.2x", (int) ((unsigned char) *s));
		}
		s++;
		i--;
	}
}

static int nextsymid=1;

string SymbolizeString(string s)
{
	static char blurgh[48]; /* Pretty much MAX_SYM */
	string r;
	if (!s->AppUseId) {
		s->AppUseId=nextsymid;
		nextsymid++;
	}
	sprintf(blurgh, "string_%d", s->AppUseId);
	r=String(blurgh);
	return r;
}

void DumpStringTable() {
	unsigned i;
	string s;
	for(i=0;i<TABSIZE;i++) {
		s=table[i];
		while(s) {
			if(s->AppUseId) {
				fprintf(codeout, ":string_%d\n\t\"", s->AppUseId);
				PrintEscaped(s, codeout);
				fputs("\"\n", codeout);
			}
			s=s->next;
		}
	}
}
