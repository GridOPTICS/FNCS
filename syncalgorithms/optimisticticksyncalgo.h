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

#define DOKILLCHILD 0
#define DONTKILLCHILD 1

#include "abssyncalgorithm.h"
#include "graceperiodspeculativesyncalgo.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

using namespace std;

namespace sim_comm{

  /*class ProcessMessage{
    uint8_t type;
    pid_t processId;
    static uint8_t* serializeMessage(ProcessMessage *given, uint8_t &size);
    static ProcessMessage* deserializeMessage(uint8_t *arr,uint8_t size);
  };
  
  class ProcessGroupManager{
  private:
      uint32_t *newprocessArr;
      uint32_t *oldprocessArr;
      uint32_t numOfProcesses,numRegistered;
      int sockfd;
      bool looping;
      sockaddr_un sockStruct;
      vector<int> connSocks;
      void killchildren();
      void killparents();
  public:
      ProcessGroupManager(uint32_t noOfProcesses);
      
      void startManager();
      void terminateManager();
      
      virtual ~ProcessGroupManager();
  };
  
  class ProcessGroupManagerClient{
  private:
    int sockfd;
   
  public:
    ProcessGroupManagerClient();
    bool isMyChildActive(pid_t childId);
    void registerParent(pid_t parentId);
    void registerChild(pid_t childId);
    void childWorked(pid_t childId);
    virtual ~ProcessGroupManagerClient();
  };*/
  

  /**
   * Optimistic sync algorithm for power simulators.
   * 
   */
  class OptimisticTickSyncAlgo : public GracePeriodSpeculativeSyncAlgo
  {
    private:
      uint64_t killChildernFlag;
      uint64_t specFailTime;
      key_t specTimeKey;
    protected:  
      /**
       * Callback function registered with comm manager. 
       */
      bool nodeSentMessage(Message *msg);
      /**
       * Callback function registered with comm manager.
       */
      bool nodeReceivedMessage(Message *msg);
      /**
       * Called by the speculative child to signal it's parent that
       * speculation has failed.
       */
      void signalParent();

      /**
       * Starts the speculative process if possible.
       * Tests if a previously started speculation is OK if not kills the child
       * 
       * @param[in] specNextTime The speculation look ahead time.
       * @return For child new granted time, 0 for parent!
       */
      virtual TIME testSpeculationState(TIME specNextTime);
      
      /**
       * Allows speculative children to communicate when speculation has failed!
       */
      void writeSpeculationFailureTime(TIME given);
      /**
       * Creates shared memory for speculative children write the fail time.
       */
      void createSpeculationTimeShm();
      /**
       * Returns the failed spec time to kids!
       */
      TIME getSpeculationFailureTime();
      
      //we will subclass these later!!!
      void failedRecalculateSpecDifference(TIME specFailTime){
	this->specDifference/=2;
	cout << "Recaulted spec diff " << this->specDifference << endl;
	//return specDifference
      }
      void succeedRecalculateSpecDifference(TIME specSuccessTime){
	this->specDifference*=2;
	//return specDifference
      }
      
    public:
      OptimisticTickSyncAlgo(AbsCommManager* interface, TIME specDifference);
      virtual ~OptimisticTickSyncAlgo();
      /** @copydoc AbsSyncAlgorithm::GetNextTime(TIME currentTime, TIME nextTime) */
      virtual TIME GetNextTime(TIME currentTime, TIME nextTime);
      /** @copydoc AbsSyncAlgorithm::doDispatchNextEvent(TIME currentTime, TIME nextTime) */
      virtual bool doDispatchNextEvent(TIME currentTime, TIME nextTime);
      /** @copydoc AbsSyncAlgorithm::timeStepStart(TIME currentTime) */
      virtual void timeStepStart(TIME currentTime);
  };

}

#endif // OPTIMISTICTICKSYNCALGO_H
