// Code by Robin DrZobin Larsson
// robin.larsson@protonmail.ch
// 2022-11-02

#include "shared_funcs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

/*
Run a command, kill the process if it hasent exit after sleep time.
@return Exit value of the command. 
*/
int run_cmd(char *cmd,char *argv[], int run_time)
{
    int child_pid;
    int child_status;

    child_pid = fork();
    if(child_pid == 0) {
        // This is run by the child process.
        int exit_code;
        
        //printf("cmd: %s\n",cmd);
        //printf("argv[0]: %s\n",argv[0]);
        //printf("argv[1]: %s\n",argv[1]);
        exit_code = execvp(cmd,argv);
        // If execvp returns, it must have failed.
        //exit(0);
        printf("execvp exit_code:%d\n",exit_code);
        // This is done by the child process.
        exit(exit_code);
    }
    else {
        // This is run by the parent.
        if(run_time == -1){
            waitpid(-1, &child_status, 0);
            return child_status;
        } else {
            sleep(run_time);
            if(waitpid(-1,&child_status,WNOHANG) == child_pid){
                return child_status;
            } else {
                kill(child_pid,SIGKILL);
                return 0;
            }
        }
    }
}

/*
Create a file on disc.
@return void.
*/
void create_seedfile(char *originalFile,size_t size,int pos,char *value,char *newFile){
    // Ptr to file
    FILE *ptr_fp;

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
    fclose(ptr_fp);    

    //printf("hex: %X\n",originalFile[0]);
}

int main(int argc, char *argv[]){
    // Full path to command to fuzzed.
    //char cmd[] = "/usr/bin/vlc";
    char cmd[] = "/home/drz/github/elna/test_binary";

    // Arguments to command to fuzz.
    char *cmd_argv[3];
    cmd_argv[0] = "test_binary";
    cmd_argv[1] = "/home/drz/github/elna/working_dir/test.txt";
    cmd_argv[2] = NULL;

    // Full path to original seed file.
    char file[] = "/home/drz/github/elna/test.txt";

    // Full path to tmp fuzzy file.
    char tmp_file[] = "/home/drz/github/elna/working_dir/test.txt";

    // Full path where to save files that crash the program.
    char out_dir[] = "/home/drz/github/elna/results/";

    // Ptr to original file on heap.
    char *original_filedata;

    // Byte possision to fuzz.
    int pos = 1;
    
    // Number of seconds to wait for fuzzed program before killing it. If set to -1 we will wait until it has exit.
    int wait_pid = -1;
    
    // File size of original seed file.
    size_t file_size = get_filesize(file);
    printf("File size is: %zu\n",file_size);

    // Read original file data to heap.
    original_filedata = read_file(file,file_size);

    // Start with this hex value.
    char value = 0x00;
    char *v_ptr = &value;

    // Start fuzzing.
    for(int i=0; i<255; i++){
        printf("\n\n");
        value++;
        printf("Value: %hhx\n",value);

        // Create file to use as input.
        create_seedfile(original_filedata,file_size,pos,v_ptr,tmp_file);

        // Run executable with newly created input.
        int status = run_cmd(cmd,cmd_argv,wait_pid);

        // Executable did not crash, remove input file.
        if(status == 0) {
            int remove_status = remove(tmp_file);
            if(remove_status != 0) {
                printf("Error removing tmp file\n");
            }
            printf("Exit code 0\n");
        }
        // Executable probably crashed, save input file. 
        else {
            printf("Exit code:%d\n",status);
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

            char new_filename[sizeof(year)+sizeof(month)+sizeof(day)+sizeof(hour)+sizeof(min)+sizeof(sec)];
            strcpy(new_filename,year);
            strcat(new_filename,month);
            strcat(new_filename,day);
            strcat(new_filename,hour);
            strcat(new_filename,min);
            strcat(new_filename,sec);

            char out_file[sizeof(out_dir) + sizeof(new_filename)];
            strcpy(out_file,out_dir);
            strcat(out_file,new_filename);

            int rename_status = rename(tmp_file,out_file);
            printf("tmpFile: %s\n",tmp_file);
            printf("outFile: %s\n",out_file);
            if(rename_status != 0) {
                printf("Error moving tmp file, exiting\n");
                return 1;
            }
        }
      
    }

    return 0;
}
