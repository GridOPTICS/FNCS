#ifndef TIME_H
#define TIME_H

#include <cstdint>
#include <iostream>
#include <exception>

using namespace std;

namespace sim_comm{

  class TimeRegisterException : exception{
  virtual const char* what() const throw()
    {
      return "Unable to get current time from simulator!";
    }
    
    
  };
  
  class TimeException : exception{
    virtual const char* what() const throw()
    {
      return "Exception while converting time";
    }
    
  };
  
  enum time_metric{
    SECONDS =0,
    MILLISECONDS,
    NANOSECONDS,
    UNKNOWN
  };
  
  
  
  typedef uint64_t TIME;
  
  inline TIME convertToMyTime(time_metric myTimeStep,TIME frameworkTime);
  inline TIME convertToFrameworkTime(time_metric myTimeStep,TIME current_time);
}

#endif