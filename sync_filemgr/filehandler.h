#include "header.h"
#include <unordered_map>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
typedef FILE* FILEPTR;
#define FILE_ID_LEN 20
#define NAME_LENGTH 7+FILE_ID_LEN

long get_file_size(FILE *f){
    long bufsize = 0;
    
    if(f != NULL){
	fseek(f, 0, SEEK_END);
	bufsize = ftell(f);
	if(bufsize == -1){ printf("Error getting fizesize\n");}
    }
    return bufsize;
}

void read_file_to_buffer(FILE * f, char* buffer, size_t bufsize) {
    if(f != NULL){

	fseek(f, 0, SEEK_SET);

	size_t newLen = fread(buffer, sizeof(char), bufsize, f);
	if(newLen == 0) {
	    printf("Error reading original file\n");
	}else{
	    buffer[++newLen] = '\0';
	}
    }else{
	
	buffer[0] = '\0';
    }

    return;
}

void append_buffer(FILE* f, char* buffer) {
}
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

	void start_fork(int id);

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

    num_files++;
    return f;
    
}

void FENIX_FILE::start_fork(int process_id) {

    int i = 0;
    char c[FILE_ID_LEN];
    char pid[2];
    
    sprintf(pid, "%d", process_id);
    for(filemap::iterator it = fmap.begin(); it != fmap.end(); it++){
    	// For all files open in Fenix, create a temporary file 

	sprintf(c, "%d", i);
	char filename[NAME_LENGTH];
	strcpy(filename, "tmpfile_");
	strcat(filename, pid);
	strcat(filename, "_");
	strcat(filename, c);

	//printf(" Process %d, Opening temporary file: %s\n", process_id, filename);


	FILE* tmp = fopen(filename, "w+");
	
	FILE* forig = it->first;
/*	long fsize = get_file_size(forig);
	printf("#characters in original log file  =%ld\n", fsize);
	char* buffer2 = (char *)malloc(sizeof(char) * (fsize+1));
	read_file_to_buffer(forig, buffer2, fsize);
	printf("%s", buffer2);
*/
	FILE* fsave = (FILE *)malloc(sizeof(FILE));
	// exchange file pointer data using a temp buffer
	//
	memcpy(fsave, forig, sizeof(FILE));
	memcpy(forig, tmp, sizeof(FILE));
	memcpy(tmp, fsave, sizeof(FILE));

	// now the key in the map, points to new file 
	// The vaue stored in the map, points to the original file
	// data
	it->second = tmp;
	delete(fsave);
	i++;
    }

}

// takes current file pointers  of child and parent and merges with
// the saved one  
void FENIX_FILE::end_fork(int select) {
    if(select == 1){
	//I am selected
	/* get my current file contents and append it to original
	 * file*/
	
	for(filemap::iterator it = fmap.begin(); it != fmap.end(); it++){
	
	    FILE* curr = it->first;
	    long fsize1 = get_file_size(curr);

	    printf("#characters in temp log file  =%ld\n", fsize1);

	    char* buffer1 = (char *)malloc(sizeof(char) * (fsize1+1));
	    read_file_to_buffer(curr, buffer1, fsize1);
	    printf("%s", buffer1);
	    //fclose(curr);
	    
	    FILE* orig = it->second; 
	    /*long fsize2 = get_file_size(orig);
	    printf("#characters in original log file  =%ld\n", fsize2);
	    char* buffer2 = (char *)malloc(sizeof(char) * (fsize2+1));
	    read_file_to_buffer(orig, buffer2, fsize2);
	    printf("%s", buffer2);
*/
	    size_t data_size = fwrite(buffer1, sizeof(char), fsize1,  orig);

	    FILE* fsave = (FILE *)malloc(sizeof(FILE));
	    // exchange file pointer data using a temp buffer
	    memcpy(fsave, orig, sizeof(FILE));
	    memcpy(orig, curr, sizeof(FILE));
	    memcpy(curr, fsave, sizeof(FILE));

	    fclose(orig);

	    free(buffer1);

	}
    
    }
    else{
	// I am not selecetd, do nothing
	// NOte : File close happends during fenix_fclose for the temp
	// file of the not choosen process 
	
	for(filemap::iterator it = fmap.begin(); it != fmap.end(); it++){
		FILE* curr = it-> first;
		fclose(curr);

	}
	
    }

    //delete(fmap);
    return;
}

void FENIX_FILE::fenix_fclose(FILEPTR f)
{
    fclose(f);
    fmap.erase(f);
}
