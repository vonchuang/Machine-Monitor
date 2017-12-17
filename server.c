#define _GNU_SOURCE

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/types.h>
#include<ctype.h>
#include<netinet/tcp.h>
#include<dirent.h>

#define MAX_CLIENT 5
#define PORT_NUM 59487

pthread_t cthread[MAX_CLIENT];
pthread_t thread_id;
int client_index = 0;
struct sockaddr_in *client_addr[MAX_CLIENT];
int clientfd[MAX_CLIENT];
int sockfd;

void *connection_handler(void *anum);
void *run(void *cnum);
void choice_handler_A(int num);
void choice_handler(char choice, int num, char pid[]);
void find_ancient(char buffer[],char parent[]);

int main(int argc, char *argv[])
{

//	int c;
	struct sockaddr_in server;
	int *anum = malloc(sizeof(int));
	//Create socket
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd == -1) {
		printf("Could not create socket...");
	}
	puts("Socket created!");

	bzero(&server, sizeof(server));
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 59478 );

	//Bind
	if(bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		//Print the error message
		printf("bind failed. Error");
		return 1;
	}
	puts("bind done");

	//Listen
	listen(sockfd, MAX_CLIENT);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
//	c = sizeof(struct sockaddr_in);

	//pthread create
	if( pthread_create( &thread_id, NULL, connection_handler,
	                    (void*) &anum) < 0) {
		printf("Could not create thread...");
		return 1;
	}

	pthread_join(thread_id, 0);
	//puts("Handler assigned!");
//	}

	if(clientfd < 0) {
		printf("Accept failed...");
		return 1;
	}
	close(sockfd);

	return 0;
}


void *connection_handler(void *anum)
{
	int *cnum = malloc(sizeof(int));
	//Print Infor
	printf("Pthread %d Create!\n", client_index);

	while(client_index < (MAX_CLIENT-1)) {
		int addrlen = sizeof(client_addr[client_index]);

		//Accept
		clientfd[client_index] = accept(sockfd,
		                                (struct sockaddr *)&client_addr[client_index], &addrlen);
		puts("Connection accepted!");

		*cnum = client_index;
		if(pthread_create( &cthread[client_index], NULL, run,(void*) cnum) < 0) {
			printf("Could not create thread...\n");
			return;
		}

		//	pthread_join(thread_id, NULL);
		puts("Handler assigned!");
		client_index++;
	}
}

void *run(void *cnum)
{
	int num = *(int *)cnum;
	//char buffer[20];
	char choice;
	int read_size;
	//Get the socket descriptor
	char client_message[2000];
	bzero(client_message, sizeof(client_message));
	puts("run!");   //test

	//Receive a message from client
	while((read_size = recv(clientfd[num], client_message, sizeof(client_message),
	                        0)) ) {
		puts("recv!");
		choice = client_message[0];

		if(choice == 'a') {
			choice_handler_A(num);
		} else if(choice >= 'b' || choice <='j') {
			memmove(client_message, client_message+1, strlen(client_message));
			printf("PID: %s", client_message);
			choice_handler(choice, num, client_message);
		} else if(choice == 'k') {
			close(clientfd[num]);
			return;
		} else {
			;
		}
		read_size = 0;
		bzero(client_message, sizeof(client_message));
	}

	if(read_size == 0) {
		puts("Client disconnected...");
		fflush(stdout);
	} else if(read_size == -1) {
		printf("recv failed...");
	}

	return;
}

//(a)list all process ids
void choice_handler_A(int num)
{
	char buffer[2000];
	bzero(buffer, sizeof(buffer));

	DIR *dirp;
	struct dirent
		*dp; //ref: http://blog.csdn.net/zhuyi2654715/article/details/7605051
	if((dirp =  opendir("/proc")) != NULL) {
		while((dp = readdir(dirp)) != NULL) {
			if(dp->d_type == DT_DIR && isdigit(dp->d_name[0])) {
				//DT_DIR: This is a directory.
				//isdigit(dp->name[0]): Check if directory's name is decimal digit.
				//dirent->d_name: file name
				strcat(buffer,
				       dp->d_name);   //char *strcat(char *strDestination, const char *strSource
				strcat(buffer, "\t");
			}
		}
		closedir(dirp);
	}
	printf("%s", buffer);
	send(clientfd[num], buffer, sizeof(buffer), 0);
}

