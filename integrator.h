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
#ifndef ABSINTEGRATOR_H
#define ABSINTEGRATOR_H

#include <fstream>
#include <sstream>
#include <string>

#include "abssyncalgorithm.h"
#include "callback.h"
#include "echo.h"
#include "simtime.h"
#include "speculationtimecalculationstrategy.h"
#include <vector>

#if DEBUG
#include <unistd.h>
#   define PID (getpid())
#   if DEBUG_TO_FILE
#       define CERR (*Debug::getEcho() << '[' << PID << "] ")
#   else
#       define CERR (cerr << '[' << PID << "] ")
#   endif
#endif

struct inst;
class Debug{
  private: 
    static Echo *instance;
  public:
    static void setEcho(string &prefix);
    static void setEcho(const char *prefix);
    static Echo* getEcho();
    static void closeEcho();
};

using namespace std;

namespace sim_comm {

class AbsNetworkInterface;

class AbsCommManager;
class ObjectCommInterface;
class BufferStrategy;
class FncsConfig;

class FNCSException: exception{
  private:
      string statemsg;
  public:
    virtual const char* what() const throw() {
	stringstream ss;
	ss << "FNCS encountered error:" << statemsg;
	return ss.str().c_str();
    }
   FNCSException(string statemsg){
    
      this->statemsg=statemsg;
    }
    
  virtual ~FNCSException() throw(){
  }
};

enum BufferStrategyType{
  BUFFER_ALL,
  BUFFER_FIRST,
  BUFFER_LAST
};


/**
 * Integrator is the main init point for the framework.
 *
 * It bridges the framework with the simulator.  Provides methods for easly
 * accessing simulator and framework parameters.  It is a singleton and all
 * methods are made static for convenience, users do not need to pass instances
 * of integrator around.
 */
class Integrator {
		friend class FncsConfig;
private:
    time_metric simTimeMetric;
    AbsCommManager *currentInterface;
    //TIME gracePeriod;
    TIME packetLostPeriod;
    CallBack<TIME,empty,empty,empty,empty>* getTimeCallBack;
    AbsSyncAlgorithm *syncAlgo;
    TIME offset;
    TIME onetimestep;
    bool allowRegistrations;
    bool stopped;
    bool IamNetworkSim;
    vector<CallBack<void,TIME,empty,empty,empty>* > start_timestep_callbacks,end_getnexttime_callbacks;
    static Integrator* instance;
    /**
     * Constructor.
     */
    Integrator(
    		AbsCommManager *currentInterface,
    		AbsSyncAlgorithm *algo,
    		time_metric simTimeStep,
		TIME packetLostPeriod,
		TIME onetimestep,
		bool IamNetworkSim);
public:

    /**
     * Returns the amount of time that constutes one time step 
     * of the simulator in framework time.
     * 
     * @return, one time step of the simulator in framework time.
     */
    static TIME getOneTimeStep();
    
    /**
     * Main synchronization method for tick-based simulator.
     *
     * This method should be called after an iteration right before the
     * simulator starts the next iteration.
     *
     * @param[in] currentTime, the current time of the simulator.
     * @param[in] nextTime, the time of the next iteration.
     * @return, the next time granted by the simulator.
     */
    static TIME getNextTime(TIME currentTime, TIME nextTime);
    
    /**
     * Should be called by a power simulator when it starts a new iteration.
     */
    static void timeStepStart(TIME currentTime);
    
    /**
     * Should be called by a network simulator when it 
     * finishes processing all events less than granted
     * time.
     * This method is the same as timeStepStart(), the different
     * name is a synctactic sugar.
     */
    static void grantTimeCompleted(TIME currentTime);

    /**
     * Registers an object for communication.
     *
     * Objects are referred to by name. Names are globally unique.
     * Registrations occur once at the beginning of an application. Once all
     * registrations have been made, the finalizeRegistrations() method should
     * be called.
     * 
     * @param objectName the name of the object that will be used by remote objects
     * @return the comminterface that provides send/receive methods for the object.
     */
    static ObjectCommInterface *getCommInterface(string objectName);

