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
#ifndef OBJECTCOMMINTERFACE_H
#define OBJECTCOMMINTERFACE_H

#include <string>
#include <vector>

#include "integrator.h"
#include "message.h"
#include "simtime.h"
#include "callback.h"

using namespace std;


namespace sim_comm {


class AbsCommInterface;
class AbsCommManager;

/**
 * Abstract class defining methods to access
 * the message buffer.
 * Subclasses should implement doBufferMessage method
 * 
 */
class BufferStrategy{
private:
  ObjectCommInterface *controlInterface;
protected:
  int getNumberOfMessage();
  void clearInbox();
  Message *getMessage(int index);
  void removeMessage(int index);
  
public:
  /**
   * This method is called when the object comm interface
   * recevied a new message. It decides whether the message
   * will be buffered or not.
   * 
   * @param[in] given the new message.
   * @return true if the message should be buffered, false otherwise.
   */
  virtual bool doBufferMessage(Message *given) =0;
  
  /**
   * Constructor
   */
  BufferStrategy();
  void setCommInterface(ObjectCommInterface *given);
};

/**
 * This class provides the send/receive message interface for objects that want
 * to communicate with other objects through comm simm.
 */
class ObjectCommInterface {
    friend class Integrator;
    friend class AbsCommManager;
    friend class BufferStrategy;
private:
    string attachedObjectName;
    vector<Message*> inbox;
    vector<Message*> outbox;
    //AbsMessage *nextMessage;
    vector<int> msgs;
    vector<int>::iterator it;
    BufferStrategy *st;
    CallBack<void,empty,empty,empty> *notifyMessage;
    CallBack<bool,Message*,empty,empty> *syncAlgoCallBackSend; 
    
    /** Constructor. */
    ObjectCommInterface(string objectName, BufferStrategy *st=NULL);

      /** Inserts a received message to the interfaces inbox.
     * The message should be a shared pointer and callers shoud not delete it.
     */
    void newMessage(Message* given);

public:
    /**
     * This sets the callback to sync algorithm that is called when an object calls
     * send().
     */
    void setSyncAlgoCallBack(CallBack<bool,Message*,empty,empty> *syncAlgoCallBackSend);
    
     /** Clears the outbox of the system */
    void clear();

   

    /** TODO */
    vector<Message*> getOutBox();
    
    /*/** Returns the number of messages the ibject has for the current time */
    //int getInboxMessagesCount();

    /** Returns true if object has more messages for the currentim time */
    bool hasMoreMessages();

    /**
     * Returns the number a message for the current time
     *
     * The returned pointer is owned by the client, the framework does not
     * delte it.
     */
    Message* getNextInboxMessage();

    /**
     * Returns all messages the object received for the current time frame.
     * The returned pointer is owned by the client, the framework does not
     * delete them.
     */
    vector<Message*> getAllInboxMessages();

    /** 
     * Sends a message the given pointer is owned by the framework and
     * framework deltes it!
     */
    void send(Message *given);

    /** Destructor. */
    ~ObjectCommInterface();
    
    /**
     * Register a callback that is called when the object receives a message.
     */
    void setMessageNotifier(CallBack<void,empty,empty,empty> *tonotify);
  
};



class KeepLastStrategy : public BufferStrategy{

public:
  virtual bool doBufferMessage(Message *given);
  /**
  * Constructor
  */
  KeepLastStrategy();
  
};

class KeepFirstStrategy : public BufferStrategy{
public:
  virtual bool doBufferMessage(Message *given);
  /**
   * Constructor
   */
  KeepFirstStrategy();
};

} /* end namespace sim_comm */

#endif /* ABSOBJECTCOMMINTERFACE_H */

