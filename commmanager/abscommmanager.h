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


#ifndef ABSCOMMMANAGER_H
#define ABSCOMMMANAGER_H

#include <exception>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>

#include "absnetworkinterface.h"
#include "message.h"


namespace sim_comm{
  
  class ObjectCommInterface;

  class CommManagerOperationNotSupportedException : public exception{
    public:
      CommManagerOperationNotSupportedException(const string &operation) throw(){
	ostringstream out;
	out << "Operation " << operation << " is not supported." << endl;
	op = out.str();
      }
      virtual ~CommManagerOperationNotSupportedException() throw(){}
      virtual const char* what() const throw() {
	  return op.c_str();
      }
    private:
      string op;
  };
  
  class ObjectInterfaceRegistrationException : public exception {
    public:
      ObjectInterfaceRegistrationException(const string &where, const int &line) throw() {
        ostringstream out;
        out << "No more registrations are allowed at this time." << endl;
        out << where << ": line " << line << endl;
        msg = out.str();
      }
      ObjectInterfaceRegistrationException(const string &object, const string &where, const int &line) throw() {
        ostringstream out;
        out << "could not locate object `" << object << "'" << endl;
        out << where << ": line " << line << endl;
        msg = out.str();
      }
      virtual ~ObjectInterfaceRegistrationException() throw() {
      }
      virtual const char* what() const throw() {
	  return msg.c_str();
      }
      private:
	  string msg;
      };
   
   class InterfaceErrorException : public exception {
      virtual const char* what() const throw() {
        return "Interface operation failed!!";
      }
    };

  class SyncAlgoErrorException : public exception {
      virtual const char* what() const throw() {
	  return "Operation not supported with this syncalgorithm.";
      }
  };

  class SerializationException : public exception{
      virtual const char* what() const throw() {
	  return "Received/Sent message serialization error.";
      } 
  };

   
  class AbsCommManager
  {
    protected :
	uint64_t sendCount; /**< @TODO doc */
	uint64_t receiveCount; /**< @TODO doc */

	map<string,ObjectCommInterface*> interfaces; /**< @TODO doc */
	bool receiverRunning; /**< @TODO doc */
	bool allowRegistrations;
      
	TIME minNetworkDelay;
	//uint8_t messageReceived();
	CallBack<bool,Message*,empty,empty> *syncAlgoCallBackSend,*syncAlgoCallBackRecv;
	sim_comm::AbsNetworkInterface *currentInterface;
	void adjustNetworkDelay(TIME msgDeliveryTime);
	AbsCommManager(AbsCommManager &given);
    public:
      /**
	  * Called by subclasses to notify about a new message.
	  * Sub classes should not notify objectcomminterface themselves
	  * instead they should call this method.
	  */
	virtual void messageReceived(Message *message);
	
	AbsCommManager(AbsNetworkInterface *networkInterface);
	virtual ~AbsCommManager();
      /**
     * Used by the integrator to register an object.
     *
     * @param[in] objectName TODO
     * @param[in] given TODO
     */
    virtual void addObjectInterface(string objectName,ObjectCommInterface *given);

    /**
     * Used by the integrator to return a registered object.
     *
     * @param[in] objectName TODO
     * @return the object, or NULL if not found
     */
    virtual ObjectCommInterface* getObjectInterface(string objectName);

    /**
     * Indicate that communication object registrations have completed.
     *
     * This method is collective across all AbsCommInterface instances in order
     * to efficiently exchange metadata.
     */
    virtual void finalizeRegistrations();

    /**
     * Starts the receiver thread.
     */
    virtual void startReceiver();

    /**
     * Returns true if the reciever thread is running.
     */
    virtual bool isReceiverRunning();

    /**
     * Kills the receiver thread.
     */
    virtual void stopReceiver();

    /**
     * Resets the counters, should be called from
     * syncalgorithms. Otherwise incosistent behavior 
     * can be observed.
     */
    void resetCounters(){
      this->sendCount=this->receiveCount=0;
    }
    /**
     * Called by the integrator to send all the messages.
     * subclasses should implement simulator specific sendalog
     */
    virtual void sendAll() =0;
    
    /**
     * Called by the sync algorithm to notify that a packetLostis lost.
     * Currently this method only works if given is an instance of commsimsync algo
     */
    virtual void packetLostCalculator(TIME currentTime) =0;
    
    
    virtual uint64_t reduceTotalSendReceive();
    
    virtual uint64_t reduceMinTime(uint64_t currentTime);
    
    virtual uint64_t reduceNetworkDelay();
    /**
     * Sets the callback for the sync algorithm.
     */
    void setSyncAlgoCallBacks(CallBack<bool,Message*,empty,empty> *syncCallBackSend,
			      CallBack<bool,Message*,empty,empty> *syncCallBackRecv){
      this->syncAlgoCallBackRecv=syncCallBackRecv;
      this->syncAlgoCallBackSend=syncCallBackSend;
    }
    
    /**
     * Calls the barier function of the network interface to pause the execution.
     */
    virtual void waitforAll();
    
    /**
     * Returns the min network delay.
     * This value is always equal to infinity for network simulator.
     */
    TIME getMinNetworkDelay(){
      
      return minNetworkDelay!=Infinity? this->minNetworkDelay : 0;
    }
    
    /**
     * Creates a copy of the commanger. It also calls 
     * the duplicate method of the AbsInterface.
     */
     virtual AbsCommManager* duplicate() =0;
    
     /**
      * Causes the calling simulator to sleep until all simulator go to sleep.
      */
     virtual bool sleep();
    /**
     * Signals to others that we are finished.
     */
    virtual void sendFinishedSignal();

    /**
     * An all gather operation on the next times of the simulators.
     */
    virtual TIME* getNextTimes(TIME nextTime,uint32_t &size);
    
    /**
     * Send notification to other processes in the
     * context, the failed signal.
     */
    virtual void sendFailed(){ this->currentInterface->sendFailed(); }
    
    /**
     * Send notification to other processes in the context
     * a succeed signal.
     */
    virtual void sendSuceed(){ this->currentInterface->sendSuceed(); }
    
    /**
     * Returns true of current networkInterface supports fork operation
     */
    bool supportsFork() { return this->currentInterface->supportsFork(); }
    
    /**
     * Executes steps in the underlying network interface to prepare for 
     * fork operation.
     */
    virtual void prepareFork();
  };

}

#endif // ABSCOMMMANAGER_H