    /**
     * Registers an object for communication.
     * 
     * Objects are referred to by name. Names are globally unique.
     * Registrations occur once at the beginning of an application. Once all
     * registrations have been made, the finalizeRegistrations() method should
     * be called.
     * 
     * @param[in] objectName the name of the object that will be used by remote objects.
     * @param[in] st the strategy to be used for bufering the messages. Framework owns the strategyobject.
     * @return the comminterface that provides send/receive methods for the object.
     */
    static ObjectCommInterface *getCommInterface(string objectName,BufferStrategy *st);
    
     /**
     * Registers an object for communication.
     * 
     * Objects are referred to by name. Names are globally unique.
     * Registrations occur once at the beginning of an application. Once all
     * registrations have been made, the finalizeRegistrations() method should
     * be called.
     * 
     * This method is for convience, instead of expecilictly constructing one 
     * of the default buffering strategies, clients can just pass the type
     * 
     * @param[in] objectName the name of the object that will be used by remote objects.
     * @param[in] buffType buffering strategy type.
     * @return the comminterface that provides send/receive methods for the object.
     */
    static ObjectCommInterface *getCommInterface(string objectName,BufferStrategyType buffType);
    
     /**
     * Registers an object for communication.
     * 
     * Objects are referred to by name. Names are globally unique.
     * Registrations occur once at the beginning of an application. Once all
     * registrations have been made, the finalizeRegistrations() method should
     * be called.
     * 
     * This method is for convience, instead of expecilictly constructing one 
     * of the default buffering strategies, clients can just pass the type
     * 
     * @param[in] objectName the name of the object that will be used by remote objects.
     * @param[in] buffType buffering strategy type.
     * @return the comminterface that provides send/receive methods for the object.
     */
    static ObjectCommInterface *getCommInterface(char *objectName,BufferStrategyType buffType);
    
    /**
     * Indicate that communication object registrations have completed.
     *
     * This method is collective across all Integrator instances in order to
     * efficiently exchange metadata.
     */
    static void finalizeRegistrations();

    /**
      * Registers an object for communication.
     *
     * Objects are referred to by name. Names are globally unique.
     * Registrations occur once at the beginning of an application. Once all
     * registrations have been made, the finalizeRegistrations() method should
     * be called.
     * This method takes char* making it easier to bridge Fenix with C/Fortran
     * 
     * @param objectName the name of the object that will be used by remote objects.
     * @return the comminterface that provides send/receive methods for the object.
     */
    static ObjectCommInterface *getCommInterface(const char *objectName);
    
    /**
     * Registers an object for communication.
     *
     * Objects are referred to by name. Names are globally unique.
     * Registrations occur once at the beginning of an application. Once all
     * registrations have been made, the finalizeRegistrations() method should
     * be called.
     * This method takes char* making it easier to bridge Fenix with C/Fortran
     * 
     * @param objectName the name of the object that will be used by remote objects.
     * @param[in] st the strategy to be used for bufering the messages. Framework owns the strategyobject.
     * @return the comminterface that provides send/receive methods for the object.
     */
    static ObjectCommInterface *getCommInterface(const char *objectName,BufferStrategy *st);
    
    /**
     * Used by speculative threading to set the 
     * commanager of childern.
     */
    static void setCommManager(AbsCommManager* given);
    
    
    /**
     * Returns to packet lost window. Messages older than currentTime - packetLostPeriod are considered lost!
     */
    static TIME getPacketLostPeriod();
    
    /**
     * TODO
     */
    static time_metric getCurSimMetric();
    
    /**
     * Initializes the integrator from a configuration file.
     */
    static void initIntegrator(const char *configFile,TIME initialTime);

    /**
     * Returns current simulator time in framework adjusted format
     */
    static TIME getCurSimTime();

    /**
     * Initializes the integrator for a tick-based simulator
     */
    static void initIntegratorGracePeriod(AbsNetworkInterface *currentInterface, 
					  time_metric simTimeStep, 
					  TIME packetLostPeriod,
					  TIME initialTime,
					  TIME onetimestep=1
 					);
    /**
     * Initializes the integrator for the communication simulator
     */
    static void initIntegratorCommunicationSim(AbsNetworkInterface *currentInterface, 
					time_metric simTimeStep, 
					TIME packetLostPeriod,
					TIME initialTime,
					TIME onetimestep=1
					      );
    
