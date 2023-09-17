#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 100


int main()
{
    pid_t pid;
    close(2);
    dup(1);
  char command[BUFFER_SIZE];

  char  ExMark='!',One='1',nine='9' , MatrixHis[BUFFER_SIZE][BUFFER_SIZE];
  int  CounterInd = 0,backgroundProcess = 0,i;
  char* actualCommand[10];
    char* indexofxcom=0;


  while (1)
  {
      int length;
    fprintf(stdout, "my-shell> ");
    memset(command, 0, BUFFER_SIZE);
    fgets(command, BUFFER_SIZE, stdin); 
    
    		
   length=strlen(command)-1; 
    
    if (command[length] == '\n')
    {
        command[length] = '\0';
    }


    if((command[0] == ExMark && command[1] == ExMark ) ||(command[0]==ExMark &&  command[1] >= One  && command[1] <= nine))
    {
        int  command_index;
        
      indexofxcom = &command[1];
      if (command[1] >=  One && command[1] <= nine)
      {
        
        command_index = atoi(indexofxcom) - 1;
        
        if (command_index < 0 || command_index >= BUFFER_SIZE ||
            MatrixHis[command_index][0] == '\0')
        {
          fprintf(stdout, "No History\n");
         continue;
        }
      }
      
     else if (command[1] ==ExMark)
      {
      
     command_index =  CounterInd - 1;
      }
     

      strncpy(command, MatrixHis[command_index],100);
    
    }

    strncpy(MatrixHis[ CounterInd], command, 100);
    
     if ( ! (strncmp(command, "exit", 4) ))
    {
      break ;
    }

    CounterInd = ( CounterInd + 1);
  if (!strncmp(command, "history", 7))
    {
      // Print history of commands
      int i = CounterInd  - 1;
      do {
        if (MatrixHis[i][0] != '\0')
        {
          printf("%d \t %s\n", i + 1, MatrixHis[i]);
          i--;
        }
      } while (i >= 0);

      continue;
    }
    i = 0;
   actualCommand[i] = strtok(command, " ");

    while (actualCommand[i] != NULL)
    {
      i++;
      actualCommand[i] = strtok(NULL, " ");
    }

    if (i > 0 && strcmp(actualCommand[i - 1], "&") == 0)
    {
     backgroundProcess = 1;
     actualCommand[i - 1] = NULL;
    }

    pid = fork();
    
    if (pid == 0)
    {

        if (execvp(actualCommand[0], actualCommand) == -1)
        {
          perror("error");
          exit(1); 
        }
    }

    else if (pid < 0)
    {
      perror("error");
    }
    
    else
    {  

      if (!backgroundProcess)
      {
        if (waitpid(pid, NULL, 0) == -1)
        {
          perror("error");
        }
      }
    }
  }
  return 0;
}


