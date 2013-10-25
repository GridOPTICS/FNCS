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

#include <assert.h>

namespace sim_comm{

void regSpecFailed(int signal)
{
  OptimisticTickSyncAlgo::handleSpecFailed();
}

void OptimisticTickSyncAlgo::handleSpecFailed()
{
#if DEBUG
  CERR << "HAndling signal fail" << endl;
#endif
  OptimisticTickSyncAlgo::specFailed=true;
}

void regSpecSucceed(int signal)
{
  OptimisticTickSyncAlgo::handleSpecSuccess();
}

void OptimisticTickSyncAlgo::handleSpecSuccess()
{
#if DEBUG
  CERR << "Handling signal success" << endl;
#endif
  OptimisticTickSyncAlgo::specSuccess=true;
}


bool OptimisticTickSyncAlgo::specFailed=false;
bool OptimisticTickSyncAlgo::specSuccess=false;

void OptimisticTickSyncAlgo::parentDie()
{
#if DEBUG
  CERR << "OptimisticTickSyncAlgo::parentDie()" << endl;
#endif
  assert(childPid>0); //ensure we are parent.
  Integrator::terminate();
  exit(EXIT_FAILURE);
}


void OptimisticTickSyncAlgo::childDied()
{
  assert(childPid>0);
  attachTimeShm();
  specFailTime=*(this->failTime);
  detachTimeShm();
  kill(childPid,SIGKILL);
  wait(nullptr);
  childTerminated();
  st->speculationFailed(specFailTime);
  specDifference=st->getSpecTime();
  //reset died flag!
  specFailed=false;
#ifdef DEBUG_WITH_PROFILE
  CERR << "Received signal that my child is dead " << specFailTime << " new specDiff " << st->getSpecTime() << " " << getCurTimeInMs() << endl;
#elif DEBUG
  CERR << "Received signal that my child is dead " << specFailTime << " new specDiff " << st->getSpecTime() << endl;
#endif
  cout << "Failed" << endl;
}


OptimisticTickSyncAlgo::OptimisticTickSyncAlgo(AbsCommManager* interface, TIME specDifference,SpeculationTimeCalculationStrategy *strategy) : AbsSyncAlgorithm(interface)
{
  if(!this->interface->supportsFork())
    throw SyncStateException(string("Optimistic algo can only be used if the underlying network itnerface supports fork!"));
  
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
#ifdef DEBUG_WITH_PROFILE
  setElsapedTimer();
#endif
  //setup shared memory
  this->shmkey=-1;
  this->shmid=-1;
  this->failTime=NULL;
  //create shared memory segment
  this->createTimeShm();
  //setup specfailure signal handler
  specFailed=false;
  signal(SIGUSR1,regSpecFailed);
  signal(SIGUSR2,regSpecSucceed);
  
  //disable kill on term, so term signal terminates only the current process.
  this->interface->doKillOnTerm(false);
}

OptimisticTickSyncAlgo::~OptimisticTickSyncAlgo()
{
    if(this->shmid>=0){
      shmctl(this->shmid,IPC_RMID,0);
    }
}

bool OptimisticTickSyncAlgo::nodeReceivedMessage(Message* msg)
{
  if(this->isChild){
    cout << "I'm child with pid:" << this->mypid << " signaling parent " << getppid() << " to kill me!!!" << Integrator::getCurSimTime() << endl;
#ifdef DEBUG_WITH_PROFILE
    CERR << "I'm child with pid:" << this->mypid << " signaling parent " << getppid() << " to kill me!!!" << Integrator::getCurSimTime() << " " << getCurTimeInMs() << endl;
#elif DEBUG
    CERR << "I'm child with pid:" << this->mypid << " signaling parent " << getppid() << " to kill me!!!" << Integrator::getCurSimTime() << endl;
#endif
    (void) kill(parentPid,SIGUSR1); //notify parent about speculaiton failure.
    detachTimeShm();
    
    return false;
  }
  else{
    return true;
  }
}

bool OptimisticTickSyncAlgo::nodeSentMessage(Message* msg)
{
#ifdef DEBUG
    CERR << "OptimisticTickSyncAlgo::nodeSentMessage(Message* msg)" << endl;
#endif
if(this->isChild){
#ifdef DEBUG_WITH_PROFILE
    CERR << "I'm child with pid:" << this->mypid << " signaling parent " << getppid() << " to kill me!!!" << Integrator::getCurSimTime() << " " << getCurTimeInMs() << endl;
#elif DEBUG
    CERR << "I'm child with pid:" << this->mypid << " signaling parent " << getppid() << " to kill me!!!" << Integrator::getCurSimTime() << endl;
#endif
#ifdef PROFILE
    specfailed();
#endif
    (void) kill(parentPid,SIGUSR1);
    detachTimeShm();
    this->interface->block();
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
    if(this->isChild && failTime!=NULL)
      *failTime=currentTime;
    return;
  }
#if DEBUG_WITH_PROFILE
      CERR <<  "Start sync time step " << currentTime << " " << getCurTimeInMs() << endl;
#elif DEBUG
      CERR << "Start sync time step " << currentTime <<  endl;
#endif
  if(this->isChild){
    //speculation worked so we kill the parent
    this->interface->waitforAll();
    pid_t tempparId=this->parentPid;
    becomeParent();
    this->specFailTime=Infinity;
#if DEBUG
    CERR << "I'm child my pid:" << this->mypid << "; killing parent " << tempparId << endl;
#endif
    //detach from shm so parent can clean it.
    detachTimeShm();
    //try to send a kill signal
    (void) kill(tempparId,SIGUSR2);
    cout << "Success" << endl;
    //create new shared memory
    createTimeShm();
    
    st->speculationSuceeded(currentTime);
    specDifference=st->getSpecTime();
   
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
      TIME myminNextTime;
      TIME currentTime=currentTimeParam;
      //we have processed upto including grated time
      if(nextTime <= grantedTime){
#ifdef PROFILE
	writeTime(currentTimeParam);
#endif
	return nextTime;
      }
      
      bool canSpeculate=false;
      bool needToRespond=false;
      busywait=false;
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
          uint64_t diff=interface->reduceTotalSendReceive();
          //nextEstTime is the granted time the simulator exptects.
	  //myminNextTime is the minimum next time the smulator can process.
	  //usually for conservative algorithm these two numbers are the same
	  //for optimistic however when get knowledge about the the dead time of child process
	  //we can use it as the granted time.
	  //if I have a packet, I can only 
          nextEstTime=currentTimeParam+convertToFrameworkTime(Integrator::getCurSimMetric(),1);
	  myminNextTime=nextEstTime;
	  TIME minnetworkdelay=interface->reduceNetworkDelay();
          if(diff==0 && !needToRespond)
          { //network stable grant next time
              myminNextTime=nextEstTime=nextTime;
	      if(specFailTime!=Infinity && nextTime < specFailTime){ //we can grant upto spec fail time
#ifdef DEBUG
    		  CERR << "I'm grating my self " << specFailTime << endl;
#endif
		  nextEstTime=specFailTime;
		  //since we know the spec will fail until this time, we won't fork!
		  canSpeculate=false;
	      }
	      else{
		canSpeculate=true;
	      }
          }
          else{
	    needToRespond=true;
	  }

      TIME minNextTime=(TIME)interface->reduceMinTime(nextEstTime);
#ifdef DEBUG
	  CERR << "Consensus on message-diff " << diff << endl;
#endif
      TIME specNextTime=0;
	  if(diff==0 && !hasChild()){ //we do speculation calculation only if we can speculate
		  //we should never attempt to exchange specDiff when Diff > 0, this is an optimization.
		  //canSpeculate can be false regardless of Diff==0, in this case
		  //we have simulator that needs to respond. So we need to signal others
		  //not to speculate at all.
		  TIME mySpecNextTime;
		  if(canSpeculate && currentTime+specDifference < this->specFailTime){ //test if it is worht speculating!
			 mySpecNextTime=currentTime+specDifference;
		  }
		  else{
			mySpecNextTime=0;
		  }
    	  specNextTime=(TIME)interface->reduceMinTime(mySpecNextTime);
      }
#ifdef DEBUG
	  CERR << "Consensus " << minNextTime << " spec: " << specNextTime << " diff:"<< specDifference << endl;
#endif
	  assert(specNextTime==0 || specNextTime > currentTime);
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
#ifdef DEBUG_WITH_PROFILE
      CERR << "Sync finished at time step " << getCurTimeInMs() << endl;
#endif
#ifdef PROFILE
    writeTime(currentTimeParam);
#endif
     
      return myminNextTime;
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
      this->isParent=false;
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
       interface->prepareFork();
#ifdef DEBUG_WITH_PROFILE
	CERR << "Creating child process for specTime " << specNextTime << " " << getCurTimeInMs() << endl;
#elif DEBUG
	CERR << "Creating child process for specTime " << specNextTime << endl;
#endif
	cout << "forking" << endl;
	this->createSpeculativeProcess();
	if(this->isChild){ //createSpeculativeProcess will modify this flag!
	 AbsCommManager* copy=interface->duplicate(); //create new contexts
	  st->startSpeculation(currentTime);
	  this->interface=copy;
	  Integrator::setCommManager(copy);
	   this->specFailTime=Infinity;
	   attachTimeShm();
	
#ifdef DEBUG
	  CERR << this->mypid << ": I'm child, current time" << currentTime << " run until no sync " << specNextTime << endl;
#endif
	  //usleep(100);
#ifdef PROFILE
	  speced();
#endif
	  return specNextTime;
	}else{
	  //TODO MEMORY LEAK!
	  AbsCommManager *copy=interface->duplicate();
	  st->startSpeculation(currentTime);
	  Integrator::setCommManager(copy);
	  this->interface=copy;
#ifdef DEBUG
	  CERR << this->mypid << ": I'm parent, current time" << currentTime << " re-init complete" <<endl;
#endif
	}
    } else if(hasChild()){
	uint64_t action;
	if(specFailed){
	    action=interface->reduceMinTime(0);
	    assert(action==0); //if we report fail, everyone should agree on fail.
	    childDied();
	}
	else if(specSuccess){
	    action=interface->reduceMinTime(1);
	    assert(action==1); //if we report success, everyone should report success;.
	    parentDie();
	}
	else{
	    action=interface->reduceMinTime(Infinity);
	    if(action==0)
	      childDied();
	    if(action==1)
	      parentDie();
	}
    }
    
    return 0;
  }
  
  void OptimisticTickSyncAlgo::attachTimeShm()
  {
     if(this->failTime==nullptr){
       this->failTime=(TIME *)shmat(this->shmid,nullptr,0);
       if(failTime<0)
	 throw SyncStateException("Attach to shared memory failed!");
     }
      
  }
  void OptimisticTickSyncAlgo::createTimeShm()
  {
   if(this->isChild){
       throw SyncStateException("Child cannot create shared memory, it is already created!");
   }else{
      if(this->failTime!=nullptr)
	throw SyncStateException("Fail time shared memory is already created?");
      this->shmkey=(key_t)getpid();
      this->shmid=shmget(this->shmkey,sizeof(TIME),IPC_CREAT|0666);
      if(shmid<0)
	throw SyncStateException("Error creating shared memory!");
   }
  }
  void OptimisticTickSyncAlgo::detachTimeShm()
  {
    if(this->failTime!=nullptr){
      if(shmdt(this->failTime)<0)
	throw SyncStateException("Shared memory detach failed");
      this->failTime=nullptr;
    }
  }

}
