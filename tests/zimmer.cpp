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

/* our headers */
#include "abscommmanager.h"
#include "integrator.h"
#include "zmqnetworkinterface.h"
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
    TIME time_desired=0;
    ZmqNetworkInterface *comm = nullptr;
    CallBack<TIME,empty,empty,empty,empty>* time_cb = nullptr;
    CallBack<void,empty,empty,empty,empty>* message_cb1 = nullptr;
    CallBack<void,empty,empty,empty,empty>* message_cb2 = nullptr;
    Echo echo(getFilename("NetSim"));

    comm = new ZmqNetworkInterface(true);
    time_cb = CreateCallback(getCurTime);
    message_cb1 = CreateCallback(receive_message1);
    message_cb2 = CreateCallback(receive_message2);

    Integrator::initIntegratorCommunicationSim(comm,MILLISECONDS,5,0);
    Integrator::setTimeCallBack(time_cb);
    Integrator::getCommInterface("simObject1")->setMessageNotifier(message_cb1);
    Integrator::getCommInterface("simObject2")->setMessageNotifier(message_cb2);
    Integrator::getCommInterface("simObject11")->setMessageNotifier(message_cb1);
    Integrator::getCommInterface("simObject22")->setMessageNotifier(message_cb2);
    Integrator::finalizeRegistrations();

    while (!Integrator::isFinished()) {
        Integrator::timeStepStart(time_current);

        //execute calculations that will solve all our problems
        echo << "NetSim: Working. Time is " << time_current << endl;
        usleep(rand()%20000); /* work */
        time_desired = time_current + 100;

        time_granted = Integrator::getNextTime(time_current, time_desired);
        echo << "NetSim:"
            << " time_current=" << time_current
            << " time_desired=" << time_desired
            << " time_granted=" << time_granted << endl;

        time_current = time_granted;
    }
    echo << "NetSim done!" << endl;

    Integrator::stopIntegrator();
}


static void generic_simulator()
{
    TIME time_granted=0;
    TIME time_desired=0;
    ZmqNetworkInterface *comm = new ZmqNetworkInterface(false);
    CallBack<TIME,empty,empty,empty,empty>* cb=CreateCallback(getCurTime);
    Echo echo(getFilename("GenSim"));

    //Integrator::initIntegratorGracePeriod(comm,SECONDS,5,0);
    IncreasingSpeculationTimeStrategy *st=new IncreasingSpeculationTimeStrategy(MILLISECONDS,60000);
    Integrator::initIntegratorOptimistic(comm,MILLISECONDS,2300000000,time_current,60000,st);
    Integrator::setTimeCallBack(cb);
    Integrator::getCommInterface("simObject1");
    Integrator::getCommInterface("simObject2");
    Integrator::finalizeRegistrations();

    while (!Integrator::isFinished() && time_current < 10) {
        Integrator::timeStepStart(time_current);

        /* work */
        {
            echo << "GenSim: Working. Time is " << time_current << endl;
            Message *message = nullptr;
            message = new Message("simObject1", "simObject2",
                    time_current, NULL, 0, 0);
            Integrator::getCommInterface("simObject1")->send(message);
            echo << "GenSim: sent message" << endl;
            time_desired = time_current + 1;
        }
        time_granted = Integrator::getNextTime(time_current, time_desired);
        echo << "GenSim:"
            << " time_current=" << time_current
            << " time_desired=" << time_desired
            << " time_granted=" << time_granted << endl;

        time_current = time_granted;
    }
    echo << "DONE!" << endl;

    Integrator::stopIntegrator();
}


static void generic_simulator2()
{
    TIME time_granted=0;
    TIME time_desired=0;
    ZmqNetworkInterface *comm = new ZmqNetworkInterface(false);
    CallBack<TIME,empty,empty,empty,empty>* cb=CreateCallback(getCurTime);
    Echo echo(getFilename("GenSim2"));

    //Integrator::initIntegratorGracePeriod(comm,SECONDS,5,0);
    IncreasingSpeculationTimeStrategy *st=new IncreasingSpeculationTimeStrategy(MILLISECONDS,60000);
    Integrator::initIntegratorOptimistic(comm,MILLISECONDS,2300000000,time_current,60000,st);
    Integrator::setTimeCallBack(cb);
    Integrator::getCommInterface("simObject11");
    Integrator::getCommInterface("simObject22");
    Integrator::finalizeRegistrations();

    while (!Integrator::isFinished() && time_current < 20) {
        Integrator::timeStepStart(time_current);

        /* work */
        {
            echo << "GenSim2: Working. Time is " << time_current << endl;
            Message *message = nullptr;
            //execute calculations that will solve all our problems
            usleep(rand()%2000);
            message = new Message("simObject11", "simObject22",
                    time_current, NULL, 0, 0);
            Integrator::getCommInterface("simObject11")->send(message);
            echo << "GenSim2: sent message" << endl;
            time_desired = time_current + 5;
        }

        time_granted = Integrator::getNextTime(time_current, time_desired);
        echo << "GenSim2:"
            << " time_current=" << time_current
            << " time_desired=" << time_desired
            << " time_granted=" << time_granted << endl;

        time_current = time_granted;
    }
    echo << "DONE!" << endl;

    Integrator::stopIntegrator();
}


int main(int argc, char **argv)
{
    int ierr = 0;
    int comm_rank = 0;
    int comm_size = 0;

    if (argc <= 1) {
        cerr << "missing rank argument" << endl;
        return -1;
    }
    else if (argc >= 3) {
        cerr << "too many agruments" << endl;
        return -1;
    }
    else {
        comm_rank = atoi(argv[1]);
    }

    if (comm_rank < 0 || comm_rank > 2) {
        cerr << "invalid comm rank (" << comm_rank << ")" << endl;
        return -1;
    }

    try {
        if (0 == comm_rank) {
            /* comm_rank 0 is the simulated network simulator */
            cout << "starting network sim" << endl;
            network_simulator();
        }
        else if (1 == comm_rank) {
            cout << "starting gen sim" << endl;
            generic_simulator();
        }
        else if (2 == comm_rank) {
            cout << "starting gen sim 2" << endl;
            generic_simulator2();
        }
        else {
            /* all other ranks are some other simulator */
            cout << "starting extra gen sim" << endl;
            generic_simulator();
        }
    } catch (const std::exception &e) {
        cout << e.what() << endl;
    }
    catch (const string &e) {
        cout << e << endl;
    }
    catch (...) {
        cout << "why won't it catch the right one??" << endl;
    }
    cout << "finished simulator function" << endl;
    return 0;
}
