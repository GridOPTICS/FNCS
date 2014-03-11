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


#include "speculationtimecalculationstrategy.h"

namespace sim_comm{
  
  SpeculationTimeCalculationStrategy::SpeculationTimeCalculationStrategy(time_metric metric,TIME initialspecTime)
  {
    this->specDiff=convertToFrameworkTime(metric,initialspecTime);
    this->speculationTime=0;
   
  }
  
  void SpeculationTimeCalculationStrategy::startSpeculation(TIME currentTime)
  {
    this->speculationTime=currentTime;
  }

  bool SpeculationTimeCalculationStrategy::sendUnknownAction(){
	  return false;
  }


  TIME SpeculationTimeCalculationStrategy::getNextSpecTime(TIME currentTime){
	  return currentTime + specDiff;
  }

  ConstantSpeculationTimeStrategy::ConstantSpeculationTimeStrategy(time_metric metric,TIME initialspecTime) : SpeculationTimeCalculationStrategy(metric,initialspecTime)
  {

  }
  
  void ConstantSpeculationTimeStrategy::speculationFailed(TIME failTime)
  {

  }
  
  void ConstantSpeculationTimeStrategy::speculationSuceeded(TIME currentTime)
  {

  }

  bool ConstantSpeculationTimeStrategy::worthSpeculation(TIME currentTime,TIME specFailTime){
	  return currentTime + specDiff < specFailTime ? true : false;
  }

  IncreasingSpeculationTimeStrategy::IncreasingSpeculationTimeStrategy(time_metric metric,TIME initialspecTime): SpeculationTimeCalculationStrategy(metric,initialspecTime)
  {

  }
  
  void IncreasingSpeculationTimeStrategy::speculationFailed(TIME failTime)
  {
   
    this->specDiff=failTime - speculationTime;
  }
  
  void IncreasingSpeculationTimeStrategy::speculationSuceeded(TIME currentTime){
    
      this->specDiff+= currentTime - speculationTime;
  }
  
  bool IncreasingSpeculationTimeStrategy::worthSpeculation(TIME currentTime,TIME specFailTime){
	  return currentTime + specDiff < specFailTime ? true : false;
  }

  InfinitySpeculationTimeStrategy::InfinitySpeculationTimeStrategy()
  	  : SpeculationTimeCalculationStrategy(NANOSECONDS, Infinity)
  {

  }

  bool InfinitySpeculationTimeStrategy::worthSpeculation(TIME currentTime, TIME specFailTime){
	  return true;
  }

  TIME InfinitySpeculationTimeStrategy::getNextSpecTime(TIME currentTime){
	  return Infinity;
  }

  InfinitySpeculationTimeStrategyWithKillLast::InfinitySpeculationTimeStrategyWithKillLast() : InfinitySpeculationTimeStrategy(){

  }

  IncreasingSpeculationTimeStrategy::~IncreasingSpeculationTimeStrategy(){ }
  ConstantSpeculationTimeStrategy::~ConstantSpeculationTimeStrategy() { }
  SpeculationTimeCalculationStrategy::~SpeculationTimeCalculationStrategy() { }
  InfinitySpeculationTimeStrategy::~InfinitySpeculationTimeStrategy(){ }
  InfinitySpeculationTimeStrategyWithKillLast::~InfinitySpeculationTimeStrategyWithKillLast() { }

bool InfinitySpeculationTimeStrategyWithKillLast::sendUnknownAction() {
	return true;
}

}
