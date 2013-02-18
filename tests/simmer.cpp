#include "config.h"

#include <mpi.h>

#include <cassert>

#include <sys/unistd.h>
#include <cstdlib>
#include "integrator.h"
#include "mpicomminterface.h"
#include "util/time.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace sim_comm;


static void network_simulator()
{
	TIME eventTime[]={102,203,800,1000,1010,3000,4500,7000,8010,9900,11000};
    MpiCommInterface *comm = new MpiCommInterface(MPI_COMM_WORLD, true);
    Integrator::initIntegratorGracePeriod(comm,MILLISECONDS,5);

	ofstream myFile;

    comm->finalizeRegistrations();

	myFile.open("OtherSim.txt");
    TIME eventTimeGranted=eventTime[0];
    int counter=0;
    for(int i=0;i<10;i++){
    	//execute calculations that will solve all our problems
    	usleep(rand()%2000);
    	//start the time sync
    	cout << "OtherSim: My current time is " << eventTime[counter] << " next time I'll skip to " << eventTime[counter+1] << endl;
    	myFile << "OtherSim: My current time is " << eventTime[i] << " next time I'll skip to " << eventTime[counter+1] << "\n";
    	eventTimeGranted=Integrator::getNextTime(eventTimeGranted,eventTime[counter+1]);
    	if(eventTimeGranted==eventTime[counter+1])
    		counter++;
    	cout << "OtherSim: I'm granted " << eventTimeGranted << endl;
    	myFile << "OtherSim: I'm granted " << eventTimeGranted << "\n";

    }
}


static void generic_simulator()
{
	TIME eventTime;
	MpiCommInterface *comm = new MpiCommInterface(MPI_COMM_WORLD, false);
	Integrator::initIntegratorGracePeriod(comm,SECONDS,5);

	ofstream myFile;

    comm->finalizeRegistrations();


	myFile.open("GenSim.txt");
	eventTime=0;
	for(int i=0;i<10;i++){
		//execute calculations that will solve all our problems
		usleep(rand()%2000);
		//start the time sync
		cout << "GenSim: My current time is " << eventTime << " next time I'll skip to " << i+1 << endl;

		myFile << "GenSim: My current time is " << eventTime << " next time I'll skip to " << i+1 << "\n";
		eventTime=Integrator::getNextTime(eventTime,(TIME)i+1);
		cout << "GenSim: I'm granted " << eventTime << endl;
		myFile << "GenSim: I'm granted " << eventTime << "\n";

	}

	cout << "DONE!" << endl;
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

    if (0 == comm_rank) {
        /* comm_rank 0 is the simulated network simulator */
        network_simulator();
    }
    else {
        /* all other ranks are some other simulator */
        generic_simulator();
    }
    cout << "DONE!" << endl;
    ierr = MPI_Finalize();
    assert(MPI_SUCCESS == ierr);
    cout << "DONE!" << endl;
    return 0;
}
