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


#include "epochssyncalgortihm.h"

namespace sim_comm{

  EpochsSyncalgortihm::EpochsSyncalgortihm(TIME syncinterval, AbsCommManager* interface): AbsSyncAlgorithm(interface)
  {
    this->nextSyncTime=syncinterval;
  }

  EpochsSyncalgortihm::~EpochsSyncalgortihm()
  {

  }
  
  TIME EpochsSyncalgortihm::GetNextTime(TIME currentTime, TIME nextTime)
  {
      TIME nextEstTime;
      TIME myMinNextTime;
      
       if(nextTime < grantedTime)
	return nextTime;

      bool busywait=false;
      bool needToRespond=false;
      //send all messages
      if(currentTime < grantedTime){ //we still have some granted time we need to barier at granted Time
	    busywait=true;
	    currentTime=grantedTime;
      }
      do{
	 if(busywait){
	    this->interface_->waitforAll();
	  }
          uint64_t diff=interface_->reduceTotalSendReceive();
          //network unstable, we need to wait!
          nextEstTime=currentTime+this->nextSyncTime; 
	  myMinNextTime=currentTime+convertToFrameworkTime(Integrator::getCurSimMetric(),1);
	  if(diff==0 && !needToRespond)
          { //network stable grant next time
              myMinNextTime=nextTime;
          }
          else{
	    needToRespond=true; //set this condition so that when the simulator wakes up from busy wait it responds to messages
	  }
	  TIME minNextTime=(TIME)interface_->reduceMinTime(nextEstTime);
	  
          //min time is the estimated next time, so grant nextEstimated time
          if(minNextTime>myMinNextTime)
              busywait=false;
	  
	  if(minNextTime==0){ //a sim signal endded
#if DEBUG
	      CERR << "End Signaled!" << endl;
#endif
	      this->finished=true;
	      return 0;
	  }
	  if(minNextTime < myMinNextTime){
		currentTime = convertToMyTime(Integrator::getCurSimMetric(),minNextTime);
		currentTime = convertToFrameworkTime(Integrator::getCurSimMetric(),currentTime);
             /* if(minNextTime+Integrator::getGracePeriod()<myminNextTime) //we have to busy wait until other sims come to this time
                  busywait=true;
              else //TODO this will cause gld to re-iterate*/
                  busywait=true;
          }
	  nextEstTime=minNextTime;
      }while(busywait);
     
      this->grantedTime=nextEstTime;
#ifdef PROFILE
      writeTime(currentTime);
#endif
      return myMinNextTime;
  }


}
  