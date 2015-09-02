#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <signal.h>
#include <inttypes.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include  <sys/types.h>
#include  <sys/socket.h>
#include <poll.h>
#include <vector>  
#include <sys/types.h>
#include <ifaddrs.h>

#define BUFFER_SIZE     256

//uint16_t htons(uint16_t hostshort);


using namespace std;
//string var;

void error(const char *message){
    perror(message);
    exit(0);
}

int SocketFileDescriptor;

void SignalHandler(int param){
	cout << "Sending Closing Message" << endl;
	//Closing Message
	
		char BufferC[BUFFER_SIZE];
		struct sockaddr_in ServerAddress;
		int Result;
		int PortNumber = 50550;
		
		 SocketFileDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(0 > SocketFileDescriptor){
        error("ERROR opening socket");
    }
    
    // Set UDP socket to enable broadcast
    int BroadcastEnable = 1;
    Result = setsockopt(SocketFileDescriptor, SOL_SOCKET, SO_BROADCAST, &BroadcastEnable, sizeof(BroadcastEnable));
    if(0 > Result){
        close(SocketFileDescriptor);
        error("ERROR setting socket option");
    }
		
		
		bzero((char *) &ServerAddress, sizeof(ServerAddress));
		ServerAddress.sin_family = AF_INET;
		//ServerAddress.sin_addr.s_addr = htonl(INADDR_BROADCAST);
		ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
		ServerAddress.sin_port = htons(PortNumber);
		bzero(BufferC, BUFFER_SIZE);
        //fgets(BufferC, BUFFER_SIZE-1, stdin);
		bzero(BufferC, BUFFER_SIZE);
		
	
		unsigned int length;
		length=sprintf((char*)BufferC, "%s", "P2PI..........");
		
		////////////////////////////////Send Closing
		BufferC[0] = 'P';
		BufferC[1] = '2';
		BufferC[2] = 'P';
		BufferC[3] = 'I';
		
		string h = "pc12.cs.ucdavis.edu";
		string u="USER";
		*(uint16_t *)(BufferC + 4) = htons(3);
		*(uint16_t *)(BufferC + 6) = htons(50550);
		*(uint16_t *)(BufferC + 8) = htons(50551);
		
		string hC = h;
		string uC = u;
		strcpy(BufferC + 10, hC.c_str());
		strcpy(BufferC + hC.length()+ 11, uC.c_str());
	
		ServerAddress.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        Result = sendto(SocketFileDescriptor, BufferC, hC.length() + uC.length() + 12 , 0, (struct sockaddr *)&ServerAddress, sizeof(ServerAddress));
	//cout <<"got here"<<endl;
    close(SocketFileDescriptor);
    exit(0);
}

void DumpUDPPacket(uint8_t *sourceip, int sport, int dport, uint8_t *payload, int length){
    int Offset;
    
    printf("Source IP  : %d.%d.%d.%d\n",sourceip[0], sourceip[1], sourceip[2], sourceip[3]);
    printf("Source Port: %d\n",sport);
    printf("Dest Port  : %d\n",dport);
    printf("Length     : %d\n",length);
    
    Offset = 0;
    while(Offset < length){
	int Index;
        for(Index = 0; Index < 16; Index++){
            if(8 == Index){
                printf(" ");   
            }
            if(Offset + Index < length){
                printf("%02X ",payload[Offset + Index]);
            }
            else{
                printf("   ");
            }
        }
        printf("   ");
		
		//int Index;
        for(Index = 0; Index < 16; Index++){
            if(8 == Index){
                printf(" ");   
            }
            if(Offset + Index < length){
                if((' ' <= payload[Offset + Index])&&('~' >= payload[Offset + Index])){
                    printf("%c",payload[Offset + Index]);
                }
                else{
                    printf(".");
                }
            }
            else{
                printf(" ");
            }
        }
        printf("\n");
        Offset += 16;
    }
    
}

