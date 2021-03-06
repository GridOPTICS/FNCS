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


#ifndef OPTIMISTICTICKSYNCALGO_H
#define OPTIMISTICTICKSYNCALGO_H

#include "abssyncalgorithm.h"
#include "speculationtimecalculationstrategy.h"
#include "graceperiodspeculativesyncalgo.h"
#include <unistd.h>
#include <vector>

#include "factorydatabase.h"
#include "fncsconfig.h"

#define ACTION_DOFAILLALL 0
#define ACTION_NOINFO 1
#define ACTION_FAILED 2
#define ACTION_SUCCESS 3
#define ACTION_UNDEFINED 4

#ifdef DEBUG_WITH_PROFILE
#include "profiler.h"
#endif


using namespace std;

namespace sim_comm{

  
  /**
   * Optimistic sync algorithm for power simulators.
   * 
   */
  //TODO: marked for refactoring, process management functionality can be realized in comm manager!
  class OptimisticTickSyncAlgo : public AbsSyncAlgorithm
  {
    private:
      
      SpeculationTimeCalculationStrategy *st;
      /**
       * @TODO I assume 3 processes!
       */
      pid_t mypid,parentPid,childPid; 
      bool isChild,isParent;

      int shmiditems;
      key_t shmkeyitems;

      void createTimeShm();
      void attachTimeShm();
      void detachTimeShm();

      //Child classes can call these but overridign is not permited.
      void doLimbo();
      void doFailAll();
      void childDied();
      void parentDie();

    protected:
      
      TIME specFailTime,minNextTime;
      uint64_t globalAction;
      struct shmitems{
      		  TIME failTime;
      		  uint16_t action;
            };
      shmitems *comm;

      void checkChild(uint64_t diff);
      void createSpeculativeProcess();
      
      /**
       * Callback function registered with comm manager. 
       */
      virtual bool nodeSentMessage(Message *msg);
      /**
       * Callback function registered with comm manager.
       */
      virtual bool nodeReceivedMessage(Message *msg);
     

      /**
       * Starts the speculative process if possible.
       * Tests if a previously started speculation is OK if not kills the child
       * 
       * @param[in] specNextTime The speculation look ahead time.
       * @return For child new granted time, 0 for parent!
       */
      virtual TIME testSpeculationState(TIME specNextTime, TIME currentTime);
      
      bool hasChild(){
	return this->childPid>0? true:false;
      }
      
      void becomeChild();
      /**
       * Sets the flags and kills the parent process.
       */
      void becomeParent();
      
      void gotChild(pid_t childpid);
     
      void childTerminated();
    public:
      OptimisticTickSyncAlgo(AbsCommManager* interface, SpeculationTimeCalculationStrategy *strategy);
      virtual ~OptimisticTickSyncAlgo();
      /** @copydoc AbsSyncAlgorithm::GetNextTime(TIME currentTime, TIME nextTime) */
      virtual TIME GetNextTime(TIME currentTime, TIME nextTime);
     
      /** @copydoc AbsSyncAlgorithm::timeStepStart(TIME currentTime) */
      virtual void timeStepStart(TIME currentTime);
      /** @copydoc AbsSyncAlgorithm::forkedNewChild() */
      virtual bool forkedNewChild(){return this->isChild;}
     
      /** @copydoc AbsSyncAlgorithm::usesFork()*/
      virtual bool usesFork() { return true; }

      static AbsSyncAlgorithm* Create(const Json::Value& param, AbsCommManager *comm);
  };

}


#endif // OPTIMISTICTICKSYNCALGO_H
