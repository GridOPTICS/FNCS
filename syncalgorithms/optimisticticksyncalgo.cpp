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

#include "optimisticticksyncalgo.h"
#include <signal.h>

#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

namespace sim_comm{

void OptimisticTickSyncAlgo::childDied(TIME dieTime)
{
  wait(NULL);
  this->specFailTime=dieTime;
}


OptimisticTickSyncAlgo::OptimisticTickSyncAlgo(AbsCommManager* interface, TIME specDifference,SpeculationTimeCalculationStrategy *strategy) : AbsSyncAlgorithm(interface)
{
  
  CallBack<bool,Message*,empty,empty> *syncAlgoCallBackSend=
    CreateObjCallback<OptimisticTickSyncAlgo*, bool (OptimisticTickSyncAlgo::*)(Message *),bool, Message*>(this,&OptimisticTickSyncAlgo::nodeSentMessage);
  CallBack<bool,Message*,empty,empty> *syncAlgoCallBackRev=
    CreateObjCallback<OptimisticTickSyncAlgo*, bool (OptimisticTickSyncAlgo::*)(Message *),bool, Message*>(this,&OptimisticTickSyncAlgo::nodeReceivedMessage);

  this->interface->setSyncAlgoCallBacks(syncAlgoCallBackSend,syncAlgoCallBackRev);
  this->specFailTime=Infinity;
  this->specDifference=specDifference;
  this->mypid=getpid();
  this->parentPid=0;
  this->childPid=0;
  this->isChild=false;
  this->isParent=true;
  this->st=strategy;
  this->busywait=false;
}

OptimisticTickSyncAlgo::~OptimisticTickSyncAlgo()
{

}

bool OptimisticTickSyncAlgo::nodeReceivedMessage(Message* msg)
{
  if(this->isChild){
    cout << "I'm child with pid:" << this->mypid << " signaling parent " << getppid() << " to kill me!!!" << Integrator::getCurSimTime() << endl;
    this->interface->sendFailed();
    return false;
  }
  else{
    return true;
  }
}

bool OptimisticTickSyncAlgo::nodeSentMessage(Message* msg)
{
  if(this->isChild){
     cout << "I'm child with pid:" << this->mypid << " signaling parent " << getppid() << " to kill me because I'm sending message!!!" << Integrator::getCurSimTime() << endl;
#ifdef PROFILE
    specFailed();
#endif
    this->interface->sendFailed();
    return false;
  }
  else{
    return true;
  }
}



void OptimisticTickSyncAlgo::timeStepStart(TIME currentTime)
{
#ifdef PROFILE
  if(!busywait)
    syncStart();
#endif
  if(currentTime < grantedTime){
    return;
  }
#if DEBUG
		   CERR << "Start sync time step " << currentTime <<  endl;
#endif
  if(this->isChild){
    this->interface->sendSuceed();
    this->interface->waitforAll();
    this->specFailTime=Infinity;
    cout << "I'm child my pid:" << this->mypid << "; killing parent " << parentPid << endl;
    if(this->st!=NULL){
      st->speculationSuceeded(currentTime,grantedTime);
      specDifference=st->getSpecTime();
    }
    becomeParent();
  }
  else{
    //cout << "I'm parent waiting for others!!" << endl;
    this->interface->waitforAll();
  }
}

bool OptimisticTickSyncAlgo::doDispatchNextEvent(TIME currentTime, TIME nextTime)
{
  return GetNextTime(currentTime,nextTime);
}



TIME OptimisticTickSyncAlgo::GetNextTime(TIME currentTimeParam, TIME nextTime)
{
      TIME nextEstTime;
      TIME currentTime=currentTimeParam;
      //we have processed upto including grated time
      if(nextTime <= grantedTime)
	return nextTime;
      
      busywait=false;
      bool canSpeculate=false;
      bool needToRespond=false;
      //send all messages
#if DEBUG
		   CERR << "Start sync " << currentTime << " " << nextTime << endl;
#endif
      if(currentTime < grantedTime){ //we still have some granted time we need to barier at granted Time
	    busywait=true;
	    currentTime=grantedTime;
      }
      if(currentTime > this->specFailTime){ //we passed beyond spec. failure time. We can speculatie again.
	this->specFailTime=Infinity;
      }
      do
      {
	  canSpeculate=false;
	  if(busywait)
	    this->timeStepStart(currentTime);
	  //we don't need barier
          uint8_t diff=interface->reduceTotalSendReceive();
          //network unstable, we need to wait!
          nextEstTime=currentTimeParam+convertToFrameworkTime(Integrator::getCurSimMetric(),1);
	  TIME minnetworkdelay=interface->reduceNetworkDelay();
          if(diff==0 && !needToRespond)
          { //network stable grant next time
              nextEstTime=nextTime;
	      canSpeculate=true;
          }
          else{
	    needToRespond=true;
	  }
	  TIME mySpecNextTime;
	  //remove this!!!!
	  //canSpeculate=false;
	 
	  if(canSpeculate && (currentTime+specDifference) < this->specFailTime){ //test if it is worht speculating!
	     mySpecNextTime=currentTime+specDifference;
	  }
	  else{
	    mySpecNextTime=0;
	  }
          //Calculate next min time step
          TIME myminNextTime=nextEstTime;
          TIME specNextTime=(TIME)interface->reduceMinTime(mySpecNextTime);
	  TIME minNextTime=(TIME)interface->reduceMinTime(myminNextTime);
	  
	  //speculation stuff
	  TIME specResult=testSpeculationState(specNextTime,currentTime);
	  if(specResult > 0) //we are in child! We are granted up to specNextTime
	      minNextTime=specNextTime;

	  if(minNextTime==0){ //a sim signal endded
#if DEBUG
	      CERR << "End Signaled!" << endl;
#endif
	      this->finished=true;
	      return 0;
	  }
	  
          if(minNextTime < myminNextTime){
	    //next time is some seconds away and the simulator has to wait so fork speculative unless we already forked
	      //update the value of the currentTime
	      currentTime=minNextTime;
	      busywait=true;
          }
          else{
	    busywait=false;
	  }

	this->grantedTime=minNextTime;
      }while(busywait);
#ifdef PROFILE
    writeTime((long int)nextEstTime);
#endif
     
      return nextEstTime;
  }
  
