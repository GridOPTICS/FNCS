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

#include "abscommmanager.h"
#include "graceperiodpesimisticsyncalgo.h"
#include "integrator.h"


namespace sim_comm
{
   GracePeriodSyncAlgo::GracePeriodSyncAlgo(AbsCommManager* interface ) : AbsSyncAlgorithm(interface)
  {
      this->algotype=ALGO_PESIMISTIC;
  }

   GracePeriodSyncAlgo::~GracePeriodSyncAlgo()
  {

  }

  bool  GracePeriodSyncAlgo::doDispatchNextEvent(TIME currentTime, TIME nextTime)
  {
    TIME syncedTime=this->GetNextTime(currentTime,nextTime);

    return syncedTime==nextTime;
  }

  TIME  GracePeriodSyncAlgo::GetNextTime(TIME currentTime, TIME nextTime)
  {
      TIME nextEstTime;

      if(nextTime < grantedTime)
	return nextTime;

      bool busywait=false;
      bool needToRespond=false;
      //send all messages

      do
      {
          uint8_t diff=interface->reduceTotalSendReceive();
          //network unstable, we need to wait!
          nextEstTime=currentTime+convertToFrameworkTime(Integrator::getCurSimMetric(),1); 
	  TIME minnetworkdelay=interface->reduceNetworkDelay();
          if(diff==0 && !needToRespond)
          { //network stable grant next time
              nextEstTime=nextTime;
          }
          else{
	    needToRespond=true; //set this condition so that when the simulator wakes up from busy wait it responds to messages
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

             /* if(minNextTime+Integrator::getGracePeriod()<myminNextTime) //we have to busy wait until other sims come to this time
                  busywait=true;
              else //TODO this will cause gld to re-iterate*/
                  busywait=true;
          }


      }while(busywait);
     
      
      this->grantedTime=nextEstTime;
      return nextEstTime;
  }

}
