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


#include "optimisticticksyncalgo.h"
#include <signal.h>

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

 uint64_t killChildernFlag;
 
 void parentSignalHandler(int num){
    //child wants to die!
    killChildernFlag=DOKILLCHILD;
  }

OptimisticTickSyncAlgo::OptimisticTickSyncAlgo(AbsCommManager* interface, TIME specDifference): GracePeriodSpeculativeSyncAlgo(interface, specDifference)
{
  signal(SIGUSR1,parentSignalHandler);
  CallBack<bool,Message*,empty,empty> *syncAlgoCallBackSend=
    CreateObjCallback<OptimisticTickSyncAlgo*, bool (OptimisticTickSyncAlgo::*)(Message *),bool, Message*>(this,&OptimisticTickSyncAlgo::nodeSentMessage);
  CallBack<bool,Message*,empty,empty> *syncAlgoCallBackRev=
    CreateObjCallback<OptimisticTickSyncAlgo*, bool (OptimisticTickSyncAlgo::*)(Message *),bool, Message*>(this,&OptimisticTickSyncAlgo::nodeReceivedMessage);

  this->interface->setSyncAlgoCallBacks(syncAlgoCallBackSend,syncAlgoCallBackRev);
  this->currentSpectDifference=this->specDifference;
}

OptimisticTickSyncAlgo::~OptimisticTickSyncAlgo()
{

}

bool OptimisticTickSyncAlgo::nodeReceivedMessage(Message* msg)
{
  if(this->isChild){
    this->signalParent(); //signal parent that we received a packet and speculaiton failed.
    return false;
  }
  else{
    this->currentSpectDifference=this->specDifference;
    killChildernFlag=DOKILLCHILD;
    return true;
  }
}

bool OptimisticTickSyncAlgo::nodeSentMessage(Message* msg)
{
  if(this->isChild){
    this->signalParent(); //signal parent that we received a packet and speculaiton failed.
    return false;
  }
  else{
    this->currentSpectDifference=this->specDifference;
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
    cout << "I'm child my time: " << currentTime << " " << grantedTime << endl;
    this->interface->waitforAll();
    pid_t parent=getppid(); //we will kill parent, so we become the parent
    this->isChild=false;
    this->hasChild=false;
    this->isParent=true;
    this->hasParent=false;
    cout << "I'm child my pid:" << getpid() << " parent " << getppid() << endl;
    this->currentSpectDifference*=2;
    kill(parent,SIGTERM); //kick the parent! yeahhhh!!
  }
  if(this->isParent)
    this->interface->waitforAll();
}

bool OptimisticTickSyncAlgo::doDispatchNextEvent(TIME currentTime, TIME nextTime)
{
  return GetNextTime(currentTime,nextTime);
}



TIME OptimisticTickSyncAlgo::GetNextTime(TIME currentTime, TIME nextTime)
{
      TIME nextEstTime;

      if(nextTime < grantedTime)
	return nextTime;
      
      bool busywait=false;
      bool canSpeculate=false;
      //send all messages
  
      if(this->isChild) //if a child comes here, algorithm is not working!
	throw SyncStateException("Child wants to sync but parent is still alive cannot happen!");
      
      do
      {
	  if(busywait)
	    this->timeStepStart(currentTime);
	  //we don't need barier
          uint8_t diff=interface->reduceTotalSendReceive();
          //network unstable, we need to wait!
          nextEstTime=currentTime+convertToFrameworkTime(Integrator::getCurSimMetric(),1);
	  TIME minnetworkdelay=interface->reduceNetworkDelay();
          if(diff==0)
          { //network stable grant next time
              nextEstTime=nextTime;
	      canSpeculate=true;
          }
     
	  TIME mySpecNextTime;
	  if(canSpeculate){ //test if it is worht speculating!
	     mySpecNextTime=currentTime+currentSpectDifference;
	  }
	  else{
	    mySpecNextTime=Infinity;
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
          if(minNextTime==myminNextTime)
              busywait=false;
	  
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
	      currentTime = convertToMyTime(Integrator::getCurSimMetric(),minNextTime);
	      currentTime = convertToFrameworkTime(Integrator::getCurSimMetric(),currentTime);
            /*if(minNextTime+Integrator::getGracePeriod()<myminNextTime) //we have to busy wait until other sims come to this time
                  busywait=true;
            else //TODO this will cause gld to re-iterate*/
                  busywait=true;
          }


      }while(busywait);
      this->grantedTime=nextEstTime;
      return nextEstTime;
  }
  

  
  
  TIME OptimisticTickSyncAlgo::testSpeculationState(TIME specNextTime)
  {
    if(!this->forkedSpeculativeProcess() && specNextTime!=Infinity){
	     //if we did not for already and specNextTime > currentTime. Lets speculate!
	     this->createSpeculativeProcess();
	     if(this->isChild){
	       //cout << "Before createLayer" << getpid() << endl;
	       this->interface->createLayer(); //duplicate network interface
	       //cout << "After createLayer" << getpid() << endl;
	       notify(getppid()); //we tell the parent that we are done with interface
	       //this->grantedTime=specNextTime;
	       //return nextTime;
	       return specNextTime;
	     }
	     else{
	       block(); //wait for child to signal that it has done with interface!
	       this->specTime=specNextTime;
	       killChildernFlag=DONTKILLCHILD;
	     }
	  }
    if(this->isParent && this->forkedSpeculativeProcess()){
	  
	  uint64_t killChild=interface->reduceMinTime(killChildernFlag);
	  if(killChild==DOKILLCHILD)
	      this->cancelChild();
    }
    return 0;
  }

}



