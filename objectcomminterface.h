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
#include "absintegrator.h"

#include "utils/time.h"
#include "absmessage.h"
#include <vector>

using namespace std;

namespace sim_comm{
  /*This class provides the send/receive message interface for objects that want to communicate with other object through comm simm
   */
class ObjectCommInterface
{
  friend class AbsIntegrator;
  private:
    string attachedObjectName;
    vector<AbsMessage*> inbox,outbox;
    ObjectCommInterface(string objectName);
    void newMessage(AbsMessage* given);
    vector<AbsMessage*> getOutBox();
    void clear();
public:
  /*Returns the number of messages the ibject has for the current time*/
  int getInboxMessagesCount();
  /*Returns true if object has more messages for the currentim time*/
  bool hasMoreMessages();
  /*Returns the number a message for the current time 
   * The returned pointer is owned by the client, the framework does not delte it.
   */
  AbsMessage* getInboxMessage();
  /*Returns all messages the object received for the current time frame.
   * The returned pointer is owned by the client, the framework does noit delete them
   */
  vector<AbsMessage*> getAllInboxMessages();
  /* Sends a message the given pointer is owned by the framework and framework deltes it!
   * 
   */
  void send(AbsMessage *given);
  ~ObjectCommInterface();
};


}
#endif // ABSOBJECTCOMMINTERFACE_H
