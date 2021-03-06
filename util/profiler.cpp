#include "profiler.h"
#include <sys/time.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

static int inreit=0;
static FILE *output=NULL;
static FILE *execPro=NULL;
static double time1;
static bool doWrite=true;

void syncStart()
{
  inreit=0;
  struct timeval t;
  gettimeofday(&t,NULL);
  time1=t.tv_sec*1000.0 + t.tv_usec / 1000.0;
  
}

void speced()
{
  execPro=NULL;
  //fprintf(execPro,"Starting new child %d\n",getpid());
  doWrite=false;
}

void specFailed()
{
  if(execPro!=NULL){
    fprintf(execPro,"PID %d failed ignore, get from parents %d!\n",getpid(),getppid());
    fflush(execPro);  
  }
}


void writeTime(long int time){

 if(!doWrite)
	 return;

 struct timeval t;
 gettimeofday(&t,NULL);
 double diff=t.tv_sec*1000.0 + t.tv_usec / 1000.0;
 diff-=time1;

  if(execPro==NULL){
    char name[100];
    snprintf(name,100,"sync%dpro.txt",getpid());
    execPro=fopen(name,"w");
  }
  fprintf(execPro,"PID: %d Sync time: %lf %ld\n",getpid(), diff,time);
  fflush(execPro);
}
