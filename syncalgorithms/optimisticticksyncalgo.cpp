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

/*static ProcessMessage* ProcessMessage::deserializeMessage(uint8_t* arr, uint8_t size)
{
  //uint8_t loc=0;
  
  if(size==0)
    throw SyncStateException("Unknown process message!!");
  
  ProcessMessage *toReturn=new ProcessMessage();
  toReturn->type=arr[0];
  
  memcpy(&this->processId,&arr[1],sizeof(pid));
  
  return processId;
}

static uint8_t ProcessMessage::serializeMessage(ProcessMessage* given, uint8_t& size)
{

  size=sizeof(uint8_t)+sizeof(pid_t);
  uint8_t *arr=new uint8_t[size];
 
  arr[0]=given->type;
  memcpy(&arr[1],given->processId,sizeof(pid_t));
  return arr;
}


  
ProcessGroupManager::ProcessGroupManager(uint32_t noOfProcesses)
{
  this->numOfProcesses=noOfProcesses;
  this->newprocessArr=new uint32_t[noOfProcesses];
  this->oldprocessArr=new uint32_t[noOfProcesses];
  this->looping=false;
  
  //open domainsocket!
  this->sockfd=socket(PF_UNIX, SOCK_STREAM,0);
  if(this->sockfd<0)
    throw SyncStateException("Cannot start process manager");
  
  unlink("./processSocket");
  
  memset(&sockStruct,sizeof(sockaddr_un),0);
  sockStruct.sun_family=AF_UNIX;
  sprintf(sockStruct.sun_path,200,"./processSocket");
  
  if(bind(sockfd,(sockaddr *)&sockStruct,sizeof(sockaddr_un))!=0)
  {
    throw SyncStateException("Cannot start process manager");
  }
}



ProcessGroupManager::~ProcessGroupManager()
{
  close(this->sockfd);
  for(int i=0;i<this->connSocks.size();i++)
    close(this->connSocks[i]);
  delete[] this->newprocessArr;
  delete[] this->oldprocessArr;
}

void ProcessGroupManager::startManager()
{
  fd_set readset;
  FD_ZERO(&readset);
  FD_SET(this->sockfd,&readset);
  
  whie(this->looping){
  }
}*/

uint64_t* speculationTime;

void handleTerm(int signum){
  cout << "Received term!!" << endl;
  if(speculationTime!=nullptr)
    shmdt(speculationTime);
  exit(0);
}

OptimisticTickSyncAlgo::OptimisticTickSyncAlgo(AbsCommManager* interface, TIME specDifference): GracePeriodSpeculativeSyncAlgo(interface, specDifference)
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
}

OptimisticTickSyncAlgo::~OptimisticTickSyncAlgo()
{

}

bool OptimisticTickSyncAlgo::nodeReceivedMessage(Message* msg)
{
  if(this->isChild){
    cout << "I'm child with pid:" << getpid() << " signaling parent " << getppid() << " to kill me!!!" << Integrator::getCurSimTime() << endl;
    //this->signalParent(); //signal parent that we received a packet and speculaiton failed.
    killChildernFlag=Integrator::getCurSimTime();
    this->writeSpeculationFailureTime(killChildernFlag);
    //doIntentionalDeadlock=true;
    return false;
  }
  else{
    killChildernFlag=DOKILLCHILD;
    return true;
  }
}

bool OptimisticTickSyncAlgo::nodeSentMessage(Message* msg)
{
  if(this->isChild){
     cout << "I'm child with pid:" << getpid() << " signaling parent " << getppid() << " to kill me because I'm sending message!!!" << Integrator::getCurSimTime() << endl;
    //this->signalParent(); //signal parent that we received a packet and speculaiton failed.
    killChildernFlag=Integrator::getCurSimTime();
    this->writeSpeculationFailureTime(killChildernFlag);
    //doIntentionalDeadlock=true;
    return false;
  }
  else{
    killChildernFlag=DOKILLCHILD;
    return true;
  }
}

void OptimisticTickSyncAlgo::signalParent()
{
  pid_t ppid=getppid();
  kill(ppid,SIGUSR1);
}

void OptimisticTickSyncAlgo::timeStepStart(TIME currentTime)
{
  if(currentTime < grantedTime)
    return;
  if(this->isChild){
    cout << "I'm child my time: " << currentTime << " " << grantedTime << " dokill" << killChildernFlag << endl;
    this->interface->waitforAll();
    uint64_t dokill=this->interface->reduceMinTime(killChildernFlag);
    if(dokill!=Infinity){
      exit(0); //child dies here, so sad!
    }
    pid_t parent=getppid(); //we will kill parent, so we become the parent
    this->isChild=false;
    this->hasChild=false;
    this->isParent=true;
    this->hasParent=false;
    cout << "I'm child my pid:" << getpid() << "; killing parent " << getppid() << endl;
    succeedRecalculateSpecDifference(currentTime);
    kill(parent,SIGTERM); //kick the parent! yeahhhh!!
  }
  if(this->isParent){
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
      
      bool busywait=false;
      bool canSpeculate=false;
      bool needToRespond=false;
      //send all messages
  
      if(currentTime < grantedTime){ //we still have some granted time we need to barier at granted Time
	    busywait=true;
	    currentTime=grantedTime;
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
	  TIME specResult=testSpeculationState(specNextTime);
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
      return nextEstTime;
  }
  

  
  
  TIME OptimisticTickSyncAlgo::testSpeculationState(TIME specNextTime)
  {
    if(!this->forkedSpeculativeProcess() && specNextTime!=0){
	     //if we did not for already and specNextTime > currentTime. Lets speculate!
	     this->createSpeculativeProcess();
	     this->specFailTime=Infinity;
	     if(this->isChild){
	       
	       //cout << "Before createLayer" << getpid() << endl;
	       this->interface->createLayer(); //duplicate network interface
	       usleep(600000);
	       killChildernFlag=Infinity;
	       return specNextTime;
	     }
	     else{
	       this->createSpeculationTimeShm();
	       usleep(600000); //wait for child to signal that it has done with interface!
	       cout << "I'm parent and I'm up!" << getpid() << " " << this->pidChild << endl;
	       cout << endl;
	       this->specTime=specNextTime;
	      
	     }
	  }
    if(this->isParent && this->forkedSpeculativeProcess()){
	  /*cout << "I'm " << getpid() << "my id killflag " << killChildernFlag << endl;
	  uint64_t killChild=interface->reduceMinTime(killChildernFlag);
	  cout << "I'm " << getpid() << "my id killflag " << killChildernFlag << " reduced " << killChild << endl;
	  if(killChild==DOKILLCHILD){
	      cout << "I'm parent " << getpid() << "I'm killing my child " << endl;
	      this->cancelChild();
	  }*/
	  //uint64_t endCode;
	  wait(nullptr);
	  this->specFailTime=this->getSpeculationFailureTime();
	  cout << "my child died!! " << getpid() << "at time " << specFailTime << endl;
	  failedRecalculateSpecDifference(specFailTime);
	  this->hasChild=false;
	  this->isChild=false;
	  this->hasParent=false;
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
    shmdt(speculationTime);
    speculationTime=nullptr;
    return toReturn;
  }
  
  void OptimisticTickSyncAlgo::writeSpeculationFailureTime(TIME given)
  {
    if(this->isParent)
      throw SyncStateException("Only child can do this op!");
    int shmid=shmget(this->specTimeKey,sizeof(uint64_t),IPC_CREAT | 0666);
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



