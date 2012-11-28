#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sysexits.h>
#include <ctype.h>
#include "text.h"
#include <fcntl.h>
#include <errno.h>
#include <rpc/rpc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <wordexp.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/time.h>

int debug = 0;
CLIENT *clnt;

void usage(char *program_name) {
	printf("Usage: %s [-d] -s <server>\n", program_name);
}


void f_ping(){
	if (debug)
		printf("PING\n");

        pingresponse pres;

        /////////////////////////////////////////////

        pres.output = malloc(sizeof(int) + 1);

        struct timeval start;
        struct timeval end;
        int rc1,rc2;

        pingrequest request;
        
        struct sockaddr_in addr;
        get_myaddress(&addr);
        
        char * reqstr = malloc(sizeof(char)*50);
        char * addrstr = inet_ntoa(addr.sin_addr);


        sprintf(reqstr,"%s:%d",addrstr,addr.sin_port);



        request.clientId.clientId_val = reqstr;
        request.clientId.clientId_len = strlen(reqstr);

        rc1=gettimeofday(&start, NULL); //Get current time


        pingproc_1(&request, &pres, clnt);
        
        //    clnt_perror(clnt,"");
        //    exit(1);
        

        rc2=gettimeofday(&end, NULL); //Get current time
        printf("%u.%06u s\n", (end.tv_sec - start.tv_sec)/2, (end.tv_usec - start.tv_usec)/2); //Print time interval
        
	// Write code here
}

void f_swap(char *src, char *dst){
	if (debug)
		printf("SWAP <SRC=%s> <DST=%s>\n", src, dst);
	
        swaprequest request;

        //////////// Add IP to struct ///////////////////////////
        struct sockaddr_in addr;
        get_myaddress(&addr);

        char * reqstr = malloc(sizeof(char)*50);
        char * addrstr = inet_ntoa(addr.sin_addr);
        sprintf(reqstr,"%s:%d",addrstr,addr.sin_port);

        request.clientId.clientId_val = reqstr;
        request.clientId.clientId_len = strlen(reqstr);
        //////////////////////////////////////////////////////////
     
        //Open file //////////////////
        char *se = (char *) malloc(sizeof(char)*1025);
        FILE *fd1 = fopen(src,"r");
        if(fd1 == NULL) perror("fopen");

        //Obtain size /////////////////
        int file_size = 0;
        fseek (fd1 , 0 , SEEK_END);
        file_size = ftell (fd1);
        rewind (fd1);
        ////////////////////////////////////////////////

        // Default characters to read from file
        int readchars;
        int written = 0;  //Write file pointer
        FILE *fd2 = fopen(dst,"a"); // Write file open
        if(fd2 == NULL) perror("fopen");
        int i = 0;
        int swapped = 0;

        while(file_size > 0)
        {
             i++;
             readchars = sizeof(char) * 1024;
             if(file_size < readchars) readchars = file_size;  //If there are less than 500 to EOF, read that number///
             if(readchars < 0) readchars = 1024 + readchars;    ////////////////////////////////////////////////////////
             file_size -= 1024; //Decrement chars left to read
             fseek(fd1,written,SEEK_SET);
             fread(se,1,readchars,fd1);

             int p = ftell(fd1);
             //printf("Readign @ %d\n",p);
             if(readchars < 1024)se[readchars] = '\0';  // REPLACE EOF WITH END OF STRING
             //printf("%s\n",se);

             request.textin.textin_val = se;
             request.textin.textin_len = readchars;             
             request.isFinished = (readchars < 1024) ? 1 : 0;
             request.part = i;
             request.swapno = swapped;

             if(i == 1)
             {
                 if(readchars < 1024)
                 {
                     request.filesize = readchars;
                 }
                 else
                 {
                     request.filesize = file_size + 1024;
                 }
             }

             swapresponse response;
             response.out.out_val = malloc(sizeof(char)*1024);
             response.swapno = 0;


             swapproc_1(&request, &response, clnt);

             if(response.out.out_val == NULL)clnt_perror(clnt,"");
             //swapped +=response->swapno;
             
             char *reqstr = response.out.out_val;
             if(readchars < 200)reqstr[readchars -1] = '\0';
             //printf("%d\n", response.swapno);
             swapped = response.swapno;

                 ///////// STORE RESULT IN FILE ///////////////////////////


                if(readchars != 200) reqstr[readchars - 1] = '\n';
                fseek(fd2,written,SEEK_SET);
                written += readchars;
                fwrite(reqstr,1,readchars,fd2);
                rewind (fd1);

               // se[readchars - 1] = '\0';  // REPLACE EOF WITH END OF STRING
                ////////////////////////////////////////////////////////////

        }
        printf("%d\n",swapped);
        free(reqstr);
        free(se);
        fclose(fd1);
        fclose(fd2);
}