  void OptimisticTickSyncAlgo::createSpeculativeProcess()
  {
      pid_t forkpid = fork();
      if (-1 == forkpid) {
	  /* I am the parent, and an error was detected */
	  perror("createSpeculativeProcess: fork");
	  exit(0);
      }
      else if (0 == forkpid) {
	  /* I am a child */
	  becomeChild();
      }
      else {
	  /* I am a parent */
	  gotChild(forkpid);
      }
  }
  
  void OptimisticTickSyncAlgo::becomeChild()
  {
      this->isChild = true;
      this->mypid=getpid();
      this->parentPid=getppid();
      this->childPid=0;
      this->isParent=false;
  }
  
  void OptimisticTickSyncAlgo::becomeParent()
  {
      this->isChild=false;
      this->isParent=true;
      this->childPid=0;
      this->mypid=getpid();
      this->parentPid=0;
  }



  void OptimisticTickSyncAlgo::childTerminated()
  {
      this->isParent=true;
      this->isChild=false;
      this->childPid=0;
      this->mypid=getpid();
      this->parentPid=0;
  }

  void OptimisticTickSyncAlgo::gotChild(pid_t childpid)
  {
      this->isParent=true;
      this->isChild=false;
      this->childPid=childpid;
      this->parentPid=0;
      this->mypid=getpid();
  }



  TIME OptimisticTickSyncAlgo::testSpeculationState(TIME specNextTime,TIME currentTime)
  {
    if(!hasChild() && specNextTime!=0){
	
	this->createSpeculativeProcess();
	this->specFailTime=Infinity;
	if(this->isChild){ //createSpeculativeProcess will modify this flag!
	 AbsCommManager* copy=interface->duplicate(); //create new contexts
	  this->interface=copy;
	  Integrator::setCommManager(copy);
	 
	  cout << this->mypid << ": I'm child, current time" << currentTime << " specDiff " << specNextTime << endl;
	  //usleep(100);
#ifdef PROFILE
	  speced();
#endif
	  return specNextTime;
	}
    }
    if(hasChild()){
      if(specFailTime!=Infinity){ //child terminated
	childTerminated();
	cout << this->mypid << ": Speculation Failed, my child" << this->childPid << "died at time " << specFailTime << ", current time " << currentTime << "spec diff:" << specDifference << endl;
	st->speculationFailed(currentTime,specFailTime);
	specDifference=st->getSpecTime();
      }
    }
    return 0;
  }
  

}