int GetLocalhostInformation(char *name, struct in_addr *addr, struct in_addr *mask){
    char Buffer[256];
    struct hostent *LocalHostEntry;
    struct ifaddrs *CurrentIFAddr, *FirstIFAddr;
    int Found = 0;
    
    if(-1 == gethostname(Buffer, 255)){
        return -1;
    }
    LocalHostEntry = gethostbyname(Buffer);
    if(NULL == LocalHostEntry){
        return -1;
    }
    strcpy(name, LocalHostEntry->h_name);
    LocalHostEntry = gethostbyname(name);
    if(NULL == LocalHostEntry){
        return -1;
    }
    bcopy((char *)LocalHostEntry->h_addr, (char *)addr, LocalHostEntry->h_length);
    
    if(0 > getifaddrs(&FirstIFAddr)){
        return -1;
    }
    CurrentIFAddr = FirstIFAddr;
    do{
        if(AF_INET == CurrentIFAddr->ifa_addr->sa_family){
            if(0 == memcmp(&((struct sockaddr_in *)CurrentIFAddr->ifa_addr)->sin_addr, addr, LocalHostEntry->h_length)){
                bcopy((char *)&((struct sockaddr_in *)CurrentIFAddr->ifa_netmask)->sin_addr, (char *)mask, LocalHostEntry->h_length);
                Found = 1;
                break;
            }
        }
        CurrentIFAddr = CurrentIFAddr->ifa_next;
    }while(NULL != CurrentIFAddr);
    freeifaddrs(FirstIFAddr);
    if(!Found){
        return -1;
    }
    return 0;
}


