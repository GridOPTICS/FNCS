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

#include <cassert>
#include <cstring>
#include <sstream>
#include <vector>

#include "message.h"

using namespace std;


namespace sim_comm {


string Message::DESTIONATION_BCAST("*");


Message::Message(
        const string &from,
        const string &to,
        TIME timeStamp,
        const uint8_t *data,
        uint32_t dataSize,
        uint8_t tag) {
#if DEBUG
    CERR << "Message::Message(...)" << endl;
#endif
    this->from=from;
    this->to=to;
    this->timeStamp=convertToFrameworkTime(Integrator::getCurSimMetric(),timeStamp);
    if (dataSize > 0) {
        this->data=new uint8_t[dataSize];
        memcpy(this->data,data,dataSize);
    }
    else {
        this->data=NULL;
    }
    this->size=dataSize;
    this->tag=tag;
#if DEBUG
    CERR << *this << endl;
#endif
}


Message::Message(
        const char *from,
        const char *to,
        TIME timeStamp,
        const uint8_t *data,
        uint32_t dataSize,
        uint8_t tag) {
#if DEBUG
    CERR << "Message::Message(...)" << endl;
#endif
    this->from=string(from);
    this->to=string(to);
    this->timeStamp=convertToFrameworkTime(Integrator::getCurSimMetric(),timeStamp)
      -Integrator::getOffset();
    
    if (dataSize > 0) {
        this->data=new uint8_t[dataSize];
        memcpy(this->data,data,dataSize);
    }
    else {
        this->data=NULL;
    }
    this->size=dataSize;
    this->tag=tag;
#if DEBUG
    CERR << *this << endl;
#endif
}


Message::Message(uint8_t *envelope, uint32_t size, uint8_t *data) {
#if DEBUG
    CERR << "Message::Message(...) using deserializeHeader" << endl;
#endif
    //this->data=data;
    this->deserializeHeader(envelope,size);
    if(data!=nullptr){
      this->data=new uint8_t[this->size];
      memcpy(this->data,data,this->size);
    }
    else{
      this->data=nullptr;
    }
#if DEBUG
    CERR << *this << endl;
#endif
}


Message::Message(const Message& other) {
#if DEBUG
    CERR << "Message::Message(const Message&)" << endl;
#endif
    this->from=other.from;
    this->to=other.to;
    this->timeStamp=other.timeStamp;
    if (other.size > 0) {
        this->data=new uint8_t[other.size];
        memcpy(this->data,other.data,other.size);
    }
    else {
        this->data=NULL;
    }
    this->size=other.size;
    this->tag=tag;
}


TIME Message::getAdjustedTime() {
#if DEBUG
    CERR << "Message::getAdjustedTime()" << endl;
#endif
    time_metric mySimMetric=Integrator::getCurSimMetric();
    return convertToMyTime(mySimMetric,this->timeStamp)+Integrator::getAdjustedOffset();
}


void Message::serializeHeader(uint8_t*& buffToReturn,uint32_t& buffSize) const {
#if DEBUG
    CERR << "Message::serializeHeader(...)" << endl;
#endif
    vector<uint8_t> buff;
    const uint8_t* timeptr=NULL;
    const uint8_t* sizeptr=NULL;

    /* attempt to guess how much space we'll need to serialize */
    buff.reserve(
            this->from.size() + 1 +
            this->to.size() + 1 +
            sizeof(this->timeStamp) + 1 +
            sizeof(this->size) + 1 +
            1 + 1);

    /* from */
    for(string::const_iterator it=this->from.begin(), limit=this->from.end();
            it!=limit; ++it) {
        buff.push_back(static_cast<uint8_t>(*it));
    }
    buff.push_back(0);

    /* to */
    for(string::const_iterator it=this->to.begin(), limit=this->to.end();
            it!=limit; ++it) {
        buff.push_back(static_cast<uint8_t>(*it));
    }
    buff.push_back(0);

    /* timeStamp */
    timeptr=reinterpret_cast<const uint8_t*>(&this->timeStamp);
    for(int i=0, limit=sizeof(this->timeStamp); i<limit; i++) {
        buff.push_back(timeptr[i]);
    }
    buff.push_back(0);

    /* size */
    sizeptr=reinterpret_cast<const uint8_t*>(&this->size);
    for(int i=0, limit=sizeof(this->size); i<limit; i++) {
        buff.push_back(sizeptr[i]);
    }
    buff.push_back(0);

    /* tag */
    buff.push_back(tag);
    buff.push_back(0);

    buffToReturn=new uint8_t[buff.size()];
    copy(buff.begin(),buff.end(),buffToReturn);
    buffSize=buff.size();
}


void Message::deserializeHeader(uint8_t *buff, uint32_t buffSize) {
#if DEBUG
    CERR << "Message::deserializeHeader(...)" << endl;
#endif
    stringstream temp;
    uint32_t it=0;
    uint8_t *timeptr=NULL;
    uint8_t *sizeptr=NULL;

    /* from */
    for(; it<buffSize; it++) {
        if(buff[it]==0) {
            break;
        }
        temp << (char)buff[it];
    }
    if (it >= buffSize) {
        throw "TODO better exception for deserialize from";
    }
    ++it;
    this->from=temp.str();
    temp.str(string());

    /* to */
    for(; it<buffSize; it++) {
        if(buff[it]==0) {
            break;
        }
        temp << (char)buff[it];
    }
    if (it >= buffSize) {
        throw "TODO better exception for deserialize to";
    }
    ++it;
    this->to=temp.str();
    temp.str(string());

    /* timeStamp */
    timeptr=reinterpret_cast<uint8_t*>(&this->timeStamp);
    for(int i=0, limit=sizeof(this->timeStamp); i<limit && it<buffSize; i++) {
        timeptr[i]=buff[it++];
    }
    if (it >= buffSize) {
        throw "TODO better exception for deserialize time";
    }
    ++it;

    /* size */
    sizeptr=reinterpret_cast<uint8_t*>(&this->size);
    for(int i=0, limit=sizeof(this->size); i<limit && it<buffSize; i++) {
        sizeptr[i]=buff[it++];
    }
    if (it >= buffSize) {
        throw "TODO better exception for deserialize size";
    }
    ++it;

    /* tag */
    this->tag=buff[it++];
    if (it >= buffSize) {
        throw "TODO better exception for deserialize tag";
    }
    ++it;
    
    if (it != buffSize) {
        throw "TODO better exception for deserialize end";
    }
}


Message::~Message() {
#if DEBUG
    CERR << "Message::~Message()" << endl;
#endif
}


bool Message::operator==(const Message &that) const {
#if DEBUG
    CERR << "Message::operator==(const Message&)" << endl;
#endif
    if (this->from != that.from) {
        return false;
    }
    if (this->to != that.to) {
        return false;
    }
    if (this->timeStamp != that.timeStamp) {
        return false;
    }
    if (this->size != that.size) {
        return false;
    }
    if (this->tag != that.tag) {
        return false;
    }
    if (this->size > 0) {
        assert(NULL != this->data);
        assert(NULL != that.data);
        return (0 == memcmp(this->data, that.data, this->size));
    }
    else {
        return true;
    }
}


bool Message::operator!=(const Message &that) const {
#if DEBUG
    CERR << "Message::operator!=(const Message&)" << endl;
#endif
    return !(*this == that);
}


ostream& operator<<(ostream &os, const Message &message) {
    os << "=== Message ===" << endl;
    os << "from '" << message.getFrom() << "'" << endl;
    os << "to '" << message.getTo() << "'" << endl;
    os << "time " << message.getTime() << endl;
    os << "tag " << static_cast<int>(message.getTag()) << endl;
    os << "size " << message.getSize() << endl;
    return os;
}

}
