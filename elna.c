// Code by Robin Larsson aka drzobin
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
#include <dirent.h>

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
        
        exit_code = execvp(cmd,argv);
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
    for(int i = 0;i < size; i++){
        if(i == pos){
            fwrite(value,1,1,ptr_fp); 
            continue;
        }
        fwrite(originalFile+i,1,1,ptr_fp); 
    }
    fclose(ptr_fp);    
}

int main(int argc, char *argv[]){
    // Full path to command to fuzzed.
    char cmd[] = "/home/drz/github/elna/crash_me";

    // Arguments to command to fuzz.
    char *cmd_argv[3];
    cmd_argv[0] = "crash_me";
    cmd_argv[1] = "/home/drz/github/elna/working_dir/test.txt";
    cmd_argv[2] = NULL;

    // Full path to folder that contains seedfiles.
    char seedfile_folder[] = "/home/drz/github/elna/seedfiles/";

    // Full path to tmp fuzzy file.
    char tmp_file[] = "/home/drz/github/elna/working_dir/test.txt";

    // Full path where to save files that crash the program.
    char out_dir[] = "/home/drz/github/elna/results/";

    // Ptr to original file on heap.
    char *original_filedata;

    // Byte offset/possition in input file to fuzz, if set to -1 then flipp all bits in input file.
    int pos = -1;
    
    // Number of seconds to wait for fuzzed program before killing it. If set to -1 we will wait until it has exit.
    int wait_pid = -1;
    
    // If this is set to 1, all bits in input file will be flipped.
    int flipp_all_bits = 0;

    // Start fuzzing.

    // Open seedfile folder.
    struct dirent *files;
    DIR *dir = opendir(seedfile_folder);
    if (dir == NULL){
        printf("Directory cannot be opened!" );
        return 0;
    }

    // This loop is run on all the seedfiles, one iteration per file.
    while ((files = readdir(dir)) != NULL){
        
        // Name of seedfile.
        char *seedfile_name = files->d_name;
        
        // The . and .. is not a files.
        if ((strcmp(seedfile_name,".") == 0) || (strcmp(seedfile_name,"..") == 0)){
            continue;
        }
        
        // Filename with full path of seedfile.
        int file_path_length  = strlen(seedfile_folder) + strlen(seedfile_name) + 1;
        char file[file_path_length];
        snprintf(file,file_path_length,"%s%s",seedfile_folder,seedfile_name);

        // Seedfile size.
        size_t file_size = get_filesize(file);

        printf("Working with seedfile: %s\n", files->d_name);
        printf("File size is: %zu\n",file_size);

        // Read original seedfile data to heap.
        original_filedata = read_file(file,file_size);
        
         // This loop is run on all offsets in seedfile, one iteration per offset in seedfile.
        int run_loop = 1;
        while(run_loop == 1){
            // If pos is -1 then we should flipp all bits in input file.
            if(pos == -1){
                flipp_all_bits = 1;
                pos = 0;
            }

            // Start with this hex value.
            char value = 0x00;
            char *v_ptr = &value;
            
            // This loop is run on all the hex values, every iteration is one hex value.
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
                    char new_filename[128];
                    //sprintf(new_filename,"pos:%d_value:%hhx",pos,value);
                    snprintf(new_filename,127,"filename:%s_offset:%d_value:0x%hhx",seedfile_name,pos,value);
                
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
            //If we should only bit flipp a specific offset then only run loop one time.
            if(flipp_all_bits == 0){
                run_loop = 0;
            // If we should bit flipp all bits in input file.
            } else {
                pos++;
                if(pos > file_size){
                    run_loop = 0;
                }
            }
        }
    }
    closedir(dir);

    return 0;
}
