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


#ifndef CONSERVATIVESLEEPINGCOMMALGO_H
#define CONSERVATIVESLEEPINGCOMMALGO_H

#include "abssyncalgorithm.h"
#include "factorydatabase.h"
#include "fncsconfig.h"

namespace sim_comm{
  
class ConservativeSleepingCommAlgo : public AbsSyncAlgorithm
{
  private:
    TIME currentState;
    TIME powersimgrantedTime;
    bool updated;
    uint64_t diff;
  public:
    ConservativeSleepingCommAlgo(AbsCommManager *interface);
    virtual ~ConservativeSleepingCommAlgo();
    virtual TIME GetNextTime(TIME currentTime, TIME nextTime);
  
    /**
     * Always returns false as this algorithm does not support forking!
     */
   virtual bool forkedNewChild(){ return false;}

   static AbsSyncAlgorithm* Create(const Json::Value& params,AbsCommManager *comm);
};

}

#endif // CONSERVATIVESLEEPINGCOMMALGO_H