    /**
     * Initializes the integrator with network delay support
     */
    static void initIntegratorConservativeSleepingTick(
			    AbsNetworkInterface* currentInterface, 
			    time_metric simTimeStep, 
			    TIME packetLostPeriod, 
			    TIME initialTime,
			    int numberofpowersims,
			    TIME ontimestep=1);
    
    
/**
     * Initializes the integrator with network delay support
     */
    static void initIntegratorConservativeSleepingComm(
			    AbsNetworkInterface* currentInterface, 
			    time_metric simTimeStep, 
			    TIME packetLostPeriod, 
			    TIME initialTime,
			    TIME onetimestep=1);
    
    /**
     * Initializes the integrator with optimistic sync algorithm
     */
    static void initIntegratorOptimistic(
        AbsNetworkInterface *currentInterface,
        time_metric simTimeStep,
	TIME packetLostPeriod,
        TIME initialTime,
	TIME specDifference,
	SpeculationTimeCalculationStrategy *strategy,
	TIME onetimestep=1);
    
    /**
     * Initializes the integrator with optimistic low overhead sync algorithm
     */
//    static void initIntegratorOptimisticLowOverhead(
//        AbsNetworkInterface *currentInterface,
//        time_metric simTimeStep,
//	TIME packetLostPeriod,
//        TIME initialTime,
//	TIME specDifference,
//	SpeculationTimeCalculationStrategy *strategy,
//	TIME onetimestep=1);
    
    /**
     * Initializes the integrator with optimistic sync algorithm for comm simm
     */
    static void initIntegratorOptimisticComm(
        AbsNetworkInterface *currentInterface,
        time_metric simTimeStep,
	TIME packetLostPeriod,
        TIME initialTime,
	TIME specDifference,
	SpeculationTimeCalculationStrategy *strategy,
	TIME onetimestep=1);
    
    /**
     * Initializes the integrator with optimistic low overhead sync algorithm for comm simm
     */
//    static void initIntegratorOptimisticCommLowOverhead(
//        AbsNetworkInterface *currentInterface,
//        time_metric simTimeStep,
//	TIME packetLostPeriod,
//        TIME initialTime,
//	TIME specDifference,
//	SpeculationTimeCalculationStrategy *strategy,
//	TIME onetimestep=1);
    
    /**
     * sets simulator callback that returns time
     */
    static void setTimeCallBack(CallBack<TIME,empty,empty,empty,empty> *t);

    /**
     * stops the integrator by calling its destructor.
     * This method should be called by the simulator to signal other simulators
     * it has finished simulating.
     */
    static void stopIntegrator();
    
    /**
     * Returns true when a simulator has signaled that it has finished.
     */
    static bool isFinished();

    /**
     * Sets the offset timestep.
     */
    static void setOffset(TIME otime);
    
    /**
     * Returns the time offset in framework time
     */
    static TIME getOffset();
    
    /**
     * 
     * Returns the time offset in simulator time 
     */
    static TIME getAdjustedOffset();
    
    /**
     * Register a callback to be executed right after the first phase of the synchronization (timeStepStart).
     *
     */
    static void registerTimeStepStartCallback(CallBack<void,TIME,empty,empty,empty>* callback);

    /**
     * Register a callback to be called right after determination of the next time step for the simulation.
     *
     */
    static void registerGetNextTimeCallback(CallBack<void,TIME,empty,empty,empty>* callback);

    /**
     * stops the integrator without sending
     * the finish signal to the simulators.
     */
    static void terminate();
    
    /**
     * Returns true only if the curreny synchronization
     * algorithm has forked a new child process.
     */
    static bool isChild();
    

    /**
     * Returns true if the current simulator
     * is a network simulator.
     * False otherwise.
     * For some sync algorithms network simulators
     * are special cases.
     */
    static bool amINetSim();

    /**
     * Returns true if current sync algo can fork
     * a new isntance of the simulator.
     */
    
    static bool canFork();
    /**
     * TODO
     */
    ~Integrator();
};

}

#endif // ABSINTEGRATOR_H
