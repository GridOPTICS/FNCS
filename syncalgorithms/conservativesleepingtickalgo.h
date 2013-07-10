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


#ifndef GRACEPERIODNETWORKDELAYSYNCALGO_H
#define GRACEPERIODNETWORKDELAYSYNCALGO_H


#include "abssyncalgorithm.h"

namespace sim_comm{
  
  
  class ConservativeSleepingTickAlgo : public sim_comm::AbsSyncAlgorithm
  {
    private:
      //pthread_mutex_t threadstatus;
      //TIME avgBusyWait;
      TIME nextTime;
      TIME minResponseTime;
      time_metric min;
      TIME powersimgrantedTime;
    public:
      ConservativeSleepingTickAlgo(AbsCommManager *interface, time_metric connectedSims[], int &connectedSimsSize);
      virtual bool doDispatchNextEvent(TIME currentTime, TIME nextTime);
      virtual TIME GetNextTime(TIME currentTime, TIME nextTime);
      virtual void timeStepStart(TIME currentTime);
      virtual ~ConservativeSleepingTickAlgo();
  };

}
#endif // GRACEPERIODNETWORKDELAYSYNCALGO_H
