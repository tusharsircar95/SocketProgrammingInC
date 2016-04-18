#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<time.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include <signal.h>

char crcKey[20] = "100000111";
float dropProbability;

int serverSocket,clientSocket;

signal_callback_handler(int signum)

{

   printf("Caught signal %d. Releasing resources...\n",signum);

   close(clientSocket);
   close(serverSocket);
   
   // Terminate program

   exit(signum);

}

int getPower(int base,int power)
{
	if(power == 0)
		return 1;
	if(power%2 == 0)
	{
		int temp = getPower(base,power/2);
		return temp*temp;
	}
	
	return base*getPower(base,power-1);
}

int checkCRC(char input[],char key[])
 {
 	char temp[30],quot[100],rem[30],key1[30];
 	int keylen=strlen(key);
 	int msglen=strlen(input);
 	int i,j;
 	strcpy(key1,key);
 	for(i=0;i<keylen-1;i++)
 		input[msglen+i]='0';
	
 	for(i=0;i<keylen;i++)
 		temp[i]=input[i];
 	for(i=0;i<msglen;i++)
	{
 		quot[i]=temp[0];
 		if(quot[i]=='0')
 			for(j=0;j<keylen;j++)
 				key[j]='0';
 		else
 		for(j=0;j<keylen;j++)
 			key[j]=key1[j];
 		
 		for(j=keylen-1;j>0;j--)
 		{
	 		if(temp[j]==key[j])
			 	rem[j-1]='0';
 			else
			 rem[j-1]='1';
 		}
 		rem[keylen-1]=input[i+keylen];
	 	strcpy(temp,rem);
 	}
 	strcpy(rem,temp);
	/*printf("\nQuotient is ");
	for(i=0;i<msglen;i++)
		printf("%c",quot[i]);
	printf("\nRemainder is ");
	for(i=0;i<keylen-1;i++)
	 	printf("%c",rem[i]);
	 	printf("\nFinal remainder is: ");
	//for(i=0;i<msglen;i++)
	// 	printf("%c",input[i]);*/
	for(i=0;i<keylen-1;i++)
		if(rem[i] != '0')
			return 0;
	return 1;
 }

void doProcessing(int clientSocket)
{
	int data_len = 1;
	int t;
	char msg[1000];
	while(data_len)
	{
		data_len = read(clientSocket,msg,1000);
		msg[data_len] = '\0';
		//printf("Message recieved in binary: %s\n",msg);
		
		float rnd = (float)((float)rand()/RAND_MAX);
		
		if(data_len && checkCRC(msg,crcKey))
		{
			
			char ack[5] = "ACK";
				
			printf("Message received (without errors): ");
			for(t=0; t<data_len-8; t++)
			{
				int j;
				int v = 0;
				for(j=t; j<t+8; j++)
				{
					if(msg[j] == '1')
						v += getPower(2,t+7-j);
				}
				printf("%c",(char)v);
				//printf(" %d ",v);
				t = t + 7;
			}
			printf("\n");
				
			//printf("Enter any number to send ACK: ");
			//scanf("%d",&t);
			printf("Sending ACK....");
			if(rnd < dropProbability)
			{
				printf("Oops! Packet dropped!\n\n");
				continue;
			}
			printf("Sent successfully!\n\n");
			int sent = send(clientSocket,ack,strlen(ack),0);
			//printf("sent %d bytes to client at %s!\n",sent,inet_ntoa(client.sin_addr));
		}
		else if(data_len)
		{
			printf("Message retrieved had some errors...\n");
			char nack[5] = "NACK";
			//printf("Enter any number to send NACK: ");
			//scanf("%d",&t);
			printf("Sending NACK....");
			if(rnd < dropProbability)
			{
				printf("Oops! Packet dropped!\n\n");
				continue;
			}
			printf("Sent successfully!\n\n");
			int sent = send(clientSocket,nack,strlen(nack),0);
		}
		else close(clientSocket);
	}
}


int main(int argc,char *argv[])
{
	 signal(SIGINT, signal_callback_handler);	
	
	int pid;
	struct sockaddr_in server,client;
	
	if(argc != 2)
	{
		printf("Useage: ./a.out port\n");
		return -1;
	}
	
	printf("Enter probability to drop packets: ");
	scanf("%f",&dropProbability);
	time_t tt;
 	srand((unsigned) time(&tt));
	
	if((serverSocket = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("socket: ");
		return -1;
	}
	
	printf("Created socket...\n");
	
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[1]));
	server.sin_addr.s_addr = INADDR_ANY;
	bzero(&server.sin_zero,8);
	
	unsigned int len = sizeof(struct sockaddr_in);	
	
	if((bind(serverSocket,(struct sockaddr*)&server,len) == -1))
	{
		perror("binding: ");
		return -1;
	}
	
	printf("Done with binding...\n");
	
	if((listen(serverSocket,5)) == -1)
	{
		perror("listening: ");
		return -1;
	}
	
	printf("Listening...\n");
	
	while(1)
	{
		if((clientSocket = accept(serverSocket,(struct sockaddr*)&client,&len)) == -1)
		{
			perror("accepting: ");
			return -1;
		}
		
		printf("Accepted...\n");
		//printf("New client connected at port %d and clientIP: %s\n",ntohs(client.sin_port),inet_ntoa(client.sin_addr));
		
		if((pid = fork()) < 0)
		{
			perror("forking: ");
			return -1;
		}
		
		
		if(pid == 0)
		{
			close(serverSocket);
			doProcessing(clientSocket);	
			printf("Done...You can terminate the server now!\n");	
			return 0;
		}
		else
			close(clientSocket);		
	}


	return 0;
}
