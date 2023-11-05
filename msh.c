//  MSH main file
// Write your msh source code here

//#include "parser.h"
#include <stddef.h>         /* NULL */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

//Incluimos librería errno para manejo de fallos
#include <errno.h>

#define MAX_COMMANDS 8
//Definimos un tamaño de buffer para copiar un fichero a otro
#define BUFFER_SIZE 1024

#define LONG_STRING 100	


// ficheros por si hay redirección
char filev[3][64];

//to store the execvp second parameter
char *argv_execvp[8];

void siginthandler(int param)
{
    printf("****  Saliendo del MSH **** \n");
    //signal(SIGINT, siginthandler);
        exit(0);
}

/**
 * Get the command with its parameters for execvp
 * Execute this instruction before run an execvp to obtain the complete command
 * @param argvv
 * @param num_command
 * @return
 */
void getCompleteCommand(char*** argvv, int num_command) {
    //reset first
    for(int j = 0; j < 8; j++)
        argv_execvp[j] = NULL;

    int i = 0;
    for ( i = 0; argvv[num_command][i] != NULL; i++)
        argv_execvp[i] = argvv[num_command][i];
}


/**
 * Main sheell  Loop  
 */
int main(int argc, char* argv[])
{
    /**** Do not delete this code.****/
    int end = 0; 
    int executed_cmd_lines = -1;
    char *cmd_line = NULL;
    char *cmd_lines[10];

    if (!isatty(STDIN_FILENO)) {
        cmd_line = (char*)malloc(100);
        while (scanf(" %[^\n]", cmd_line) != EOF){
            if(strlen(cmd_line) <= 0) return 0;
            cmd_lines[end] = (char*)malloc(strlen(cmd_line)+1);
            strcpy(cmd_lines[end], cmd_line);
            end++;
            fflush (stdin);
            fflush(stdout);
        }
    }

    /*********************************/

    char ***argvv = NULL;
    int num_commands;
    
    int acc = 0;		//Variable de entorno


    while (1) 
    {
        int status = 0;
        int command_counter = 0;
        int in_background = 0;
        signal(SIGINT, siginthandler);

        // Prompt 
        write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

        // Get command
        //********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN NORMAL/CORRECTION MODE***************
        executed_cmd_lines++;
        if( end != 0 && executed_cmd_lines < end) {
            command_counter = read_command_correction(&argvv, filev, &in_background, cmd_lines[executed_cmd_lines]);
        }
        else if( end != 0 && executed_cmd_lines == end){
            return 0;
        }
        else{
            command_counter = read_command(&argvv, filev, &in_background); //NORMAL MODE
        }
        //************************************************************************************************


        /************************ STUDENTS CODE ********************************/
        if (command_counter > 0) {
            if (command_counter > MAX_COMMANDS){
                printf("Error: Numero máximo de comandos es %d \n", MAX_COMMANDS);
            }
            else {
                // Print command
                //print_command(argvv, filev, in_background);
            } 
        }
        
        // ACORDARSE SE HACER CASOS DE ERROR
        // QUITAR COMMAND COUNTER
        
        pid_t pid;
        int fd3[3], fichero;
        char string[LONG_STRING];
        
        //PROGRAMA MYCALC
        
        if ( (strcmp(argvv[0][0],"mycalc")) == 0){
        
        	if( ((strcmp(argvv[0][2], "add") != 0) && (strcmp(argvv[0][2], "mod") != 0)) || argvv[0][3] == NULL ){
        	
				snprintf(string, LONG_STRING, "[ERROR] La estructura del comando es mycalc <operando_1> <add/mod> <operando_2>\n");
				write(STDERR_FILENO, string, strlen(string));
        	}
		    	
		    	int num1, num2;
		    	num1 = atoi(argvv[0][1]);
			num2 = atoi(argvv[0][3]);
		    	//FUNCIONALIDAD ADD
			if( (strcmp(argvv[0][2], "add")) == 0){
					
				char buffer[8];
				const char *value = buffer;
				sprintf(buffer, "%d", acc);
		
				acc += num1 + num2;
				sprintf(buffer, "%d", acc);
				
				
				if( setenv("Acc",value,1) == -1){
					perror("Error al iniciar la variable de entorno\n");
					return -1;
			}
				
				snprintf(string, LONG_STRING, "[OK] %i + %i = %i; Acc %s\n", num1, num2, num1 + num2, getenv("Acc"));
				write(STDERR_FILENO, string, strlen(string));

			//FUNCIONALIDAD MOD
			}else if ((strcmp(argvv[0][2], "mod")) == 0){
				
				snprintf(string, LONG_STRING, "[OK] %i %% %i = %i; Cociente %i\n", num1, num2, num1%num2, num1/num2);
				write(STDERR_FILENO, string, strlen(string));
			
			}
			
			//PROGRAMA MYCP
        }else if(strcmp (argvv[0][0], "mycp") == 0){
        	if (argvv[0][1] != NULL && argvv[0][2] != NULL){
        	
        		//Abrimos los dos ficheros
        		int fich1, fich2;
        		if ((fich1 = open(argvv[0][1], O_RDONLY,0644)) == -1){
        			printf("[ERROR] Error al abrir el fichero origen\n");
        			return -1;
        		}
        		if ((fich2 = open(argvv[0][2], O_TRUNC | O_RDWR| O_CREAT,0664)) == -1){
        			printf("[ERROR] Error al abrir el fichero de salida\n");
        			return -1;
        		}
        		
        		//Copiamos el contenido del primer fichero en el segundo
        		char buffer[BUFFER_SIZE];
        		int n_read;
        		while((n_read = read(fich1, buffer,  BUFFER_SIZE)) > 0){
					if(write(fich2, buffer, n_read) < n_read){
						perror("El archivo no se ha copiado correctamente\n");
						close(fich1);
						close(fich2);
						return -1;
					}
				}
        			close(fich1);
				close(fich2);
				
				snprintf(string, LONG_STRING, "[OK] Copiado con exito el fichero %s a %s\n",argvv[0][1],argvv[0][2]);
				write(STDERR_FILENO, string, strlen(string));

        	
        	
        	}else{
        		printf("[ERROR] La estructura del comando es mycp <fichero origen><fichero destino>\n");
        	}
        }else if (argvv[0][0] != NULL){
       	
       	int fd[2], fd2[2];		//FICHERO  PIPE
       	int i;
       	
		if (command_counter > 1){
			if (pipe(fd2) == -1) {
                		perror("Error al crear una nueva tuberia");
                		exit(-1);
            		}
		}
        	
       	for (i = 0; i < command_counter; i++){
       		
       		pid = fork();
       		
       		
       		switch(pid){
       			case -1:
				    perror("Error al crear un hijo");
				    return -1;
				    
				case 0:		//HIJO
					
					//Redirección de la entrada
					if (i != 0 ){
					//REDIRIJO LA ENTRADA AL FICHERO fd
					close(STDIN_FILENO);
					dup(fd[STDIN_FILENO]);
					close(fd[STDIN_FILENO]);		//fd[0]
		       		close(fd[STDOUT_FILENO]);		//fd[1]
	       			}
	       		
		       		//Redirección de entrada
		       		if (i == 0 && filev[STDIN_FILENO][0] != '0') {
						if (close(STDIN_FILENO) == -1) {
						    perror("Error al cerrar el descriptor de entrada estandar en el hijo");
						    exit(-1);
						}

						if (open(filev[STDIN_FILENO], O_RDONLY) == -1) {
						    perror("Error al abrir el fichero de entrada en el hijo");
						    exit(-1);
						}
		            		}
	       			
	       			//Redirección de la salida
		       		if ( i != (command_counter-1) ){
				       	//REDIRIJO LA SALIDA AL FICHERO fd2
				      		close(STDOUT_FILENO);
						dup(fd2[STDOUT_FILENO]);
						close(fd2[STDIN_FILENO]);		//fd2[0]
				      		close(fd2[STDOUT_FILENO]);		//fd2[1]
				      	}
				      	
				      	if (i == command_counter - 1) {
				      	
				      		//Redirección de salida
						if (filev[STDOUT_FILENO][0] != '0') {
						    if (close(STDOUT_FILENO) == -1) {
						        perror("Error al cerrar el descriptor de salida estandar en el hijo");
						        exit(-1);
						    }
						    if (open(filev[STDOUT_FILENO], O_WRONLY | O_CREAT | O_TRUNC,0644) == -1) {
						        perror("Error al abrir el fichero de salida en el hijo");
						        exit(-1);
						    }
						}
						
						//Redirección de error
						if (filev[STDERR_FILENO][0] != '0') {
						    if (close(STDERR_FILENO) == -1) {
						        perror("Error al cerrar el descriptor de salida de error estandar en el hijo");
						        exit(-1);
						    }
						    if (open(filev[STDERR_FILENO], O_WRONLY | O_CREAT | O_TRUNC,644) == -1) {
						        perror("Error al abrir el fichero de salida de error en el hijo");
						        exit(-1);
						    }
						}
                    			}
			      		
		       		execvp( argvv[i][0], argvv[i]);
		       		perror(NULL);
                    			return -1;
		       	
		       	default:		//PADRE
		       	
		       		if (command_counter > 1) {
		       			if (i > 0){
			       			close(fd[STDIN_FILENO]);		//fd[0]
				       		close(fd[STDOUT_FILENO]);		//fd[1]
			       		}
		       			
		       			if (i < (command_counter-1)){
				       		fd[STDIN_FILENO] = fd2[STDIN_FILENO];
						    	fd[STDOUT_FILENO] = fd2[STDOUT_FILENO];
						    	if (pipe(fd2) == -1) {
								perror("Error al crear una nueva tuberia");
								exit(-1);
            						}
				            	
		                    		}
		            		}
		            		if(in_background == 0 && i == command_counter - 1){
						while ( wait(&status) != pid );
							if ( status < 0 ) {
								printf("ERROR EN LA EJECUCIÓN DEL HIJO\n");
								return -1;
							}

					} else if (in_background == 1){
						printf("[%d]\n",pid);		//Imprimimos el PID del proceso que está en background.
					}
		            		
       		}
       			
       		
       	}
       
      	}else{
		printf("\n");      	
      	}
        
     } 
    return 0;
    
}
