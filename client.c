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

int main(int argc, char* argv[]) {

	int port;
	int clientSocket;
	int connectStatus;
	int readCount, writeCount;
	char buffer[512], *bufferPtr;
	struct sockaddr_in serverAddr;
	int qrScanFile, htmlPageFile;
	char htmlPage[] = "page.html";

	system("clear");	

	if (argc != 4) {
		fprintf(stderr, "Usage: %s <Server IP> <Server Port> <QR Scan Filename>\n", argv[0]);
		exit(1);
	}

	port = atoi(argv[2]);

	printf("QR Code Client\n");

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (clientSocket == -1)	{
		fprintf(stderr, "ERROR: Could not create socket\n");
		exit(1);
	}

	printf("INFO: Client started\n");
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
	serverAddr.sin_port = htons(port);
	connectStatus = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

	if (connectStatus == -1)	{
		fprintf(stderr, "ERROR: Could not connect to Server\n");
		exit(1);
	}

	printf("INFO: Connected to server\n");
	

	qrScanFile = open(argv[3], O_RDONLY);

	if(qrScanFile == -1) {
		fprintf(stderr, "ERROR: File access error - qrScan.png\n");
		exit(1);
	}

	while((readCount = read(qrScanFile, buffer, 512)) > 0)
	{
		writeCount = 0;
		bufferPtr = buffer;
		while (writeCount < readCount)
		{
			readCount -= writeCount;
			bufferPtr += writeCount;
			writeCount = write(clientSocket, bufferPtr, readCount);
			if (writeCount == -1) {
				fprintf(stderr, "ERROR: Could not write file to Server\n");
				exit(1);
			}
		}
	}
	
	close(qrScanFile);

	/* UN COMMENT
	htmlPageFile = open(htmlPage, O_WRONLY | O_CREAT, 0777);

	if(htmlPageFile == -1) {
		fprintf(stderr, "ERROR: File creation error - page.html\n");
		exit(1);
	}
	
	printf("Waiting");
	
	while ((readCount = read(clientSocket, buffer, 512)) > 0) {
		write(htmlPageFile, buffer, 512);
	}

	if(readCount == -1) {
		fprintf(stderr, "ERROR: Could not read from socket\n");
		exit(1);
	}

	close(htmlPageFile);
	*/
	
}

