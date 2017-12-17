#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/stat.h>
#include<stdint.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netdb.h>

#define PORT_NUM 59478

int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[1000];

	//Create socket
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		printf("ERROR opening socket");
		exit(0);
	}
	puts("Open socket!");

	//Initialize socket
	//server = gethostbyname("127.0.0.1");

	//bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = PF_INET;
	//bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
	//     server->h_length);
	serv_addr.sin_port = htons(PORT_NUM);
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	//Connect
	if(connect(sockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
		printf("ERROR connecting");
		exit(0);
	}
	puts("Connected!");

	while(1) {
		//print information
		printf("==============================================\n");
		printf("(a)list all process ids\n");
		printf("(b)thread'sIDs\n");
		printf("(c)child's PIDs\n");
		printf("(d)process name\n");
		printf("(e)state of process(D,R,S,T,t,W,X,Z\n");
		printf("(f)command line of excuting process(cmdline)\n");
		printf("(g)parent's PID\n");
		printf("(h)all ancients of PIDs\n");
		printf("(i)virtual memory size(VmSize)\n");
		printf("(j)physical memory size(VmRSS)\n");
		printf("(k)exit\n");
		printf("Which?\n");

		//input a~k
		char choice;
		//char choice[2];
		int choicePID = 0;
		bzero(buffer, sizeof(buffer));

		scanf("%c", &choice);
		//scanf("%s", choice);

		//(k)->exit
		/*		if(choice == "k") {
					printf("Exit\n");
					exit(1);
				} else if(choice == "a") {
					sprintf(buffer, "%s", choice);
				} else if(choice == "b" || choice == "l") {
					printf("pid?\n");
					scanf("%d",&choicePID);
					sprintf(buffer, "%s%d", choice, choicePID);
				} else {
					sprintf(buffer, " ");
				}
		*/		if(choice == 'k') {
			printf("Exit\n");
			exit(1);
		} else if(choice == 'a') {
			sprintf(buffer, "%c", choice);
		} else if(choice >= 'b' && choice <= 'l') {
			printf("pid?\n");
			scanf("%d",&choicePID);
			sprintf(buffer, "%c%d", choice, choicePID);
		} else {
			sprintf(buffer, " ");
		}
		send(sockfd, buffer, sizeof(buffer), 0);
		puts("Write to socket");

		recv(sockfd, buffer, sizeof(buffer), 0);
		printf("%s\n", buffer);
	}
	close(sockfd);

	return 0;
}

