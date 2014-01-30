#include "header.h"
#include <unordered_map>
#include <string.h>
#include <stdio.h>

using namespace std;
typedef FILE* FILEPTR;


struct eqKey {

    bool operator()(const FILEPTR &f1, const FILEPTR &f2) {
    
	if(f1 == f2)
	    return true;
	else
	    return false;
    }
};

//typedef std::unordered_map<FILEPTR, FILEPTR, eqKey> filemap;
typedef std::unordered_map<FILEPTR, FILEPTR> filemap;
//define an eqaul to fuction for file pointers

class FENIX_FILE {

    private:

	// keep map of all log files that have been opened using
	// fenix_fopen

	filemap fmap;
	int num_files;

    public:
	FENIX_FILE(){ num_files = 0;}

	FILEPTR fenix_fopen(char* filename, char* mode);
	void fenix_fclose(FILEPTR f);

	void start_fork();

	void end_fork(int select);

};

// open a file and keep track of file pointer
FILEPTR FENIX_FILE::fenix_fopen(char* filename, char* mode)
{
    // open the file using C coomand
    FILEPTR f = fopen(filename, mode);

    FILEPTR tmp = NULL;
    //add it to your list of opened files
    fmap[f] = tmp;
    return f;
    //fmap[0] = 0;
    //return 0;
}

void FENIX_FILE::start_fork() {

    int i = 0;
    for(filemap::iterator it = fmap.begin(); it != fmap.end(); it++){
    	// For all files open in Fenix, create a temporary file 
	printf("%d\n",i);
	
	//std::string filename;
	
	char* filename = "temp";
	FILE* tmp = fopen(filename, "w+");
	
	FILE* forig = it->first;

	FILE* fsave = (FILE *)malloc(sizeof(FILE));
	// copy original filepointer to fsave
	memcpy(fsave, forig, sizeof(FILE));
	memcpy(forig, tmp, sizeof(FILE));

	delete(tmp);
	i++;
    }

}

// takes current file pointers  of child and parent and merges with
// the saved one  
void FENIX_FILE::end_fork(int select) {
    if(select == 0){
	//delete evrything
    }
    else{
	// do stuff
    }
    return;
}

void FENIX_FILE::fenix_fclose(FILEPTR f)
{
    fclose(f);
    fmap.erase(f);
}
