#define  _GNU_SOURCE
#include <stdio.h> // printf()
#include <sys/wait.h> // wait()
#include <stdlib.h> // exit()
#include <unistd.h> // getpid(), fork()
#include <string.h>

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported


void exec_no_pipe(char** command);
void exec_pipe(char*** commands, int i, int pc);

int main(int argc, char **argv){

    while (1) {
        printf("jsh$ ");
        // Variable Declaration
        char* line = NULL;
        size_t buf_size = 0;
        size_t line_size = 0;
        int i = 0;
        int j = 0;
        int wc = 0; // word count
        int pc = 0; // pipe count

        int commands_lens[MAXLIST];
        

        // Get input
        line_size = getline(&line, &buf_size, stdin);

        if (strcmp("exit\n", line) == 0) return 0; 

        // Count words/spaces and pipes
        for (i = 0; i < line_size; i++){
            // New word
            if ((line[i] == ' ') &&  (i != 0)
                && (line[i-1] != '|') &&  (line[i-1] != ' ')){
                    wc++;    
            // Pipe case        
            } else if (line[i] == '|'){
                if (line[i-1] != ' ') wc++;
                commands_lens[pc] = wc;
                pc++;
                wc = 0;
            }
        }
        if (line[line_size-2] != ' ' && line[line_size-2] != '|'){
            wc++;
        }
        commands_lens[pc] = wc;
        

        // No pipe case
        if (pc == 0){
            char* command_nop[commands_lens[0]];
            
            // Parses string
            for(i = 0; i < commands_lens[0]; i++){
                command_nop[i] = strsep(&line, " ");
                
                // Removes newline at th end
                if(i == commands_lens[0] - 1){
                    command_nop[i][strlen(command_nop[i])-1] = '\0';
                }
            }
            // Readds array terminator
            command_nop[commands_lens[0]] = NULL;


            exec_no_pipe(command_nop);
        // Pipe case
        } else{
            char** commands[pc + 1];
            char* commands_wo_pipes[pc + 1];


            // Separates by pipes
            for(i = 0; i < pc + 1; i++){
                commands_wo_pipes[i] = strsep(&line, "|");
                if (commands_wo_pipes[i] == NULL) break;
            }


            // Loops through commands; i is the command #
            for(i = 0; i < pc + 1; i++){
                char** command = malloc(commands_lens[i] * sizeof(char *));
        
                // Parses command ith string; j is the word # in command i
                for(j = 0; j < commands_lens[i]; j++){
                    command[j] = strsep(&commands_wo_pipes[i], " ");
                    
                    // Found null; trying again
                    if(command[j][0] == NULL) {
                        j--;
                    // if (strlen(command[j]) == 0) j--;
                    }
                    else{
                        // Removes newline at the end or all commands
                        if(i == pc && j == commands_lens[i] - 1){
                            command[j][strlen(command[j])-1] = '\0';
                        }
                    }
                }
                // Reads array terminator
                command[commands_lens[i]] = NULL;
                commands[i] = command;
            }

            exec_pipe(commands, i, pc);

            // for(i = 0; i < pc + 1; i++){
            //     free(commands[i]);
            // }
        }
        // free(line);
    }
    return 0;
}


void exec_no_pipe(char** command){
    int rc = fork();
    int status;

    if (rc < 0){
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0){ // child (new process)
        printf("child sees (command only): %s\n", command[0]);
        execvp(command[0], command);
        printf("jsh error: Command not found\n");
        exit(127);
    } else { // parent process
        int rc_wait = waitpid(rc, &status, 0);
        if (WIFEXITED(status)) {
            printf("jsh status: %d\n", WEXITSTATUS(status));
        }
    }
}


void exec_pipe(char*** commands, int i, int pc){
    int pipes[pc * 2];
    int status, rc;

    for (i = 0; i < pc; i++){
        if (pipe(pipes + i * 2) < 0){
            fprintf(stderr, "creating pipes failed\n");
            exit(1);
        }
    }
    i = 0;
    while (i < pc + 1) {
        rc = fork();
        if (rc < 0){
            fprintf(stderr, "fork failed\n");
            exit(1);
        } else if (rc == 0){ // child (new process)
            if (i != 0) {
                if (dup2(pipes[(i - 1) * 2], 0) < 0){
                    fprintf(stderr, "dup2 failed\n");
                    exit(1);
                }
            } if (i != pc) {
                if (dup2(pipes[i * 2 + 1], 1) < 0){
                    fprintf(stderr, "dup2 failed\n");
                    exit(1);
                }
            }
            for (int j = 0; j < 2 * pc; j++){
                close(pipes[j]);
            }
            if (execvp(commands[i][0], commands[i]) < 0){
                printf("jsh error: Command not found\n");
                exit(127);
            }
        }
        i++;
    }

    for (i = 0; i < 2 * pc; i++){
        close(pipes[i]);
    }

    for (i = 0; i < pc +1; i++){
        int rc_wait = waitpid(rc, &status, 0);
    }

    // if (WIFEXITED(status)) {
        printf("jsh status: %d\n", WEXITSTATUS(status));
    // }
    return;
}