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
#include "config.h"

#include "optimisticlowoverheadticksyncalgo.h"

#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <assert.h>

namespace sim_comm{


void OptimisticLowOverheadTickSyncAlgo::childDied()
{
    assert(childPid>0);
    specFailTime=comm->failTime;
    (void) kill(this->childPid,SIGKILL);
    wait(nullptr);
    st->speculationFailed(specFailTime);

#ifdef DEBUG_WITH_PROFILE
    CERR << "Received signal that my child is dead " << specFailTime << " new specDiff " << st->getSpecTime() << " " << getCurTimeInMs() << endl;
#elif DEBUG
    CERR << "Received signal that my child is dead " << specFailTime << " new specDiff " << st->getSpecTime() << endl;
#endif
    cout << "Failed" << endl;
}


OptimisticLowOverheadTickSyncAlgo::OptimisticLowOverheadTickSyncAlgo(AbsCommManager* interface, TIME specDifference,SpeculationTimeCalculationStrategy *strategy) : AbsSyncAlgorithm(interface)
{
    if(!this->interface->supportsFork())
        throw SyncStateException(string("Optimistic algo can only be used if the underlying network itnerface supports fork!"));

    CallBack<bool,Message*,empty,empty,empty> *syncAlgoCallBackSend=
        CreateObjCallback<OptimisticLowOverheadTickSyncAlgo*, bool (OptimisticLowOverheadTickSyncAlgo::*)(Message *),bool, Message*>(this,&OptimisticLowOverheadTickSyncAlgo::nodeSentMessage);
    CallBack<bool,Message*,empty,empty,empty> *syncAlgoCallBackRev=
        CreateObjCallback<OptimisticLowOverheadTickSyncAlgo*, bool (OptimisticLowOverheadTickSyncAlgo::*)(Message *),bool, Message*>(this,&OptimisticLowOverheadTickSyncAlgo::nodeReceivedMessage);

    this->interface->setSyncAlgoCallBacks(syncAlgoCallBackSend,syncAlgoCallBackRev);
    this->specFailTime=Infinity;
    this->mypid=getpid();
    this->parentPid=0;
    this->childPid=0;
    this->isChild=false;
    this->isParent=true;

    this->st=strategy;
    this->busywait=false;
#ifdef DEBUG_WITH_PROFILE
    setElsapedTimer();
#endif

    //create shared memory segment
    this->comm=nullptr;
    this->createTimeShm();

    //disable kill on term, so term signal terminates only the current process.
    this->interface->doKillOnTerm(false);
}


OptimisticLowOverheadTickSyncAlgo::~OptimisticLowOverheadTickSyncAlgo()
{
    detachTimeShm();
    if(this->st!=nullptr)
        delete st;
}


bool OptimisticLowOverheadTickSyncAlgo::nodeReceivedMessage(Message* msg)
{
    if(this->isChild){
        cout << "I'm child with pid:" << this->mypid << " signaling parent " << getppid() << " to kill me!!!" << Integrator::getCurSimTime() << endl;
#ifdef DEBUG_WITH_PROFILE
        CERR << "I'm child with pid:" << this->mypid << " signaling parent " << getppid() << " to kill me!!!" << Integrator::getCurSimTime() << " " << getCurTimeInMs() << endl;
#elif DEBUG
        CERR << "I'm child with pid:" << this->mypid << " signaling parent " << getppid() << " to kill me!!!" << Integrator::getCurSimTime() << endl;
#endif
        comm->action=ACTION_FAILED;
        exit(EXIT_FAILURE);
    }
    else{
        return true;
    }
}


bool OptimisticLowOverheadTickSyncAlgo::nodeSentMessage(Message* msg)
{
#ifdef DEBUG
    CERR << "OptimisticLowOverheadTickSyncAlgo::nodeSentMessage(Message* msg)" << endl;
#endif
    if(this->isChild){
#ifdef DEBUG_WITH_PROFILE
        CERR << "I'm child with pid:" << this->mypid << " signaling parent " << getppid() << " to kill me!!!" << Integrator::getCurSimTime() << " " << getCurTimeInMs() << endl;
#elif DEBUG
        CERR << "I'm child with pid:" << this->mypid << " signaling parent " << getppid() << " to kill me!!!" << Integrator::getCurSimTime() << endl;
#endif
#ifdef PROFILE
        specFailed();
#endif
        comm->action=ACTION_FAILED;
        exit(EXIT_FAILURE);
    }
    else{
        return true;
    }
}


void OptimisticLowOverheadTickSyncAlgo::timeStepStart(TIME currentTime)
{
#ifdef PROFILE
    if(!busywait)
        syncStart();
#endif
    if(currentTime < grantedTime){
        return;
    }
#if DEBUG_WITH_PROFILE
    CERR <<  "Start sync time step " << currentTime << " " << getCurTimeInMs() << endl;
#elif DEBUG
    CERR << "Start sync time step " << currentTime <<  endl;
#endif
    if(this->isChild){
#if DEBUG
        CERR << "I'm child my pid:" << this->mypid << "; exit(success); " << endl;
#endif
        cout << "Success" << endl;
        comm->failTime=currentTime;
        comm->action=ACTION_SUCCESS;
        exit(EXIT_SUCCESS);
    }
    else{
        //cout << "I'm parent waiting for others!!" << endl;
        this->interface->waitforAll();
    }
}


bool OptimisticLowOverheadTickSyncAlgo::doDispatchNextEvent(TIME currentTime, TIME nextTime)
{
    return GetNextTime(currentTime,nextTime);
}



TIME OptimisticLowOverheadTickSyncAlgo::GetNextTime(TIME currentTimeParam, TIME nextTime)
{
    TIME nextEstTime;
    TIME myminNextTime;
    TIME currentTime=currentTimeParam;
    //we have processed upto including grated time
    if(nextTime <= grantedTime){
#ifdef PROFILE
        writeTime(currentTimeParam);
#endif
        return nextTime;
    }

    bool canSpeculate=false;
    bool needToRespond=false;
    busywait=false;
    //send all messages
#if DEBUG
    CERR << "Start sync " << currentTime << " " << nextTime << endl;
#endif
    if(currentTime < grantedTime){ //we still have some granted time we need to barier at granted Time
        busywait=true;
        currentTime=grantedTime;
    }
    if(currentTime > this->specFailTime){ //we passed beyond spec. failure time. We can speculatie again.
        this->specFailTime=Infinity;
    }
    do
    {
        canSpeculate=false;
        if(busywait)
            this->timeStepStart(currentTime);
        //we don't need barier
        uint64_t diff=interface->reduceTotalSendReceive();
        //nextEstTime is the granted time the simulator exptects.
        //myminNextTime is the minimum next time the smulator can process.
        //usually for conservative algorithm these two numbers are the same
        //for optimistic however when get knowledge about the the dead time of child process
        //we can use it as the granted time.
        //if I have a packet, I can only 
        nextEstTime=currentTimeParam+Integrator::getOneTimeStep();
        myminNextTime=nextEstTime;
        TIME minnetworkdelay=interface->reduceNetworkDelay();
        if(diff==0 && !needToRespond)
        { //network stable grant next time
            myminNextTime=nextEstTime=nextTime;
            if(specFailTime!=Infinity && nextTime < specFailTime){ //we can grant upto spec fail time
#ifdef DEBUG
                CERR << "I'm grating my self " << specFailTime << endl;
#endif
                nextEstTime=specFailTime;
                //since we know the spec will fail until this time, we won't fork!
                canSpeculate=false;
            }
            else{
                canSpeculate=true;
            }
        }
        else{
            needToRespond=true;
        }
        TIME minNextTime=nextEstTime;

#ifdef DEBUG
        CERR << "Consensus on message-diff " << diff << endl;
#endif
        TIME specNextTime=0;
        if(diff==0){
            if(!hasChild()){ //we do speculation calculation only if we can speculate
                //we should never attempt to exchange specDiff when Diff > 0, this is an optimization.
                //canSpeculate can be false regardless of Diff==0, in this case
                //we have simulator that needs to respond. So we need to signal others
                //not to speculate at all.
                TIME mySpecNextTime;

                if(canSpeculate && st->worthSpeculation(currentTime,specFailTime)){ //test if it is worht speculating!
                    mySpecNextTime=st->getNextSpecTime(currentTime);
                }
                else{
                    mySpecNextTime=0;
                }
                //if we don't have a child, we aggregate specNextTime
                //with with minNextTime
                specNextTime=mySpecNextTime;
                interface->aggreateReduceMin(minNextTime,specNextTime);
            }
            else{
                //if we have child we aggregate action
                //with minNextTime
                this->globalAction=comm->action;
                interface->aggreateReduceMin(minNextTime,this->globalAction);
            }
        } else{ //well, we have message so we use regular reduce min op.
            minNextTime=(TIME)interface->reduceMinTime(nextEstTime);
        }
        //aggregate reduce min operation.

#ifdef DEBUG
        CERR << "Consensus " << minNextTime << " spec: " << specNextTime << " diff:"<< st->getSpecTime() << endl;
        assert(specNextTime==0 || specNextTime > currentTime);
#endif
        //speculation stuff
        TIME specResult=testSpeculationState(specNextTime,currentTime);
        if(specResult > 0) //we are in child! We are granted up to specNextTime
            minNextTime=specNextTime;

        if(minNextTime==0){ //a sim signal endded
#if DEBUG
            CERR << "End Signaled!" << endl;
#endif
            this->finished=true;
            return 0;
        }

        if(minNextTime < myminNextTime){
            //next time is some seconds away and the simulator has to wait so fork speculative unless we already forked
            //update the value of the currentTime
            currentTime=minNextTime;
            busywait=true;
        }
        else{
            busywait=false;
        }

        this->grantedTime=minNextTime;

    }while(busywait);

#ifdef PROFILE
    writeTime(currentTimeParam);
#endif

    return myminNextTime;
}



void OptimisticLowOverheadTickSyncAlgo::createSpeculativeProcess()
{
    pid_t forkpid = fork();
    if (-1 == forkpid) {
        /* I am the parent, and an error was detected */
        perror("createSpeculativeProcess: fork");
        exit(0);
    }
    else if (0 == forkpid) {
        /* I am a child */
        becomeChild();
    }
    else {
        /* I am a parent */
        gotChild(forkpid);
    }
}


void OptimisticLowOverheadTickSyncAlgo::becomeChild()
{
    this->isParent=false;
    this->isChild=true;
    this->mypid=getpid();
    this->parentPid=getppid();
    this->childPid=0;
}


void OptimisticLowOverheadTickSyncAlgo::gotChild(pid_t childpid)
{
    this->isParent=true;
    this->isChild=false;
    this->mypid=getpid();
    this->parentPid=0;
    this->childPid=childpid;
}


TIME OptimisticLowOverheadTickSyncAlgo::testSpeculationState(TIME specNextTime,TIME currentTime)
{
    if(!hasChild() && specNextTime!=0){
#ifdef DEBUG_WITH_PROFILE
        CERR << "Creating child process for specTime " << specNextTime << " " << getCurTimeInMs() << endl;
#elif DEBUG
        CERR << "Creating child process for specTime " << specNextTime << endl;
#endif
        cout << "forking" << endl;
        this->createSpeculativeProcess();
        if(this->isChild){ //createSpeculativeProcess will modify this flag!
            st->startSpeculation(currentTime);
            this->specFailTime=Infinity;
#ifdef DEBUG
            CERR << this->mypid << ": I'm child, current time" << currentTime << " run until no sync " << specNextTime << endl;
#endif

#ifdef PROFILE
            speced();
#endif
            return specNextTime;
        }else{
            st->startSpeculation(currentTime);
#ifdef DEBUG
            CERR << this->mypid << ": I'm parent, current time" << currentTime << " re-init complete" << comm->action << endl;
#endif
            return 0;
        }
    } 
    if(hasChild()){
#if DEBUG
        CERR << "MY action is " << globalAction << endl;
#endif
        switch(globalAction){
            case ACTION_FAILED:
                childDied();
                break;
            case ACTION_SUCCESS:
                childDied();
                break;
            default:
                break;
        }

    }

    return 0;
}

void OptimisticLowOverheadTickSyncAlgo::createTimeShm()
{
    if(this->isChild){
        throw SyncStateException("Child cannot create shared memory, it is already created!");
    }else{
        if(this->comm!=nullptr)
            throw SyncStateException("Fail time shared memory is already created?");
        this->comm = (shmitems*) mmap(
                NULL, sizeof(shmitems), PROT_READ|PROT_WRITE,
                MAP_SHARED|MAP_ANONYMOUS, -1, 0);
        if (this->comm == MAP_FAILED) {
            throw SyncStateException("Error creating shared memory!");
        }
    }
}

void OptimisticLowOverheadTickSyncAlgo::detachTimeShm()
{
    if(this->comm!=nullptr){
        if (munmap(this->comm, sizeof(shmitems)) == -1) {
            throw SyncStateException("Shared memory detach failed");
        }
        this->comm=nullptr;
    }
}


}
