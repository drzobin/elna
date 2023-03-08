// Code by Robin Larsson aka drzobin
// robin.larsson@protonmail.ch
// me@drz.se
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
#include <signal.h>

/*
Run a command, kill the process if it hasent exit after sleep time.
@return Signal that killed the process. 
*/
int run_cmd(char *cmd,char *argv[], int run_time)
{
    int child_pid;
    int child_status;

    child_pid = fork();
    if(child_pid == 0) {
        // This is run by the child process.
        execvp(cmd,argv);

        return 0;
        //int exit_code;
        
        //exit_code = execvp(cmd,argv);
        // printf("execvp exit_code:%d\n",exit_code);
        // This is done by the child process.
        //exit(exit_code);
        //if(exit_code == -1){
        //    return 1;
        //}else{
        //    return 0;
        //}
    }
    else {
        // This is run by the parent.
        if(run_time == -1){
            waitpid(-1, &child_status, 0);

            // Return 1 if pid was killed by signal.
            if(WIFSIGNALED(child_status) == 1){
                // Return signal number.
                return WTERMSIG(child_status);
            } else {
                return 0;
            }    
        } else {
            sleep(run_time);
            if(waitpid(-1,&child_status,WNOHANG) == child_pid){
                // Return 1 if pid was killed by signal.
                if(WIFSIGNALED(child_status) == 1){
                    printf("Signal:%i\n",WTERMSIG(child_status));
                
                    // Return signal number.
                    return WTERMSIG(child_status);
                } else {
                    return 0;
                }    
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

int main(int argc, char **argv){
    // Full path to tmp working file. This is the bitflipped seedfile that will be written to an removed many times.
    char *tmp_file;
    
    // Full path to folder that contains seedfiles.
    char *seedfile_folder;
    
    // Full path where to save files that crash the program.
    char *out_dir;
    
    // Byte offset/possition in input file to fuzz, if set to -1 then flipp all bits in input file.
    int pos = -1;
    
    // Number of seconds to wait for fuzzed program before killing it. If set to -1 we will wait until it has exit.
    int wait_pid = -1;
    
    int i;
    while (1) {
        char c;

        c = getopt (argc, argv, "s:o:w:p:t:");
        if (c == -1) {
            /* We have finished processing all the arguments. */
            break;
        }
        switch (c) {
        // Set seedfile folder, this is the folder that contins the seedfiles.
        case 's':
            printf ("Setting seedfiles folder to: %s\n", optarg);
            seedfile_folder = &optarg[0];
            break;
        // Set output dir, this is where the crashes/results will be saved.
        case 'o':
            printf ("Setting out/results dir to: %s\n", optarg);
            out_dir = &optarg[0];
            break;
        // Set working file, this is the bitflipped seedfile that will be written and removes many times.
        case 'w':
            printf ("Setting working file to: %s\n", optarg);
            tmp_file = &optarg[0];
            break;
        // Set offset/possition in seedfile that should be flipped, if -1 all offsets/possistions will be flipped.
        case 'p':
            printf ("Setting offset to be fuzzed to: %s\n", optarg);
            sscanf(optarg,"%d",&pos);
            break;
        // Set time before killing fuzzed program, if -1 we will wait until fuzzed program exit.
        case 't':
            printf ("Setting time to: %s\n", optarg);
            sscanf(optarg,"%d",&wait_pid);
            break;
        case '?':
        default:
            printf ("Usage: %s -s [seedfile folder] -o [output folder] -w [working file]\n\n",argv[0]);
            printf ("Arguments: \n");
            printf ("-s Full path to folder with seedfiles\n");
            printf ("-o Full path to folder where output/crashes/results/stats should be saved\n");
            printf("-w Full path to file where mutated seedfile should be saved\n");
            printf("-p Offset/possistion in seedfile that should be flipped, if -1 all offsets/possistions will be flipped, this is default\n");
            printf("-t Time to wait before killing fuzzed program, if -1 we will wait until fuzzedprogram exit itself, this is default\n");
        }
    }

    // Number of argument left to parse + 1 for NULL.
    int arg_left = (argc - optind) + 1;
    char *cmd_argv[arg_left];

    // This is the argument that is left, lets parse them to get the program to be fuzzed and its args.
    int count = 0;
    for(; optind < argc; optind++){     
        if(strcmp(argv[optind],"@@") == 0){
            cmd_argv[count] = tmp_file;
        } else {
            cmd_argv[count] = argv[optind];
        }
        count = count + 1;
    }
    cmd_argv[arg_left] = NULL;

    // Now set the values of "argc" and "argv" to the values after the options have been processed, above.
    argc -= optind;
    argv += optind;

    // If this is set to 1, all bits in input file will be flipped.
    int flipp_all_bits = 0;

    // File to write status to.
    int status_file_len = strlen(out_dir) + strlen("/status.log") + 1;
    char status_file[status_file_len];
    snprintf(status_file, status_file_len, "%s%s", out_dir,"/status.log");
     

    // Start fuzzing.

    // Open seedfile folder.
    struct dirent *files;
    DIR *dir = opendir(seedfile_folder);
    if (dir == NULL){
        printf("Error: Directory seedfile folder that was set to %s can not be open. \n",seedfile_folder);
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

        printf("Working with seedfile: %s\n", file);

        // Ptr to original file on heap.
        char *original_filedata;

        // Read original seedfile data to heap.
        original_filedata = read_file(file,file_size);

        if(flipp_all_bits == 1){
            pos = 0;
        }

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
                // Write status to cmd and status_file when value is 0xEE.
                if(value == '\xee'){
                    printf("Target:%s Seedfile:%s Offset:%i Value:0xEE\n",cmd_argv[0],seedfile_name,pos);
                    FILE *status_file_ptr = fopen(status_file, "a");
                    fprintf(status_file_ptr,"Target:%s Seedfile:%s Offset:%i Value:0xEE\n",cmd_argv[0],seedfile_name,pos);
                    fclose(status_file_ptr);
                }

                // Create file to use as input.
                create_seedfile(original_filedata,file_size,pos,v_ptr,tmp_file);

                // Run executable with newly created input.
                int status = run_cmd(cmd_argv[0],cmd_argv,wait_pid);

                // Executable was killed by signal, save input file.
                if(status != 0){
                    printf("Killed with signal:%d\n",status);
                    char new_filename[128];
                    snprintf(new_filename,127,"filename:%s_offset:%d_value:0x%hhx",seedfile_name,pos,value);
                
                    char out_file[sizeof(out_dir) + sizeof(new_filename) + 1];
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
                // Executable was not killed by signal, remove input file.
                else {
                    int remove_status = remove(tmp_file);
                    if(remove_status != 0) {
                        printf("Error removing tmp file\n");
                    }
                    //printf("Exit code 0\n");
                }
                value++;
            }
            // If we should only bit flipp a specific offset then only run loop one time.
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
