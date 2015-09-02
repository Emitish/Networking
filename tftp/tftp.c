#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <dirent.h>
#include  <setjmp.h>

#define BUFFER_SIZE     256
#define RRQ		1
#define WRQ 	2
#define DATA 	3
#define ACK		4
#define ERROR	5

#define PACKETSIZE 	1024
#define FILES 		5
#define FILENAMESIZE 255
#define DATASIZE 	512
#define INPUTSIZE   100

jmp_buf  JumpBuffer;
void signal_handler(int);


void error(const char *message){
    perror(message);
    exit(1);
}

void signal_handler(int signum){
	char input;
	signal(signum,SIG_IGN);
	printf("Do you want to exit? Y or N\n");
	input = getchar();
	if(input == 'y' || input == 'Y')
		exit(0);
	else{
		signal(SIGINT, signal_handler);
		getchar();
	}

}


void tftpPut(int SocketFileDescriptor, char* filename){
	struct ACKS {
		signed short int opcode;
		signed short int fragment;
	};
	
	struct PAYLOAD {
		unsigned short int opcode;
		unsigned short int fragment;
		char info[DATASIZE];
	};
	
	printf("Got in Put function\n");

 
  int increment = 1; 
  int fragNo = 0;
  FILE* file = fopen(filename, "ab");
  char Buffer[DATASIZE+1];

  struct PAYLOAD* packets = malloc(sizeof(struct PAYLOAD));
  bzero(packets, sizeof(packets));
  
  struct ACKS* ack = malloc(sizeof(struct ACKS));
  bzero(ack, sizeof(ack));

  
  int ClientLength = sizeof(struct sockaddr);
  int length = 0; 

  while(1) {
    fragNo++;
    struct sockaddr ClientAddress;
	
    if(recvfrom(SocketFileDescriptor, packets, sizeof(struct PAYLOAD), 0, &ClientAddress, &ClientLength) < 0){
      continue;
    }
	
    length = strlen(packets->info);
    if(length == 0) break;
    strcpy(Buffer, packets->info);
	
    if(length > DATASIZE) {
      Buffer[DATASIZE] = '\0';
      length = strlen(Buffer);
    }
	
    fwrite(Buffer, sizeof(char), length, file);
    ack->opcode = htons(ACK);
	ack->fragment = htons(increment);
    
    if(sendto(SocketFileDescriptor, ack, sizeof(struct ACKS), 0, &ClientAddress, ClientLength) < 0){
      exit(1);
    }
	
    if(length < DATASIZE){
		break;
	}
	
	fragNo = increment;
	increment++;
}
	fclose(file);
	printf("Done putting file\n");
}

void sendFile(int SocketFileDescriptor, struct sockaddr* ClientAddress, int ClientLength, char* filename){
	printf("In send function\n");
	
	struct ACKS { 
		signed short int opcode;
		signed short int fragment;
	};
    struct PAYLOAD {
		unsigned short int opcode;
		unsigned short int fragment;
		char info[DATASIZE];
	};
	
  
  int length = 0;
  int fragNo = 0; 
  
  FILE *file = fopen(filename, "something");
  char Buffer[DATASIZE+1]; 
 
  struct PAYLOAD* packet = malloc(sizeof(struct PAYLOAD));
  bzero(packet, sizeof(packet));
  
  struct ACKS* ack = malloc(sizeof(struct ACKS));
  bzero(ack, sizeof(ack));

  while(length = fread(Buffer, sizeof(char), DATASIZE, file)) {

    fragNo++; 
    packet->opcode = htons(DATA); 
    packet->fragment = htons(fragNo); 
	Buffer[length] = '\0'; 

    bzero(packet->info, DATASIZE);
    memcpy(packet->info, Buffer, strlen(Buffer));
	
    printf("got ack");

    if(ntohs(ack->opcode) == ACK) {
		printf("got here");
	
      if(ntohs(ack->fragment) == fragNo) { 
			if(length < DATASIZE) { 
				break;
			}
		else { 
			continue;
		}
      }
    
	} 
  }
	fclose(file);
}


