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

#include "message.h"
#include "util/time.h"
#include <cstring>
#include <vector>
#include <sstream>

namespace sim_comm{
  
string Message::DESTIONATION_BCAST("*");

Message::Message(string from, string to,TIME timeStamp)
{
  this->from=from;
  this->to=to;
  this->timeStamp=convertToFrameworkTime(Integrator::getCurSimMetric(),timeStamp);
}
 
Message::Message(char* from, char* to,TIME timeStamp)
{
  this->from=string(from);
  this->to=string(to);
  this->timeStamp=convertToFrameworkTime(Integrator::getCurSimMetric(),timeStamp);
}

Message::Message(string from, string to, TIME timeStamp, uint8_t* data, uint32_t dataSize, uint8_t tag)
{
  this->from=from;
  this->to=to;
  this->timeStamp=convertToFrameworkTime(Integrator::getCurSimMetric(),timeStamp);
  this->data=new uint8_t[dataSize];
  memcpy(this->data,data,dataSize);
  this->size=dataSize;
  this->tag=tag;
}

Message::Message(char* from, char* to, TIME timeStamp, uint8_t* data, uint32_t dataSize, uint8_t tag)
{
  this->from=string(from);
  this->to=string(to);
  this->timeStamp=convertToFrameworkTime(Integrator::getCurSimMetric(),timeStamp);
  this->data=new uint8_t[dataSize];
  memcpy(this->data,data,dataSize);
  this->size=dataSize;
  this->tag=tag;
}



Message::Message(const Message& other)
{
  this->from=other.from;
  this->to = other.to;
  this->data=new uint8_t[other.size];
  this->size=other.size;
  memcpy(this->data,data,size);
  this->tag=tag;
  
}

Message::Message(uint8_t* given,uint32_t size)
{
  this->deserialize(given,size);
}
  
bool Message::isBroadCast(){
  return this->to.compare(Message::DESTIONATION_BCAST)==0;
  
}
 

TIME Message::getAdjustedTime()
{
  time_metric mySimMetric=Integrator::getCurSimMetric();
  return convertToMyTime(mySimMetric,this->timeStamp);
}

void Message::serialize(uint8_t*& buffToReturn,uint32_t& buffSize)
{
  vector<uint8_t> buff;
  
  string::iterator it;
  
  for(it=this->from.begin();it!=this->from.end();++it){
    buff.push_back((uint8_t)*it);
    
  }
  
  buff.push_back(0);
  
   for(it=this->to.begin();it!=this->to.end();++it){
    buff.push_back((uint8_t)*it);
    
  }
  
  buff.push_back(0);
  
  uint8_t* timeptr=(uint8_t *)&this->timeStamp;
  
  for(int i=0;i<sizeof(this->timeStamp);i++){
  
    buff.push_back(timeptr[i]);
  }
  
  buff.push_back(tag);
  
  /*timeptr=&this->size;
   for(int i=0;i<sizeof(this->size);i++){
  
    buff.push_back(timeptr[i]);
  }*/
  
  
  for(int i=0;i<size;i++){
    buff.push_back(data[i]);
  }
  
  buffToReturn=new uint8_t[buff.size()];
  copy(buff.begin(),buff.end(),buffToReturn);
  buffSize=buff.size();
}


void Message::peek(uint8_t* peekBuffer, uint32_t size)
{
  uint32_t copySize= this->size < size? this->size : size;
  
  memcpy(peekBuffer,this->data,copySize);
}

uint8_t* Message::getData()
{
  uint8_t* toReturn=new uint8_t[this->size];
  memcpy(toReturn,this->data,this->size);
  
  return toReturn;
}

TIME Message::getTime()
{
  return this->timeStamp;
}

void Message::deserialize(uint8_t *buff,uint32_t buffSize)
{
  stringstream temp;
  
  uint32_t it=0;
  
  for(;it<buffSize;it++){
  
    if(buff[it]==0)
      break;
    temp << (char)buff[it];
  }
  ++it;
  this->from=temp.str();
  temp.str(string());
  
   for(;it<buffSize;it++){
  
    if(buff[it]==0)
      break;
    temp << (char)buff[it];
  }
  this->to=temp.str();
  temp.str(string());
  ++it;
  uint8_t* timeptr=(uint8_t*)&this->timeStamp;
  
  for(int i=0;i<sizeof(this->timeStamp);i++){
  
    timeptr[i]=buff[it++];
  }
  
  this->tag=buff[it++];
  
  vector<uint8_t> dataBuff;
  for(;it<buffSize;it++){
  
    dataBuff.push_back(buff[it]);
  }
  
  this->size=dataBuff.size();
  
  this->data=new uint8_t[this->size];
  copy(dataBuff.begin(),dataBuff.end(),this->data);
}

Message::~Message()
{
  delete[] this->data;
}


}
