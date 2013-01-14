#include <stdio.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <stdlib.h> 	
#include <string.h> 	
#include <unistd.h> 	
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>

#define USERAGENT "HTMLGET 1.1"


struct httpParams {
	char *host;
	char *path;
	char *getQuery;
};

int initializeServer(int port);
int getRequest(int serverSocket);
void readFromClient(int clientSocket);
struct httpParams prepareGetQuery();
void sendToClient(int clientSocket, struct httpParams httpParam);
int connect_to(char *, char *);
void *get_addr(struct sockaddr *);

int main(int argc, char* argv[]) {

	int port;
	int serverSocket, clientSocket;
	struct httpParams httpParam;
	
	system("clear");	

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
		exit(1);
	}

	port = atoi(argv[1]);

	serverSocket = initializeServer(port);	

	while(1) {
	
		clientSocket = getRequest(serverSocket);
		readFromClient(clientSocket);
		httpParam = prepareGetQuery();
		sendToClient(clientSocket, httpParam);
		
		exit(1);

	}
}

int initializeServer(int port) {

	int bindStatus, listenStatus;
	int serverSocket;
	struct sockaddr_in serverAddr;

	printf("QR Code Server\n");

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (serverSocket == -1)	{
		fprintf(stderr, "ERROR: Could not create socket\n");
		exit(1);
	}

	printf("INFO: Server started\n");
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);
	bindStatus = bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));


	if (bindStatus == -1)	{
		fprintf(stderr, "ERROR: Could not bind to socket\n");
		exit(1);
	}

	listenStatus = listen(serverSocket, 5);

	if (listenStatus == -1)	{
		fprintf(stderr, "ERROR: Could not listen to socket\n");
		exit(1);
	}

	return serverSocket;

}

int getRequest(int serverSocket) {

	int clientSocket;
	int clientLen;

	struct sockaddr_in clientAddr;

	clientLen = sizeof(clientAddr);
	clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &clientLen);

	if(clientSocket == -1) {
		fprintf(stderr, "ERROR: Could not accept connection\n");
		exit(1);	
	}

	printf("INFO: Client connected - %s\n", inet_ntoa(clientAddr.sin_addr));

	return clientSocket;	
	
}

void readFromClient(int clientSocket) {
	
	char buffer[512];	
	char qrScan[] = "qrScan.png";
	int qrScanFile;
	int readCount;

	qrScanFile = open(qrScan, O_WRONLY | O_CREAT, 0777);

	if(qrScanFile == -1) {
		fprintf(stderr, "ERROR: File creation error - qrScan.png\n");
		exit(1);
	}

	while ((readCount = read(clientSocket, buffer, 512)) > 0) {
		write(qrScanFile, buffer, 512);
	}

	if(readCount == -1) {
		fprintf(stderr, "ERROR: Could not read from socket\n");
		exit(1);
	}

	close(qrScanFile);
	
}


struct httpParams prepareGetQuery() {
	
	char *host, *path, *getQuery;
	char lookupTextHttp[] = "http";
	char lookupTextSlash[] = "/";
	char *location, *url;
   	char buffer[512];
	int urlLength, hostLength, pathLength = 0;
        char *getQueryTmpl = "GET /%s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\nAccept-language:en\r\n\r\n";
  	FILE *qrContent;
	struct httpParams httpParam;

	system("java -cp javase.jar:core.jar com.google.zxing.client.j2se.CommandLineRunner qrScan.png > qrContent.txt");
	qrContent = fopen ("qrContent.txt", "rt") ;
   	
	if(qrContent == NULL) {
	        puts("ERROR: File open error - qrContent.txt") ;
		exit(1);
      	}

   	while((fgets(buffer, 512, qrContent) != NULL)) {
		location = strstr(buffer, lookupTextHttp);
		if(location!=NULL) {
			break;
		}
	}
	
	url = (char *)malloc(strlen(location) * sizeof(char));
	sscanf(location,"http://%s", url);
	urlLength = strlen(url);
	
	fclose(qrContent);

	path = strstr(url, lookupTextSlash);
	
	if(path!=NULL) {
		pathLength = strlen(path);
	}
	
	hostLength = urlLength - pathLength;
	host = 	(char *)malloc(hostLength * sizeof(char));

  	strncpy(host, url, hostLength);
	
	printf("INFO: Host is %s\n", host);
	printf("INFO: Path is %s\n", path);
	
	getQuery = (char *)malloc(hostLength+pathLength+strlen(USERAGENT)+strlen(getQueryTmpl));
	
	if(path!=NULL) {
		sprintf(getQuery, getQueryTmpl, path+1, host, USERAGENT);	
	}
	else {
		sprintf(getQuery, getQueryTmpl, "", host, USERAGENT);		
	}
	
	httpParam.host = host;
	httpParam.path = path;
	httpParam.getQuery = getQuery;
	
	return httpParam;
}

void sendToClient(int clientSocket, struct httpParams httpParam) {
	
	int serverSocket;
//	int connectStatus;
	char buffer[512];
	int bytesRead, totalBytesSent, bytesSent;
	int sent = 0;
	int temp;

	//1. Get Server Socket and send the HTTP request
	serverSocket = connect_to(httpParam.host,"80");;

	printf("sock_fd:%d\n",serverSocket);
	while(sent < strlen(httpParam.getQuery))
	{ 
		temp= send(serverSocket, httpParam.getQuery+sent, strlen(httpParam.getQuery)-sent, 0);
		if(temp == -1)
		{
      			perror("Can't send query");
      			exit(1);
    		}
   		sent += temp;
  	}

	printf("bytes sent: %d\n",sent);


	do {
        	bzero(buffer, sizeof(buffer));
       		bytesRead = recv(serverSocket, buffer, sizeof(buffer), 0);
        	if(bytesRead>0) {
			totalBytesSent = 0;
			while (totalBytesSent < strlen(buffer)) {
				bytesSent = send(clientSocket, buffer+totalBytesSent, strlen(buffer)-totalBytesSent, 0);
				// if (temp_sent == -1) return -1;
				totalBytesSent += bytesSent;
			}
		}
    }
    while (bytesRead>0);
}


int connect_to(char * host, char * port) 
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	printf("port:%s\n",port);
	if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return -1;
	}
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
		p->ai_protocol)) == -1) 
		{
			perror("client: socket");
			continue;
		}
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("client: connect");
		continue;
			}
		break;
	}
	if (p == NULL) 
	{
		fprintf(stderr, "client: failed to connect\n");
		return -1;
	}

	inet_ntop(p->ai_family, get_addr((struct sockaddr *)p->ai_addr),	s, sizeof s);
	fprintf(stdout, "client: connecting to %s\n", s);
	freeaddrinfo(servinfo); 
	return sockfd;
}

void *get_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
