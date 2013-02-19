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

#include <stdint.h>

#include <cstring>
#include <string>

#include "integrator.h"
#include "simtime.h"

using namespace std;


namespace sim_comm {
class Message {
public:
    static string DESTIONATION_BCAST;
private:
    string from;
    string to;
    TIME timeStamp;
    uint8_t tag;
    uint8_t *data;
    uint32_t size;
public:
    /** Explicit constructor. */
    Message(const string &from,
            const string &to,
            TIME timeStamp,
            uint8_t *data,
            uint32_t dataSize,
            uint8_t tag=0);

    /** Explicit constructor. */
    Message(const char *from,
            const char *to,
            TIME timeStamp,
            uint8_t *data,
            uint32_t dataSize,
            uint8_t tag=0);

    /** Constructor which deserializes the given envelop. */
    Message(uint8_t *envelop, uint32_t size, uint8_t *data=NULL);

    Message(const Message& other);

    virtual ~Message();

    /** Returns the time stamp of message adjusted to sime time. */
    TIME getAdjustedTime();

    /** Returns the framework time of the message. */
    inline TIME getTime() const {
        return this->timeStamp;
    }

    inline string getFrom() const {
        return this->from;
    }

    inline string getTo() const {
        return this->to;
    }

    inline uint8_t getTag() const {
        return this->tag;
    }

    inline uint32_t getSize() const {
        return this->size;
    }

    inline const uint8_t* const getData() const {
        return data;
    }

    inline void setData(const uint8_t *data, uint32_t dataSize) {
        /* remove old data */
        if (NULL != this->data) {
            delete [] this->data;
            this->data=NULL;
        }
        /* create new data */
        if (dataSize > 0) {
            this->data=new uint8_t[dataSize];
            memcpy(this->data,data,dataSize);
        }
        else {
            this->data=NULL;
        }
    }

    inline bool isBroadCast() const {
        return this->to.compare(Message::DESTIONATION_BCAST)==0;
    }

    void serializeHeader(uint8_t* &buffToReturn, uint32_t &buffSize) const;

    void deserializeHeader(uint8_t *buff, uint32_t buffSize);

    bool operator==(const Message &that) const;
    bool operator!=(const Message &that) const;
};

ostream& operator<<(ostream &os, const Message &message);

}
#endif // ABSMESSAGE_H