void f_hash(char *src){
	if (debug)
		printf("HASH <SRC=%s>\n", src);
	
        

        //////////// Add IP to struct ///////////////////////////
        struct sockaddr_in addr;
        get_myaddress(&addr);

        char * reqstr = malloc(sizeof(char)*50);
        char * addrstr = inet_ntoa(addr.sin_addr);
        sprintf(reqstr,"%s:%d",addrstr,addr.sin_port);


        //////////////////////////////////////////////////////////

        //Open file //////////////////
        char *se = (char *) malloc(sizeof(char)*1028);
        FILE *fd1 = fopen(src,"r");
        if(fd1 == NULL) perror("fopen");

        //Obtain size /////////////////
        int file_size = 0;
        fseek (fd1 , 0 , SEEK_END);
        file_size = ftell (fd1);
        rewind (fd1);
        ////////////////////////////////////////////////
        int readchars = 0;
        int written = 0;
        int hash = 0;

        int i = 0;

        int hasParts = (file_size < 1024) ? 0 : 1;
        while(file_size > 0)
        {
             i++;
             readchars = sizeof(char) * 1024;
             if(file_size < readchars) readchars = file_size;  //If there are less than 500 to EOF, read that number///
             if(readchars < 0) readchars = 1024 + readchars;    ////////////////////////////////////////////////////////
             file_size -= 1024; //Decrement chars left to read
             fseek(fd1,written,SEEK_SET);
             fread(se,1,readchars,fd1);

             written += readchars;

             struct hashrequest request;
             request.clientId.clientId_val = malloc(sizeof(char)*50);
             request.clientId.clientId_val = reqstr;
             request.clientId.clientId_len = strlen(reqstr);
             request.textin.textin_val = se;
             request.textin.textin_len = readchars;
             request.isFinished = (readchars < 1024) ? 1 : 0;
             request.part = i;
             if(request.isFinished == 1) request.hash = hash;
             request.hash = hash;
             request.hasParts = hasParts;

                          if(i == 1)
             {
                 if(readchars < 1024)
                 {
                     request.filesize = readchars;
                 }
                 else
                 {
                     request.filesize = file_size + 1024;
                 }
             }
             
             struct hashresponse response;
             hashproc_1(&request, &response, clnt);

             hash = response.hash;
             //printf("%d\n",hash);
             //printf("hashAcum %d\n",hash);
        }
        printf("%d\n",hash);
        free(se);
        fclose(fd1);

	// Write code here
}

void f_check(char *src, int hash){
	if (debug)
		printf("CHECK <SRC=%s> <HASH=%d>\n", src, hash);
        //////////// Add IP to struct ///////////////////////////
        struct sockaddr_in addr;
        get_myaddress(&addr);

        char * reqstr = malloc(sizeof(char)*50);
        char * addrstr = inet_ntoa(addr.sin_addr);
        sprintf(reqstr,"%s:%d",addrstr,addr.sin_port);


        //////////////////////////////////////////////////////////

        //Open file //////////////////
        char *se = (char *) malloc(sizeof(char)*1028);
        FILE *fd1 = fopen(src,"r");
        if(fd1 == NULL) perror("fopen");

        //Obtain size /////////////////
        int file_size = 0;
        fseek (fd1 , 0 , SEEK_END);
        file_size = ftell (fd1);
        rewind (fd1);
        ////////////////////////////////////////////////
        int readchars = 0;
        int written = 0;
        int servhash = 0;

        int i = 0;

        int hasParts = (file_size < 1024) ? 0 : 1;
        struct checkresponse response;

        while(file_size > 0)
        {
             i++;
             readchars = sizeof(char) * 1024;
             if(file_size < readchars) readchars = file_size;  //If there are less than 500 to EOF, read that number///
             if(readchars < 0) readchars = 1024 + readchars;    ////////////////////////////////////////////////////////
             file_size -= 1024; //Decrement chars left to read
             fseek(fd1,written,SEEK_SET);
             fread(se,1,readchars,fd1);

             written += readchars;

             struct checkrequest request;
             request.clientId.clientId_val = malloc(sizeof(char)*50);
             request.clientId.clientId_val = reqstr;
             request.clientId.clientId_len = strlen(reqstr);
             request.textin.textin_val = se;
             request.textin.textin_len = readchars;
             request.isFinished = (readchars < 1024) ? 1 : 0;
             request.part = i;
             if(request.isFinished == 1) request.servHash = servhash;
             request.servHash = servhash;
             request.hasParts = hasParts;
             request.clieHash = hash;

             if(i == 1)
             {
                 if(readchars < 1024)
                 {
                     request.filesize = readchars;
                 }
                 else
                 {
                     request.filesize = file_size + 1024;
                 }
             }

             
             checkproc_1(&request, &response, clnt);

             servhash = response.hashAcum;
             
        }
        int result = response.isCorrect[0];
        char *status = (result == 1) ? "OK" : "FAIL";
        printf("%s\n",status);
        free(se);
        fclose(fd1);

	
	// Write code here
}

