#ifndef SIMTIME_H
#define SIMTIME_H

#include <iostream>
#include <exception>

#include "csimtime.h"

using namespace std;

namespace sim_comm {

class TimeRegisterException : exception {
    virtual const char* what() const throw() {
        return "Unable to get current time from simulator!";
    }


};

class TimeException : exception {
    virtual const char* what() const throw() {
        return "Exception while converting time";
    }

};


TIME convertToMyTime(time_metric myTimeStep,TIME frameworkTime);
TIME convertToFrameworkTime(time_metric myTimeStep,TIME current_time);
}

#endif
