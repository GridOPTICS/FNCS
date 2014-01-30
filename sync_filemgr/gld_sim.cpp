#include "filehandler.h"
#include "header.h"

int main()
{
    //open a fenix_file   
    FENIX_FILE logfile;

    char *filename = "original_file.txt";

    //FILE *f = fopen(filename, "w+");
    FILE *f =logfile.fenix_fopen(filename, "w+");
    if(f == NULL) {
	printf("ERror opening the file\n");
	return(1);
    }


    fprintf(f, "log: Before fork\n");
    fflush(f);
    
    pid_t PID = fork();

    if(PID == 0) {
	//I am the child
	int child = 0;
	logfile.start_fork(child);
	fprintf(f,"log: After fork: I am the child\n");
	logfile.end_fork(0);
    } else {
	// I am the parent
	int parent = 1;
	logfile.start_fork(parent);
	fprintf(f,"log: After fork : I am the parent\n");
    	logfile.end_fork(1);
    }
   
    fprintf(f, "log: writing after end_fork, pid =(%d)\n", PID);
    
    logfile.fenix_fclose(f);
    return 0;
}

