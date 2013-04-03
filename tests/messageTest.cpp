#include "config.h"

#include <cstring>
#include <cstdlib>

#include "integrator.h"
#include "message.h"
#include "mpinetworkinterface.h"
#include "util/simtime.h"

using namespace sim_comm;


TIME getCurTime(){
    return 10;
}


int main(int argc, char **argv){

  Message *original=NULL;
  Message *deserialized=NULL;
  uint8_t *serbuff=NULL;
  uint32_t size=0;
  uint8_t data[100];
  MpiNetworkInterface *comm=NULL;
  
  bzero(data,100);
  data[99]=50;
  data[50]=1;
  data[0]=99;
  
  MPI_Init(&argc, &argv);
  comm = new MpiNetworkInterface(MPI_COMM_WORLD, true);
  CallBack<TIME,empty,empty,empty>* cb=CreateCallback(getCurTime);

  Integrator::initIntegratorGracePeriod(comm,SECONDS,5,10);
  cout << "Integrator initialized" << endl;
  Integrator::setTimeCallBack(cb);
  Integrator::finalizeRegistrations();
  
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

  Integrator::stopIntegrator();
  cout << "Integrator stopped" << endl;

  delete original;
  delete deserialized;
  delete [] serbuff;
  delete cb;
  
  return 0;
}
