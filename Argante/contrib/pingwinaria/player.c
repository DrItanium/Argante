#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define RAND(x) ((int) (((float)x)*rand()/(RAND_MAX+1.0)))

char ekran[24][81];
char vscr[24][81];

void load_screen(int num) {
  FILE* dupa;
  char b[100];
  int i=0;
  memset(ekran,' ',sizeof(ekran));
  sprintf(b,"plansza%d.txt",num);
  dupa=fopen(b,"r");
  if (!dupa) {
    printf("\nKoniec prezentacji (%d plansz).\n",num);
    exit(0);
  }
  for (;i<24;i++) {
    fgets(ekran[i],80,dupa);
    if (ekran[i][strlen(ekran[i])-1]=='\n') ekran[i][strlen(ekran[i])-1]=' ';
    ekran[i][strlen(ekran[i])]==' ';
    ekran[i][80]=0;
  }
  fclose(dupa);}


void splash_screen(void) {
  int i;
  int a,b;
  int q;
  for (q=0;q<150;q++) {
    memset(vscr,' ',sizeof(vscr));
    for (a=0;a<24;a++) {
     for (b=0;b<80;b++) {
       int xpos,ypos;
       ypos=b+sin((float)q/15.0)*((150-q)/15);
       xpos=a+sin((float)q/10.0)*((150-q)/15);
       if (q>20 || (!(RAND(20-q))))
       if (ypos>=0 && ypos<80 && xpos>=0 && xpos<24)
         vscr[a][b]=ekran[xpos][ypos];
     }
     vscr[a][80]=0;
     vscr[a][strlen(vscr[a])]=' ';
     vscr[a][80]=0;
    }
    printf("\033[01;44m\033[0H"); // Top of the world.
    for (i=0;i<24;i++) printf("%s\n",vscr[i]);
    usleep(10000);
  }
}


#define RAND(x) ((int) (((float)x)*rand()/(RAND_MAX+1.0)))


void desplash_screen(void) {
  int i;
  int a,b;
  int q;
  for (q=0;q<80;q++) {
    memset(vscr,' ',sizeof(vscr));
    for (a=0;a<24;a++) {
     for (b=0;b<80;b++) {
       int xpos,ypos;
       ypos=b+RAND((cos((float)q/55.0)*((float)q*2.0)));
       xpos=a+RAND((sin((float)q/75.0)*((float)q*2.0)));
       if ((q<50) || !(RAND(q-50)))
       if (ypos>=0 && ypos<80 && xpos>=0 && xpos<24)
         vscr[a][b]=ekran[xpos][ypos];
     }
     vscr[a][80]=0;
     vscr[a][strlen(vscr[a])]=' ';
     vscr[a][80]=0;
    }
    printf("\033[01;44m\033[0H"); // Top of the world.
    for (i=0;i<24;i++) printf("%s\n",vscr[i]);
    usleep(10000);
  }
}


void booting(void) {
  int i;
  printf("\033[01;44m\033[0H"); // Top of the world.
  printf("\n\n\n\n\n\n\n"
         "\t\t .------------------------------------------.\n"
         "\t\t | Microsoft (R) PowerPoint (TM) 5.1273.16H |\n"
         "\t\t `------------------------------------------'\n\n");
  for (i=0;i<101;i++) {
    printf("\t\tSYSTEM IS BOOTING UP - PLEASE WAIT: %d%% DONE...\r",i);
    usleep(RAND(400000));
 }
 printf("\n\n              System booted successfully. Hit any key to try again.\n");
 sleep(1);
}



main(int argc,char* argv[]) {
  int numer=0;
  char b[100];
  setbuffer(stdout,0,0);
  printf("\033[01;44m\033[2J\033[0H"); // Top of the world.
  if (argc==1) booting();
  fgets(b,100,stdin);
  while (1) {
    load_screen(numer++);
    splash_screen();
    fgets(b,100,stdin);
    desplash_screen();
  }
}

