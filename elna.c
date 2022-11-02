// Code by Robin DrZobin Larsson
// robin.larsson@protonmail.ch
// 2022-11-02

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

/*
Get the size of a file.
@return The filesize, or 0 if the file does not exist.
*/
size_t getFilesize(const char* filename) {
    struct stat st;

    if(stat(filename, &st) != 0) {
        return 0;
    }

    return st.st_size;   
}

/*
Run a command, kill the process if it hasent exit after sleep time.
@return Exit value of the command. 
*/
int runcmd(char *cmd,char *argv[], int runTime)
{
    int child_pid;
    int child_status;

    child_pid = fork();
    if(child_pid == 0) {
        // This is done by the child process.
        execvp(cmd,argv);
        // If execvp returns, it must have failed.
        exit(0);
    }
    else {
        // This is run by the parent.
        sleep(runTime);
        if(waitpid(-1,&child_status,WNOHANG) == child_pid){
            if(child_status != 0){
                return child_status;
            } 
            else {
                return 0;
            }
        } else {
            kill(child_pid,SIGKILL);
            return 0;
        }
    }
}

/*
Reade a file on disc and puts the content on heap.
@return Char pointer to content on heap.
*/
char *readFile(char *file,size_t size){
    // Ptr to file on heap
    char *ptr_file;

    // Ptr to file
    FILE *ptr_fp;

    // Allocate  memory on heap for file
    //ptr_file = (char *)malloc(size);
    ptr_file = (char *)malloc(size);


    // Open file for reading
    if((ptr_fp = fopen(file, "rb"))==NULL)
    {
        printf("Unable to open the file!\n");
	exit(1);
    }
    else 
    {
        printf("Opened file successfully for reading.\n");
    }

    // Reading file into memory
    if(fread(ptr_file, size, 1, ptr_fp) != 1)
    {
        printf( "Read error!\n" );
	exit( 1 );
    }
    else 
    {
        printf( "Read was successful.\n" );
    }
    fclose(ptr_fp);


    for(int i = 0;i < size; i++){
        printf("pos: %d hex: %hhx\n",i,ptr_file[i]);
    }

    return ptr_file;
}

/*
Create a file on disc.
@return void.
*/
void createSeedFile(char *originalFile,size_t size,int pos,char *value,char *newFile){
    // Ptr to file
    FILE *ptr_fp;

    // Filename of fuzz file
    char fileName[] = "/input";

    // Open file for writing
    ptr_fp = fopen(newFile,"a");

    //write to file
    //fwrite(originalFile,size,1,ptr_fp);    
    for(int i = 0;i < size; i++){
        if(i == pos){
            fwrite(value,1,1,ptr_fp); 
            continue;
        }
        fwrite(originalFile+i,1,1,ptr_fp); 
    }    

    //printf("hex: %X\n",originalFile[0]);
}

int main(){
    // Full path to command to fuzzed.
    //char cmd[] = "/usr/bin/vlc";
    char cmd[] = "/bin/ls";

    // Arguments to command to fuzz.
    char *argv[3];
    argv[0] = "ls";
    argv[1] = "/lhhl";
    argv[2] = NULL;

    // Full path to original seed file.
    char file[] = "/home/drz/Dropbox/projects/elna/not_kitty.tiff";

    // Full path to tmp fuzzy file.
    char tmpFile[] = "/home/drz/Dropbox/projects/elna/tmp/in.dat";

    // Full path where to save files that crash the program.
    char outDir[] = "/home/drz/Dropbox/projects/elna/out/";

    // Ptr to original file on heap.
    char *originalFileData;

    // Byte possision to fuzz.
    int pos = 2;

    // File size of original seed file.
    size_t fileSize = getFilesize(file);
    printf("File size is: %zu\n",fileSize);

    // Read original file data to heap.
    originalFileData = readFile(file,fileSize);

    // Start with this hex value.
    char value = 0x00;
    char *v_ptr = &value;

    // Start fuzzing.
    for(int i=0; i<255; i++){
        value++;
        printf("Value: %hhx\n",value);

        // Create file to use as input.
        createSeedFile(originalFileData,fileSize,pos,v_ptr,tmpFile);

        // Run executable with newly created input.
        int status = runcmd(cmd,argv,1);

        // Executable did not crash, remove input file.
        if(status == 0) {
            int remove_status = remove(tmpFile);
            if(remove_status != 0) {
                printf("Error removing tmp file\n");
            }
        }
        // Executable probably crashed, save input file. 
        else {
            time_t T= time(NULL);
            struct  tm tm = *localtime(&T);

            char year[4];
            sprintf(year, "%d", tm.tm_year+1900);

            char month[2];
            sprintf(month,"%d",tm.tm_mon+1);

            char day[2];
            sprintf(day,"%d", tm.tm_mday);

            char hour[2];
            sprintf(hour,"%d",tm.tm_hour);

            char min[2];
            sprintf(min,"%d",tm.tm_min);

            char sec[2];
            sprintf(sec,"%d",tm.tm_sec);

            char newFileName[sizeof(year)+sizeof(month)+sizeof(day)+sizeof(hour)+sizeof(min)+sizeof(sec)];
            strcpy(newFileName,year);
            strcat(newFileName,month);
            strcat(newFileName,day);
            strcat(newFileName,hour);
            strcat(newFileName,min);
            strcat(newFileName,sec);

            char outFile[sizeof(outDir) + sizeof(newFileName)];
            strcpy(outFile,outDir);
            strcat(outFile,newFileName);

            int rename_status = rename(tmpFile,outFile);
            printf("tmpFile: %s\n",tmpFile);
            printf("outFile: %s\n",outFile);
            if(rename_status != 0) {
                printf("Error moving tmp file, exiting\n");
                return 1;
            }
        }
      
    }

    return 0;
}
