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


#ifndef ABSMESSAGE_H
#define ABSMESSAGE_H
#include <string>
#include <stdint.h>
#include "util/time.h"
#include "integrator.h"

using namespace std;
namespace sim_comm {
class Message {
public:
    static string DESTIONATION_BCAST;
private:
    string from, to;
    TIME timeStamp;
    uint8_t tag;
    uint8_t *data;
    uint32_t size;
public:
    Message(string from,string to,TIME timeStamp);
    Message(const char *from,const char *to, TIME timeStamp);
    Message(string from,string to,TIME timeStamp,uint8_t* data,uint32_t dataSize,uint8_t tag=0);
    Message(const char *from,const char *to, TIME timeStamp,uint8_t* data,uint32_t dataSize,uint8_t tag=0);
    Message(const Message& other);
    /**
     * De serializes a message from the given array. The array is copied, so callers are responsible for deleteing it!
     * 
     */
    Message(uint8_t *given,uint32_t size);
    virtual ~Message();

    /* Returns the time stamp of message adjusted to sime time*/
    TIME getAdjustedTime();

    /* Returns the framework time of the message*/
    TIME getTime();

    string getTo() const {
        return to;
    }
    string getFrom() const {
        return from;
    }
    uint8_t getTag() const {
        return tag;
    }
    uint32_t getSize() const {
        return size;
    }
    void peek(uint8_t *peekBuffer,uint32_t size);
    /*Returns a copy of the data*/
    uint8_t* getData();
    bool isBroadCast();
    virtual void serialize(uint8_t*& buffToReturn,uint32_t& buffSize);
    virtual void deserialize(uint8_t *buff,uint32_t buffSize);
};

}
#endif // ABSMESSAGE_H