void choice_handler(char choice, int num, char pid[])
{
	char buffer[2000];
	bzero(buffer, sizeof(buffer));

	DIR *dirp;
	struct dirent *dp;

	if(choice == 'b') {         //(b)thread's IDs
		char dirName[50];
		sprintf(dirName, "/proc/%s/task", pid);
		strcpy(buffer, "[thread ID] ");
		if((dirp = opendir(dirName)) != NULL) {
			while((dp = readdir(dirp)) != NULL) {
				if((dp->d_type == DT_DIR) && isdigit(dp->d_name[0])) {
					strcat(buffer, dp->d_name);
					strcat(buffer, "\t");
				}
			}
			closedir(dirp);
		} else {
			printf("Process doesn't exit.\n");
			strcpy(buffer, "Process doesn't exit.");
		}
		printf("%s", buffer);
		send(clientfd[num], buffer, sizeof(buffer), 0);
	} else if(choice == 'c') {   //(c)child'sPIDs
		char dir_name[50];
		sprintf(dir_name, "/proc/%s/task/%s/children", pid, pid);
		FILE *fproc = fopen(dir_name, "r");
		char child[10];

		if(fproc) {
			sprintf(buffer, "[Child'sPIDs]");
			while(fscanf(fproc, "%s", child) == 1) {
				printf("%s\t", child);
				strcat(buffer, child);
				strcat(buffer, " ");
			}
		} else {
			printf("Process doesn't exit.\n");
			sprintf(buffer, "Process doesn't exit.");
		}
		printf("%s", buffer);
		send(clientfd[num], buffer, sizeof(buffer), 0);
	} else if(choice == 'd') {   //(d)process name
		char dir_name[50];
		sprintf(dir_name, "/proc/%s/status", pid);
		FILE *fproc = fopen(dir_name, "r");
		char search[100];

		if(fproc) {
			while(fgets(search, 6, fproc) != NULL) {
				printf("%s\n", search);
				if(strcmp(search,"Name:") == 0) {
					fgets(search, sizeof(search), fproc);
					sprintf(buffer, "[Process name]%s", search);
				}
			}
		} else {
			printf("Process doesn't exit.\n");
			sprintf(buffer, "Process doesn't exit.");
		}
		printf("%s", buffer);
		send(clientfd[num], buffer, sizeof(buffer), 0);
	} else if(choice == 'e') {   //(e)state of process(D,R,S,T,t,W,X,Z)
		char dir_name[50];
		sprintf(dir_name, "/proc/%s/status", pid);
		FILE *fproc = fopen(dir_name, "r");
		char search[100];

		if(fproc) {
			while(fgets(search, 7, fproc) != NULL) {
				printf("%s\n", search);
				if(strcmp(search,"State:") == 0) {
					fgets(search, sizeof(search), fproc);
					sprintf(buffer, "[State of process]%s", search);
				}
			}
		} else {
			printf("Process doesn't exit.\n");
			sprintf(buffer, "Process doesn't exit.");
		}
		printf("%s", buffer);
		send(clientfd[num], buffer, sizeof(buffer), 0);
	} else if(choice == 'f') {   //(f)command line of excuting process(cmdline)
		char dir_name[50];
		sprintf(dir_name, "/proc/%s/cmdline", pid);
		FILE *fproc = fopen(dir_name, "r");
		char search[500];

		if(fproc) {
			sprintf(buffer, "[cmdline]");
			while(fgets(search, sizeof(search), fproc) != NULL) {
				printf("%s\n", search);
				sprintf(buffer, "%s", search);
			}
		} else {
			printf("Process doesn't exit.\n");
			sprintf(buffer, "Process doesn't exit.");
		}
		printf("%s", buffer);
		send(clientfd[num], buffer, sizeof(buffer), 0);
	} else if(choice == 'g') {   //(g)parent's PIDs
		char dir_name[50];
		sprintf(dir_name, "/proc/%s/status", pid);
		FILE *fproc = fopen(dir_name, "r");
		char search[100];

		if(fproc) {
			while(fgets(search, 6, fproc) != NULL) {
				printf("%s\n", search);
				if(strcmp(search,"PPid:") == 0) {
					fgets(search, sizeof(search), fproc);
					sprintf(buffer, "[Parent's PID]%s", search);
				}
			}
		} else {
			printf("Process doesn't exit.\n");
			sprintf(buffer, "Process doesn't exit.");
		}
		printf("%s", buffer);
		send(clientfd[num], buffer, sizeof(buffer), 0);

	} else if(choice == 'h') {   //(h)all ancients of PIDs
		char dir_name[50];
		sprintf(dir_name, "/proc/%s/status", pid);
		FILE *fproc = fopen(dir_name, "r");
		char search[100];

		if(fproc) {
			sprintf(buffer, "[All ancients of PIDs]");
			while(fgets(search, 6, fproc) != NULL) {
				if(strcmp(search,"PPid:") == 0) {
					fgets(search, sizeof(search), fproc);
					printf("%s\n", search);
					memmove(search, search+1, strlen(search));
					char *ancient = strtok(search, "\n");
					strcat(buffer, search);
					strcat(buffer, " ");
					printf("ancient: %s", ancient);
					find_ancient(buffer, ancient);
				}
			}
		} else {
			printf("Process doesn't exit.\n");
			sprintf(buffer, "Process doesn't exit.");
		}
		printf("%s", buffer);
		send(clientfd[num], buffer, sizeof(buffer), 0);

	} else if(choice == 'i') {   //(i)virtual memory size(VmSize)
		char dir_name[50];
		sprintf(dir_name, "/proc/%s/status", pid);
		FILE *fproc = fopen(dir_name, "r");
		char search[100];

		if(fproc) {
			while(fgets(search, 8, fproc) != NULL) {
				printf("%s\n", search);
				if(strcmp(search,"VmSize:") == 0) {
					fgets(search, sizeof(search), fproc);
					sprintf(buffer, "[VmSize]%s", search);
				}
			}
		} else {
			printf("Process doesn't exit.\n");
			sprintf(buffer, "Process doesn't exit.");
		}
		printf("%s", buffer);
		send(clientfd[num], buffer, sizeof(buffer), 0);
	} else if(choice == 'j') {   //(j)physical memory size(VmRSS)
		char dir_name[50];
		sprintf(dir_name, "/proc/%s/status", pid);
		FILE *fproc = fopen(dir_name, "r");
		char search[100];

		if(fproc) {
			while(fgets(search, 7, fproc) != NULL) {
				printf("%s\n", search);
				if(strcmp(search,"VmRSS:") == 0) {
					fgets(search, sizeof(search), fproc);
					sprintf(buffer, "[VmRSS]%s", search);
				}
			}
		} else {
			printf("Process doesn't exit.\n");
			sprintf(buffer, "Process doesn't exit.");
		}
		printf("%s", buffer);
		send(clientfd[num], buffer, sizeof(buffer), 0);
	}

}

void find_ancient(char buffer[],char child[])
{
	char dir_name[50];
	sprintf(dir_name, "/proc/%s/status", child);
	FILE *fproc = fopen(dir_name, "r");
	char search[100];
	printf("/proc/%s/status\n", child);
	if(fproc) {
		//sprintf(buffer, "[All ancients of PIDs]");
		while(fgets(search, 6, fproc) != NULL) {
			if(strcmp(search,"PPid:") == 0) {
				fgets(search, sizeof(search), fproc);
				printf("%s\n", search);
				memmove(search, search+1, strlen(search));
				char *ancient = strtok(search, "\n");
				strcat(buffer, search);
				strcat(buffer, " ");
				printf("ancient: %s\n", ancient);
				find_ancient(buffer, ancient);
			}
		}
	}
	printf("%s", buffer);
	//send(clientfd[num], buffer, sizeof(buffer), 0);

}
