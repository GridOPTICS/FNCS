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


#ifndef SPECULATIONTIMECALCULATIONSTRATEGY_H
#define SPECULATIONTIMECALCULATIONSTRATEGY_H

#include "simtime.h"

namespace sim_comm{
  /**
   * Abstract class defining operations for 
   * changing (recalculting) the speculation time.
   */
class SpeculationTimeCalculationStrategy
{
  protected:
    TIME initialTime;
  public:
    SpeculationTimeCalculationStrategy(time_metric metric, TIME initialspecTime);
    virtual void speculationFailed(TIME currentTime,TIME nextTime) = 0;
    virtual void speculationSuceeded(TIME currentTime,TIME nextTime) =0;
    inline TIME getSpecTime(){
      return this->initialTime;
    }
};

/**
 * This strategy does not change the speculation time.
 */
class ConstantSpeculationTimeStrategy : public SpeculationTimeCalculationStrategy
{
  public:
    ConstantSpeculationTimeStrategy(time_metric metric, TIME initialspecTime);
    virtual void speculationFailed(TIME currentTime,TIME nextTime){};
    virtual void speculationSuceeded(TIME currentTime,TIME nextTime){};
};

/**
 * This strategy doubles the speculatoinTime iff speculation works.
 * Otherwise it reduces the speculation time by half.
 * 
 */
class IncreasingSpeculationTimeStrategy : public SpeculationTimeCalculationStrategy
{
  private:
    TIME startup;
  public:
    IncreasingSpeculationTimeStrategy(time_metric metric, TIME initialspecTime);
    virtual void speculationFailed(TIME currentTime,TIME nextTime);
    virtual void speculationSuceeded(TIME currentTime,TIME nextTime);
};

}

#endif // SPECULATIONTIMECALCULATIONSTRATEGY_H
