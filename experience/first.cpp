#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include "selfmap.hh"

using namespace std;

int main (int argc, char *argv[]) { 
    struct stat buf;
    int fd, length, status, i, j, k;
    void *mm_file;
    char *string = "this is a lowercase-sentence.";
    length = strlen(string);

    fd = open(argv[1], O_CREAT | O_RDWR, 0666); //Creates file with name given at command line
    write(fd, string, strlen(string)); //Writes the string to be modified to the file
    fstat(fd, &buf); //used to determine the size of the file

    //Establishes the mapping
    mm_file = mmap(NULL, (size_t) buf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    //Initializes child processes
    pid_t MC0;
    pid_t MC1;

	//selfmap::getInstance();
    //Creates series of child processes which share the same parent ID
    if((MC0 = fork()) == 0) {
        printf("Child 1 %d reads: \n %s\n", getpid(), mm_file);
        //{convert file content to uppercase string};
        for (i = 0; i < length; i++) {
            string[i] = toupper(string[i]);
        }
        //sync the new contents to the file
        msync(0, (size_t) buf.st_size, MS_SYNC);
        printf("Child 1 %d reads again: \n %s\n", getpid(), mm_file);
        exit(EXIT_SUCCESS); //Exits process
    } else if ((MC1 = fork()) == 0) {
        sleep(1); //so that child 2 will perform its task after child 1 finishes
        ("Child 2 %d reads: \n %s\n", getpid(), mm_file);
        //{remove hyphens}
        for (j = 0; j < length; i++) {
            if (string[i] == '-') {
                string[i] = ' ';
            }
        }
        //sync the new contents to the file
        msync(0, (size_t) buf.st_size, MS_SYNC);
        printf("Child 2 %d reads again: \n %s\n", getpid(), mm_file);
        exit(EXIT_SUCCESS); //Exits process
   } 

   // Waits for all child processes to finish before continuing.
   waitpid(MC0, &status, 0);
   waitpid(MC1, &status, 0);

   return 0;
}
