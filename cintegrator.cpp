/*
    Copyright (c) 2013, <copyright holder> <email>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY <copyright holder> <email> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL <copyright holder> <email> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "config.h"

#include "cintegrator.h"

#include "mpinetworkinterface.h"
#include "integrator.h"
#include "callback.h"

using namespace sim_comm;


void InitMPI(int *arc,char ***argv){

  MPI_Init(arc,argv);
}

void finalizeMPI(){
  MPI_Finalize();
  
}

void timeStepStart(TIME currentTime)
{
  Integrator::timeStepStart(currentTime);
}


uint8_t isFinished()
{
  return (uint8_t)Integrator::isFinished();
}

 void initIntegratorGracePeriod(enum time_metric simTimeStep, 
				TIME packetLostPeriod, TIME initialTime){
   MpiNetworkInterface *comm = new MpiNetworkInterface(MPI_COMM_WORLD, false);
   Integrator::initIntegratorGracePeriod(comm,SECONDS,packetLostPeriod,initialTime);
 }
 
void initIntegratorSpeculative(enum time_metric simTimeStep, 
				TIME packetLostPeriod, TIME initialTime, TIME specTime){
   MpiNetworkInterface *comm = new MpiNetworkInterface(MPI_COMM_WORLD, false);
   Integrator::initIntegratorSpeculative(comm,SECONDS,packetLostPeriod,initialTime,specTime);			 
}

void initIntegratorNetworkDelay(time_metric simTimeStep, TIME packetLostPeriod, TIME initialTime)
{
  MpiNetworkInterface *comm = new MpiNetworkInterface(MPI_COMM_WORLD, false);
  Integrator::initIntegratorNetworkDelaySupport(comm,SECONDS,packetLostPeriod,initialTime);
}


void finalizeRegistrations()
{
  Integrator::finalizeRegistrations();
}

void setregistercallback(TIME (*callback)())
{
  CallBack<TIME,empty,empty,empty> *cb=CreateCallback(callback);
  Integrator::setTimeCallBack(cb);
}

void stopIntegrator()
{
  Integrator::stopIntegrator();
}

TIME getNextTime(TIME currentTime, TIME nextTime)
{
  return Integrator::getNextTime(currentTime,nextTime);
}

void setOffset(TIME initialTime)
{
  Integrator::setOffset(initialTime);
}
