#include "EncryptionLibraryALL.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <ifaddrs.h>
#include <sys/poll.h>
#include <time.h>
#include <signal.h>
#include <poll.h>
#include <vector> 

#define BUFFER_SIZE     256
using namespace std;

string Username = "USER";
char ThisName[256];
int SocketFileDescriptor;
short unsigned  UDPPortNumber = 50550;
short unsigned  TCPPortNumber = 50551;
struct sockaddr_in ServerAddress, ClientAddress;

void error(const char *message){
    perror(message);
    exit(0);
}



//Initialize NED
	uint64_t n = 0;
	uint64_t e = 0;
	uint64_t d = 0;
class Packet{
	public:	
	char signature[4];
	uint16_t type;
	uint16_t UdpPort;
	uint16_t TcpPort; 
	char data[246];
     };
class EncryptBroadcastPacket{
	public:	
	char signature[4];
	uint16_t thisType;
	uint16_t High0;
	uint16_t High1;
	uint16_t Low0;
	uint16_t Low1;
	//uint16_t thisType;
	char data[246];
     };
class UserEntryPacket{
	public:	
	uint32_t Entry;
	uint16_t UdpPort;
	char data[250];
     };
class UserEntry{
	public:	
	uint16_t UdpPort;
	string Hostname;
	uint16_t TcpPort; 
	string Username;
     };
	 
int GetLocalhostInformation(char *name, struct in_addr *addr){
    char Buffer[256];
    struct hostent *LocalHostEntry;
    if(-1 == gethostname(Buffer, 255)){return -1;}
    LocalHostEntry = gethostbyname(Buffer);
    if(NULL == LocalHostEntry){return -1;}
    strcpy(name, LocalHostEntry->h_name);
    LocalHostEntry = gethostbyname(name);
    if(NULL == LocalHostEntry){return -1;}
    bcopy((char *)LocalHostEntry->h_addr, (char *)addr, LocalHostEntry->h_length);
    return 0;
}

void SignalHandler(int param){
	cout << "Sending Closing Message" << endl;
	//Closing Message
	char hotname[255];
	struct in_addr thishostaddress;
	 GetLocalhostInformation(hotname, &thishostaddress);
	 
	Packet * ClosePacket = new Packet;
	memset(ClosePacket->data,'\0',246);
	strncpy(ClosePacket->signature, "P2PI", 4);
	ClosePacket->type = htons(0003);
	ClosePacket->UdpPort= htons(UDPPortNumber);
	ClosePacket->TcpPort = htons(TCPPortNumber);
	strcpy(ClosePacket->data, hotname);
	strcpy(ClosePacket->data + strlen(ClosePacket->data) +1, Username.c_str());
	cout<<hotname<<endl;
	ServerAddress.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    sendto(SocketFileDescriptor, (void*)ClosePacket, (10 + strlen(ClosePacket->data)+2+Username.length()), 0, (struct sockaddr *)&ServerAddress, sizeof(ServerAddress));	//cout <<"got here"<<endl;
    close(SocketFileDescriptor);
    exit(0);
}

