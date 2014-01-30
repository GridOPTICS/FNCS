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
	//logfile.start_fork()
	    fprintf(f,"log: After fork: I am the child\n");
	//logfile.end_fork("CHILD_FAILURE");
    } else {
	// I am the parent
	//logfile.start_fork();
	fprintf(f,"log: After fork : I am the parent\n");
    	//ogfile.end_fork();
    }
   
    fprintf(f, "log: writing after end_fork()\n");
    
    //logfile.fclose();
    logfile.fenix_fclose(f);
    return 0;
}

