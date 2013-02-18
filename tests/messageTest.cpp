#include <string.h>

#include "integrator.h"
#include "message.h"
#include "util/time.h"

using namespace sim_comm;

int main(){

  uint8_t data[100];
  
  bzero(data,100);
  data[99]=50;
  data[50]=1;
  data[0]=99;
  
  Integrator::initIntegratorGracePeriod(NULL,SECONDS,5,10);
  
  Message *t=new Message("selim","selim",0,data,100,0);
  uint8_t *serbuff;
  uint32_t size;
  t->serialize(serbuff,size);
  Message *deser=new Message(serbuff,size);
  
  uint8_t *dataCopy=deser->getData();
  
  if(dataCopy[0]!=data[0])
    throw "SERIALIZATION FAILED!";
   if(dataCopy[99]!=data[99])
    throw "SERIALIZATION FAILED!";
    if(dataCopy[50]!=data[50])
    throw "SERIALIZATION FAILED!";
  return 0;
}
