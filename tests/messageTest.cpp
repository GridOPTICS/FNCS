#include "config.h"

#include <cstring>
#include <cstdlib>

#include "integrator.h"
#include "message.h"
#include "util/simtime.h"

using namespace sim_comm;

int main(){

  Message *original=NULL;
  Message *deserialized=NULL;
  uint8_t *serbuff=NULL;
  uint32_t size=0;
  uint8_t data[100];
  
  bzero(data,100);
  data[99]=50;
  data[50]=1;
  data[0]=99;
  
  Integrator::initIntegratorGracePeriod(NULL,SECONDS,5,10);
  cout << "Integrator initialized" << endl;
  
  original=new Message("selim","selim",0,data,100,0);
  cout << "Created Message" << endl;

  original->serializeHeader(serbuff,size);
  cout << "Serialized Message" << endl;

  deserialized=new Message(serbuff,size,data);
  cout << "Deserialized Message" << endl;
  
  cout << "Comparing Messages... ";
  if (*original != *deserialized) {
      cout << "FAILED" << endl;
      exit(EXIT_FAILURE);
  }
  else {
      cout << "OK" << endl;
  }

  return 0;
}
