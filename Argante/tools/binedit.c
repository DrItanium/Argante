/*

   Argante virtual OS
   ------------------

   Modify .bin headers
   (C) 2000 Bulba <bulba@intelcom.pl>

   Status: done

   Author:     Bulba <bulba@intelcom.pl>
   Maintainer: Bulba <bulba@intelcom.pl>

*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "config.h"
#include "console.h"
#include "bformat.h"


struct bformat info;
void * code = NULL;
int	codesize = 0;
void * data = NULL;
int	datasize = 0;

void
usage (char *name)
{
    printk("Usage: %s \\\n"
	   "    <filename> [-p <priorytet>] [-s <sygnaturka>] [-a <domain>] \\\n"
	   "    [-r <domain>] [-d <domain>] [-u <uid>] [-i <ipc>] [-f <flags>] \\\n"
	   "    [-o <outfile>]\n\n"
	   "    filename   - nazwa pliku do edycji\n"
	   "    priorytet  - cyferkowo wyra�ony priorytet\n"
	   "    sygnaturka - znakowo wyra�ona sygnaturka\n"
	   "    domain     - cyfrowo wyra�ona domena (a=dodaj, r=usu�, d=current)\n"
	   "    uid        - cyfrowo wyra�ony uid w domenie\n"
	   "    ipc        - cyfrowo wyra�ony identyfikator IPC/rIPC\n"
	   "    flags      - cyfrowo wyra�one flagi (unsupported)\n"
	   "    outfile    - jak nie chcemy zniszczy� naszego orygina�u\n\n"
	   ,name);
    exit(0);
}

int
wczytaj_naglowek (FILE *f)
{
    if (fread(&info, sizeof(info), 1, f) != 1) {
	printk("[B��d] Z�y plik albo uszkodzony nag��wek...\n");
	exit(-1);
    }
    if ((info.magic1 != 0xdefaced) || (info.magic2 != 0xdeadbeef)) {
	printk("[B��d] Zapoda�e� tandetny plik...\n");
	exit(-1);
    }
    return 1;
}

int
wczytaj_kod_i_dane (FILE *f)
{
    if (info.bytesize) code = (void *)malloc(info.bytesize * 12);
    if (info.datasize) data = (void *)malloc(info.datasize * 4);
    if ((info.bytesize && !code) || (info.datasize && !data)) {
	printk("[B��d] Problemy z pami�ci�...\n");
	exit(-1);
    }
    if (info.bytesize) { 
	if (fread(code, info.bytesize * 12, 1, f) != 1) {
	    printk("[B��d] Codesegment obci�ty, albo problemy z plikiem...\n");
	    exit(-1);
	} else codesize = info.bytesize * 12;
    }
    if (info.datasize) {
	if (fread(data, info.datasize * 4, 1, f) != 1) {
	    printk("[B��d] Datasegment obci�ty, albo problemy z plikiem...\n");
	    exit(-1);
	} else datasize = info.datasize * 4;
    }
    return 1;
}

int
dumpnij_naglowek(char *name)
{
    int i;
    printk("[OK] Aktualny nag��wek pliku\n"
	   "    nazwa:      %s\n"
           "    sygnaturka:\n\t%.64s\n"
           "    flagi:      <unsupported>\n"
	   "    priorytet:  %d\n"
	   "    IPC id:     0x%08x\n"
	   "    init IP:    0x%08x\n"
	   "    domain/uid: [%d:%d]\n"
	   "    codesize:   0x%08x instrukcji\n"
	   "    datasize:   0x%08x dwus��w\n"
	   ,name, info.signature, info.priority, (info.ipc_reg >0 ? info.ipc_reg : 0),
	   info.init_IP, info.current_domain, info.domain_uid,
	   info.bytesize, info.datasize);
    if (info.memflags != 0) {
	printk("    memflags:   ");
	if (info.memflags & 1) printk("READ ");
	if (info.memflags & 2) printk("WRITE");
	printk("\n");
    } else printk("    memflags:   NONE\n");
    printk("    domains:    ");
    for (i=0; ((i<MAX_EXEC_DOMAINS) && (info.domains[i])); i++)
	printk("%d ", info.domains[i]);
    if (i==0) printk("NONE\n"); else printk("\n");
    return 1;
}

int
zgraj_plik(FILE *f, int full)
{
    // uwzgl�dniono mo�liwo�� zmieniania d�ugo�ci segment�w danych i kodu
    size_t ile;
    if (fwrite(&info, sizeof(info), 1, f) != 1) {
	printk("[B��d] Nie mog� zapisa� zmian (nag��wek)...\n");
	return 0;
    }
    if (!full) return 1;
    if (codesize != info.bytesize) {
	if (codesize < info.bytesize) ile = codesize; else ile = info.bytesize;
    } else ile = codesize;
    if (fwrite(code, ile*12, 1, f) != 1) {
	printk("[B��d] Nie mog� zapisa� zmian (code)...\n");
	return 0;
    }
    if (info.datasize) {
	if (info.datasize != datasize) {
	    if (datasize < info.datasize) ile = datasize; else ile = info.datasize;
    	} else ile = datasize;
	if (fwrite(data, ile*4, 1, f) != 1) {
	    printk("[B��d] Nie mog� zapisa� zmian (data)...\n");
	    return 0;
	}
    }
    return 1;
}

int
main (int argc, char ** argv)
{
    int i;
    int zmiany = 0;
    FILE *f;
    char * outfile;

    printk("Edytor nag��wk�w plik�w wykonywalnych systemu %s ver %d.%03d (c) 2000 Bulba\n"
           "%s system (c) Micha� Zalewski <lcamtuf@tpi.pl>\n\n",
	   SYSNAME, SYS_MAJOR, SYS_MINOR, SYSNAME);
    if (argc < 2) usage(argv[0]);
    if (!(f = fopen(argv[1], "r"))) {
	printk("[B��d] Otwarcie pliku niemo�liwe... \n");
	perror("fopen");
	exit(-1);
    }
    outfile = argv[1];
    wczytaj_naglowek(f);
    wczytaj_kod_i_dane(f);
    fclose(f);
    dumpnij_naglowek(argv[1]);
    for (i = 2; i<argc; i++) {
	if (!strcmp(argv[i], "-s")) {
	    i++;
	    if (i<argc) {
		strncpy(info.signature, argv[i], 64);
		printk("[�yczenie] Sygnaturka na:\n\t%.64s\n", info.signature);
		zmiany++;
	    }
	    continue;
	}
	if (!strcmp(argv[i], "-o")) {
	    i++;
	    if (i<argc) {
		outfile = argv[i];
		printk("[�yczenie] Wyj�cie do pliku: %s\n", outfile);
		zmiany++;
	    }
	    continue;
	}
	if (!strcmp(argv[i], "-p")) {
	    i++;
	    if (i<argc) {
		int p = 0;
		if ((sscanf(argv[i], "%i", &p) == 1) && (p>0)) {
		    info.priority = p;
		    printk("[�yczenie] Priorytet na: %d\n", info.priority);
		    zmiany++;
		} else {
		    printk("[Fatalna Herezja] Priorytet bez sensu, Kowalski...\n");
		    exit(-1);
		}
	    }
    	    continue;
	}
	if (!strcmp(argv[i], "-u")) {
	    i++;
	    if (i<argc) {
		int p = 0;
		if (sscanf(argv[i], "%i", &p) == 1) {
		    info.domain_uid = p;
		    printk("[�yczenie] Uid na: %d\n", info.domain_uid);
		    zmiany++;
		} else {
		    printk("[Fatalna Herezja] Ten UID jest ca�kiem nienormalny...\n");
		    exit(-1);
		}
	    }
	    continue;
	}
	if (!strcmp(argv[i], "-f")) {
	    i++;
	    if (i<argc) {
		int p = 0;
		if (sscanf(argv[i], "%i", &p) == 1) {
		    printk("[Herezja] Nie obs�ugujemy narazie flag...\n");
		} else {
		    printk("[Fatalna Herezja] Flagi jakby nieziemskie...\n");
		    exit(-1);
		}
	    }
	    continue;
	}
	if (!strcmp(argv[i], "-d")) {
	    i++;
	    if (i<argc) {
		int p = 0;
		if (sscanf(argv[i], "%i", &p) == 1) {
		    info.current_domain = p;
		    printk("[�yczenie] Domain na: %d\n", info.current_domain);
		    zmiany++;
		} else {
		    printk("[Fatalna Herezja] Ta domena nie jst do niczego podobna...\n");
		    exit(-1);
		}
	    }
    	    continue;
	}
	if (!strcmp(argv[i], "-i")) {
	    i++;
	    if (i<argc) {
		int p = 0;
		if (sscanf(argv[i], "%i", &p) == 1) {
		    info.ipc_reg = p;
		    printk("[�yczenie] IPC id na: %d\n", info.ipc_reg);
		    zmiany++;
		} else {
		    printk("[Fatalna Herezja] We�, podaj ludzkie IPC id...\n");
		    exit(-1);
		}
	    }
    	    continue;
	}
	if (!strcmp(argv[i], "-a")) {
	    i++;
	    if (i<argc) {
		int p = 0;
		if (sscanf(argv[i], "%i", &p) == 1) {
		    int k,jest=0;
		    for (k=0; ((k<MAX_EXEC_DOMAINS) && (info.domains[k])); k++) 
			if (info.domains[k] == p) { jest = 1; break; }
		    printk("[�yczenie] Doda� domene: %d\n", p);
		    if (jest) printk("[Herezja] Tja, tylko �e ona ju� jest dodana\n"); else
		    if (k<MAX_EXEC_DOMAINS-1) {
			info.domains[k++] = p;
			info.domains[k] = 0;
			zmiany++;
		    } else printk("[Herezja] A ty nie za du�o domen chcesz mie�?\n");
		} else {
		    printk("[Fatalna Herezja] Ehem, jak� domene doda�?...\n");
		    exit(-1);
		}
	    }
    	    continue;
	}
	if (!strcmp(argv[i], "-r")) {
	    i++;
	    if (i<argc) {
		int p = 0;
		if (sscanf(argv[i], "%i", &p) == 1) {
		    int k, jest=0;
		    for (k=0; ((k<MAX_EXEC_DOMAINS) && (info.domains[k])); k++) 
			if (info.domains[k] == p) { jest = 1; break; }
		    if (jest)
			while (k<MAX_EXEC_DOMAINS-1) {
			    info.domains[k] = info.domains[k+1];
			    k++;
			}
		    info.domains[MAX_EXEC_DOMAINS-1] = 0;
		    printk("[�yczenie] Usun�� domene: %d\n", p);
		    if (!jest) printk("[Herezja] Tylko gdzie ty j� widzisz?\n"); else zmiany++;
		} else {
		    printk("[Fatalna Herezja] Ehem, kogo chcia�e� wywali�?...\n");
		    exit(-1);
		}
	    }
    	    continue;
	}
	printk("[Fatalna Herezja] Jakie� magiczne parametry?\n");
	usage(argv[0]);
    }

    if (zmiany) {
	if (outfile != argv[1]) {
	    // nowy plik
	    if (!(f = fopen(outfile, "w"))) {
		printk("[B��d] Problemy z wyj�ciem...\n");
		perror("fopen");
		exit(-1);
	    }
	    if (zgraj_plik(f,1)) {
		fclose(f);
	    } else {
		fclose(f);
		unlink(outfile);
		printk("[B��d] Twe modlitwy o zmian� nie zosta�y wys�uchane...\n");
		exit(-1);
	    }
	} else {
	    if (!(f = fopen(outfile, "r+"))) {
		printk("[B��d] Problemy z wyj�ciem...\n");
		perror("fopen");
		exit(-1);
	    }	    
	    if (zgraj_plik(f,0)) {
		fclose(f);
	    } else {
		fclose(f);
		unlink(outfile);
		printk("[B��d] Twe modlitwy o zmian� nie zosta�y wys�uchane...\n");
		exit(-1);
	    }
	}    
    }
    printk("[OK] Pobo�ne �yczenia spe�nione...\n\n");
    return 0;
}
