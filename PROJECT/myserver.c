/*
   A simple server in the internet domain using TCP
   Usage:./server port (E.g. ./server 10000 )
*/
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/wait.h>

void buffer_cutting(char *arr[1024],char *buffer, char *cut_point){
	int i = 0;

	char *ptr = strtok(buffer, cut_point);
	while(ptr != NULL){
		arr[i++] = ptr;
		ptr = strtok(NULL, cut_point);
	}
	arr[i] = NULL;

	return;
}

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
   int sockfd, newsockfd; //descriptors rturn from socket and accept system calls
   int portno; // port number
   socklen_t clilen;

   char buffer[1024];

   /* sockaddr_in: Structure Containing an Internet Address */
   struct sockaddr_in serv_addr, cli_addr;

   int n;
   if (argc < 2) {
      fprintf(stderr,"ERROR : no port provided\n");
      exit(1);
   }

   /*
      Create a new socket
      AF_INET: Address Domain is Internet
      SOCK_STREAM: Socket Type is STREAM Socket
   */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0)
      error("ERROR : opening socket");

   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = atoi(argv[1]); //atoi converts from String to Integer
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY; //for the server the IP address is always the address that the server is running on
   serv_addr.sin_port = htons(portno); //convert from host to network byte order

   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //Bind the socket to the server address
            error("ERROR : on binding");

   listen(sockfd,5); // Listen for socket connections. Backlog queue (connections to wait) is 5

   clilen = sizeof(cli_addr);
   /*accept function:
      1) Block until a new connection is established
      2) the new socket descriptor will be used for subsequent communication with the newly connected client.
   */
   while(1){
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
      if (newsockfd < 0)
         error("ERROR : on accept");

      bzero(buffer, 1024);

      n = read(newsockfd, buffer, 1024);
      if (n < 0) error("ERROR : reading from socket");

      printf("Here is the message: \n%s\n",buffer);

      char *arr[1024];
      buffer_cutting(arr,buffer," ");

      char filename_and_filetype[30];
      if (!strcmp(arr[0], "GET") || !strcmp(arr[0], "POST")){	// GET or POST only.
    	  strcpy(filename_and_filetype, arr[1]+1);		// We will use filename and type only.
      } else{
    	  continue;
      }

      FILE *myfile = fopen(filename_and_filetype, "r");			// Open file.
      if (myfile == NULL)											// Error check
    	  error("ERROR : open file");

      char *filename = strtok(filename_and_filetype, ".");		// index.html's 'index'.
      char *filetype = strtok(NULL, "\n");						// index.html's 'html'.

      size_t myfile_size;											// File size check
      fseek(myfile, 0, SEEK_END);									//
      myfile_size = ftell(myfile);									//
      fseek(myfile, 0, SEEK_SET);									//

      char *file_buf = (char *)malloc(myfile_size + 1);			// Declare file buffer.
      n = fread(file_buf, myfile_size, 1, myfile);				// Read file to file buffer.

      char *response = (char *)malloc(myfile_size + 2048);		// Declare response message.

      char *content_type  = (char *)malloc(20);

      if(!strcmp(filetype, "html"))								// If file is 'html'.
    	  content_type = "text/html";
      else if (!strcmp(filetype, "jpeg") || !strcmp(filetype, "jpg")) // If file is 'jpeg' or 'jpg'.
    	  content_type = "image/jpeg";
      else if(!strcmp(filetype, "gif"))							// If file is 'gif'.
    	  content_type = "image/gif";
      else if(!strcmp(filetype, "pdf"))							// If file is 'pdf'.
    	  content_type = "application/pdf";
      else if(!strcmp(filetype, "mp3"))							// If file is 'mp3'.
    	  content_type = "audio/mp3";

      sprintf(response,												// Make response message.

            "HTTP/1.1 200 OK\r\n"
            "Content-Length: %ld\r\n"
            "Content-Type: %s\r\n"
    		  "\n"

            ,myfile_size, content_type);

      memcpy(response + strlen(response), file_buf, myfile_size);	// Merge response and file_buf.

      if(send(newsockfd, response, strlen(response) + myfile_size, 0) == -1) // Send message to socket.
         error("ERROR failed to send data to client");

      printf("END\n\n");
   }

   close(sockfd);
   close(newsockfd);

   return 0;
}
