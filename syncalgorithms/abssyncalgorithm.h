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


#ifndef ABSSYNCALGORITHM_H
#define ABSSYNCALGORITHM_H

#include "simtime.h"

#ifdef PROFILE
#include "profiler.h"
#endif

#ifdef DEBUG_WITH_PROFILE
#include "profiler.h"
#endif

#include <sstream>

using namespace std;

namespace sim_comm
{
  class AbsCommManager;

  class SyncAlgoException: exception{
    private:
	TIME syncedTime;
    public:
      virtual const char* what() const throw() {
	  stringstream ss;
	  ss << "Incorrect sync time:" << syncedTime;
	  return ss.str().c_str();
      }
      SyncAlgoException(TIME syncedMinTime){
      
	this->syncedTime=syncedMinTime;
      }
  };
  
  class SyncStateException: exception{
  private:
      string statemsg;
  public:
    virtual const char* what() const throw() {
	stringstream ss;
	ss << "Incorrect Speculation state:" << statemsg;
	return ss.str().c_str();
    }
   SyncStateException(string statemsg){
    
      this->statemsg=statemsg;
    }
    
  virtual ~SyncStateException() throw(){
  }
};
  
  class AbsSyncAlgorithm
  {
    protected:
	AbsCommManager* interface;
	bool finished;
	TIME grantedTime;

	bool busywait;
    public:
	AbsSyncAlgorithm(AbsCommManager* currentInterface);
	virtual ~AbsSyncAlgorithm();
	/**
	 * Call this method to determine whether the simulator can skip to the next time.
	 * If not, the method will block until a certain amount of time is granted for simulator.
	 */
	virtual TIME GetNextTime(TIME currentTime,TIME nextTime) = 0;
	
	
	bool isFinished();
	
	
	virtual TIME needsSync(TIME nextTime){
	
	      if(nextTime < grantedTime)
		    return grantedTime;
	      return nextTime;
	}
	
	/**
	 * Called when the simulator is starting a time step!
	 */
	virtual void timeStepStart(TIME currentTime);

	/**
	 * Returns true if the synchronization
	 * algorithm and has forked a new child!
	 */
	virtual bool forkedNewChild() =0;
	
	
	/**
	 * Returns true if sync algorithm
	 * will use fork to speed up co-sims
	 */
	virtual bool usesFork(){ return false; }
	
  };
  
}
#endif // ABSSYNCALGORITHM_H