int PrintBuffer(char Buffer[256] ){
	int i;
	for ( i = 0; i < 256; i++ ) {
		putc( isprint(Buffer[i]) ? Buffer[i] : '.' , stdout );
		} 
	return 0;
}
void DisplayMessage(char *data, int length){
    int Offset = 0;
    int Index;
    
    
    while(Offset < length){
        printf("%04X ", Offset);
        for(Index = 0; Index < 16; Index++){
            if((Offset + Index) < length){
                printf("%02X ",data[Offset + Index]);
            }
            else{
                printf("   ");
            }
        }
        for(Index = 0; Index < 16; Index++){
            if((Offset + Index) < length){
                if((' ' <= data[Offset + Index])&&(data[Offset + Index] <= '~')){
                    printf("%c",data[Offset + Index]);
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


int main(int argc, char *argv[]){
	
	short unsigned  TCPPortNumber = 50551;
	short unsigned  AuthPortNumber = 50552;
	int UDPTimeout = 5;
	int UDPTimeoutMax = 60;
	string Username = "USER";
	int SocketFileDescriptor2, BroadcastEnable;
	
	socklen_t ClientLength;
	char Buffer[BUFFER_SIZE], TCPBuffer[BUFFER_SIZE];
	time_t RawTime;
    struct tm TimeOut;
	Packet * UDPPacket;
	Packet * Broadcast = new Packet;
	Packet * Reply;
	Packet * TCPConnect = new Packet;
	Packet * TCPAcceptPacket = new Packet;
	Packet * TCPPacket = new Packet;
//ENCRYPTION 
//ENCRYPTION 
//ENCRYPTION 	
//ENCRYPTION 	
	EncryptBroadcastPacket * EncryptPacket = new EncryptBroadcastPacket;
	//StartEncryptPacket * StartPacket = new StartEncryptPacket;
	
	//ASK FOR PASSWORD
	string password;
	cout<< "Password: ";
	cin >> password;
	//cout<< password << endl;  
	
	//PASSWORD TO NED: uses StringToPublicNED
	StringToPublicNED(password.c_str(),n,e,d);
	cout << "N = "<< n << endl;
	cout << "E = "<< e << endl;
	cout << "D = "<< d << endl;
	
	//Generate Random Value: uses GenerateRandomValue()
	uint64_t RandomValue = GenerateRandomValue();
	cout << "Random Value: "<< RandomValue << endl;
	
	uint64_t N = 2932334010766554497;
	uint64_t E = 5;
	uint64_t D = 2345867205873393293;

	//Encrypt Random Value
	PublicEncryptDecrypt(RandomValue, E , N);
	
	//uint64_t tempHardcodeSecretNumber =  190392693;
	uint64_t tempHardcodeSecretNumber = 2385049040186691416;

	
	
	
	
	
	Packet * TCPUnavailable = new Packet;
	Packet * TCPRequestUser = new Packet;
	
	vector <pollfd> fds;
	fds.clear();
	//USERLIST
	int n;
	int Result;
	vector<UserEntry> UserList;
	UserEntry TempUser;
	
	//TCP
	int TCPSocketFileDescriptor, TCPSocketFileDescriptor2;
	struct sockaddr_in TCPServerAddress, TCPClientAddress;
	
	for ( int i=1; i <argc; i++){
		if (strcmp(argv[i], "-u")==0){
			//cout<<argv[i]<<endl;
			if ((i+1)<argc){
				if(strncmp(argv[i+1], "-",1)==0){
					Username = "USER";
					continue;
				}
				else{
					Username = argv[i+1];
					i++;
				}	
			}
		}
		else if(strcmp(argv[i], "-up")==0){
			//cout<<argv[i]<<endl;
			if ((i+1)<argc){
				if(strncmp(argv[i+1], "-",1)==0){
					UDPPortNumber = 50550;
					continue;
				}
				else{
					UDPPortNumber = atoi(argv[i+1]);
					i++;
				}
			}
		}
		else if(strcmp(argv[i], "-tp")==0){
			//cout<<argv[i]<<endl;
			if ((i+1)<argc){
				if(strncmp(argv[i+1], "-",1)==0){
					TCPPortNumber = 50551;
					continue;
				}
				else{
					TCPPortNumber = atoi(argv[i+1]);
					i++;
				}
			}
		}
		else if(strcmp(argv[i], "-dt")==0){
	//		cout<<argv[i]<<endl;
			if ((i+1)<argc){
				if(strncmp(argv[i+1], "-",1)==0){
					UDPTimeout = 5;
					continue;
				}
				else{
					UDPTimeout = atoi(argv[i+1]);
					i++;
				}
			}
		}
		else if(strcmp(argv[i], "-dm")==0){
		//	cout<<argv[i]<<endl;
			if ((i+1)<argc){
				if(strncmp(argv[i+1], "-",1)==0){
					UDPTimeoutMax = 60;
					continue;
				}
				else{
					UDPTimeoutMax = atoi(argv[i+1]);
					i++;
				}
			}
		}
		else if(strcmp(argv[i], "-ap")==0){
			//cout<<argv[i]<<endl;
			if ((i+1)<argc){
				if(strncmp(argv[i+1], "-",1)==0){
					TCPPortNumber = 50552;
					continue;
				}
				else{
					TCPPortNumber = atoi(argv[i+1]);
					i++;
				}
			}
		}
		else{
			cout<<"default: "<<argv[i]<<endl;
		}
	}

	struct in_addr ThisAddress;
    char ThisName[256];
    char AddressBuffer[INET_ADDRSTRLEN];
	
	cout<<"Username = "<<  Username <<endl;
	cout<<"UDP Port = "<< UDPPortNumber <<endl;
	cout<<"TCP Port = "<<  TCPPortNumber <<endl;
	cout<<"Auth Port =  "<< AuthPortNumber<<endl;
	cout<<"Mintimeout = "<<  UDPTimeout  <<endl;
	cout<<"Maxtimeout = "<<  UDPTimeoutMax <<endl;
		
//Get Local Host Info    
    if(0 == GetLocalhostInformation(ThisName, &ThisAddress)){
        inet_ntop(AF_INET, &ThisAddress, AddressBuffer, INET_ADDRSTRLEN);
//        printf("Hostname = %s\nIP = %s\n",ThisName, AddressBuffer);
    }
//Create Socket and Check
	if (( SocketFileDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
		perror("TFTP Server: ERROR opening Socket.\n");
		return 0;
	}
	TCPSocketFileDescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(0 > SocketFileDescriptor){
        perror("ERROR opening socket");
		return 0;
    }
// Set UDP socket to enable broadcast
	BroadcastEnable = 1;
    if(0 >  setsockopt(SocketFileDescriptor, SOL_SOCKET, SO_BROADCAST, &BroadcastEnable, sizeof(BroadcastEnable))){
        close(SocketFileDescriptor);
        perror("ERROR setting socket option");
		return 0;
    }
//Binding Socket
    cout<<"TFTP Server: Start Binding"<<endl;	
    bzero((char *) &ServerAddress, sizeof(ServerAddress));
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    ServerAddress.sin_port = htons(UDPPortNumber);
	if(bind(SocketFileDescriptor, (struct sockaddr *)&ServerAddress, sizeof(ServerAddress)) == -1){  
		perror("ERROR on binding");
		return 0;
	}
	cout<<"BIND SUCCESS."<<endl;




//TCP Binding Socket
    cout<<"TCP Server: Start Binding"<<endl;	
    bzero((char *) &TCPServerAddress, sizeof(TCPServerAddress));
    TCPServerAddress.sin_family = AF_INET;
    TCPServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    TCPServerAddress.sin_port = htons(TCPPortNumber);
	
	if(bind(TCPSocketFileDescriptor, (struct sockaddr *)&TCPServerAddress, sizeof(TCPServerAddress)) == -1){  
		perror("ERROR on TCP binding");
		return 0;
	}
	cout<<"TCP BIND SUCCESS."<<endl;
	
//Creating Auth Packet	
	memset(EncryptPacket->data,'\0',246);
	strncpy(EncryptPacket->signature, "P2PI", 4);
	EncryptPacket->thisType = htons(0x010);
	EncryptPacket->High0 = htons(tempHardcodeSecretNumber>>48);
	EncryptPacket->High1 = htons(tempHardcodeSecretNumber>>32);
    EncryptPacket->Low0 = htons(tempHardcodeSecretNumber>>16);
	EncryptPacket->Low1 = htons(tempHardcodeSecretNumber);

	strcpy(EncryptPacket->data, Username.c_str());
	//cout<< EncryptPacket->Low0<<endl;
//Creating UDP Broadcast Packet	
	memset(Broadcast->data,'\0',246);
	strncpy(Broadcast->signature, "P2PI", 4);
	Broadcast->type = htons(0001);
	Broadcast->UdpPort= htons(UDPPortNumber);
	Broadcast->TcpPort = htons(TCPPortNumber);
	strcpy(Broadcast->data, ThisName);
	strcpy(Broadcast->data + strlen(Broadcast->data) +1, Username.c_str());
//Creating TCP Connect Packet
	memset(TCPConnect->data,'\0',246);
	strncpy(TCPConnect->signature, "P2PI", 4);
	TCPConnect->type = htons(0004);
	strcpy(TCPConnect->data,Username.c_str());
//Creating TCP Accept Packet
	memset(TCPAcceptPacket->data,'\0',246);
	strncpy(TCPAcceptPacket->signature, "P2PI", 4);
	TCPAcceptPacket->type = htons(0005);
//Creating TCP Unavailable Packet
	memset(TCPUnavailable->data,'\0',246);
	strncpy(TCPUnavailable->signature, "P2PI", 4);
	TCPUnavailable->type = htons(0006);
//Creating TCP Request Userlist Packet
	memset(TCPRequestUser->data,'\0',246);
	strncpy(TCPRequestUser->signature, "P2PI", 4);
	TCPRequestUser->type = htons(0007);

	
	
	
	listen(TCPSocketFileDescriptor, 100);
	
	
	//PPOLL SETUP
	//UDP POLL PUSHED
	pollfd TempPoll;
	TempPoll.fd = SocketFileDescriptor;
	TempPoll.events = POLLIN;
	fds.push_back(TempPoll);
	//TCP POLL PUSHED
	TempPoll.fd = TCPSocketFileDescriptor;
	TempPoll.events = POLLIN;
	fds.push_back(TempPoll);
	int pollingResult;
	char inputx;
	while(1){
	signal(SIGINT, SignalHandler);
//Packet Sending	
		if(UserList.size() ==0){
			ServerAddress.sin_addr.s_addr = htonl(INADDR_BROADCAST);
			// if(0 > sendto(SocketFileDescriptor, (void*)Broadcast, (10 + strlen(Broadcast->data)+2+Username.length()), 0, (struct sockaddr *)&ServerAddress, sizeof(ServerAddress))){ 
				// perror("ERROR sending to server");
				// return 0;
			// }
			if(0 > sendto(SocketFileDescriptor, (void*)EncryptPacket, (14+ strlen(EncryptPacket->data)), 0, (struct sockaddr *)&ServerAddress, sizeof(ServerAddress))){ 
				perror("ERROR sending to server"); 
				return 0;
			}
			//Result = recvfrom(SocketFileDescriptor, Buffer, BUFFER_SIZE, 0, (struct sockaddr *)&ClientAddress, &ClientLength);

			
			cout<<"Sending discovery, timeout = "<<UDPTimeout<<"s"<<endl;
			//Result = recvfrom(SocketFileDescriptor, Buffer, BUFFER_SIZE, 0, (struct sockaddr *)&ClientAddress, &ClientLength);
			//DisplayMessage(Buffer, Result);
			//ServerAddress.sin_port = htons(AuthPortNumber);
			
			Result = recvfrom(SocketFileDescriptor, Buffer, BUFFER_SIZE, 0, (struct sockaddr *)&ClientAddress, &ClientLength);
			//DisplayMessage(Buffer, Result);
			
		}	
		
//POLLING SETUP
		//cout << "POLLING..............." << endl;
		pollingResult = poll(fds.data(), fds.size(), UDPTimeout*1000);
		
//DONE POLLING
		if (pollingResult > 0){
		//POLLIN ON UDP ACCEPTING PEOPLE
			if(fds.at(0).revents & POLLIN){
				//cout << "In Poll " << pollingResult << endl;
				for(int i = 0; i < pollingResult; i++)
				{
					
					bzero(Buffer, BUFFER_SIZE);
					ClientLength = sizeof(ClientAddress);
			// Receive message from client
					//printf("..............WAITING ON RESPONSE.....\n");
					Result = recvfrom(SocketFileDescriptor, Buffer, BUFFER_SIZE, 0, (struct sockaddr *)&ClientAddress, &ClientLength);
					if(0 > Result){
						perror("ERROR receive from client");
						return 0;
					}   
					//cout << "Packet!"<< endl;
					//DisplayMessage(Buffer, Result);
					UDPPacket = (Packet *)Buffer;
				
					if (ntohs(UDPPacket->type) == 1){
					cout << "UDP: Recieved Discovery Message"<< endl;
						for(int j=0; j<(10 + strlen(Broadcast->data)+2+Username.length()); j++){
							if(strcmp((char*)Broadcast + j, Buffer + j) != 0){ // IF THE PACKET IS NOT MY OWN BROADCAST
								//USERLIST ENTRY CONSTRUCTION
								TempUser.UdpPort = ntohs(UDPPacket->UdpPort);
								TempUser.Hostname =UDPPacket->data;
								TempUser.TcpPort = ntohs(UDPPacket->TcpPort);
								TempUser.Username = UDPPacket->data + TempUser.Hostname.length() + 1;
								//Check if anready in Userlist
								if(UserList.size() ==0){
									UserList.push_back(TempUser);
								}
								else{
									int inUserList = 0;
									for(int k = 0; k < UserList.size(); k++){
										//IF UserEntry is the same as one that is already in the list, break
										if(((UserList[k].UdpPort == TempUser.UdpPort) &&(UserList[k].Hostname == TempUser.Hostname)
											&&(UserList[k].TcpPort == TempUser.TcpPort)&&(UserList[k].Username == TempUser.Username)) == 1){
												inUserList = 1;
												break;
										}
									}
									//if UserEntry is not in UserList
									if (inUserList == 0){
										UserList.push_back(TempUser);
									}
								}
								//Reply Packet Construction 
								cout<<endl<<endl<<"Preparing Packet....."<<endl;
								memset(UDPPacket->data,'\0',246);
								strncpy(UDPPacket->signature, "P2PI", 4);
								UDPPacket->type = htons(0002);
								UDPPacket->UdpPort= htons(UDPPortNumber);
								UDPPacket->TcpPort = htons(TCPPortNumber);
								strcpy(UDPPacket->data, ThisName);
								strcpy(UDPPacket->data + strlen(UDPPacket->data) +1, Username.c_str());
								if(0 > sendto(SocketFileDescriptor, (void*)UDPPacket, (10 + strlen(UDPPacket->data)+2+Username.length()), 0, (struct sockaddr *)&ClientAddress, sizeof(ClientAddress))){ 
									perror("ERROR sending to server");
									return 0;
								}
								cout<<"UDP: Reply Sent"<<endl;			
							}
						}
					}
					//IF RECEIVE A REPLY, ADD TO USERLIST
					else if(ntohs(UDPPacket->type) == 2){
						cout<<"UDP: Received Reply Message"<< endl;
						//BUILDING DATA FOR USERLIST
						TempUser.UdpPort = ntohs(UDPPacket->UdpPort);
						TempUser.Hostname =UDPPacket->data;
						TempUser.TcpPort = ntohs(UDPPacket->TcpPort);
						TempUser.Username = UDPPacket->data + TempUser.Hostname.length() + 1;
						//CHECK IF USER IS ALREADY IN USERLIST
						if(UserList.size() ==0){
							UserList.push_back(TempUser);
						}
						else{
							int inUserList = 0;
							for(int k = 0; k < UserList.size(); k++){
								//IF UserEntry is the same as one that is already in the list, break
								if(((UserList[k].UdpPort == TempUser.UdpPort) &&(UserList[k].Hostname == TempUser.Hostname)
									&&(UserList[k].TcpPort == TempUser.TcpPort)&&(UserList[k].Username == TempUser.Username)) == 1){
										inUserList = 1;
										break;
								}
							}
							//if UserEntry is not in UserList
							if (inUserList == 0){
								UserList.push_back(TempUser);
							}
						}
					}
					else if (ntohs(UDPPacket->type) == 3){
						cout << "UDP: Got a Closing Message " << endl;
						for(int k = 0; k < UserList.size(); k++){
							//IF UserEntry is in userlist, delete
							if(((UserList[k].UdpPort == TempUser.UdpPort) &&(UserList[k].Hostname == TempUser.Hostname)
								&&(UserList[k].TcpPort == TempUser.TcpPort)&&(UserList[k].Username == TempUser.Username)) == 1){
									UserList.erase(UserList.begin()+ k);
									//fds.erase(fds.begin()+2+k);
							}
						}
					}
					
					//PRINTING OUT USERLIST
					cout<<"Discovered "<<UserList.size()<<" users:"<<endl;
					for(int k = 0; k < UserList.size(); k++){
						cout<<" User "<<k<<" "<<UserList[k].Username<<"@"<<UserList[k].Hostname;
						cout<<" on UDP "<<UserList[k].UdpPort<<", TCP "<<UserList[k].TcpPort<<endl;
					}
				}
			}
			
			//POLLIN ON TCP ACCEPTING PEOPLE
			if(fds.at(1).revents & POLLIN) {
				//ACCEPT TCP
				TCPSocketFileDescriptor2 = accept(TCPSocketFileDescriptor,(struct sockaddr *)&ClientAddress, &ClientLength);
				if (TCPSocketFileDescriptor2 < 0){
					cout<<"TCP: ERROR on accept"<<endl;
				}
				
				int InPollList = 0;
				for(int i = 0; i < fds.size();i++){
					if (TCPSocketFileDescriptor2 == fds.at(i).fd){
						InPollList = 1;
					}
				}
				if(InPollList == 0){	
					TempPoll.fd = TCPSocketFileDescriptor2;
					TempPoll.events = POLLIN;
					fds.push_back(TempPoll);
					cout<<"TCP: Accepted File Descriptor"<<endl;
				}
				
				//SEND ESTABLISH COMMUNICATION MESSAGE
				Result = write(TCPSocketFileDescriptor2, (void*)TCPConnect,6+Username.length());
				if(0 > Result){ 
					perror("ERROR writing to socket");
				}
				cout<<"TCP: Sent Establish Communication Message"<<endl;				
			}
			for(int pollindex = 2; pollindex < fds.size(); pollindex++){
				//cout<<"poops"<<endl;
				if(fds.at(pollindex).revents & POLLIN) {
					//cout<<"POLL FILE DESCRIPTOR: "<< pollindex<<endl;
					char signature[4];
					uint16_t type;
					//READ SIGNATURE
					Result = read(fds.at(pollindex).fd, signature,4);
					if(0 > Result){ 
						perror("Error Reading Packet Signature");
					}
					//cout<<"TCP: Packet Signature Recieved : "<<signature <<endl;
					//READ TYPE
					Result = read(fds.at(pollindex).fd, &type,2);
					if(0 > Result){ 
						perror("Error Reading Packet Type");
					}
					type = ntohs(type);
					//cout<<"TCP: Packet Type Recieved : "<<type<<endl;
					//IF TYPE 4
					if (type ==4){
						char username[255];
						//READ IN USER
						Result = read(fds.at(pollindex).fd, username,255);
						if(0 > Result){ 
							perror("Error Reading Packet Signature");
						}
						//cout<<"TCP: You are about to be connected to "<<username<<endl;
						 
						Result = write(TCPSocketFileDescriptor2, (void*)TCPAcceptPacket,6);
						if(0 > Result){ 
							perror("ERROR writing to socket");
						}		
					}
					if (type ==5 ){
						cout << "TCP: accepted your connection " << endl;
						cout << UserList[pollindex-2].Username<<"@"<< UserList[pollindex-2].Hostname<< " accepted your connection " << endl;
					}
					if (type ==6){
						cout << UserList[pollindex-2].Username<<"@"<< UserList[pollindex-2].Hostname<< "TCP:  Sent User Unavaliable message"<<endl;
					}
					if (type == 7){
						//cout<<"preparing type 7 packet"<<endl;
						Result = write(fds.at(pollindex).fd, "P2PI",4);
						if(0 > Result){ 
							perror("ERROR writing to socket");
						}
						uint16_t temp = htons(0x0008);
						Result = write(fds.at(pollindex).fd, &temp ,2);
						if(0 > Result){ 
							perror("ERROR writing to socket");
						}
						uint32_t userlistsize = htonl(UserList.size());
						Result = write(fds.at(pollindex).fd, &userlistsize ,4);
						if(0 > Result){ 
							perror("ERROR writing to socket");
						}
						uint16_t temp16;
						uint32_t temp32;
						for(int k = 0; k < UserList.size(); k++){
							temp32 = htonl(k);
							Result = write(fds.at(pollindex).fd, &temp32 ,4);
							temp16 =  htons(UserList[k].UdpPort);
							Result = write(fds.at(pollindex).fd, &temp16 ,2);
							Result = write(fds.at(pollindex).fd, UserList[k].Hostname.c_str(), UserList[k].Hostname.length()+1);
							temp = htons(UserList[k].TcpPort);
							Result = write(fds.at(pollindex).fd, &temp ,2);
							Result = write(fds.at(pollindex).fd, UserList[k].Username.c_str(), UserList[k].Username.length()+1);
							
						
						}
						
				//	strlen(Broadcast->data);
					}
					if (type ==9){
						cout<< UserList[pollindex-2].Username <<"@"<< UserList[pollindex-2].Hostname<< "  Sent a message!"<<endl;
						bzero(Buffer, BUFFER_SIZE);
						Result = read(fds.at(pollindex).fd, Buffer,255);
						cout <<UserList[pollindex-2].Username<<": "<<Buffer<<endl<<endl;
						bzero(Buffer, BUFFER_SIZE);
						cout <<"Reply: "<<endl;
						cin>> Buffer;
						Packet * MessagePacket = new Packet;
						memset(MessagePacket->data,'\0',246);
						strncpy(MessagePacket->signature, "P2PI", 4);
						MessagePacket->type = htons(0x0009);
						strcpy(MessagePacket->data, Buffer);
						Result = write(fds.at(pollindex).fd, (void*)MessagePacket, 6+strlen(Buffer));
							
						
						
					}
					
					
				}
			}
		}
		if((UserList.size() ==0)){
			if( UDPTimeout < UDPTimeoutMax){
				UDPTimeout=UDPTimeout*2;
			}
		}
		//int inputx;
		//cout<<"Menu:"<<endl<<"1. Send a Message."<<endl<<"2. Do Nothing"<<endl;
		//cin>>inputx>>endl;
		
	}
	close(TCPSocketFileDescriptor);
	close(SocketFileDescriptor);
	//cout<<(char *)Broadcast<<endl;
	
	return 0;
}



//FUNCTION TO PRINT ENTRY
//cout<<UserList[k].UdpPort<<endl<<UserList[k].Hostname<<endl<<UserList[k].TcpPort<<endl<<UserList[k].Username<<endl;










