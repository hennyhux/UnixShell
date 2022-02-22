#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE     80

/* setup() reads in the next command line string stored in inputBuffer, separating it
into distinct tokens using whitespace as delimiters. setup() modifies the args
parameter so that it holds pointers to the null-terminated strings that are the tokens in
the most recent user command line as well as a NULL pointer, indicating the end of
the argument list, which comes after the string pointers that have been assigned to args. */

void setup(char inputBuffer[], char *args[], int *background)
{

    int length,                                          /* number of characters in the command line */
        start,                                           /* index for the beginning of next command parameter */
        i,                                               /* index to access inputBuffer arrray */
        j;                                               /* index of where to place the next parameter into args[] */

    
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);  /* number of characters user entered */

    start = -1;
    j     = 0;

    if (length == 0) {
        /* Ctrl-D was entered, end of user command stream 
           Want to make it clear we ended here.  Without output, that's not clear. 
           This is helpful when the command entered is the myshell program itself. */
        printf("\n\nProgram use terminated.\n\n");
        exit(0);                                         /* 0 is success as a return status */
    }

    if (length < 0) {
        perror("error reading the command");
        exit(1);                                         /* Terminate with error code of 1 */
    }

    
    for (i = 0; i < length; i++) {                       /* Check every character in the inputBuffer array */
        switch (inputBuffer[i]) {
            case ' '  :
            case '\t' :                                  /* Argument separators */
                if(start != -1) {
                    args[j] = &inputBuffer[start];       /* Set up pointer */
                    j++;                                 /* increment number of args */
                }
                inputBuffer[i] = '\0';                   /* Add a null character '\0'; making a C 'string' */
                start = -1;                              /* Note that no longer reading in a word. */
                break;


            case '\n':                                   /* Final char examined */
                if (start != -1) {
                    args[j] = &inputBuffer[start];       /* Set up pointer */
                    j++;                                 /* increment number of args */
                }
                inputBuffer[i] = '\0';                   /* Add a null character '\0'; making a C 'string' */
                args[j] = NULL;                          /* No more arguments to this command */
                break;


            case '&':                                    /* background character */
                *background = 1;                         /* set background */
                inputBuffer[i] = '\0';                   /* Add a null character '\0'; making a C 'string' */
                break;


            default :                                    /* Some other character */
                if (start == -1)                         /* if not alreadying reading in next word... */
                    start = i;                           /* ... mark its start */
        }
    }
    args[j] = NULL;                                      /* Just in case the input line was > 80 */
    
    for (i = 0; i < j; i++)
        printf("args %d: %s\n",i,args[i]);               /* print each arg */  
    printf("backgr: %d\n\n", *background);               /* ... and background status */
}


int main(void)
{
    char  inputBuffer[MAX_LINE];                         /* buffer to hold the command entered */
    int   background;                                    /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE/2+1];                            /* command line arguments */
    int  commandCount = 1;

    while (1) {
        background   = 0;
        printf("\nCOMMAND[%i] > " , commandCount);       /* display a prompt */
        fflush(stdout) ;                                 /* since prompt has no '\n' need to flush the output so prompt is visible */
        setup(inputBuffer,args,&background);             /* get next command */

        /* Your parent-child process code goes here.  The steps are:
            (1) fork a child process using fork()
            (2) the child process will invoke execvp()
            (3) if background == 0, the parent will wait,
                otherwise the parent continues on to the subsequent setup() function call for the next command. 
            (4) Determine where/how to implement the 'cd' command. */

        if (args[0] != NULL && !strcmp(args[0], "cd")){
            if (chdir(args[1])){                          /*chdir returns -1 on error*/
                printf("Error, %s is not a proper directory." , args[1]);
                commandCount++;
            }
            continue;
        }            

        pid_t pid;

        /*Fork a child process*/
        pid = fork();
        /*If error occured, so fork() returns -1*/
        if (pid < 0){ 
            commandCount++; 
            fprintf(stderr, "Fork failed");
            exit(1);
        } 
        
        /*Child process*/
        else if (pid == 0){
            commandCount++;
            if (execvp(args[0], args) < 0){              /*The exec() function will only return if an error occured */
                printf("Error, %s is not a correct command" , args[0]);
                exit(1);
            }
        } else {                                         /*Parent process*/
            if (background == 0){                        /*Running in the background and waiting for the child process to be complete*/
                waitpid(pid, &background, 0);            /*system call suspends execution of the calling process until a child specified by pid argument has changed state.*/
            }
        }

        commandCount++; 
    }
    return 0;
}

