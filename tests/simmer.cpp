#include "config.h"

#include <mpi.h>

#include <cassert>

#include "integrator.h"
#include "mpicomminterface.h"
#include "util/time.h"

using namespace std;
using namespace sim_comm;


static void network_simulator()
{
    MpiCommInterface *comm = new MpiCommInterface(MPI_COMM_WORLD);
    Integrator::initIntegrator(comm,MILLISECONDS,5);
}


static void generic_simulator()
{
    MpiCommInterface *comm = new MpiCommInterface(MPI_COMM_WORLD);
    Integrator::initIntegrator(comm,SECONDS,5);
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

    ierr = MPI_Finalize();
    assert(MPI_SUCCESS == ierr);

    return 0;
}
