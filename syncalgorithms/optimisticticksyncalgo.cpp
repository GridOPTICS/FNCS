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

uint64_t* speculationTime;

void handleTerm(int signum){
  cout << "Received term!!" << endl;
  if(speculationTime!=nullptr)
    shmdt(speculationTime);
  Integrator::terminate();
  exit(0);
}

OptimisticTickSyncAlgo::OptimisticTickSyncAlgo(AbsCommManager* interface, TIME specDifference,SpeculationTimeCalculationStrategy *strategy) : AbsSyncAlgorithm(interface)
{
  
  signal(SIGTERM,handleTerm); //gracefully kill sim!
  CallBack<bool,Message*,empty,empty> *syncAlgoCallBackSend=
    CreateObjCallback<OptimisticTickSyncAlgo*, bool (OptimisticTickSyncAlgo::*)(Message *),bool, Message*>(this,&OptimisticTickSyncAlgo::nodeSentMessage);
  CallBack<bool,Message*,empty,empty> *syncAlgoCallBackRev=
    CreateObjCallback<OptimisticTickSyncAlgo*, bool (OptimisticTickSyncAlgo::*)(Message *),bool, Message*>(this,&OptimisticTickSyncAlgo::nodeReceivedMessage);

  this->interface->setSyncAlgoCallBacks(syncAlgoCallBackSend,syncAlgoCallBackRev);
  this->specTimeKey=2654;
  speculationTime=nullptr;
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
   
    killChildernFlag=Integrator::getCurSimTime();
    this->writeSpeculationFailureTime(killChildernFlag);
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
   
    killChildernFlag=Integrator::getCurSimTime();
#ifdef PROFILE
    specFailed();
#endif
    this->writeSpeculationFailureTime(killChildernFlag);
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
  if(this->isChild){
    this->interface->waitforAll();
    uint64_t dokill=this->interface->reduceMinTime(killChildernFlag);
    if(dokill!=Infinity){
      exit(0); //child dies here, so sad!
    }
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
	  canSpeculate=false;
	 
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
	  
	  //normal conservative algorithm
          //min time is the estimated next time, so grant nextEstimated time
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
      kill(this->parentPid,SIGTERM);
      this->parentPid=0;
  }

  void OptimisticTickSyncAlgo::terminateChild()
  {
      this->isChild=false;
      this->isParent=true;
      this->parentPid=0;
      kill(this->childPid,SIGTERM);
      wait(NULL);
      this->isChild=false;
      this->childPid=0;
      this->mypid=getpid();
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
	 AbsCommManager* copy=interface->duplicate();
	  this->interface=copy;
	  Integrator::setCommManager(copy);
	  killChildernFlag=Infinity; //if tihs value is anything other than Infinity spec has failed!
	  cout << this->mypid << ": I'm child, current time" << currentTime << " specDiff " << specNextTime << endl;
	  //usleep(100);
#ifdef PROFILE
	  speced();
#endif
	  return specNextTime;
	}
	else{
	  this->createSpeculationTimeShm();
	  
	}
    }
    if(hasChild()){
      this->specFailTime=this->getSpeculationFailureTime();
      if(specFailTime==0)
	this->specFailTime=Infinity;
      this->specFailTime=interface->reduceMinTime(specFailTime);
      //cout << this->mypid << ": I'm parent and reduced spec failed time is " << this->specFailTime << endl;
      if(specFailTime!=Infinity){ //child terminated
	shmdt(speculationTime);
	terminateChild();
	cout << this->mypid << ": Speculation Failed, my child" << this->childPid << "died at time " << specFailTime << ", current time " << currentTime << "spec diff:" << specDifference << endl;
	st->speculationFailed(currentTime,specFailTime);
	specDifference=st->getSpecTime();
      }
    }
    return 0;
  }
  
void OptimisticTickSyncAlgo::createSpeculationTimeShm()
{
  if(this->isChild)
    throw SyncStateException("Only parent can do this op!");
  
  int shmid=shmget(this->specTimeKey,sizeof(uint64_t),IPC_CREAT | 0666);
  if(shmid < 0){
    throw new SyncStateException("Shm operation failed!");
  }
  
  speculationTime=(uint64_t*)shmat(shmid,nullptr, 0);
  if(speculationTime==nullptr){
     throw new SyncStateException("Shm operation failed!");
  }
  *speculationTime=0;
}

  
  TIME OptimisticTickSyncAlgo::getSpeculationFailureTime()
  {
    if(this->isChild)
      throw SyncStateException("Only parent can do this op!");
    if(speculationTime==NULL)
      throw SyncStateException("Shared memory error!");
    TIME toReturn=*speculationTime;

    return toReturn;
  }
  
  void OptimisticTickSyncAlgo::writeSpeculationFailureTime(TIME given)
  {
    if(!this->isChild)
      throw SyncStateException("Only child can do this op!");
    int shmid=shmget(this->specTimeKey,sizeof(uint64_t), 0666);
    if(shmid < 0){
      throw new SyncStateException("Shm operation failed!");
    }
    
    speculationTime=(uint64_t*)shmat(shmid,nullptr, 0);
    if(speculationTime==nullptr){
      throw new SyncStateException("Shm operation failed!");
    }
    *speculationTime=given;
    shmdt(speculationTime);
    speculationTime=nullptr;
  }

}



