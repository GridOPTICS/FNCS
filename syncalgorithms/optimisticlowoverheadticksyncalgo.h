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


#ifndef OPTIMISTICLOWOVERHEADTICKSYNCALGO_H
#define OPTIMISTICLOWOVERHEADTICKSYNCALGO_H

#include "abssyncalgorithm.h"
#include "speculationtimecalculationstrategy.h"
#include "graceperiodspeculativesyncalgo.h"
#include <unistd.h>
#include <vector>

#define ACTION_FAILED 0
#define ACTION_SUCCESS 1
#define ACTION_UNDEFINED 2

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
class OptimisticLowOverheadTickSyncAlgo : public AbsSyncAlgorithm
{
    private:

        SpeculationTimeCalculationStrategy *st;
        pid_t mypid;
        pid_t parentPid;
        pid_t childPid; 
        bool isChild;
        bool isParent;

        void createTimeShm();
        void detachTimeShm();
        void childDied();

    protected: 

        TIME specFailTime;
        uint64_t globalAction;
        struct shmitems{
            TIME failTime;
            uint16_t action;
        };
        shmitems *comm;

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

        void gotChild(pid_t childpid);

        void childTerminated();
    public:
        OptimisticLowOverheadTickSyncAlgo(AbsCommManager* interface, TIME specDifference,SpeculationTimeCalculationStrategy *strategy);
        virtual ~OptimisticLowOverheadTickSyncAlgo();
        /** @copydoc AbsSyncAlgorithm::GetNextTime(TIME currentTime, TIME nextTime) */
        virtual TIME GetNextTime(TIME currentTime, TIME nextTime);
        /** @copydoc AbsSyncAlgorithm::doDispatchNextEvent(TIME currentTime, TIME nextTime) */
        virtual bool doDispatchNextEvent(TIME currentTime, TIME nextTime);
        /** @copydoc AbsSyncAlgorithm::timeStepStart(TIME currentTime) */
        virtual void timeStepStart(TIME currentTime);
        /** @copydoc AbsSyncAlgorithm::forkedNewChild() */
        virtual bool forkedNewChild(){return this->isChild;}

        /** @copydoc AbsSyncAlgorithm::usesFork()*/
        virtual bool usesFork() { return true; }
};

}


#endif // OPTIMISTICLOWOVERHEADTICKSYNCALGO_H
