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

#include <string>

#include "callback.h"
#include "simtime.h"
#include "abssyncalgorithm.h"

using namespace std;

namespace sim_comm {

class AbsCommInterface;
class ObjectCommInterface;

/**
 * Integrator is the main init point for the framework.
 * It bridges the framework with the simulator.
 * Provides methods for easly accessing simulator and framework parameters.
 * It is a singleton and all methods are made static for convenience, users do not need to
 * pass instances of integrator around.
 */
class Integrator {
private:
    time_metric simTimeMetric;
    AbsCommInterface *currentInterface;
    TIME gracePreiod;
    CallBack<TIME,empty,empty,empty>* getTimeCallBack;
    AbsSyncAlgorithm *syncAlgo;
    TIME offset;
    bool allowRegistrations;
    bool stopped;

    static Integrator* instance;

    /**
     * Constructor.
     */
    Integrator(
    		AbsCommInterface *currentInterface,
    		AbsSyncAlgorithm *algo,
    		time_metric simTimeStep,
    		TIME gracePeriod);
public:

public:

    /**
     * Main synchronization method for tick-based simulator. This method should be called
     * after an iteration right before the simulator starts the next iteration.
     * @param[in] currentTime, the current time of the simulator.
     * @param[in] nextTime, the time of the next iteration.
     * @return, the next time granted by the simulator.
     */
    static TIME getNextTime(TIME currentTime, TIME nextTime);

    /**
     * TODO
     */
    static bool doDispatchNextEvent(TIME currentTime, TIME nextTime);

    /**
     * Registers an object for communication.
     *
     * Objects are referred to by name. Names are globally unique.
     * Registrations occur once at the beginning of an application. Once all
     * registrations have been made, the finalizeRegistrations() method should
     * be called.
     */
    static ObjectCommInterface *getCommInterface(string objectName);

    /**
     * Indicate that communication object registrations have completed.
     *
     * This method is collective across all Integrator instances in order to
     * efficiently exchange metadata.
     */
    static void finalizeRegistrations();

    /**
     * TODO
     */
    static ObjectCommInterface *getCommInterface(char *objectName);

    /**
     * Returns the simulator adjusted grace period
     */
    static TIME getAdjustedGracePeriod();

    /**
     * Returns the grace period in framework time
     */
    static TIME getGracePreiod();

    /**
     * TODO
     */
    static time_metric getCurSimMetric();

    /**
     * Returns current simulator time in framework adjusted format
     */
    static TIME getCurSimTime();

    /**
     * Initializes the integrator for a tick-based simulator
     */
    static void initIntegratorGracePeriod(AbsCommInterface *currentInterface, 
					  time_metric simTimeStep, 
					  TIME gracePeriod, TIME initialTime);
    /**
     * Initializes the integrator for the communication simulator
     */
    void initIntegratorCommunicationSim(AbsCommInterface *currentInterface, 
					time_metric simTimeStep, 
					TIME gracePeriod, TIME initialTime);


    /**
     * sets simulator callback that returns time
     */
    static void setTimeCallBack(CallBack<TIME,empty,empty,empty> *t);

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
    ~Integrator();
};

}

#endif // ABSINTEGRATOR_H
