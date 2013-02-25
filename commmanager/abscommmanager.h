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
	bool doincrementCountersInSendReceive;
	map<string,ObjectCommInterface*> interfaces; /**< @TODO doc */
	bool receiverRunning; /**< @TODO doc */
	bool allowRegistrations;
      
	/**
	  * Called by subclasses to notify about a new message.
	  * Sub classes should not notify objectcomminterface themselves
	  * instead they should call this method.
	  */
	virtual void messageReceived(Message *message);
	//uint8_t messageReceived();
	
	sim_comm::AbsNetworkInterface *currentInterface;
    public:
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
     * Called by the integrator to send all the messages.
     * subclasses should implement simulator specific sendalog
     */
    virtual void sendAll() =0;
    
    /**
     * Called by the sync algorithm to notify that a packetLostis lost.
     * Currently this method only works if given is an instance of commsimsync algo
     */
    virtual void packetLost() =0;
    
    
    virtual uint64_t reduceTotalSendReceive();
    
    virtual uint64_t reduceMinTime(uint64_t currentTime);
    
  };

}

#endif // ABSCOMMMANAGER_H