int main(int argc, char *argv[]){
    int SocketFileDescriptor,SocketFileDescriptor2, PortNumber,tpPort, Result, TPResult, BroadcastEnable;
    socklen_t ServerLength,ClientLength;
    struct sockaddr_in ServerAddress,ServerAddress2,ClientAddress;
    char Buffer[BUFFER_SIZE];
	char Buffer2[BUFFER_SIZE];
	char BufferD[BUFFER_SIZE];
	char BufferR[BUFFER_SIZE];
	char BufferC[BUFFER_SIZE];
	char BufferRead[BUFFER_SIZE];
	char TPBuffer[BUFFER_SIZE];
	struct pollfd fds[200];
	int nfds = 2;
	int n;
	string user;
	vector<string> UserList;
	struct hostent *Server2;
	
	//Defaults:
	PortNumber = 50550;
	tpPort = 50551;
	string u = "User";
	string h = "pc12.cs.ucdavis.edu";
	int initTimeout = 5;
	int maxTimeout = 60;
	int timeout = initTimeout * 1000;
	
	
	int optind=1;
    // decode arguments
    while ((optind < argc) && (argv[optind][0]=='-')) {
        string sw = argv[optind];
		/*if(argc == 1){
			cout <<"Set as Default"<< endl;
			break;
		}*/
        if (sw=="-up") {
            optind++;
            PortNumber = atoi(argv[optind]);
        }
		else if (sw=="-u") {
            optind++;
			u.clear();
            u = (argv[optind]);
        }
		else if (sw=="-tp") {
            optind++;
            tpPort = atoi(argv[optind]);
        }
		else if (sw=="-dt") {
            optind++;
            initTimeout = atoi(argv[optind]);
        }
		else if (sw=="-dm") {
            optind++;
            maxTimeout = atoi(argv[optind]);
        }
	}
	//cout <<u<<endl;
	
    if((1 > PortNumber)||(65535 < PortNumber)){
        fprintf(stderr,"Port %d is an invalid port number\n",PortNumber);
        exit(0);
    }
    // Create UDP/IP socket
    printf("Creating socket\n");
    SocketFileDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(0 > SocketFileDescriptor){
        error("ERROR opening socket");
    }
    
    // Set UDP socket to enable broadcast
    BroadcastEnable = 1;
    Result = setsockopt(SocketFileDescriptor, SOL_SOCKET, SO_BROADCAST, &BroadcastEnable, sizeof(BroadcastEnable));
    if(0 > Result){
        close(SocketFileDescriptor);
        error("ERROR setting socket option");
    }
    
    signal(SIGTERM, SignalHandler);
    signal(SIGINT, SignalHandler);
    signal(SIGUSR1, SignalHandler);
	 
    
    // Setup ServerAddress data structure
    bzero((char *) &ServerAddress, sizeof(ServerAddress));
    ServerAddress.sin_family = AF_INET;
    //ServerAddress.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    ServerAddress.sin_port = htons(PortNumber);
	
	// Binding socket to port
    if(0 > bind(SocketFileDescriptor, (struct sockaddr *)&ServerAddress, sizeof(ServerAddress))){ 
        error("ERROR on binding");
    }
	
	/////////////////////////Testing
	 // Create TCP/IP socket
    // printf("Creating socket\n");
    // SocketFileDescriptor2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // if(0 > SocketFileDescriptor2){
        // error("ERROR opening socket");
    // }
    //Convert/resolve host name 
    // Server2 = gethostbyname(argv[1]);
    // if(NULL == Server2){
        // fprintf(stderr,"ERROR, no such host\n");
        // exit(0);
    // }
    // Setup ServerAddress data structure
    // bzero((char *) &ServerAddress2, sizeof(ServerAddress2));
    // ServerAddress2.sin_family = AF_INET;
    // bcopy((char *)Server2->h_addr, (char *)&ServerAddress2.sin_addr.s_addr, Server2->h_length);
    // ServerAddress2.sin_port = htons(tpPort);
    //Connect to server
    // if(0 > connect(SocketFileDescriptor2, (struct sockaddr *)&ServerAddress2, sizeof(ServerAddress2))){
        // error("ERROR connecting");
    // }
    // printf("Please enter the message: ");
    // bzero(TPBuffer, BUFFER_SIZE);
    // fgets(TPBuffer, BUFFER_SIZE-1, stdin);
    
    
    // bzero(TPBuffer, BUFFER_SIZE);
    // Read data from server
    // TPResult = read(SocketFileDescriptor2, TPBuffer, BUFFER_SIZE-1);
	// TPResult = sendto(SocketFileDescriptor2, TPBuffer, 12 , 0, (struct sockaddr *)&ServerAddress2, sizeof(ServerAddress2));
	/////////////////////////////////////
	
	  
	
	int loop = 1;
    while(1){
		signal(SIGINT, SignalHandler);
		
        printf("[Press Enter]");
		
		bzero(BufferD, BUFFER_SIZE);
        fgets(BufferD, BUFFER_SIZE-1, stdin);
		bzero(BufferD, BUFFER_SIZE);

	
		unsigned int length;
		length=sprintf((char*)BufferD, "%s", "P2PI..........");
		
		////////////////////////////////Send Discovery
		BufferD[0] = 'P';
		BufferD[1] = '2';
		BufferD[2] = 'P';
		BufferD[3] = 'I';
		
		
		*(uint16_t *)(BufferD + 4) = htons(1);
		*(uint16_t *)(BufferD + 6) = htons(PortNumber);
		*(uint16_t *)(BufferD + 8) = htons(tpPort);
		
		string hD = h;
		string uD = u;
		strcpy(BufferD + 10, hD.c_str());
		strcpy(BufferD + hD.length()+ 11, uD.c_str());
	
		ServerAddress.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        Result = sendto(SocketFileDescriptor, BufferD, hD.length() + uD.length() + 12 , 0, (struct sockaddr *)&ServerAddress, sizeof(ServerAddress));
		///////////////////////
		
		
        bzero(Buffer, BUFFER_SIZE);
        //fgets(Buffer, BUFFER_SIZE-1, stdin);
		bzero(Buffer, BUFFER_SIZE);
		
		memset(fds, 0, sizeof(nfds));
		fds[0].fd = SocketFileDescriptor;
		fds[0].events = POLLIN;
		int pollingResult;
		
		
		if(timeout <= (maxTimeout*1000)){
			pollingResult = poll(fds, nfds, timeout);
		}
		else {
			cout << "MAX TIMEOUT EXCCEEDED, TIMEOUT IS " << timeout << endl;
			loop = 1;
		}
		if(pollingResult < 0){
			perror("   poll() failed");
			break;
		}
	if(pollingResult == 0)
	{
		cout << "in timeout" << endl;
        bzero(Buffer, BUFFER_SIZE);
		//Send Discover
		Buffer[0] = 'P';
		Buffer[1] = '2';
		Buffer[2] = 'P';
		Buffer[3] = 'I';
		
		
		*(uint16_t *)(Buffer + 4) = htons(1);
		*(uint16_t *)(Buffer + 6) = htons(50550);
		*(uint16_t *)(Buffer + 8) = htons(50551);
		
		//string h = "pc12.cs.ucdavis.edu";
		//cout << u << endl;
		
		strcpy(Buffer + 10, h.c_str());
		strcpy(Buffer + h.length()+ 11, u.c_str());
	
		ServerAddress.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        Result = sendto(SocketFileDescriptor, Buffer, h.length() + u.length() + 12 , 0, (struct sockaddr *)&ServerAddress, sizeof(ServerAddress));
         if(0 > Result){ 
            error("ERROR sending to server");
		}
		
		//Increase Timeout
		timeout = timeout * 2;
	}
	else {
		cout << "In Poll" << pollingResult << endl;
		for(int i = 0; i < pollingResult; i++)
		{
			int Result2;
			bzero(Buffer2, BUFFER_SIZE);
	
			ClientLength = sizeof(ClientAddress);	
			Result2 = recvfrom(SocketFileDescriptor, Buffer2, BUFFER_SIZE, 0, (struct sockaddr *)&ClientAddress, &ClientLength);
			if(Result2 < 0){
				error("ERROR receive from client");
			}
		
		uint16_t  type;
		uint16_t ClientUDP;
		uint16_t ClientTCP;
		type = *((uint16_t *)(Buffer2 + 4));
		type = ntohs(type);
		ClientUDP = *((uint16_t *)(Buffer2 + 6));
		ClientUDP = ntohs(ClientUDP);
		ClientTCP = *((uint16_t *)(Buffer2 + 8));
		ClientTCP = ntohs(ClientTCP);
		string ClientHost = "";
		string ClientUser = "";
		
		ClientHost = string(Buffer2 + 10);
		ClientUser = string(Buffer2 + 11 + ClientHost.length());
		cout << "type: " << type << endl;

		
		if(type == 1)
		{
		cout<<" Type is 1. Got Discovery. Send Reply [Press Enter]"<<endl;
		bzero(BufferR, BUFFER_SIZE);
        fgets(BufferR, BUFFER_SIZE-1, stdin);
		bzero(BufferR, BUFFER_SIZE);

	
		unsigned int length;
		length=sprintf((char*)BufferR, "%s", "P2PI..........");
		
		////////////////////////////////Send Reply
		BufferR[0] = 'P';
		BufferR[1] = '2';
		BufferR[2] = 'P';
		BufferR[3] = 'I';
		
		
		*(uint16_t *)(BufferR + 4) = htons(2);
		*(uint16_t *)(BufferR + 6) = htons(50550);
		*(uint16_t *)(BufferR + 8) = htons(50551);
		
		string hR = h;
		string uR = u;
		strcpy(BufferR + 10, hR.c_str());
		strcpy(BufferR + hR.length()+ 11, uR.c_str());
	
		ServerAddress.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        Result = sendto(SocketFileDescriptor, BufferR, hR.length() + uR.length() + 12 , 0, (struct sockaddr *)&ServerAddress, sizeof(ServerAddress));
		///////////////////////			
		break;	
		}
		
		else if(type == 2)
		{
			cout<<"got type 2 - Received Reply"<< endl;
			UserList.push_back(ClientHost + ClientUser);

			cout << "UserList: " << endl;
			for(int j = 0; j < UserList.size(); j++)
			{	
				cout << UserList[j] << endl;	
			}
			break;
		}
		i++;
		}
		}

	} 
		
		
    close(SocketFileDescriptor);
    return 0;
}
