/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#define _GNU_SOURCE
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <dirent.h>

struct arg_struct {
  int arg1;
  struct sockaddr_in arg2;
};

static void *tcp_thread(void *voidargs)
{
    #define MAXBUFFER 8192
    #define BUFFER_SIZE 1024
    struct arg_struct *args = (struct arg_struct *)voidargs;
    int tcpsockfd = (int) args->arg1;
    struct sockaddr_in tcp_cli_addr = (struct sockaddr_in) args -> arg2;
    int length  = sizeof(tcp_cli_addr);
    char tcpbuffer[ MAXBUFFER ];
    char tcpbuffer2[ MAXBUFFER ];
    //int status;
    int n;
    
    do
    {
      n = recv( tcpsockfd, tcpbuffer, BUFFER_SIZE, 0 );
      if ( n == 0 )
      {
        printf( "Client disconnected\n" );
        close(tcpsockfd);
        return NULL;
      }
      
      int i = 0;
      strncpy(tcpbuffer2,tcpbuffer, BUFFER_SIZE);
      char *beforeNewlineSize = strtok(tcpbuffer, "\n");
      int commandSize = strlen(beforeNewlineSize);
      char *token = strtok(tcpbuffer, " ");
      char* commands[4];
      //save
        while (token != NULL && i < 3)
          {
            commands[i] = strdup(token);
            token = strtok (NULL, " ");
            i++;
          }
        //Received SAVE mouse.txt 917
        printf("Received message to %s \n", commands[1]);
        fflush(stdout);
        //save the file, check if file exsists
        char fname[30] = "storage/";
        strcat(fname, commands[1]);
        fflush(stdout);
          // file doesn't exist
          // save file
          int sizeOfFile = atoi(commands[2]);
          //open the file
          int amountWritten = 0;
          int bufferRemaining = sizeOfFile;
          FILE *fp;
          fp = fopen( fname , "a" );
          char saveData[BUFFER_SIZE];
          strcpy(saveData,strchr(tcpbuffer2, '\n'));
          fflush(stdout);
          //if file is smaller than buffer can hold
          if(BUFFER_SIZE-commandSize > sizeOfFile){
            fwrite(saveData +1 , 1 , sizeOfFile , fp );
            fclose(fp);
          }
          else{
            fwrite(saveData +1 , 1 , BUFFER_SIZE-commandSize , fp );
            fclose(fp);
            amountWritten += BUFFER_SIZE-commandSize;
            bufferRemaining -= amountWritten; 
            //add the rest of the with recv
            while(amountWritten < sizeOfFile){
              //less than buffer remaining
              if(BUFFER_SIZE < bufferRemaining){
              tcpbuffer2[0] = '\0';
              fp = fopen(fname, "a");
              n = recv( tcpsockfd, tcpbuffer2, BUFFER_SIZE, 0 );
               // printf("%d\n %s\n",amountWritten, tcpbuffer2);
                fflush(stdout);
              fflush(stdout);
              fwrite(tcpbuffer2, 1 , BUFFER_SIZE , fp );
              fclose(fp);
              amountWritten += BUFFER_SIZE;
              }
              //more than buffer remaining
              else{
              tcpbuffer2[0] = '\0';
              fp = fopen(fname, "a");
              n = recv( tcpsockfd, tcpbuffer2, bufferRemaining, 0 );
                //printf("THis ONEEE \n%d\n %s\n",amountWritten, tcpbuffer2);
                fflush(stdout);
              fwrite(tcpbuffer2 , 1 , bufferRemaining , fp );
              fclose(fp);
              bufferRemaining -= bufferRemaining;
              amountWritten += bufferRemaining;
              }
          printf(" Stored file \"%s\" (%s bytes)\n" ,commands[1], commands[2]);
          fflush(stdout);
          //send ack
          sendto( tcpsockfd, "ACK\n", 4, 0, (struct sockaddr *) &tcp_cli_addr, sizeof(tcp_cli_addr)  );
          printf(" Sent ACK\n");
          fflush(stdout);
        }
      }
    }while(n>0);
    close(tcpsockfd);
    return NULL;
    fflush(stdout);
}

int main(int argc, char *argv[])
{

     #define MAXBUFFER 8192
     #define BUFFER_SIZE 1024
     int tcpsockfd, tcpportno, udpportno, udpsockfd;
     struct sockaddr_in tcp_serv_addr, tcp_cli_addr;
     struct sockaddr_in udp_serv_addr, udp_cli_addr;
     char udpbuffer[ MAXBUFFER ];
     int status;
     int n;
     //check for port
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     //make storage directory
     mkdir("./storage/", 0777) ;
     //TCP SERVER
     //open socket for tcp connection
     tcpsockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (tcpsockfd < 0) {
         fprintf(stderr,"ERROR, opening socket\n");
         exit(1);
      }
     bzero((char *) &tcp_serv_addr, sizeof(tcp_serv_addr));
     tcpportno = atoi(argv[1]);
     tcp_serv_addr.sin_family = AF_INET;
     tcp_serv_addr.sin_addr.s_addr = INADDR_ANY;
     tcp_serv_addr.sin_port = htons(tcpportno);
     if (bind(tcpsockfd, (struct sockaddr *) &tcp_serv_addr, sizeof(tcp_serv_addr)) < 0) {
         fprintf(stderr,"ERROR, binding\n");
         exit(1);
     }
      printf("Started server\n");
      fflush(stdout);
      listen(tcpsockfd,5);
      printf("Listening for TCP connections on port: %d\n", tcpportno);
      fflush(stdout);
      fd_set fds;
      //start of tcp/udp logic
      while(1)
      {
        FD_ZERO(&fds);
        FD_SET(tcpsockfd, &fds);
        status = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
        if (status==-1) {
           fprintf(stderr,"ERROR, select\n");
           exit(1);
        }

        if(status > 0)
        {
            //TCP LOGIC
            if (FD_ISSET(tcpsockfd, &fds)) {
              int length = sizeof(tcp_serv_addr);
              pthread_t tid;
              n = accept(tcpsockfd,(struct sockaddr *) &tcp_cli_addr,(socklen_t *) &length);
              struct arg_struct args;
              args.arg1 = n;
              args.arg2 = tcp_cli_addr;
              printf("Rcvd incoming TCP connection from: %s\n", inet_ntoa(tcp_cli_addr.sin_addr));  
              pthread_create(&tid, NULL, tcp_thread, (void *) &args);
              pthread_detach(tid);
            }
        }
       }
}
