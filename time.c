#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
  char buffer[30];
  struct timeval tv;

  time_t curtime;
  gettimeofday(&tv, NULL);
  curtime=tv.tv_sec;

  strftime(buffer,30,"%T.",localtime(&curtime));
  printf("Anulacion %s%ld\n",buffer,tv.tv_usec);

  return 0;
}
