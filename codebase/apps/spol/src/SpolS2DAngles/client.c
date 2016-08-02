/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
//  client.c - UDP client 
// Last modify dec 7 2010 by paloma@ucar.edu
// Last modify dec 9 2010 by paloma@ucar.edu: change to accept broad cast. 

/*
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


#include "packetize.h"
 
#define PORT 2080
#define DEST_ADDR "192.168.0.50" //Server addr
*/



#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
 
#define PORT 12080
#define DEST_ADDR "255.255.255.255" //broadcast addr
 
int main(int argc, char *argv[])
{
        int sockfd;
        char buf[128];
        struct sockaddr_in sendaddr;
        struct sockaddr_in recvaddr;
        int numbytes;
        int addr_len;
        int broadcast=1;
	int     n;

	// open file for test 
	FILE *p = NULL; 
	const char *file = "udpdec13.txt"; 
	size_t len = 0; 
	p = fopen(file, "w");

	if (p == NULL){
	printf("Error opening a file: %s\n", file); 
	} 
 
        if((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
        {
                perror("socket");
                exit(1);
        }
       if((setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,
                                &broadcast,sizeof broadcast)) == -1)
        {
                perror("setsockopt - SO_SOCKET ");
                exit(1);
        }
 
        printf("Socket created\n");
 
        sendaddr.sin_family = AF_INET;
        sendaddr.sin_port = htons(PORT);
        sendaddr.sin_addr.s_addr = INADDR_ANY;
        memset(sendaddr.sin_zero,'\0', sizeof sendaddr.sin_zero);
 
 
        recvaddr.sin_family = AF_INET;
        recvaddr.sin_port = htons(PORT);
        recvaddr.sin_addr.s_addr = INADDR_ANY;
        memset(recvaddr.sin_zero,'\0',sizeof recvaddr.sin_zero);
        if(bind(sockfd, (struct sockaddr*) &recvaddr, sizeof recvaddr) == -1)
        {
                perror("bind");
                exit(1);
        }
 
addr_len = sizeof sendaddr;

 if ((numbytes = recvfrom(sockfd, buf, sizeof buf, 0,
              	(struct sockaddr *)&sendaddr, (socklen_t *)&addr_len)) == -1)
        	{
       			perror("recvfrom");
      			exit(1);
   		}		
		
        
	while (1)
	{
        	/*
		if ((numbytes = recvfrom(sockfd, buf, sizeof buf, 0,
              	(struct sockaddr *)&sendaddr, (socklen_t *)&addr_len)) == -1)
        	{
       			perror("recvfrom");
      			exit(1);
   		}		
		*/
          n=read(sockfd, buf, sizeof(buf));
		len = strlen (buf);
		fwrite (buf, len,1,p); 	
	}
	


        //printf("%s",buf);
 	fclose (p); 
        return 0;
}

 /*
int main(int argc, char *argv[])
{
	int sockfd;
	char buf[10];
	struct hostent *he;
	struct sockaddr_in sendaddr;
	//struct sockaddr_in recvaddr;
	int numbytes;
	int addr_len;
	
	if((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}
	
	printf("Socket created\n");
	
	if ((he=gethostbyname(DEST_ADDR)) == NULL) 
	{   // get the host info
	   herror("gethostbyname");
   	exit(1);
   	}
 
	printf("Host found\n");
 
	sendaddr.sin_family = AF_INET;
	sendaddr.sin_port = PORT;
	sendaddr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(sendaddr.sin_zero,'\0', sizeof sendaddr.sin_zero);
	
	addr_len = sizeof sendaddr;
	if ((numbytes = recvfrom(sockfd, buf, sizeof buf, 0,
              (struct sockaddr *)&sendaddr, (socklen_t *)&addr_len)) == -1) 
	{
   	perror("recvfrom");
      exit(1);
   }
	
	printf("%s",buf);
	
	return 0;
}

*/



//// Old code 
/*
main(argc, argv)
     int     argc;
     char    *argv[];
{ 
  struct  sockaddr_in sad; // structure to hold an IP address     
  int     clientSocket;    // socket descriptor                    
  struct  hostent  *ptrh;  // pointer to a host table entry       
  
  char    *host;           // pointer to host name                
  int     port;            // protocol port number                  
  
  char    Sentence[128]; 
  char    modifiedSentence[128]; 
  char    buff[128];
  int     n;
  
  if (argc != 3) {
    fprintf(stderr,"Usage: %s server-name port-number\n",argv[0]);
    exit(1);
  }
  
  // Extract host-name from command-line argument 
  host = argv[1];         // if host argument specified   
  
  // Extract port number  from command-line argument 
  port = atoi(argv[2]);   // convert to binary            
  
  // Create a socket. 
  
  clientSocket = socket(PF_INET, SOCK_DGRAM, 0);
  if (clientSocket < 0) {
    fprintf(stderr, "socket creation failed\n");
    exit(1);
  }
 
  // determine the server's address 
 
  memset((char *)&sad,0,sizeof(sad)); // clear sockaddr structure 
  sad.sin_family = AF_INET;           // set family to Internet     
  sad.sin_port = htons((u_short)port);
  ptrh = gethostbyname(host); // Convert host name to equivalent IP address and copy to sad. 
  if ( ((char *)ptrh) == NULL ) {
    fprintf(stderr,"invalid host: %s\n", host);
    exit(1);
  }
  memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);
  
  // Read a sentence from user 
  
  // printf("Sentence   :   "); gets(Sentence);
  
  // Send the sentence to the server  
 Sentence ['a'];  
  n=sendto(clientSocket, Sentence, strlen(Sentence)+1,0 ,
 	   (struct sockaddr *) &sad, sizeof(struct sockaddr));
  
  printf(" Client sent %d bytes to the server\n", n);
  
  // Get the modified sentence from the server and write it to the screen
  modifiedSentence[0]='\0';
  n=read(clientSocket, buff, sizeof(buff));
  while(n > 0){
    strncat(modifiedSentence,buff,n);
    if (buff[n-1]=='\0') break;
    n=read(clientSocket, buff, sizeof(buff));
  }
  
  printf(" %s\n",modifiedSentence);
  
  // Close the socket. 
  
  close(clientSocket);
  
}
*/

