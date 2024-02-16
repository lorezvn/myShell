#include "myShell.h"

void execute_command(char ** argv) {

    /* Gestione dei comandi non built-in (tutti i comandi != cd e quit)*/

    pid_t pid = fork();

    //Fork fallita
    if (pid < 0) {
        printf("fork failed");
        exit(EXIT_FAILURE);
    }

    //Processo Figlio
    else if (pid == 0) {
        
        //Esecuzione comando -> command not found se errore
        if (execvp(argv[0], argv) < 0) {
            printf("%s: command not found\n", argv[0]);
            exit(1);
        }   
        exit(0);
    }
}

void change_directory(char ** argv) {

    /* Gestione del comando cd */

    int size = 0;

    for (int i=0; argv[i] != NULL; i++) {
        size++;
    }
    
    //cd (home)
    if (size == 1) {
        chdir(getenv("HOME"));
    } 
    
    //cd (path specificato)
    else if (size == 2) {

        char absolute_path[PATH_MAX];

        //calcolo del path assoluto
        if (realpath(argv[1], absolute_path) == NULL) {
            printf("myShell: cd: %s: %s\n", argv[1], strerror(errno));    
        }

        //chdir
        else {
            if (chdir(absolute_path) == -1) {
                printf("myShell: cd: %s: %s\n", argv[1], strerror(errno));
            }
        }
    }

    //Piu' di un argomento specificato
    else {
        printf("myShell: cd: too many arguments\n");    
    }
}

void get_command(char * command, char **argv) {

    /* Gestione del singolo comando specificato */

    //Split sugli spazi -> argv popolato
    char * token = strtok(command, " ");
    int i = 0;

    while (token  != NULL) {
        argv[i] = token;
        token = strtok(NULL, " ");
        i++;
    }

    argv[i] = NULL;

}

int main(int argc, char ** args) {

    char cwd[PATH_MAX];
    char buffer[BUFFER_SIZE];
    char * argv[BUFFER_SIZE];
    char * commands[BUFFER_SIZE];
    FILE * batch_file = NULL;
    bool quit = false;
    int num_commands = 0;

    if (argc > 2) {
        printf("usage: ./myShell [batchFile]\n");
        exit(EXIT_FAILURE);
    }

    // Modalita' Batch -> File di input
    if (argc == 2) {
        batch_file = fopen(args[1], "r");
        if (batch_file == NULL) {
            perror("myShell: Opening file");
            exit(EXIT_FAILURE);
        }
    }

    while (!quit) {

        fflush(stdout);

        // Modalita' Batch (NO prompt)
        if (batch_file != NULL) {

            //EOF
            if (fgets(buffer, BUFFER_SIZE, batch_file) == NULL) {

                //Chiusura file
                if (fclose(batch_file) != 0) {
                    perror("myShell: Closing file");
                    exit(EXIT_FAILURE);
                }
                break;
            }

            //riga vuota
            if (strcmp(buffer, "\n") == 0) {
                continue;
            }

            //stampa della riga di comando eseguita
            printf("\n> %s\n", buffer);
        }

        // Modalita' Interattiva
        else {
            //Prompt -> current working directory
            getcwd(cwd, sizeof(cwd));
            printf("\033[1;34m%s\x1b[0m$ ", cwd);

            //Ctrl-D
            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
                printf("\n");
                break;
            }
        }

        //Rimozione di \n 
        if(buffer[strlen(buffer)-1] == '\n'){
            buffer[strlen(buffer)-1] = '\0';    
        }


        //Gestione di più comandi separati da ;
        int i = 0;
        char * token = strtok(buffer, ";");

        while (token  != NULL) {
            commands[i] = token;
            token = strtok(NULL, ";");
            i++;
        }

        num_commands = i;

        //Esecuzione comandi
        for (int i=0; i < num_commands; i++) {

            //parse input
            get_command(commands[i], argv);

            //formattazione errata
            if (argv[0] == NULL) {
                continue;
            }

            //Gestione quit
            if (strcmp(argv[0], "quit") == 0) {
                
                quit = true;
            }

            //Gestione cd
            else if (strcmp(argv[0], "cd") == 0) {
                change_directory(argv);
            }

            //Gestione di comandi non built-in
            else {
                execute_command(argv);
            }
        }

        //Attesa che tutti i processi figli terminino
        while (wait(NULL) > 0);

        //Se è stato inserito quit come comando -> exit 
        if (quit) {

            //Chiusura file (se specificato)
            if (batch_file != NULL) {
                if (fclose(batch_file) != 0) {
                    perror("myShell: Closing file");
                    exit(EXIT_FAILURE);
                }
            }
            exit(EXIT_SUCCESS);
        }
    }
}

