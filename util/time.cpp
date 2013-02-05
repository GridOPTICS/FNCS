#include "time.h"

namespace sim_comm{

   inline TIME convertToMyTime(time_metric myTimeStep,TIME frameworkTime){
    
    switch (myTimeStep){
      case SECONDS:
	return frameworkTime/1000000000;
      case MILLISECONDS:
	return frameworkTime/1000000;
      case NANOSECONDS:
	return frameworkTime;
      default: 
	throw TimeException();
    }
    
  }
  
  inline TIME convertToFrameworkTime(time_metric myTimeStep,TIME current_time){
  
    switch (myTimeStep){
      case SECONDS:
	return current_time*1000000000;
      case MILLISECONDS:
	return current_time*1000000;
      case NANOSECONDS:
	return current_time;
      default:
	throw TimeException();
    }
  }
}