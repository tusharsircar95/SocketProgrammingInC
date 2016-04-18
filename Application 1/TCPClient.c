#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<time.h>
#include<limits.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<errno.h>

char crcKey[20] = "100000111";
float BER;

void appendRemainder(char input[],char key[])
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
	{
		// printf("%c",rem[i]);
		input[msglen+i] = rem[i];
	}
	input[msglen+keylen-1] = '\0';
 }
 
 void convertToBinary(char msg[],char modifiedMsg[])
 {
 	int i1,i2=0,j;
 	int l = strlen(msg);
 	for(i1=0; i1<l; i1++)
 	{
 		int c = (int)msg[i1];
 		for(j=7; j>=0; j--)
 		{
 			if(c & 1)
 				modifiedMsg[i2+j] = '1';
 			else modifiedMsg[i2+j] = '0';
 			c = c>>1;
 		}
 		i2 = i2 + 8;
 	}
 	modifiedMsg[i2] = '\0';
 }

void messageTransform(char msg[],char modifiedMsg[])
{
	convertToBinary(msg,modifiedMsg);
	//printf("Binary form: %s\n",modifiedMsg);
	appendRemainder(modifiedMsg,crcKey);
	//adding errors
	int i,l;
	float rnd;
	for(i=0,l=strlen(modifiedMsg); i<l; i++)
	{
		rnd = (float)((float)rand()/RAND_MAX);
		if(rnd <= BER)
		{
			if(modifiedMsg[i] == '0')
				modifiedMsg[i] = '1';
			else modifiedMsg[i] = '0';
		}
	}
	//printf("Tampered form: %s\n",modifiedMsg);
	//printf("\n");
	
}

int main(int argc,char *argv[])
{

	int clientSocket,serverSocket;
	struct sockaddr_in client,server;
	
	if(argc != 3)
	{
		printf("Usage ./a.out ip_address port");
		return -1;
	}
	
	printf("Enter BER (probability of bit errors): ");
	scanf("%f",&BER);
	time_t tt;
 	srand((unsigned) time(&tt));
 	
	
	if((clientSocket = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("socket: ");
		return -1;
	}
	
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	server.sin_addr.s_addr = inet_addr(argv[1]);
	bzero(&server.sin_zero,8);
	
	int len = sizeof(struct sockaddr_in);
	struct timeval tv;
	tv.tv_sec = 5;  /* 5 Secs Timeout */
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors

	setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	
	
	//connect to the server
	if((connect(clientSocket,(struct sockaddr*)&server,len)) == -1)
	{
		perror("connecting: ");
		return -1;
	}
		
	int noOfMessages;
	int i = 0;
	char messages[50][200];
	char modifiedMsg[10000];
	char reply[5];
	
	while(1)
	{
		printf("Enter number of messages you wish to send: ");
		scanf("%d",&noOfMessages);
		getchar();
		
		for(i=0; i<noOfMessages; i++)
			gets(messages[i]);	
		
		for(i=0; i<noOfMessages; i++)
		{
			printf("\n");
			messageTransform(messages[i],modifiedMsg);
			int sentLen = send(clientSocket,modifiedMsg,strlen(modifiedMsg),0);
			
			printf("Sent message %d/%d. Waiting for ACK/NACK...\n",i+1,noOfMessages);
			int recvLen = recv(clientSocket,reply,5,0);
			if(recvLen == -1)
			{
				printf("Timeout. Re-transmitting...\n");
				i--;
				continue;
				//break;
			}
			//printf("RL: %d\n",recvLen);
			reply[recvLen] = '\0';
			printf("Reply received: %s\n",reply);
			if(strcmp(reply,"NACK") == 0)
			{
				printf("Message had some error. Re-transmitting...\n");
				i--;
			}
		}
		close(clientSocket);
		close(serverSocket);
		break;
	}
	



	return 0;
}
