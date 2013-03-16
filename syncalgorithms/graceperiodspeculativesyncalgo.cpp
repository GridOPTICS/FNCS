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


#include "graceperiodspeculativesyncalgo.h"

namespace sim_comm{
  
GracePeriodSpeculativeSyncAlgo::GracePeriodSpeculativeSyncAlgo(AbsCommManager *interface, TIME specDifference) : AbsSyncAlgorithm(interface){
  this->specDifference=specDifference;
  this->interface=interface;
  CallBack<void,Message*,empty,empty> *syncAlgoCallBackSend=
    CreateObjCallback<GracePeriodSpeculativeSyncAlgo*, void (GracePeriodSpeculativeSyncAlgo::*)(Message *),void, Message*>(this,&GracePeriodSpeculativeSyncAlgo::sentMessage);
  CallBack<void,Message*,empty,empty> *syncAlgoCallBackRev=
    CreateObjCallback<GracePeriodSpeculativeSyncAlgo*, void (GracePeriodSpeculativeSyncAlgo::*)(Message *),void, Message*>(this,&GracePeriodSpeculativeSyncAlgo::receivedMessage);
 
}

GracePeriodSpeculativeSyncAlgo::~GracePeriodSpeculativeSyncAlgo()
{

}

void GracePeriodSpeculativeSyncAlgo::cancelChild()
{
  //in parent: cancled child
  //in child terminate child
}

void GracePeriodSpeculativeSyncAlgo::createSpeculativeProcess()
{
  //fork speculative process.
}

bool GracePeriodSpeculativeSyncAlgo::forkedSpeculativeProcess()
{
    //return true if a speculative process is forked and not cancled. (from child and parent);
    
    //return false otherwise
    //note that if speculation succeeded then it returns false until the child form a previous speculaiton forks.
}

bool GracePeriodSpeculativeSyncAlgo::isExecutingChild()
{
  //return true if the current executing process is the speculative child
}

void GracePeriodSpeculativeSyncAlgo::speculationSucceed()
{
  //let child nknow that it can speculative. 
  //after this call forkedSpeculativePRocees in the child will return false (so it can firther speculate).
}

void GracePeriodSpeculativeSyncAlgo::receivedMessage(Message* msg)
{
  if(isExecutingChild()){
    //speculative process received a message, not allowed termiante speculative child
    cancelChild();
    
  }
  else{
    if(forkedSpeculativeProcess()){
      //we are at parent and the parent wants to recevied message so we should cancle speculation
      cancelChild();
    }
    //we are at parent and there is not speculation so we go on
  }
}

//we can make it so the send in spec process causes it to wait for parent to continue
void GracePeriodSpeculativeSyncAlgo::sentMessage(Message* msg)
{
  if(isExecutingChild()){
    //speculative process wants to send a message, not allowed termiante speculative child
    cancelChild();
    
  }
  else{
    if(forkedSpeculativeProcess()){
      //we are at parent and the parent wants to send message so we should cancle speculation
      cancelChild();
    }
    //we are at parent and there is not speculation so we go on
  }
}

void GracePeriodSpeculativeSyncAlgo::waitForChild()
{
  //this method is called when the speculation succeeds. It causes the parent 
  //parent to wait for the speculative process to complete. After child finishes
  //the main process should terminate
}

bool GracePeriodSpeculativeSyncAlgo::doDispatchNextEvent(TIME currentTime, TIME nextTime)
{
    TIME syncedTime=this->GetNextTime(currentTime,nextTime);

    return syncedTime==nextTime;
}

TIME   GracePeriodSpeculativeSyncAlgo::GetNextTime(TIME currentTime, TIME nextTime)
  {
      TIME nextEstTime;

      if(nextTime < grantedTime)
	return nextTime;
      
      bool busywait=false;

      //send all messages

      do
      {
          uint8_t diff=interface->reduceTotalSendReceive();
          //network unstable, we need to wait!
          nextEstTime=currentTime+convertToFrameworkTime(Integrator::getCurSimMetric(),1);
          if(diff==0)
          { //network stable grant next time
              nextEstTime=nextTime;
          }
     
      
      
	  if(!this->isExecutingChild() && this->forkedSpeculativeProcess() && currentTime==this->specTime){
	    //speculation worked!!!!
	    this->speculationSucceed();
	    this->waitForChild();
	  }
	  

          //Calculate next min time step
          TIME myminNextTime=nextEstTime;
          TIME minNextTime=(TIME)interface->reduceMinTime(myminNextTime);

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
	    if(!this->forkedSpeculativeProcess() && nextTime-currentTime > this->specDifference){	
	      this->specTime=nextTime;
	      this->createSpeculativeProcess();
	      if(this->isExecutingChild())
		return nextTime; //let speculation continue
	    }
            if(minNextTime+Integrator::getGracePeriod()<myminNextTime) //we have to busy wait until other sims come to this time
                  busywait=true;
            else //TODO this will cause gld to re-iterate
                  busywait=false;
          }


      }while(busywait);
      this->grantedTime=nextEstTime;
      return nextEstTime;
  }

}
