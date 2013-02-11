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

#include "util/callback.h"
#include "util/time.h"

using namespace std;

namespace sim_comm {

class AbsCommInterface;
class ObjectCommInterface;

class Integrator {
private:
    time_metric simTimeMetric;
    int numberOfCommNodes;
    AbsCommInterface *currentInterface;
    TIME gracePreiod;
    CallBack<TIME,empty,empty,empty>* getTimeCallBack;

    static Integrator* instance;

public:
    /**
     * TODO
     */
    Integrator(
            AbsCommInterface *currentInterface,
            time_metric simTimeStep,
            int numberOfCommNodes,
            TIME gracePeriod);

    /**
     * TODO
     */
    virtual TIME getNextTime(TIME currentTime, TIME nextTime);

    /**
     * TODO
     */
    virtual bool doDispatchNextEvent(TIME currentTime, TIME nextTime);

    /**
     * TODO
     */
    static ObjectCommInterface *getCommInterface(string objectName);

    /**
     * TODO
     */
    static ObjectCommInterface *getCommInterface(char *objectName);

#if 0
    /**
     * TODO
     */
    static int getRank();

    /**
     * TODO
     */
    static int getNumberOfCommNodes();
#endif

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
     * TODO
     */
    static void initIntegrator(
            AbsCommInterface *currentInterface,
            time_metric simTimeStep,
            int numberOfCommNodes,
            TIME gracePeriod);

    /**
     * sets simulator callback that returns time
     */
    static void setTimeCallBack(CallBack<TIME,empty,empty,empty> *t);
};

}

#endif // ABSINTEGRATOR_H
