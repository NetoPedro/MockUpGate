#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>


#define TEMPORARY_SIZE 256
#define PORT "19999"

 struct message {
	unsigned char version; 
	unsigned char sub_version; 
	unsigned char data_length[2]; 
	unsigned char message_type; 
	unsigned char encryption_type; 
	unsigned char timestamp [4]; 
	char data_content[TEMPORARY_SIZE] ; 
	};


int toString(char a[]) {
  int c, sign, offset, n;
 
  if (a[0] == '-') {  // Handle negative integers
    sign = -1;
  }
 
  if (sign == -1) {  // Set starting position to convert
    offset = 1;
  }
  else {
    offset = 0;
  }
 
  n = 0;
 
  for (c = offset; a[c] != '\0'; c++) {
    n = n * 10 + a[c] - '0';
  }
 
  if (sign == -1) {
    n = -n;
  }
 
  return n;
}

int establish_new_connection(char * dns){
	//Mudar metodo . Implementar segundo PL15
		int err, sock;
		unsigned long f, i, n, num;
		unsigned char bt;
		struct addrinfo  req, *list;

		try_again : ;
		
		
		printf("Trying to connect\n") ;
		bzero((char *)&req,sizeof(req));
		// let getaddrinfo set the family depending on the supplied server address
		req.ai_family = AF_UNSPEC;
		req.ai_socktype = SOCK_STREAM;
		err=getaddrinfo(dns, PORT , &req, &list);
		if(err) {
				printf("Failed to get server address, error: %s\n",gai_strerror(err)); exit(1); }
		sock=socket(list->ai_family,list->ai_socktype,list->ai_protocol);
		if(sock==-1) {
				perror("Failed to open socket\n"); freeaddrinfo(list); exit(1);}
		if(connect(sock,(struct sockaddr *)list->ai_addr, list->ai_addrlen)==-1) { perror("Failed connect"); freeaddrinfo(list); close(sock); sleep(15);; goto try_again;}
		 
		printf("Connected\n"); 
		return sock;
	}
	
void send_type1_message(int fd,  char *  numberDoors,  char * idGate){
	struct message message_type1 ;
	message_type1.version  = '1';
	message_type1.sub_version = '0';
	message_type1.message_type = '1';
	message_type1.encryption_type = '0';
	long times = (unsigned) time(NULL);
	int k = 0;
	for(; k< 4 ; k++){
		message_type1.timestamp[k] = times%256; 
		times = times/256; 
		}
	message_type1.data_content[1] = atoi(idGate); 
	message_type1.data_content[0] = atoi(numberDoors); 
	
	short length = strlen(message_type1.data_content);
	short length2 = length +10;
	for(k= 0; k< 2 ; k++){
		message_type1.data_length[k] = length%256; 
		length = length/256; 
		}
	write(fd,&message_type1,length2);
	printf("Mensagem enviada Tipo 1\n");
	
	
	}
	
void read_type2_message(int fd){
	struct message message_type2;
	read(fd, &message_type2,10);
	printf("Mensagem Lida Tipo 2\n");
	
	}
void send_type3_message(int fd, unsigned char * doors, int size){
	struct message message_type3 ;
	message_type3.version  = '1';
	message_type3.sub_version = '0';
	message_type3.message_type = '3';
	message_type3.encryption_type = '0'; 
	int times = (unsigned) time(NULL);
	int k = 0;
	for(; k< 4 ; k++){
		message_type3.timestamp[k] = times%256; 
		times = times/256; 
		}  
	char string[TEMPORARY_SIZE-1] ; 
	int j = 0; 
	strcpy(message_type3.data_content,doors); 
	for(;j<size;j++){
			message_type3.data_content[j] = doors[j]; 
	}
	short length = size;
	short length2 = size +10;
	for(k = 0; k< 2 ; k++){
		message_type3.data_length[k] = length%256; 
		length = length/256; 
		}
	write(fd,&message_type3,length2);
	printf("Mensagem enviada Tipo 3\n");
	}

void initializeDoors(unsigned char * doors,int size){
	printf("%d\n",size); 
	int i = 0; 
	for(;i<size ;i++){
			doors[i] = 0;
		}
    doors[size] = 0;
	}

int main(int argc , char** argv){
	//Introduzir tamanho vindo por parametro
	if(argc != 4){
		printf("Insert IPv4/IPv6/DNS + GateID + DoorsNumber as arguments");
		exit(0);
		}
	
	fd_set rfds, rfds_master;
	int size =  toString(*(argv+3));
	unsigned char doors[size+1];
	initializeDoors(doors, size);
    printf("%s\n",doors);
    printf("%s\n",*(argv + 2));
    
	while(1){
	broken_connection: ;
	int fd  = establish_new_connection(argv[1]);
	send_type1_message(fd, argv[3], argv[2]);
	        printf("OI\n");
	int connected = 1;
		while(connected){

			//Check if the timeout works
			struct timeval tv ; 
			tv.tv_sec = 60; 
			tv.tv_usec = 0; 
			FD_ZERO(&rfds);
			FD_SET(0,&rfds); FD_SET(fd,&rfds);
			//sleep(45);
			int retVal = select(fd+1,&rfds,NULL,NULL,&tv);
			
			if(FD_ISSET(0,&rfds)) {

				close(fd);
				printf("Desconectado\n");
				sleep(15);
				goto broken_connection;
			}
			 
			read_type2_message(fd);
		
			send_type3_message(fd, doors,size);
		}
	}
	
	return 0; 
	
	}
