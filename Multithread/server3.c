#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#define foreach(item, array) \
    for(int keep = 1, \
            count = 0,\
            size = sizeof (array) / sizeof *(array); \
        keep && count != size; \
        keep = !keep, count++) \
      for(item = (array) + count; keep; keep = !keep)

int clientCount = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

typedef struct Channel Channel;
typedef struct Arg_thread Arg_thread;
typedef struct Client Client;

struct Channel
{
	char name[30];
	int id;
};

struct Client
{
	int index;
	int sockID;
	struct sockaddr_in clientAddr;
	int *channel;
	int len;
};

struct Arg_thread
{
	struct Channel *listChannels;
	struct Client *Client;
	int *nbChannels;
};

Channel listChannels[10];
int nbChannels = 3;

Client client[1024];
pthread_t thread[1024];

Arg_thread arg_thread = {listChannels, NULL, &nbChannels};

void initiateChannel(Channel list[])
{
	list[0].id = 0;
	strcpy(list[0].name, "Main");
	list[1].id = 1;
	strcpy(list[1].name, "Gaming");
	list[2].id = 2;
	strcpy(list[2].name, "Gossip");
}


void *doNetworking(void *arg_thread)
{
	Arg_thread *arg = (Arg_thread *)arg_thread;
	Client *clientDetail = (Client *)arg->Client;
	Channel *listC = arg->listChannels;
	int* nbChannels = arg->nbChannels;
	int index = clientDetail->index;
	int clientSocket = clientDetail->sockID;
	int* channel = clientDetail->channel;

	printf("Client %d connected.\n", index + 1);

	while (1)
	{

		char data[1024];
		int read = recv(clientSocket, data, 1024, 0);
		data[read] = '\0';

		char output[1024];

		//renvoie la liste de channels
		if (strcmp(data, "CHANNELS") == 0)
		{
			
			int l = 0;

			for (int i = 0; i < *nbChannels; i++)
			{

				l += snprintf(output + l, 1024, "Channel %d - %s.\n", listC[i].id, listC[i].name);
			}

			send(clientSocket, output, 1024, 0);
			continue;
		}

		// Create channel
		if (strcmp(data, "CREATE") == 0)
		{

			read = recv(clientSocket, data, 1024, 0);
			data[read] = '\0'; // le nom du futur channel est dans data
			//Creation of the new channel
			listC[*nbChannels].id = *nbChannels;
			strcpy(listC[*nbChannels].name, data+1);
			*nbChannels = *nbChannels + 1;

			snprintf(output, 1024, "Your channel \"%s\" is created.\n", data);
			
			send(clientSocket, output, 1024, 0);
			continue;
		}

		// Join un channel
		if (strcmp(data, "JOIN") == 0)
		{
			read = recv(clientSocket, data, 1024, 0);
			data[read] = '\0';
			int idC = atoi(data);
			*channel = idC;

			//snprintf(output, 1024, "You joined channel n° \"%d\".\n", idC);
			snprintf(output, 1024, "You joined channel n° \"%d\".\n", *channel);
			
			send(clientSocket, output, 1024, 0);
			continue;
		}

		if (strcmp(data, "LIST") == 0)
		{

			int l = 0;

			for (int i = 0; i < clientCount; i++)
			{

				if (i != index)
					l += snprintf(output + l, 1024, "Client %d is at socket %d.\n", i + 1, client[i].sockID);
			}

			send(clientSocket, output, 1024, 0);
			continue;
		}
		if (strcmp(data, "SEND") == 0)
		{
			if(*channel == -1){
				snprintf(output, 1024, "You are not in a channel.\n");
				send(clientSocket, output, 1024, 0);
			} else {
				//read = recv(clientSocket, data, 1024, 0);
				//data[read] = '\0';

				//int id = atoi(data) - 1;

				read = recv(clientSocket, data, 1024, 0);
				data[read] = '\0';
				
				snprintf(output, 1024, "%d - TEST: %s.\n", data, read);
				send(clientSocket, output, 1024, 0);

				for (int i; i < clientCount; i++){
					if(*(client[i].channel) == *channel && i != index){
						send(client[i].sockID, data, 1024, 0); // au bout de la 3eme fois (ou d'un laps de temps), rien ne s'envoie
					}
				}
			}

			
		}
	}

	return NULL;
}

int main()
{
	initiateChannel(listChannels);

	int serverSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);
	serverAddr.sin_addr.s_addr = htons(INADDR_ANY);

	if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
		return 0;

	if (listen(serverSocket, 1024) == -1)
		return 0;

	printf("Server started listenting on port 8080 ...........\n");

	while (1)
	{
		
		client[clientCount].sockID = accept(serverSocket, (struct sockaddr *)&client[clientCount].clientAddr, &client[clientCount].len);
		client[clientCount].index = clientCount;
		client[clientCount].channel = (int*) malloc( sizeof(int) );
		*(client[clientCount].channel) = -1;

		arg_thread.Client = &client[clientCount];

		pthread_create(&thread[clientCount], NULL, doNetworking, (void *)&arg_thread);

		clientCount++;
	}

	for (int i = 0; i < clientCount; i++)
		pthread_join(thread[i], NULL);
}