void f_stat(){


        statrequest request;

        struct sockaddr_in addr;
        get_myaddress(&addr);

        char * reqstr = malloc(sizeof(char)*50);
        char * addrstr = inet_ntoa(addr.sin_addr);


        sprintf(reqstr,"%s:%d",addrstr,addr.sin_port);



        request.clientId.clientId_val = reqstr;
        request.clientId.clientId_len = strlen(reqstr);

        statresponse sres;

        /////////////////////////////////////////////


	statproc_1(&request, &sres, clnt);
        //{
        //    perror("");
        //    exit(1);
        //}

        printf("%d %d %d %d %d\n",sres.pingnum, sres.swapnum, sres.hashnum, sres.checknum, sres.statnum);
	// Write code here
}

void f_quit(){
	if (debug)
		printf("QUIT\n");
        exit(0);
	// Write code here
}

void shell() {
	char line[1024];
	char *pch;
	int exit = 0;
	
	wordexp_t p;
	char **w;
	int ret;
	
	memset(&p, 0, sizeof(wordexp));
	
	do {
		fprintf(stdout, "c> ");
		memset(line, 0, 1024);
		pch = fgets(line, 1024, stdin);
		
		if ( (strlen(line)>1) && ((line[strlen(line)-1]=='\n') || (line[strlen(line)-1]=='\r')) )
			line[strlen(line)-1]='\0';
		
		ret=wordexp((const char *)line, &p, 0);
		if (ret == 0) {
			w = p.we_wordv;
		
			if ( (w != NULL) && (p.we_wordc > 0) ) {
				if (strcmp(w[0],"ping")==0) {
					if (p.we_wordc == 1)
						f_ping();
					else
						printf("Syntax error. Use: ping\n");
				} else if (strcmp(w[0],"swap")==0) {
					if (p.we_wordc == 3)
						f_swap(w[1],w[2]);
					else
						printf("Syntax error. Use: swap <source_file> <destination_file>\n");
				} else if (strcmp(w[0],"hash")==0) {
					if (p.we_wordc == 2)
						f_hash(w[1]);
					else
						printf("Syntax error. Use: hash <source_file>\n");
				} else if (strcmp(w[0],"check")==0) {
					if (p.we_wordc == 3)
						f_check(w[1], atoi(w[2]));
					else
						printf("Syntax error. Use: check <source_file> <hash>\n");
				} else if (strcmp(w[0],"stat")==0) {
					if (p.we_wordc == 1)
						f_stat();
					else
						printf("Syntax error. Use: stat\n");
				} else if (strcmp(w[0],"quit")==0) {
					if (p.we_wordc == 1) {
						f_quit();
						exit = 1;
					} else {
						printf("Syntax error. Use: quit\n");
					}
				} else {
					fprintf(stderr, "Error: command '%s' not valid.\n", w[0]);
				}
			}
			
			wordfree(&p);
		}
	} while ((pch != NULL) && (!exit));
}

int main(int argc, char *argv[]){
	char *program_name = argv[0];
	int opt;
	char *server;

	setbuf(stdout, NULL);
	
	// Parse command-line arguments
	while ((opt = getopt(argc, argv, "ds:")) != -1) {
		switch (opt) {
			case 'd':
				debug = 1;
				break;
			case 's':
				server = optarg;
				break;
			case '?':
				if ((optopt == 's') || (optopt == 'p'))
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
			default:
				usage(program_name);
				exit(EX_USAGE);
		}
	}
	
	if (debug)
		printf("SERVER: %s\n", server);
	
	char *host;

        char *expparam = "-s";
        int valid = strcmp(argv[1],expparam);

        if(argc != 3 || valid != 0) {
            usage(argv[0]);
            exit(1);
	}

	host = argv[2];  // Get host from parameters

        //////////// Create udp connection with host ////////////////////////
        if((clnt = clnt_create(host, SERVER, SERVERPROGVERS, "tcp")) == NULL)
        {

            printf("Error connecting to %s\n",host);
            exit(1);
        }

        
	
	shell();
	
	exit(EXIT_SUCCESS);
}

