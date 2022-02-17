#define  _GNU_SOURCE
#include <stdio.h> // printf()
#include <sys/wait.h> // wait()
#include <stdlib.h> // exit()
#include <unistd.h> // getpid(), fork()
#include <string.h>

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported

// main()

// getinput()
// input --> forkm
// if input == "exit"; terminate

// int readInput(char* str)
// {
//     char* line = NULL;
//     size_t linesize = 0;

  
//     if (getline(&line, &linesize, stdin) == -1){
//         if (feof(stdin)) {
//             printf("ree");
//             printf(line);
//             return 0; // We recieved an EOF
            
//         } else  {
//             printf(line);
//             perror("readline");
//             return -1;
//         }
//     } else{
//         printf("jsh says: what is ");
//         printf(line);
//         return 1 ;
//     }
// }

// void jsh_loop(void){
//     int rc = fork();

//     if (rc < 0){
//         fprintf(stderr, "fork failed\n");
//         exit(1);
//     } else if (rc == 0){ // child (new process)
//         printf("hello, I am child (pid: %d)\n", (int) getpid());
//         // char *myargs[3] = {"wc", "shell.c", NULL};
//         execvp(myargs[0], myargs);
//         printf("ree");


//         execvp(word, )
        
//     } else { // parent process
//         int rc_wait = wait(NULL);
//         printf("hello, I am parent of %d (rc_wait:%d) (pid:%d)\n",
//                 rc, rc_wait, (int) getpid());
//     }

// }

int main(int argc, char **argv){
    // Initialize strings for command, args, and piped args
    // char inputString[MAXCOM]; //, *parsedArgs[MAXLIST];
    // char* parsedArgsPiped[MAXLIST];
    // int execFlag = 0;

    // continue loop
    // char* word;

    while (1) {
        printf("jsh$ ");
        // take input
        // if (readInput(inputString))
        //     continue;


        // * String Parsing
        // Variable Declaration
        char* line = NULL;
        size_t buf_size = 0;
        size_t line_size = 0;
        int i = 0;
        int wc = 0; // word count
        int pc = 0; // pipe count
        int status = 0;

        int commands_lens[MAXLIST];
        


        // Get input
        line_size = getline(&line, &buf_size, stdin);
        printf("jsh says: what is %s", line);

        // Count words/spaces and pipes
        for (i = 0; i < line_size; i++){
            // printf("%c, wc: %d, pc: %d\n", line[i], wc, pc);
            if (line[i] == ' '){
                wc++;
            } else if (line[i] == '|'){
                // TODO: White space detection around piping
                commands_lens[pc] = wc;
                // printf("PIPE FOUND AT: %d, prev pc: %d, wc: %d\n", i, pc, wc);
                pc++;
                wc = 0;
            }
        }
        if(pc == 0) wc++;
        commands_lens[pc] = wc;

        
        // printf("There are %d words.\n", wc);
        printf("\tThere are %d pipes.\n", pc);

        for (i = 0; i <= pc; i++){
            printf("\tCommand %d has len: %d\n", i, commands_lens[i]);
        }

        // No pipe case
        char* command[commands_lens[0]];
        
        if (pc == 0){
            printf("No pipe case\n");
            printf("\tLen of curr command: %d\n", commands_lens[0]);
            
            // Parses string
            for(i = 0; i < commands_lens[0]; i++){
                command[i] = strsep(&line, " ");
                printf("\tWord: %d is %s\n", i, command[i]);

                // Removes newline
                if(i == commands_lens[0] - 1){
                    // printf("\tReplacing newline in %s:\n", command[i]);
                    // printf("\t%s len: %ld\n", command[i], strlen(command[i]));
                    command[i][strlen(command[i])-1] = '\0';
                    
                    printf("\t\tAfter newline removal: %s\n", command[i]);
                    // printf("\t%s len: %ld\n", command[i], strlen(command[i]));
                }
            }
            command[commands_lens[0]] = NULL; // the len of the command gets all messed up
        }
        else{
            char* command[commands_lens[0]];
        }


        // break into commands

        // iterate over commands
        // for each command fork

        // in child: get command specific command

        // * jsh Loop
        int rc = fork();

        if (rc < 0){
            fprintf(stderr, "fork failed\n");
            exit(1);
        } else if (rc == 0){ // child (new process)
            printf("hello, I am child (pid: %d)\n", (int) getpid());
            // char *myargs[3] = {"wc", "shell.c", NULL};
            // execvp(myargs[0], myargs);
            // printf("ree");

            // char* commandargs = strcat(word, line);
            // printf(commandargs);



            execvp(command[0], command);
            
        } else { // parent process
            int rc_wait = wait(&status);
            printf("Status:%d\n", status);
            printf("hello, I am parent of %d (rc_wait:%d) (pid:%d)\n",
                    rc, rc_wait, (int) getpid());
        }

        // jsh_loop();
        // process
        // execFlag = processString(inputString,s
        // parsedArgs, parsedArgsPiped);
        // execflag returns zero if there is no command
        // or it is a builtin command,
        // 1 if it is a simple command
        // 2 if it is including a pipe.
  
        // execute
        // if (execFlag == 1)
        //     execArgs(parsedArgs);
  
        // if (execFlag == 2)
        //     execArgsPiped(parsedArgs, parsedArgsPiped);
    }
    return 0;
}