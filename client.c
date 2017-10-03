#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>

#define BUFFER_SIZE 1024

char* append_strings(const char* strold, const char* strnew) {
    size_t len = strlen(strold) + strlen(strnew) + 1;
    char *out = malloc(len);
    sprintf(out, "%s%s", strold, strnew);
    return out;
}

bool write_server(char* name, int sd, char* msg) {
    int n = write(sd, msg, strlen(msg));
    if (n < strlen(msg)) {
//        fprintf(stderr, "%s ERROR: write() failed\n", name);
        return false;
    }
    printf("%s sends %s", name, msg);
    return true;
}

bool read_server(char* name, int sd) {
    char buffer[BUFFER_SIZE];
    int n = read(sd, buffer, BUFFER_SIZE - 1);
    if (n == -1) {
        fprintf(stderr, "%s ERROR: read() failed\n", name);
        return false;
    }
    else {
        if (n == 0) printf("%s rcvd no data; also, server socket was closed\n", name);
        else {
            buffer[n] = '\0';
            printf("%s recieves %s", name, buffer);
        }
        return true;
    }
}

int main(int argc, char *argv[]) {

     //check for port
     if (argc < 3) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

    FILE *fp;
    char fname[30] = "storage/";
    strcat(fname, argv[2]);
    fp = fopen( fname , "a" );

    int tcpportno;
    tcpportno = atoi(argv[1]);
    
    remove("storage/legend.txt");
    remove("storage/chicken.txt");

    char* name = "client B";
    int sd = socket(PF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        fprintf(stderr, "ERROR: socket() failed\n");
        return EXIT_FAILURE;
    }
    
    struct hostent* hp = gethostbyname("localhost");
    if (hp == NULL) {
        fprintf(stderr, "%s ERROR: gethostbyname() failed\n", name);
        return EXIT_FAILURE;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    memcpy((void*) &server.sin_addr, (void*) hp->h_addr, hp->h_length);
    unsigned short port = tcpportno; //argv[1]
    server.sin_port = htons(port);
    if (connect(sd, (struct sockaddr*) &server, sizeof(server)) == -1) {
        fprintf(stderr, "ERROR: connect() failed\n");
        return EXIT_FAILURE;
    }
    printf("%s connects\n", name);
    int i;
    i = 0;
    while(true){
      char str[100] = "";
      scanf("%s", str);
      char msg2[300] = "SAVE ";
      strcat(msg2, argv[2]);
      strcat(msg2, " 300\n");
      strcat(msg2, "[");
      strcat(msg2, argv[1]);
      strcat(msg2, "]");
      strcat(msg2, " PID ");
      char pid[10];
      snprintf(pid, 10,"%d",(int)getpid());
      strcat(msg2, pid );
      strcat(msg2, " ITERATION #" );
      char ival[10];
      snprintf(ival, 10,"%d",(int)i);
      strcat(msg2, ival );
      strcat(msg2, "\n");
      strcat(msg2, str);
      strcat(msg2, "\n");
      char* msg = &msg2[0];
      i = i +1;
      //if (!write_server(name, sd, msg)) return EXIT_FAILURE;
      //if (!read_server(name, sd)) return EXIT_FAILURE;
      write_server(name,sd,msg);
    }
    return EXIT_SUCCESS;
    
}