int main(int argc, char *argv[]){
	printf("Starting server. \n");
	
	signal(SIGINT, signal_handler);
	

	
	
	int  NewSocketFileDescriptor, SocketFileDescriptor, PortNumber, InsideBuff;
	char Buffer[BUFFER_SIZE];

	if(argc == 1){
		PortNumber = 49999;	
    }
	else{
		PortNumber = atoi(argv[1]);
	}
	
    //PortNumber = atoi(argv[1]);
	
	printf("Connected to port %d.\n", PortNumber);
    if((1 > PortNumber)||(65535 < PortNumber)){
        fprintf(stderr,"Port %d is an invalid port number\n",PortNumber);
        exit(0);
    }
    // Create UDP/IP socket
    SocketFileDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(0 > SocketFileDescriptor){
        error("ERROR opening socket");
    }
    // Setup ServerAddress info structure
	struct sockaddr_in ServerAddress;
    bzero((char *) &ServerAddress, sizeof(ServerAddress));
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_addr.s_addr = INADDR_ANY;
    ServerAddress.sin_port = htons(PortNumber);
    
	// Binding socket to port
    if(0 > bind(SocketFileDescriptor, (struct sockaddr *)&ServerAddress, sizeof(ServerAddress))){ 
        error("ERROR on binding");
    }
    /*ClientLength = sizeof(ClientAddress);
    bzero(Buffer, BUFFER_SIZE);*/
	
	
	struct sockaddr ClientAddress; int ClientLength= sizeof(struct sockaddr);
	
    // Receive message from client
	//////////////////////////////////////
	struct ACKS {
		signed short int opcode;
		signed short int fragment;
	};
	
	struct REQUEST {
		signed short int opcode;
		char filename[FILENAMESIZE];
		char mode[FILENAMESIZE];
		char zero1;
		char zero2;
	}
	__attribute__((packed));
	

	while(1){	
		ClientLength = sizeof(ClientAddress);
		bzero(Buffer, BUFFER_SIZE);
		
		struct REQUEST* request = malloc(sizeof(struct REQUEST));
		bzero(request, sizeof(request));
		struct ACKS* ack = malloc(sizeof(struct ACKS));
		bzero(ack, sizeof(ack));
			
		InsideBuff = recvfrom(SocketFileDescriptor, request, sizeof(struct REQUEST), 0, (struct sockaddr *)&ClientAddress, &ClientLength);
		
		if(0 > InsideBuff){
			error("ERROR receive from client");
		}
		
		//printf("Here is the message: %s\n", Buffer);
		// Send message to client
		//InsideBuff = sendto(SocketFileDescriptor, "I got your message", 18, 0, (struct sockaddr *)&ClientAddress, ClientLength);
		if(0 > InsideBuff){
			error("ERROR sending to client");
		}
		
		char filename[FILENAMESIZE];
		strcpy(filename, request->filename);
		//printf(filename);
		if(fork() == 0){
				printf("Got in fork\n");
				close(SocketFileDescriptor);
				//printf("got in here fork2");
				int Child_Proc = socket(AF_INET, SOCK_DGRAM, 0);
				struct sockaddr_in ChildAddress; 
				bzero((char*)&ChildAddress, sizeof(ChildAddress));
				ChildAddress.sin_family = AF_INET;
				ChildAddress.sin_addr.s_addr = htonl(INADDR_ANY);
				bind(Child_Proc, (struct sockaddr*)&ChildAddress, sizeof(ChildAddress));
				//printf("got in fokr");
				//printf("%d\n",*opcode);
				
		switch(ntohs(request->opcode)){
			
			case WRQ:
			printf("Got WRQ\n");
			ack->opcode = htons(ACK); 
			ack->fragment = htons(0);
			sendto(SocketFileDescriptor, ack, sizeof(struct ACKS), 0, (struct sockaddr *)&ClientAddress, ClientLength);
			tftpPut(SocketFileDescriptor, filename);
			close(Child_Proc); exit(0);
			break;
				
			case RRQ:
			printf("Got RRQ\n");
			ack->opcode = htons(ACK);
			ack->fragment = htons(0);
			sendto(SocketFileDescriptor, ack, sizeof(struct ACKS), 0, &ClientAddress, ClientLength);
			//printf("got here\n");
			sendFile(SocketFileDescriptor, &ClientAddress, ClientLength, filename);
			//printf("got here\n");
			close(Child_Proc);
			//printf("got here\n");
			exit(0);
			//printf("got here\n");
			break;
		}
		
		
		}
	
	}
	
    //close(SocketFileDescriptor);
    return 0; 	
	
	
}
