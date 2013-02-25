/* autoconf header */
#include "config.h"

/* C headers */
#include <sys/unistd.h>

/* C++ STL */
#include <cassert>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>

/* C 3rd party headers */
#include <mpi.h>

/* our headers */
#include "abscommmanager.h"
#include "integrator.h"
#include "mpinetworkinterface.h"
#include "objectcomminterface.h"
#include "util/callback.h"
#include "util/simtime.h"

using namespace std;
using namespace sim_comm;


TIME getCurTime(){
    return 10;
}

static void network_simulator()
{
    TIME eventTime[]={102,203,800,1000,1010,3000,4500,7000,8010,9900,11000};
    TIME eventTimeGranted=eventTime[0];
    MpiNetworkInterface *comm = new MpiNetworkInterface(MPI_COMM_WORLD, true);
    CallBack<TIME,empty,empty,empty>* cb=CreateCallback(getCurTime);
    ofstream myFile("OtherSim.txt");
    int counter=0;

    Integrator::initIntegratorCommunicationSim(comm,MILLISECONDS,5,0);
    Integrator::setTimeCallBack(cb);
    Integrator::getCommInterface("simObject1");
    Integrator::getCommInterface("simObject2");
    Integrator::finalizeRegistrations();

    for(int i=0;i<10;i++){
        //execute calculations that will solve all our problems
        usleep(rand()%2000);
        //start the time sync
        cout << "OtherSim: My current time is " << eventTime[counter]
            << " next time I'll skip to " << eventTime[counter+1] << endl;
        myFile << "OtherSim: My current time is " << eventTime[i]
            << " next time I'll skip to " << eventTime[counter+1] << endl;
        eventTimeGranted=Integrator::getNextTime(
                eventTimeGranted,eventTime[counter+1]);
        if(eventTimeGranted==eventTime[counter+1]) {
            counter++;
        }
        cout << "OtherSim: I'm granted " << eventTimeGranted << endl;
        myFile << "OtherSim: I'm granted " << eventTimeGranted << endl;
        if(Integrator::isFinished()) {
            break;
        }
    }
    cout << "I'm done!" << endl;
    myFile << "I'm done!" << endl;

    Integrator::stopIntegrator();
}


static void generic_simulator()
{
    TIME eventTime;
    MpiNetworkInterface *comm = new MpiNetworkInterface(MPI_COMM_WORLD, false);
    CallBack<TIME,empty,empty,empty>* cb=CreateCallback(getCurTime);
    ofstream myFile("GenSim.txt");

    Integrator::initIntegratorGracePeriod(comm,SECONDS,5,0);
    Integrator::setTimeCallBack(cb);
    Integrator::getCommInterface("simObject1");
    Integrator::getCommInterface("simObject2");
    Integrator::finalizeRegistrations();

    eventTime=0;
    for(int i=0;i<10;i++){
        Message *message = nullptr;
        //execute calculations that will solve all our problems
        usleep(rand()%2000);
        //start the time sync
        cout << "GenSim: My current time is " << eventTime
            << " next time I'll skip to " << i+1 << endl;
        myFile << "GenSim: My current time is " << eventTime
            << " next time I'll skip to " << i+1 << endl;
        eventTime=Integrator::getNextTime(eventTime,(TIME)i+1);
        cout << "GenSim: I'm granted " << eventTime << endl;
        myFile << "GenSim: I'm granted " << eventTime << endl;
        message = new Message("simObject1", "simObject2", eventTime, NULL, 0, 0);
        Integrator::getCommInterface("simObject1")->send(message);
    }
    cout << "DONE!" << endl;
    myFile << "DONE!" << endl;

    Integrator::stopIntegrator();
}


int main(int argc, char **argv)
{
    int ierr = 0;
    int comm_rank = 0;
    int comm_size = 0;

    ierr = MPI_Init(&argc, &argv);
    assert(MPI_SUCCESS == ierr);

    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
    assert(MPI_SUCCESS == ierr);

    ierr = MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    assert(MPI_SUCCESS == ierr);

    assert(2 == comm_size);

    try {
        if (0 == comm_rank) {
            /* comm_rank 0 is the simulated network simulator */
            network_simulator();
        }
        else {
            /* all other ranks are some other simulator */
            generic_simulator();
        }
    } catch (const std::exception &e) {
        cout << e.what() << endl;
        MPI_Abort(MPI_COMM_WORLD, -1);
    }
    catch (const string &e) {
        cout << e << endl;
        MPI_Abort(MPI_COMM_WORLD, -1);
    }
    catch (...) {
        cout << "why won't it catch the right one??" << endl;
        MPI_Abort(MPI_COMM_WORLD, -1);
    }
    cout << "DONE!" << endl;
    ierr = MPI_Finalize();
    assert(MPI_SUCCESS == ierr);
    cout << "DONE!" << endl;
    return 0;
}
