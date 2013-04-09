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

/* test headers */
#include "echo.h"

using namespace std;
using namespace sim_comm;


static TIME time_current=0;
vector<Message*> messages;


string getFilename(const char *prefix) {
    ostringstream os;
    os << prefix << "_" << getpid() << ".txt";
    return os.str();
}


TIME getCurTime() {
    return time_current;
}


static void receive_message1()
{
    cout << "netsim: simObject1 got message" << endl;
}


static void receive_message2()
{
    cout << "netsim: simObject2 got message" << endl;
}


static void network_simulator()
{
    TIME time_granted=0;
    MpiNetworkInterface *comm = nullptr;
    CallBack<TIME,empty,empty,empty>* time_cb = nullptr;
    CallBack<void,empty,empty,empty>* message_cb1 = nullptr;
    CallBack<void,empty,empty,empty>* message_cb2 = nullptr;
    Echo echo(getFilename("NetSim"));

    comm = new MpiNetworkInterface(MPI_COMM_WORLD, true);
    time_cb = CreateCallback(getCurTime);
    message_cb1 = CreateCallback(receive_message1);
    message_cb2 = CreateCallback(receive_message2);

    Integrator::initIntegratorCommunicationSim(comm,MILLISECONDS,5,0);
    Integrator::setTimeCallBack(time_cb);
    Integrator::getCommInterface("simObject1")->setMessageNotifier(message_cb1);
    Integrator::getCommInterface("simObject2")->setMessageNotifier(message_cb2);
    Integrator::finalizeRegistrations();

    while (!Integrator::isFinished()) {
        time_granted = Integrator::getNextTime(time_granted, time_current+100);
        echo << "NetSim: My current time is " << time_current
            << " next time I'll skip to " << time_granted << endl;
        while (time_current+50 <= time_granted) {
            time_current += 50;
            echo << "NetSim: Working. Time is " << time_current << endl;
            //execute calculations that will solve all our problems
            usleep(rand()%2000); /* work */
        }
    }
    echo << "NetSim done!" << endl;

    Integrator::stopIntegrator();
}


static void generic_simulator()
{
    TIME time_granted=0;
    MpiNetworkInterface *comm = new MpiNetworkInterface(MPI_COMM_WORLD, false);
    CallBack<TIME,empty,empty,empty>* cb=CreateCallback(getCurTime);
    Echo echo(getFilename("GenSim"));

    Integrator::initIntegratorGracePeriod(comm,SECONDS,5,0);
    Integrator::setTimeCallBack(cb);
    Integrator::getCommInterface("simObject1");
    Integrator::getCommInterface("simObject2");
    Integrator::finalizeRegistrations();

    while (!Integrator::isFinished() && time_current < 10) {
        time_granted=Integrator::getNextTime(time_current,time_current+1);
        //start the time sync
        echo << "GenSim: My current time is " << time_current
            << " next time I'll skip to " << time_granted << endl;
        while (time_current+1 <= time_granted) {
            Message *message = nullptr;
            //execute calculations that will solve all our problems
            usleep(rand()%2000);
            message = new Message("simObject1", "simObject2",
                    time_current, NULL, 0, 0);
            Integrator::getCommInterface("simObject1")->send(message);
            echo << "GenSim: sent message" << endl;
            time_current += 1;
        }
    }
    echo << "DONE!" << endl;

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
    cout << "finished simulator function" << endl;
    ierr = MPI_Finalize();
    assert(MPI_SUCCESS == ierr);
    cout << "finished MPI_Finalize!" << endl;
    return 0;
}
