#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
  char buffer[30];
  char prueba[30];
  struct timeval tv;
  FILE *fout;
  fout = fopen("prueba.txt", "a");


  time_t curtime;
  gettimeofday(&tv, NULL);
  curtime=tv.tv_sec;

  strftime(buffer,30,"%T.",localtime(&curtime));
  printf("Anulacion %s%ld\n",buffer,tv.tv_usec);
  char *c;
  c = (char *) tv.tv_usec;

  printf("%s", buffer);
  printf("%ld\n", tv.tv_usec);

  fputs(buffer,fout);
  fputs(c, fout);
  return 0;
}
